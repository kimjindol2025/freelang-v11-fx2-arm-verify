#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  VALUE_STRING = 0,
  VALUE_INT = 1,
  VALUE_ARRAY = 2
} ValueType;

typedef struct {
  char **items;
  size_t len;
  size_t cap;
} StringArray;

typedef struct {
  char *key;
  ValueType type;
  char *string_value;
  long int_value;
  StringArray array_value;
} KeyValue;

typedef struct {
  char *name;
  KeyValue *items;
  size_t len;
  size_t cap;
} Section;

typedef struct {
  Section *sections;
  size_t len;
  size_t cap;
} Document;

static void *xmalloc(size_t size) {
  void *ptr = malloc(size);
  if (!ptr) {
    fprintf(stderr, "out of memory\n");
    exit(1);
  }
  return ptr;
}

static void *xrealloc(void *ptr, size_t size) {
  void *next = realloc(ptr, size);
  if (!next) {
    fprintf(stderr, "out of memory\n");
    exit(1);
  }
  return next;
}

static char *xstrdup(const char *src) {
  size_t len = strlen(src);
  char *copy = (char *)xmalloc(len + 1);
  memcpy(copy, src, len + 1);
  return copy;
}

static char *substr_dup(const char *src, size_t start, size_t len) {
  char *copy = (char *)xmalloc(len + 1);
  memcpy(copy, src + start, len);
  copy[len] = '\0';
  return copy;
}

static void trim_in_place(char *text) {
  size_t start = 0;
  size_t len = strlen(text);
  while (start < len && isspace((unsigned char)text[start])) {
    start++;
  }
  while (len > start && isspace((unsigned char)text[len - 1])) {
    len--;
  }
  if (start > 0) {
    memmove(text, text + start, len - start);
  }
  text[len - start] = '\0';
}

static int is_word_char(char ch) {
  return isalnum((unsigned char)ch) || ch == '_';
}

static void array_push(StringArray *array, char *item) {
  if (array->len == array->cap) {
    array->cap = array->cap ? array->cap * 2 : 4;
    array->items = (char **)xrealloc(array->items, array->cap * sizeof(char *));
  }
  array->items[array->len++] = item;
}

static Section *document_add_section(Document *doc, const char *name) {
  if (doc->len == doc->cap) {
    doc->cap = doc->cap ? doc->cap * 2 : 4;
    doc->sections = (Section *)xrealloc(doc->sections, doc->cap * sizeof(Section));
  }
  doc->sections[doc->len].name = xstrdup(name);
  doc->sections[doc->len].items = NULL;
  doc->sections[doc->len].len = 0;
  doc->sections[doc->len].cap = 0;
  return &doc->sections[doc->len++];
}

static KeyValue *section_upsert_key(Section *section, const char *key) {
  size_t i;
  for (i = 0; i < section->len; i++) {
    if (strcmp(section->items[i].key, key) == 0) {
      return &section->items[i];
    }
  }
  if (section->len == section->cap) {
    section->cap = section->cap ? section->cap * 2 : 4;
    section->items = (KeyValue *)xrealloc(section->items, section->cap * sizeof(KeyValue));
  }
  section->items[section->len].key = xstrdup(key);
  section->items[section->len].type = VALUE_STRING;
  section->items[section->len].string_value = NULL;
  section->items[section->len].int_value = 0;
  section->items[section->len].array_value.items = NULL;
  section->items[section->len].array_value.len = 0;
  section->items[section->len].array_value.cap = 0;
  return &section->items[section->len++];
}

static void free_value(KeyValue *item) {
  size_t i;
  free(item->string_value);
  for (i = 0; i < item->array_value.len; i++) {
    free(item->array_value.items[i]);
  }
  free(item->array_value.items);
  item->string_value = NULL;
  item->array_value.items = NULL;
  item->array_value.len = 0;
  item->array_value.cap = 0;
}

static void kv_set_string(KeyValue *item, char *value) {
  free_value(item);
  item->type = VALUE_STRING;
  item->string_value = value;
}

static void kv_set_int(KeyValue *item, long value) {
  free_value(item);
  item->type = VALUE_INT;
  item->int_value = value;
}

static void kv_set_array(KeyValue *item, StringArray *value) {
  free_value(item);
  item->type = VALUE_ARRAY;
  item->array_value = *value;
}

static char *strip_matching_char(char *value, char quote) {
  size_t start = 0;
  size_t end = strlen(value);
  while (start < end && value[start] == quote) {
    start++;
  }
  while (end > start && value[end - 1] == quote) {
    end--;
  }
  return substr_dup(value, start, end - start);
}

static int is_digits_only(const char *value) {
  size_t i;
  if (value[0] == '\0') {
    return 0;
  }
  for (i = 0; value[i]; i++) {
    if (!isdigit((unsigned char)value[i])) {
      return 0;
    }
  }
  return 1;
}

static void json_print_string(const char *text) {
  const unsigned char *p = (const unsigned char *)text;
  putchar('"');
  while (*p) {
    switch (*p) {
      case '\\':
        fputs("\\\\", stdout);
        break;
      case '"':
        fputs("\\\"", stdout);
        break;
      case '\b':
        fputs("\\b", stdout);
        break;
      case '\f':
        fputs("\\f", stdout);
        break;
      case '\n':
        fputs("\\n", stdout);
        break;
      case '\r':
        fputs("\\r", stdout);
        break;
      case '\t':
        fputs("\\t", stdout);
        break;
      default:
        if (*p < 0x20) {
          printf("\\u%04x", *p);
        } else {
          putchar(*p);
        }
        break;
    }
    p++;
  }
  putchar('"');
}

static void json_print_array(const StringArray *array) {
  size_t i;
  putchar('[');
  for (i = 0; i < array->len; i++) {
    if (i > 0) {
      fputs(", ", stdout);
    }
    json_print_string(array->items[i]);
  }
  putchar(']');
}

static void json_print_document(const Document *doc) {
  size_t i;
  putchar('{');
  for (i = 0; i < doc->len; i++) {
    size_t j;
    if (i > 0) {
      fputs(", ", stdout);
    }
    json_print_string(doc->sections[i].name);
    fputs(": ", stdout);
    putchar('{');
    for (j = 0; j < doc->sections[i].len; j++) {
      KeyValue *item = &doc->sections[i].items[j];
      if (j > 0) {
        fputs(", ", stdout);
      }
      json_print_string(item->key);
      fputs(": ", stdout);
      if (item->type == VALUE_INT) {
        printf("%ld", item->int_value);
      } else if (item->type == VALUE_ARRAY) {
        json_print_array(&item->array_value);
      } else {
        json_print_string(item->string_value ? item->string_value : "");
      }
    }
    putchar('}');
  }
  puts("}");
}

static void free_document(Document *doc) {
  size_t i;
  for (i = 0; i < doc->len; i++) {
    size_t j;
    free(doc->sections[i].name);
    for (j = 0; j < doc->sections[i].len; j++) {
      free(doc->sections[i].items[j].key);
      free_value(&doc->sections[i].items[j]);
    }
    free(doc->sections[i].items);
  }
  free(doc->sections);
}

static char *read_file_or_null(const char *path) {
  FILE *fp = fopen(path, "rb");
  long size;
  char *buffer;
  size_t read_size;
  if (!fp) {
    return NULL;
  }
  if (fseek(fp, 0, SEEK_END) != 0) {
    fclose(fp);
    return NULL;
  }
  size = ftell(fp);
  if (size < 0) {
    fclose(fp);
    return NULL;
  }
  if (fseek(fp, 0, SEEK_SET) != 0) {
    fclose(fp);
    return NULL;
  }
  buffer = (char *)xmalloc((size_t)size + 1);
  read_size = fread(buffer, 1, (size_t)size, fp);
  buffer[read_size] = '\0';
  fclose(fp);
  return buffer;
}

static char *next_line(const char *text, size_t *offset) {
  size_t start = *offset;
  size_t end = start;
  char *line;
  if (text[start] == '\0') {
    return NULL;
  }
  while (text[end] && text[end] != '\n') {
    end++;
  }
  line = substr_dup(text, start, end - start);
  if (end > start && line[end - start - 1] == '\r') {
    line[end - start - 1] = '\0';
  }
  *offset = text[end] == '\n' ? end + 1 : end;
  return line;
}

static char *append_with_space(char *base, const char *suffix) {
  size_t base_len = strlen(base);
  size_t suffix_len = strlen(suffix);
  char *merged = (char *)xrealloc(base, base_len + 1 + suffix_len + 1);
  merged[base_len] = ' ';
  memcpy(merged + base_len + 1, suffix, suffix_len + 1);
  return merged;
}

static void parse_array_value(KeyValue *item, char *array_text) {
  char *cursor;
  StringArray array;
  trim_in_place(array_text);
  if (array_text[0] == '[') {
    memmove(array_text, array_text + 1, strlen(array_text));
  }
  if (array_text[0] != '\0') {
    size_t len = strlen(array_text);
    if (len > 0 && array_text[len - 1] == ']') {
      array_text[len - 1] = '\0';
    }
  }
  trim_in_place(array_text);
  array.items = NULL;
  array.len = 0;
  array.cap = 0;
  cursor = array_text;
  while (*cursor) {
    char *comma = strchr(cursor, ',');
    char *part;
    char *clean;
    if (comma) {
      part = substr_dup(cursor, 0, (size_t)(comma - cursor));
      cursor = comma + 1;
    } else {
      part = xstrdup(cursor);
      cursor += strlen(cursor);
    }
    trim_in_place(part);
    clean = strip_matching_char(part, '"');
    free(part);
    part = strip_matching_char(clean, '\'');
    free(clean);
    if (part[0] != '\0') {
      array_push(&array, part);
    } else {
      free(part);
    }
  }
  kv_set_array(item, &array);
}

static void parse_toml(const char *text, Document *doc) {
  size_t offset = 0;
  Section *current = NULL;
  while (1) {
    char *line = next_line(text, &offset);
    char *trimmed;
    if (!line) {
      break;
    }
    trim_in_place(line);
    trimmed = line;
    if (trimmed[0] == '\0' || trimmed[0] == '#') {
      free(line);
      continue;
    }
    if (trimmed[0] == '[') {
      size_t len = strlen(trimmed);
      if (len >= 2 && trimmed[len - 1] == ']') {
        char *section_name = substr_dup(trimmed, 1, len - 2);
        trim_in_place(section_name);
        current = document_add_section(doc, section_name);
        free(section_name);
      }
      free(line);
      continue;
    }
    if (!current) {
      free(line);
      continue;
    }
    if (is_word_char(trimmed[0])) {
      size_t pos = 1;
      while (trimmed[pos] && (is_word_char(trimmed[pos]) || trimmed[pos] == '-')) {
        pos++;
      }
      {
        size_t gap = pos;
        while (trimmed[gap] && isspace((unsigned char)trimmed[gap])) {
          gap++;
        }
        if (trimmed[gap] == '=') {
          size_t value_start;
          char *key = substr_dup(trimmed, 0, pos);
          char *value;
          KeyValue *item = section_upsert_key(current, key);
          free(key);
          value_start = gap + 1;
          while (trimmed[value_start] && isspace((unsigned char)trimmed[value_start])) {
            value_start++;
          }
          value = xstrdup(trimmed + value_start);
          trim_in_place(value);
          if (value[0] == '[') {
            while (!strchr(value, ']')) {
              char *next = next_line(text, &offset);
              if (!next) {
                break;
              }
              trim_in_place(next);
              value = append_with_space(value, next);
              free(next);
            }
            parse_array_value(item, value);
            free(value);
          } else if (value[0] == '"' || value[0] == '\'') {
            char *string_value = strip_matching_char(value, value[0]);
            kv_set_string(item, string_value);
            free(value);
          } else if (is_digits_only(value)) {
            kv_set_int(item, strtol(value, NULL, 10));
            free(value);
          } else {
            kv_set_string(item, value);
          }
        }
      }
    }
    free(line);
  }
}

static Section *find_section(Document *doc, const char *name) {
  size_t i;
  for (i = 0; i < doc->len; i++) {
    if (strcmp(doc->sections[i].name, name) == 0) {
      return &doc->sections[i];
    }
  }
  return NULL;
}

static KeyValue *find_key(Section *section, const char *key) {
  size_t i;
  if (!section) {
    return NULL;
  }
  for (i = 0; i < section->len; i++) {
    if (strcmp(section->items[i].key, key) == 0) {
      return &section->items[i];
    }
  }
  return NULL;
}

static void print_key_value(KeyValue *item) {
  if (!item) {
    return;
  }
  if (item->type == VALUE_STRING) {
    fputs(item->string_value ? item->string_value : "", stdout);
  } else if (item->type == VALUE_INT) {
    printf("%ld", item->int_value);
  } else {
    json_print_array(&item->array_value);
  }
}

static void print_runtime_payload(Document *doc) {
  Section *runtime = find_section(doc, "runtime");
  KeyValue *packages = find_key(runtime, "packages");
  KeyValue *profiles = find_key(runtime, "profiles");
  int has_packages = packages && packages->type == VALUE_ARRAY && packages->array_value.len > 0;
  int has_profiles = profiles && profiles->type == VALUE_ARRAY && profiles->array_value.len > 0;
  int emitted = 0;
  if (!has_packages && !has_profiles) {
    return;
  }
  putchar('{');
  if (has_packages) {
    json_print_string("packages");
    fputs(": ", stdout);
    json_print_array(&packages->array_value);
    emitted = 1;
  }
  if (has_profiles) {
    if (emitted) {
      fputs(", ", stdout);
    }
    json_print_string("profiles");
    fputs(": ", stdout);
    json_print_array(&profiles->array_value);
  }
  puts("}");
}

static int parse_count_or_die(const char *text) {
  char *end = NULL;
  long value;
  if (!text || text[0] == '\0') {
    return -1;
  }
  value = strtol(text, &end, 10);
  if (!end || *end != '\0' || value < 0) {
    return -1;
  }
  if (value > 2147483647L) {
    return -1;
  }
  return (int)value;
}

static void print_new_payload_from_lists(int profile_count, char **profiles, int package_count, char **packages) {
  int emitted = 0;
  int i;
  putchar('{');
  if (profile_count > 0) {
    json_print_string("profiles");
    fputs(": ", stdout);
    putchar('[');
    for (i = 0; i < profile_count; i++) {
      if (i > 0) {
        fputs(", ", stdout);
      }
      json_print_string(profiles[i]);
    }
    putchar(']');
    emitted = 1;
  }
  if (package_count > 0) {
    if (emitted) {
      fputs(", ", stdout);
    }
    json_print_string("packages");
    fputs(": ", stdout);
    putchar('[');
    for (i = 0; i < package_count; i++) {
      if (i > 0) {
        fputs(", ", stdout);
      }
      json_print_string(packages[i]);
    }
    putchar(']');
  }
  puts("}");
}

typedef struct {
  char *name;
  char *action;
} PackageAction;

typedef struct {
  PackageAction *items;
  size_t len;
  size_t cap;
} PackageActionList;

static void package_action_list_push(PackageActionList *list, char *name, char *action) {
  if (list->len == list->cap) {
    size_t next_cap = list->cap ? list->cap * 2 : 4;
    list->items = (PackageAction *)xrealloc(list->items, next_cap * sizeof(PackageAction));
    list->cap = next_cap;
  }
  list->items[list->len].name = name;
  list->items[list->len].action = action;
  list->len++;
}

static void free_package_action_list(PackageActionList *list) {
  size_t i;
  for (i = 0; i < list->len; i++) {
    free(list->items[i].name);
    free(list->items[i].action);
  }
  free(list->items);
  list->items = NULL;
  list->len = 0;
  list->cap = 0;
}

static void skip_json_ws(const char *text, size_t *pos) {
  while (text[*pos] && isspace((unsigned char)text[*pos])) {
    (*pos)++;
  }
}

static int json_consume_char(const char *text, size_t *pos, char expected) {
  skip_json_ws(text, pos);
  if (text[*pos] != expected) {
    return 0;
  }
  (*pos)++;
  return 1;
}

static char *json_parse_string(const char *text, size_t *pos) {
  size_t start;
  char *out;
  size_t out_len = 0;
  if (!json_consume_char(text, pos, '"')) {
    return NULL;
  }
  start = *pos;
  out = (char *)xmalloc(strlen(text + start) + 1);
  while (text[*pos]) {
    unsigned char ch = (unsigned char)text[*pos];
    if (ch == '"') {
      (*pos)++;
      out[out_len] = '\0';
      return out;
    }
    if (ch == '\\') {
      unsigned char esc;
      (*pos)++;
      esc = (unsigned char)text[*pos];
      if (!esc) {
        break;
      }
      switch (esc) {
        case '"': out[out_len++] = '"'; break;
        case '\\': out[out_len++] = '\\'; break;
        case '/': out[out_len++] = '/'; break;
        case 'b': out[out_len++] = '\b'; break;
        case 'f': out[out_len++] = '\f'; break;
        case 'n': out[out_len++] = '\n'; break;
        case 'r': out[out_len++] = '\r'; break;
        case 't': out[out_len++] = '\t'; break;
        case 'u':
          if (text[*pos + 1] && text[*pos + 2] && text[*pos + 3] && text[*pos + 4]) {
            *pos += 4;
            out[out_len++] = '?';
            break;
          }
          free(out);
          return NULL;
        default:
          out[out_len++] = (char)esc;
          break;
      }
      (*pos)++;
      continue;
    }
    out[out_len++] = (char)ch;
    (*pos)++;
  }
  free(out);
  return NULL;
}

static int json_skip_literal(const char *text, size_t *pos, const char *literal) {
  size_t i = 0;
  skip_json_ws(text, pos);
  while (literal[i]) {
    if (text[*pos + i] != literal[i]) {
      return 0;
    }
    i++;
  }
  *pos += i;
  return 1;
}

static int json_skip_value(const char *text, size_t *pos);

static int json_skip_array(const char *text, size_t *pos) {
  if (!json_consume_char(text, pos, '[')) {
    return 0;
  }
  skip_json_ws(text, pos);
  if (text[*pos] == ']') {
    (*pos)++;
    return 1;
  }
  while (text[*pos]) {
    if (!json_skip_value(text, pos)) {
      return 0;
    }
    skip_json_ws(text, pos);
    if (text[*pos] == ',') {
      (*pos)++;
      continue;
    }
    if (text[*pos] == ']') {
      (*pos)++;
      return 1;
    }
    return 0;
  }
  return 0;
}

static int json_skip_object(const char *text, size_t *pos) {
  if (!json_consume_char(text, pos, '{')) {
    return 0;
  }
  skip_json_ws(text, pos);
  if (text[*pos] == '}') {
    (*pos)++;
    return 1;
  }
  while (text[*pos]) {
    char *key = json_parse_string(text, pos);
    if (!key) {
      return 0;
    }
    free(key);
    if (!json_consume_char(text, pos, ':')) {
      return 0;
    }
    if (!json_skip_value(text, pos)) {
      return 0;
    }
    skip_json_ws(text, pos);
    if (text[*pos] == ',') {
      (*pos)++;
      continue;
    }
    if (text[*pos] == '}') {
      (*pos)++;
      return 1;
    }
    return 0;
  }
  return 0;
}

static int json_skip_value(const char *text, size_t *pos) {
  skip_json_ws(text, pos);
  if (text[*pos] == '"') {
    char *tmp = json_parse_string(text, pos);
    if (!tmp) {
      return 0;
    }
    free(tmp);
    return 1;
  }
  if (text[*pos] == '{') {
    return json_skip_object(text, pos);
  }
  if (text[*pos] == '[') {
    return json_skip_array(text, pos);
  }
  if (json_skip_literal(text, pos, "true") || json_skip_literal(text, pos, "false") || json_skip_literal(text, pos, "null")) {
    return 1;
  }
  if (text[*pos] == '-' || isdigit((unsigned char)text[*pos])) {
    size_t p = *pos;
    if (text[p] == '-') {
      p++;
    }
    while (isdigit((unsigned char)text[p])) {
      p++;
    }
    if (text[p] == '.') {
      p++;
      while (isdigit((unsigned char)text[p])) {
        p++;
      }
    }
    if (text[p] == 'e' || text[p] == 'E') {
      p++;
      if (text[p] == '+' || text[p] == '-') {
        p++;
      }
      while (isdigit((unsigned char)text[p])) {
        p++;
      }
    }
    *pos = p;
    return 1;
  }
  return 0;
}

static int json_parse_top_level_ok(const char *text, int *ok_value) {
  size_t pos = 0;
  if (!json_consume_char(text, &pos, '{')) {
    return 0;
  }
  skip_json_ws(text, &pos);
  if (text[pos] == '}') {
    return 1;
  }
  while (text[pos]) {
    char *key = json_parse_string(text, &pos);
    if (!key) {
      return 0;
    }
    if (!json_consume_char(text, &pos, ':')) {
      free(key);
      return 0;
    }
    if (strcmp(key, "ok") == 0) {
      free(key);
      if (json_skip_literal(text, &pos, "true")) {
        *ok_value = 1;
        return 1;
      }
      if (json_skip_literal(text, &pos, "false")) {
        *ok_value = 0;
        return 1;
      }
      return 0;
    }
    free(key);
    if (!json_skip_value(text, &pos)) {
      return 0;
    }
    skip_json_ws(text, &pos);
    if (text[pos] == ',') {
      pos++;
      continue;
    }
    if (text[pos] == '}') {
      return 1;
    }
    return 0;
  }
  return 0;
}

static char *json_parse_top_level_error(const char *text) {
  size_t pos = 0;
  if (!json_consume_char(text, &pos, '{')) {
    return NULL;
  }
  skip_json_ws(text, &pos);
  if (text[pos] == '}') {
    return xstrdup("");
  }
  while (text[pos]) {
    char *key = json_parse_string(text, &pos);
    char *value;
    if (!key) {
      return NULL;
    }
    if (!json_consume_char(text, &pos, ':')) {
      free(key);
      return NULL;
    }
    if (strcmp(key, "error") == 0) {
      free(key);
      value = json_parse_string(text, &pos);
      if (!value) {
        return xstrdup("");
      }
      return value;
    }
    free(key);
    if (!json_skip_value(text, &pos)) {
      return NULL;
    }
    skip_json_ws(text, &pos);
    if (text[pos] == ',') {
      pos++;
      continue;
    }
    if (text[pos] == '}') {
      return xstrdup("");
    }
    return NULL;
  }
  return NULL;
}

static int json_parse_package_object(const char *text, size_t *pos, PackageActionList *list) {
  char *name = NULL;
  char *action = NULL;
  if (!json_consume_char(text, pos, '{')) {
    return 0;
  }
  skip_json_ws(text, pos);
  if (text[*pos] == '}') {
    (*pos)++;
    package_action_list_push(list, xstrdup(""), xstrdup(""));
    return 1;
  }
  while (text[*pos]) {
    char *key = json_parse_string(text, pos);
    if (!key) {
      free(name);
      free(action);
      return 0;
    }
    if (!json_consume_char(text, pos, ':')) {
      free(key);
      free(name);
      free(action);
      return 0;
    }
    if (strcmp(key, "name") == 0) {
      free(name);
      name = json_parse_string(text, pos);
      if (!name) {
        free(key);
        free(action);
        return 0;
      }
    } else if (strcmp(key, "action") == 0) {
      free(action);
      action = json_parse_string(text, pos);
      if (!action) {
        free(key);
        free(name);
        return 0;
      }
    } else {
      if (!json_skip_value(text, pos)) {
        free(key);
        free(name);
        free(action);
        return 0;
      }
    }
    free(key);
    skip_json_ws(text, pos);
    if (text[*pos] == ',') {
      (*pos)++;
      continue;
    }
    if (text[*pos] == '}') {
      (*pos)++;
      package_action_list_push(list, name ? name : xstrdup(""), action ? action : xstrdup(""));
      return 1;
    }
    free(name);
    free(action);
    return 0;
  }
  free(name);
  free(action);
  return 0;
}

static int json_parse_top_level_packages(const char *text, PackageActionList *list) {
  size_t pos = 0;
  if (!json_consume_char(text, &pos, '{')) {
    return 0;
  }
  skip_json_ws(text, &pos);
  if (text[pos] == '}') {
    return 1;
  }
  while (text[pos]) {
    char *key = json_parse_string(text, &pos);
    if (!key) {
      return 0;
    }
    if (!json_consume_char(text, &pos, ':')) {
      free(key);
      return 0;
    }
    if (strcmp(key, "packages") == 0) {
      free(key);
      if (!json_consume_char(text, &pos, '[')) {
        return 0;
      }
      skip_json_ws(text, &pos);
      if (text[pos] == ']') {
        pos++;
        return 1;
      }
      while (text[pos]) {
        if (!json_parse_package_object(text, &pos, list)) {
          return 0;
        }
        skip_json_ws(text, &pos);
        if (text[pos] == ',') {
          pos++;
          continue;
        }
        if (text[pos] == ']') {
          pos++;
          return 1;
        }
        return 0;
      }
      return 0;
    }
    free(key);
    if (!json_skip_value(text, &pos)) {
      return 0;
    }
    skip_json_ws(text, &pos);
    if (text[pos] == ',') {
      pos++;
      continue;
    }
    if (text[pos] == '}') {
      return 1;
    }
    return 0;
  }
  return 0;
}

static void print_package_group(PackageActionList *list, const char *target_action, const char *prefix) {
  size_t i;
  int emitted = 0;
  for (i = 0; i < list->len; i++) {
    const char *action = list->items[i].action ? list->items[i].action : "";
    const char *name = list->items[i].name ? list->items[i].name : "";
    int match = strcmp(target_action, "skipped") == 0 ? strcmp(action, "skipped") == 0 : strcmp(action, "skipped") != 0;
    if (!match || name[0] == '\0') {
      continue;
    }
    if (!emitted) {
      fputs(prefix, stdout);
      emitted = 1;
    } else {
      fputs(", ", stdout);
    }
    fputs(name, stdout);
  }
  if (emitted) {
    putchar('\n');
  }
}

static int print_new_init_ok(const char *text) {
  int ok_value = 0;
  if (!json_parse_top_level_ok(text, &ok_value)) {
    return 0;
  }
  fputs(ok_value ? "True" : "False", stdout);
  return 1;
}

static int print_new_init_error(const char *text) {
  char *error_value = json_parse_top_level_error(text);
  if (!error_value) {
    return 0;
  }
  fputs(error_value, stdout);
  free(error_value);
  return 1;
}

static int print_new_init_package_summary(const char *text) {
  PackageActionList list;
  list.items = NULL;
  list.len = 0;
  list.cap = 0;
  if (!json_parse_top_level_packages(text, &list)) {
    free_package_action_list(&list);
    return 0;
  }
  print_package_group(&list, "added", "  + ");
  print_package_group(&list, "skipped", "  ↩ skipped: ");
  free_package_action_list(&list);
  return 1;
}

typedef struct {
  char *name;
  int installed;
  size_t package_count;
  StringArray depends;
} ProfileInfo;

typedef struct {
  ProfileInfo *items;
  size_t len;
  size_t cap;
} ProfileInfoList;

static void profile_info_list_push(ProfileInfoList *list, ProfileInfo *info) {
  if (list->len == list->cap) {
    size_t next_cap = list->cap ? list->cap * 2 : 4;
    list->items = (ProfileInfo *)xrealloc(list->items, next_cap * sizeof(ProfileInfo));
    list->cap = next_cap;
  }
  list->items[list->len++] = *info;
}

static void free_profile_info_list(ProfileInfoList *list) {
  size_t i;
  for (i = 0; i < list->len; i++) {
    size_t j;
    free(list->items[i].name);
    for (j = 0; j < list->items[i].depends.len; j++) {
      free(list->items[i].depends.items[j]);
    }
    free(list->items[i].depends.items);
  }
  free(list->items);
  list->items = NULL;
  list->len = 0;
  list->cap = 0;
}

static int json_parse_nonneg_int(const char *text, size_t *pos, long *value) {
  long result = 0;
  skip_json_ws(text, pos);
  if (!isdigit((unsigned char)text[*pos])) {
    return 0;
  }
  while (isdigit((unsigned char)text[*pos])) {
    result = result * 10 + (text[*pos] - '0');
    (*pos)++;
  }
  *value = result;
  return 1;
}

static int json_skip_array_count(const char *text, size_t *pos, size_t *count) {
  size_t local_count = 0;
  if (!json_consume_char(text, pos, '[')) {
    return 0;
  }
  skip_json_ws(text, pos);
  if (text[*pos] == ']') {
    (*pos)++;
    *count = 0;
    return 1;
  }
  while (text[*pos]) {
    if (!json_skip_value(text, pos)) {
      return 0;
    }
    local_count++;
    skip_json_ws(text, pos);
    if (text[*pos] == ',') {
      (*pos)++;
      continue;
    }
    if (text[*pos] == ']') {
      (*pos)++;
      *count = local_count;
      return 1;
    }
    return 0;
  }
  return 0;
}

static int json_parse_string_array(const char *text, size_t *pos, StringArray *array) {
  if (!json_consume_char(text, pos, '[')) {
    return 0;
  }
  skip_json_ws(text, pos);
  if (text[*pos] == ']') {
    (*pos)++;
    return 1;
  }
  while (text[*pos]) {
    char *item = json_parse_string(text, pos);
    if (!item) {
      return 0;
    }
    array_push(array, item);
    skip_json_ws(text, pos);
    if (text[*pos] == ',') {
      (*pos)++;
      continue;
    }
    if (text[*pos] == ']') {
      (*pos)++;
      return 1;
    }
    return 0;
  }
  return 0;
}

static int json_parse_bool(const char *text, size_t *pos, int *value) {
  if (json_skip_literal(text, pos, "true")) {
    *value = 1;
    return 1;
  }
  if (json_skip_literal(text, pos, "false")) {
    *value = 0;
    return 1;
  }
  return 0;
}

static int json_parse_profile_object(const char *text, size_t *pos, ProfileInfoList *list) {
  ProfileInfo info;
  info.name = NULL;
  info.installed = 0;
  info.package_count = 0;
  info.depends.items = NULL;
  info.depends.len = 0;
  info.depends.cap = 0;
  if (!json_consume_char(text, pos, '{')) {
    return 0;
  }
  skip_json_ws(text, pos);
  if (text[*pos] == '}') {
    (*pos)++;
    profile_info_list_push(list, &info);
    return 1;
  }
  while (text[*pos]) {
    char *key = json_parse_string(text, pos);
    if (!key) {
      free(info.name);
      free_profile_info_list(&(ProfileInfoList){&info, info.name ? 1 : 0, info.name ? 1 : 0});
      return 0;
    }
    if (!json_consume_char(text, pos, ':')) {
      free(key);
      free(info.name);
      free_profile_info_list(&(ProfileInfoList){&info, info.name ? 1 : 0, info.name ? 1 : 0});
      return 0;
    }
    if (strcmp(key, "name") == 0) {
      free(info.name);
      info.name = json_parse_string(text, pos);
      if (!info.name) {
        free(key);
        free_profile_info_list(&(ProfileInfoList){&info, 0, 0});
        return 0;
      }
    } else if (strcmp(key, "installed") == 0) {
      if (!json_parse_bool(text, pos, &info.installed)) {
        free(key);
        free_profile_info_list(&(ProfileInfoList){&info, 0, 0});
        return 0;
      }
    } else if (strcmp(key, "packages") == 0) {
      if (!json_skip_array_count(text, pos, &info.package_count)) {
        free(key);
        free_profile_info_list(&(ProfileInfoList){&info, 0, 0});
        return 0;
      }
    } else if (strcmp(key, "depends") == 0) {
      size_t j;
      for (j = 0; j < info.depends.len; j++) {
        free(info.depends.items[j]);
      }
      free(info.depends.items);
      info.depends.items = NULL;
      info.depends.len = 0;
      info.depends.cap = 0;
      if (!json_parse_string_array(text, pos, &info.depends)) {
        free(key);
        free_profile_info_list(&(ProfileInfoList){&info, 0, 0});
        return 0;
      }
    } else {
      if (!json_skip_value(text, pos)) {
        free(key);
        free_profile_info_list(&(ProfileInfoList){&info, 0, 0});
        return 0;
      }
    }
    free(key);
    skip_json_ws(text, pos);
    if (text[*pos] == ',') {
      (*pos)++;
      continue;
    }
    if (text[*pos] == '}') {
      (*pos)++;
      profile_info_list_push(list, &info);
      return 1;
    }
    free_profile_info_list(&(ProfileInfoList){&info, 0, 0});
    return 0;
  }
  free_profile_info_list(&(ProfileInfoList){&info, 0, 0});
  return 0;
}

static int print_profile_list_output(const char *text) {
  size_t pos = 0;
  long count = 0;
  int have_count = 0;
  int have_profiles = 0;
  ProfileInfoList list;
  size_t i;
  list.items = NULL;
  list.len = 0;
  list.cap = 0;
  if (!json_consume_char(text, &pos, '{')) {
    return 0;
  }
  skip_json_ws(text, &pos);
  if (text[pos] == '}') {
    free_profile_info_list(&list);
    return 0;
  }
  while (text[pos]) {
    char *key = json_parse_string(text, &pos);
    if (!key) {
      free_profile_info_list(&list);
      return 0;
    }
    if (!json_consume_char(text, &pos, ':')) {
      free(key);
      free_profile_info_list(&list);
      return 0;
    }
    if (strcmp(key, "count") == 0) {
      if (!json_parse_nonneg_int(text, &pos, &count)) {
        free(key);
        free_profile_info_list(&list);
        return 0;
      }
      have_count = 1;
    } else if (strcmp(key, "profiles") == 0) {
      if (!json_consume_char(text, &pos, '[')) {
        free(key);
        free_profile_info_list(&list);
        return 0;
      }
      skip_json_ws(text, &pos);
      if (text[pos] == ']') {
        pos++;
      } else {
        while (text[pos]) {
          if (!json_parse_profile_object(text, &pos, &list)) {
            free(key);
            free_profile_info_list(&list);
            return 0;
          }
          skip_json_ws(text, &pos);
          if (text[pos] == ',') {
            pos++;
            continue;
          }
          if (text[pos] == ']') {
            pos++;
            break;
          }
          free(key);
          free_profile_info_list(&list);
          return 0;
        }
      }
      have_profiles = 1;
    } else {
      if (!json_skip_value(text, &pos)) {
        free(key);
        free_profile_info_list(&list);
        return 0;
      }
    }
    free(key);
    skip_json_ws(text, &pos);
    if (text[pos] == ',') {
      pos++;
      continue;
    }
    if (text[pos] == '}') {
      break;
    }
    free_profile_info_list(&list);
    return 0;
  }
  if (!have_count || !have_profiles) {
    free_profile_info_list(&list);
    return 0;
  }
  printf("프로파일 %ld개:\n", count);
  for (i = 0; i < list.len; i++) {
    size_t j;
    const char *icon = list.items[i].installed ? "✓" : "○";
    const char *name = list.items[i].name ? list.items[i].name : "";
    printf("  %s %-10s  %2zu개 함수", icon, name, list.items[i].package_count);
    if (list.items[i].depends.len > 0) {
      fputs(" (depends: ", stdout);
      for (j = 0; j < list.items[i].depends.len; j++) {
        if (j > 0) {
          fputs(", ", stdout);
        }
        fputs(list.items[i].depends.items[j], stdout);
      }
      putchar(')');
    }
    putchar('\n');
  }
  free_profile_info_list(&list);
  return 1;
}

static const char *str_or_none(const char *value) {
  return value ? value : "None";
}

static int print_profile_add_output(const char *text) {
  size_t pos = 0;
  int ok = 0;
  int have_ok = 0;
  char *action = NULL;
  char *error = NULL;
  char *profile = NULL;
  char *expected = NULL;
  char *got = NULL;
  StringArray resolved_profiles;
  PackageActionList packages;
  size_t i;
  resolved_profiles.items = NULL;
  resolved_profiles.len = 0;
  resolved_profiles.cap = 0;
  packages.items = NULL;
  packages.len = 0;
  packages.cap = 0;

  if (!json_consume_char(text, &pos, '{')) {
    return 0;
  }
  skip_json_ws(text, &pos);
  if (text[pos] == '}') {
    return 0;
  }

  while (text[pos]) {
    char *key = json_parse_string(text, &pos);
    if (!key) {
      goto fail;
    }
    if (!json_consume_char(text, &pos, ':')) {
      free(key);
      goto fail;
    }
    if (strcmp(key, "ok") == 0) {
      if (!json_parse_bool(text, &pos, &ok)) {
        free(key);
        goto fail;
      }
      have_ok = 1;
    } else if (strcmp(key, "action") == 0) {
      free(action);
      action = json_parse_string(text, &pos);
      if (!action) {
        free(key);
        goto fail;
      }
    } else if (strcmp(key, "error") == 0) {
      free(error);
      error = json_parse_string(text, &pos);
      if (!error) {
        free(key);
        goto fail;
      }
    } else if (strcmp(key, "profile") == 0) {
      free(profile);
      profile = json_parse_string(text, &pos);
      if (!profile) {
        free(key);
        goto fail;
      }
    } else if (strcmp(key, "expected") == 0) {
      free(expected);
      expected = json_parse_string(text, &pos);
      if (!expected) {
        free(key);
        goto fail;
      }
    } else if (strcmp(key, "got") == 0) {
      free(got);
      got = json_parse_string(text, &pos);
      if (!got) {
        free(key);
        goto fail;
      }
    } else if (strcmp(key, "packages") == 0) {
      free_package_action_list(&packages);
      packages.items = NULL;
      packages.len = 0;
      packages.cap = 0;
      if (!json_consume_char(text, &pos, '[')) {
        free(key);
        goto fail;
      }
      skip_json_ws(text, &pos);
      if (text[pos] == ']') {
        pos++;
      } else {
        while (text[pos]) {
          if (!json_parse_package_object(text, &pos, &packages)) {
            free(key);
            goto fail;
          }
          skip_json_ws(text, &pos);
          if (text[pos] == ',') {
            pos++;
            continue;
          }
          if (text[pos] == ']') {
            pos++;
            break;
          }
          free(key);
          goto fail;
        }
      }
    } else if (strcmp(key, "resolved_profiles") == 0) {
      size_t j;
      for (j = 0; j < resolved_profiles.len; j++) {
        free(resolved_profiles.items[j]);
      }
      free(resolved_profiles.items);
      resolved_profiles.items = NULL;
      resolved_profiles.len = 0;
      resolved_profiles.cap = 0;
      if (!json_parse_string_array(text, &pos, &resolved_profiles)) {
        free(key);
        goto fail;
      }
    } else {
      if (!json_skip_value(text, &pos)) {
        free(key);
        goto fail;
      }
    }
    free(key);
    skip_json_ws(text, &pos);
    if (text[pos] == ',') {
      pos++;
      continue;
    }
    if (text[pos] == '}') {
      break;
    }
    goto fail;
  }

  if (!have_ok) {
    goto fail;
  }
  if (ok) {
    if (!action) {
      goto fail;
    }
    printf("✓ %s\n", action);
    for (i = 0; i < packages.len; i++) {
      const char *pkg_action = packages.items[i].action ? packages.items[i].action : "";
      const char *pkg_name = packages.items[i].name ? packages.items[i].name : "";
      const char *icon = strcmp(pkg_action, "skipped") == 0 ? "↩" : "+";
      printf("  %s %s (%s)\n", icon, pkg_name, pkg_action);
    }
    if (resolved_profiles.len > 0) {
      fputs("  depends chain: ", stdout);
      for (i = 0; i < resolved_profiles.len; i++) {
        if (i > 0) {
          fputs(" → ", stdout);
        }
        fputs(resolved_profiles.items[i], stdout);
      }
      putchar('\n');
    }
  } else {
    printf("✗ %s\n", error ? error : "알 수 없는 오류");
    if (error && strcmp(error, "PROFILE_ABI_MISMATCH") == 0) {
      printf("  프로파일: %s expected=%s got=%s\n", str_or_none(profile), str_or_none(expected), str_or_none(got));
    }
  }

  free(action);
  free(error);
  free(profile);
  free(expected);
  free(got);
  for (i = 0; i < resolved_profiles.len; i++) {
    free(resolved_profiles.items[i]);
  }
  free(resolved_profiles.items);
  free_package_action_list(&packages);
  return 1;

fail:
  free(action);
  free(error);
  free(profile);
  free(expected);
  free(got);
  for (i = 0; i < resolved_profiles.len; i++) {
    free(resolved_profiles.items[i]);
  }
  free(resolved_profiles.items);
  free_package_action_list(&packages);
  return 0;
}

static int print_pkg_list_output(const char *text) {
  size_t pos = 0;
  StringArray names;
  StringArray args_list;
  const char *label = "functions";
  size_t i;
  names.items = NULL;
  names.len = 0;
  names.cap = 0;
  args_list.items = NULL;
  args_list.len = 0;
  args_list.cap = 0;

  if (!json_consume_char(text, &pos, '{')) {
    return 0;
  }
  skip_json_ws(text, &pos);
  if (text[pos] == '}') {
    goto fail;
  }
  while (text[pos]) {
    char *key = json_parse_string(text, &pos);
    if (!key) {
      goto fail;
    }
    if (!json_consume_char(text, &pos, ':')) {
      free(key);
      goto fail;
    }
    if (strcmp(key, "functions") == 0 || strcmp(key, "fns") == 0) {
      label = strcmp(key, "functions") == 0 ? "functions" : "fns";
      if (!json_consume_char(text, &pos, '[')) {
        free(key);
        goto fail;
      }
      skip_json_ws(text, &pos);
      if (text[pos] == ']') {
        pos++;
      } else {
        while (text[pos]) {
          char *name = NULL;
          char *args = NULL;
          if (!json_consume_char(text, &pos, '{')) {
            free(key);
            goto fail;
          }
          skip_json_ws(text, &pos);
          if (text[pos] == '}') {
            pos++;
            name = xstrdup("?");
            args = xstrdup("?");
          } else {
            while (text[pos]) {
              char *fkey = json_parse_string(text, &pos);
              if (!fkey) {
                free(key);
                free(name);
                free(args);
                goto fail;
              }
              if (!json_consume_char(text, &pos, ':')) {
                free(key);
                free(fkey);
                free(name);
                free(args);
                goto fail;
              }
              if (strcmp(fkey, "name") == 0) {
                free(name);
                name = json_parse_string(text, &pos);
                if (!name) {
                  free(key);
                  free(fkey);
                  free(args);
                  goto fail;
                }
              } else if (strcmp(fkey, "args") == 0) {
                free(args);
                args = json_parse_string(text, &pos);
                if (!args) {
                  free(key);
                  free(fkey);
                  free(name);
                  goto fail;
                }
              } else {
                if (!json_skip_value(text, &pos)) {
                  free(key);
                  free(fkey);
                  free(name);
                  free(args);
                  goto fail;
                }
              }
              free(fkey);
              skip_json_ws(text, &pos);
              if (text[pos] == ',') {
                pos++;
                continue;
              }
              if (text[pos] == '}') {
                pos++;
                break;
              }
              free(key);
              free(name);
              free(args);
              goto fail;
            }
            if (!name) name = xstrdup("?");
            if (!args) args = xstrdup("?");
          }
          array_push(&names, name);
          array_push(&args_list, args);
          skip_json_ws(text, &pos);
          if (text[pos] == ',') {
            pos++;
            continue;
          }
          if (text[pos] == ']') {
            pos++;
            break;
          }
          free(key);
          goto fail;
        }
      }
    } else {
      if (!json_skip_value(text, &pos)) {
        free(key);
        goto fail;
      }
    }
    free(key);
    skip_json_ws(text, &pos);
    if (text[pos] == ',') {
      pos++;
      continue;
    }
    if (text[pos] == '}') {
      break;
    }
    goto fail;
  }

  printf("설치된 패키지 %zu개:\n", names.len);
  for (i = 0; i < names.len; i++) {
    size_t j;
    for (j = i + 1; j < names.len; j++) {
      if (strcmp(names.items[i], names.items[j]) > 0) {
        char *tmp = names.items[i];
        names.items[i] = names.items[j];
        names.items[j] = tmp;
        tmp = args_list.items[i];
        args_list.items[i] = args_list.items[j];
        args_list.items[j] = tmp;
      }
    }
  }
  for (i = 0; i < names.len; i++) {
    printf("  %s  (%s인자)\n", names.items[i], args_list.items[i]);
  }
  for (i = 0; i < names.len; i++) {
    free(names.items[i]);
    free(args_list.items[i]);
  }
  free(names.items);
  free(args_list.items);
  (void)label;
  return 1;

fail:
  for (i = 0; i < names.len; i++) {
    free(names.items[i]);
    free(args_list.items[i]);
  }
  free(names.items);
  free(args_list.items);
  return 0;
}

static void print_key_array_lines(KeyValue *item) {
  size_t i;
  if (!item || item->type != VALUE_ARRAY) {
    return;
  }
  for (i = 0; i < item->array_value.len; i++) {
    fputs(item->array_value.items[i], stdout);
    putchar('\n');
  }
}

static int print_run_pkg_import_output(const char *text, const char *pkg_name) {
  size_t pos = 0;
  int ok = 0;
  int have_ok = 0;
  long duration_ms = -1;
  char *error = NULL;
  if (!json_consume_char(text, &pos, '{')) {
    return 0;
  }
  while (1) {
    char *key;
    skip_json_ws(text, &pos);
    if (text[pos] == '}') {
      break;
    }
    key = json_parse_string(text, &pos);
    if (!key) {
      free(error);
      return 0;
    }
    if (!json_consume_char(text, &pos, ':')) {
      free(key);
      free(error);
      return 0;
    }
    if (strcmp(key, "ok") == 0) {
      if (!json_parse_bool(text, &pos, &ok)) {
        free(key);
        free(error);
        return 0;
      }
      have_ok = 1;
    } else if (strcmp(key, "error") == 0) {
      free(error);
      error = json_parse_string(text, &pos);
      if (!error) {
        free(key);
        return 0;
      }
    } else if (strcmp(key, "build") == 0) {
      if (!json_consume_char(text, &pos, '{')) {
        free(key);
        free(error);
        return 0;
      }
      while (1) {
        char *bkey;
        skip_json_ws(text, &pos);
        if (text[pos] == '}') {
          pos++;
          break;
        }
        bkey = json_parse_string(text, &pos);
        if (!bkey) {
          free(key);
          free(error);
          return 0;
        }
        if (!json_consume_char(text, &pos, ':')) {
          free(key);
          free(bkey);
          free(error);
          return 0;
        }
        if (strcmp(bkey, "durationMs") == 0) {
          if (!json_parse_nonneg_int(text, &pos, &duration_ms)) {
            free(key);
            free(bkey);
            free(error);
            return 0;
          }
        } else {
          if (!json_skip_value(text, &pos)) {
            free(key);
            free(bkey);
            free(error);
            return 0;
          }
        }
        free(bkey);
        skip_json_ws(text, &pos);
        if (text[pos] == ',') {
          pos++;
          continue;
        }
        if (text[pos] == '}') {
          pos++;
          break;
        }
        free(key);
        free(error);
        return 0;
      }
    } else {
      if (!json_skip_value(text, &pos)) {
        free(key);
        free(error);
        return 0;
      }
    }
    free(key);
    skip_json_ws(text, &pos);
    if (text[pos] == ',') {
      pos++;
      continue;
    }
    if (text[pos] == '}') {
      break;
    }
    free(error);
    return 0;
  }
  if (!have_ok) {
    free(error);
    return 0;
  }
  if (ok) {
    if (duration_ms >= 0) {
      printf("  ✅ %s (%ldms)\n", pkg_name, duration_ms);
    } else {
      printf("  ✅ %s (?ms)\n", pkg_name);
    }
  } else {
    printf("  ❌ %s — %s\n", pkg_name, error ? error : "");
  }
  free(error);
  return 1;
}

static void print_run_pkg_import_payload(const char *pkg_name) {
  putchar('{');
  json_print_string("name");
  fputs(": ", stdout);
  json_print_string(pkg_name);
  fputs(", ", stdout);
  json_print_string("build");
  fputs(": true", stdout);
  puts("}");
}

int main(int argc, char **argv) {
  const char *mode = "json";
  const char *path = "fx.toml";
  const char *query = NULL;
  char *text;
  Document doc;

  if (argc >= 2 && strcmp(argv[1], "--get") == 0) {
    if (argc < 4) {
      return 1;
    }
    mode = "get";
    query = argv[2];
    path = argv[3];
  } else if (argc >= 2 && strcmp(argv[1], "--runtime-payload") == 0) {
    if (argc < 3) {
      return 1;
    }
    mode = "runtime-payload";
    path = argv[2];
  } else if (argc >= 2 && strcmp(argv[1], "--new-payload") == 0) {
    int profile_count;
    int package_count;
    int expected_argc;
    if (argc < 6 || strcmp(argv[2], "--profiles") != 0) {
      return 1;
    }
    profile_count = parse_count_or_die(argv[3]);
    if (profile_count < 0) {
      return 1;
    }
    if (4 + profile_count + 2 >= argc) {
      return 1;
    }
    if (strcmp(argv[4 + profile_count], "--packages") != 0) {
      return 1;
    }
    package_count = parse_count_or_die(argv[5 + profile_count]);
    if (package_count < 0) {
      return 1;
    }
    expected_argc = 6 + profile_count + package_count;
    if (argc != expected_argc) {
      return 1;
    }
    print_new_payload_from_lists(profile_count, argv + 4, package_count, argv + 6 + profile_count);
    return 0;
  } else if (argc >= 2 && strcmp(argv[1], "--new-init-ok") == 0) {
    if (argc < 3) {
      return 1;
    }
    mode = "new-init-ok";
    path = argv[2];
  } else if (argc >= 2 && strcmp(argv[1], "--new-init-error") == 0) {
    if (argc < 3) {
      return 1;
    }
    mode = "new-init-error";
    path = argv[2];
  } else if (argc >= 2 && strcmp(argv[1], "--new-init-package-summary") == 0) {
    if (argc < 3) {
      return 1;
    }
    mode = "new-init-package-summary";
    path = argv[2];
  } else if (argc >= 2 && strcmp(argv[1], "--profile-list") == 0) {
    if (argc < 3) {
      return 1;
    }
    mode = "profile-list";
    path = argv[2];
  } else if (argc >= 2 && strcmp(argv[1], "--profile-add-output") == 0) {
    if (argc < 3) {
      return 1;
    }
    mode = "profile-add-output";
    path = argv[2];
  } else if (argc >= 2 && strcmp(argv[1], "--pkg-list") == 0) {
    if (argc < 3) {
      return 1;
    }
    mode = "pkg-list";
    path = argv[2];
  } else if (argc >= 2 && strcmp(argv[1], "--get-array-lines") == 0) {
    if (argc < 4) {
      return 1;
    }
    mode = "get-array-lines";
    query = argv[2];
    path = argv[3];
  } else if (argc >= 2 && strcmp(argv[1], "--run-pkg-import-output") == 0) {
    if (argc < 4) {
      return 1;
    }
    mode = "run-pkg-import-output";
    query = argv[2];
    path = argv[3];
  } else if (argc >= 2 && strcmp(argv[1], "--run-pkg-import-payload") == 0) {
    if (argc < 3) {
      return 1;
    }
    query = argv[2];
    print_run_pkg_import_payload(query);
    return 0;
  } else if (argc >= 2) {
    path = argv[1];
  }

  text = read_file_or_null(path);
  doc.sections = NULL;
  doc.len = 0;
  doc.cap = 0;
  if (!text) {
    if (strcmp(mode, "json") == 0) {
      puts("{}");
    }
    return 0;
  }

  if (strcmp(mode, "new-init-ok") == 0) {
    int ok = print_new_init_ok(text);
    free(text);
    return ok ? 0 : 1;
  }
  if (strcmp(mode, "new-init-error") == 0) {
    int ok = print_new_init_error(text);
    free(text);
    return ok ? 0 : 1;
  }
  if (strcmp(mode, "new-init-package-summary") == 0) {
    int ok = print_new_init_package_summary(text);
    free(text);
    return ok ? 0 : 1;
  }
  if (strcmp(mode, "profile-list") == 0) {
    int ok = print_profile_list_output(text);
    free(text);
    return ok ? 0 : 1;
  }
  if (strcmp(mode, "profile-add-output") == 0) {
    int ok = print_profile_add_output(text);
    free(text);
    return ok ? 0 : 1;
  }
  if (strcmp(mode, "pkg-list") == 0) {
    int ok = print_pkg_list_output(text);
    free(text);
    return ok ? 0 : 1;
  }
  if (strcmp(mode, "run-pkg-import-output") == 0) {
    int ok = print_run_pkg_import_output(text, query ? query : "");
    free(text);
    return ok ? 0 : 1;
  }

  parse_toml(text, &doc);

  if (strcmp(mode, "get") == 0) {
    char *query_copy = xstrdup(query);
    char *dot = strchr(query_copy, '.');
    if (dot) {
      Section *section;
      KeyValue *item;
      *dot = '\0';
      section = find_section(&doc, query_copy);
      item = find_key(section, dot + 1);
      print_key_value(item);
    }
    free(query_copy);
  } else if (strcmp(mode, "get-array-lines") == 0) {
    char *query_copy = xstrdup(query);
    char *dot = strchr(query_copy, '.');
    if (dot) {
      Section *section;
      KeyValue *item;
      *dot = ' ';
      section = find_section(&doc, query_copy);
      item = find_key(section, dot + 1);
      print_key_array_lines(item);
    }
    free(query_copy);
  } else if (strcmp(mode, "runtime-payload") == 0) {
    print_runtime_payload(&doc);
  } else {
    json_print_document(&doc);
  }

  free_document(&doc);
  free(text);
  return 0;
}
