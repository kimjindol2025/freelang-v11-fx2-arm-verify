#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  char **items;
  size_t len;
  size_t cap;
} StringList;

typedef struct {
  const char *s;
  size_t len;
  size_t pos;
} Parser;

typedef struct {
  char *package_name;
  char *declared_name;
  long long args;
  int args_set;
  char *body;
  int skip_runtime_shim;
  int skip_runtime_shim_set;
} PackageMetadata;

typedef struct {
  PackageMetadata *items;
  size_t len;
  size_t cap;
} MetadataList;

static char *xstrdup(const char *s) {
  size_t len = strlen(s) + 1;
  char *copy = (char *)malloc(len);
  if (!copy) return NULL;
  memcpy(copy, s, len);
  return copy;
}

static void strlist_init(StringList *list) {
  list->items = NULL;
  list->len = 0;
  list->cap = 0;
}

static void strlist_free(StringList *list) {
  if (!list) return;

  for (size_t i = 0; i < list->len; i++) {
    free(list->items[i]);
  }
  free(list->items);
  strlist_init(list);
}

static int strlist_has(StringList *list, const char *value) {
  if (!list || !value) return 0;
  for (size_t i = 0; i < list->len; i++) {
    if (strcmp(list->items[i], value) == 0) return 1;
  }
  return 0;
}

static int strlist_has_from(const StringList *list, const char *value, size_t end) {
  if (!list || !value) return 0;
  for (size_t i = 0; i < end && i < list->len; i++) {
    if (strcmp(list->items[i], value) == 0) return 1;
  }
  return 0;
}

static int strlist_append(StringList *list, const char *value) {
  if (!value) return 1;

  char *dup = xstrdup(value);
  if (!dup) return 0;

  if (list->len == list->cap) {
    size_t next = list->cap ? list->cap * 2 : 8;
    char **next_items = (char **)realloc(list->items, next * sizeof(char *));
    if (!next_items) {
      free(dup);
      return 0;
    }
    list->items = next_items;
    list->cap = next;
  }

  list->items[list->len++] = dup;
  return 1;
}

static int strlist_append_uniq(StringList *list, const char *value) {
  if (strlist_has(list, value)) return 1;
  return strlist_append(list, value);
}

static void metadata_init(PackageMetadata *meta) {
  meta->package_name = NULL;
  meta->declared_name = NULL;
  meta->args = 1;
  meta->args_set = 0;
  meta->body = NULL;
  meta->skip_runtime_shim = 0;
  meta->skip_runtime_shim_set = 0;
}

static void metadata_free(PackageMetadata *meta) {
  if (!meta) return;
  free(meta->package_name);
  free(meta->declared_name);
  free(meta->body);
  metadata_init(meta);
}

static void metallist_init(MetadataList *list) {
  list->items = NULL;
  list->len = 0;
  list->cap = 0;
}

static void metallist_free(MetadataList *list) {
  if (!list) return;

  for (size_t i = 0; i < list->len; i++) {
    metadata_free(&list->items[i]);
  }

  free(list->items);
  metallist_init(list);
}

static int metallist_append(MetadataList *list, const PackageMetadata *meta) {
  if (!meta) return 1;

  if (list->len == list->cap) {
    size_t next = list->cap ? list->cap * 2 : 8;
    PackageMetadata *next_items = (PackageMetadata *)realloc(list->items, next * sizeof(PackageMetadata));
    if (!next_items) return 0;
    list->items = next_items;
    list->cap = next;
  }

  PackageMetadata copy;
  metadata_init(&copy);
  copy.args = meta->args;
  copy.args_set = meta->args_set;
  copy.skip_runtime_shim = meta->skip_runtime_shim;
  copy.skip_runtime_shim_set = 1;
  copy.package_name = meta->package_name ? xstrdup(meta->package_name) : NULL;
  copy.declared_name = meta->declared_name ? xstrdup(meta->declared_name) : NULL;
  copy.body = meta->body ? xstrdup(meta->body) : xstrdup("");
  if ((meta->package_name && !copy.package_name) ||
      (meta->declared_name && !copy.declared_name) ||
      (meta->body && !copy.body)) {
    metadata_free(&copy);
    return 0;
  }

  if (!copy.body) copy.body = xstrdup("");

  if (!copy.body) {
    metadata_free(&copy);
    return 0;
  }

  list->items[list->len++] = copy;
  return 1;
}

static void parser_skip_ws(Parser *p) {
  while (p->pos < p->len) {
    unsigned char ch = (unsigned char)p->s[p->pos];
    if (ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t') {
      p->pos++;
      continue;
    }
    break;
  }
}

static int parser_expect(Parser *p, char c) {
  parser_skip_ws(p);
  if (p->pos >= p->len || p->s[p->pos] != c) return 0;
  p->pos++;
  return 1;
}

static int parser_check(Parser *p, char c) {
  parser_skip_ws(p);
  return (p->pos < p->len && p->s[p->pos] == c);
}

static int parser_append(char ch, char **buf, size_t *len, size_t *cap) {
  if (*len + 2 > *cap) {
    size_t next = *cap ? *cap * 2 : 16;
    while (next < *len + 2) next *= 2;
    char *tmp = (char *)realloc(*buf, next);
    if (!tmp) return 0;
    *buf = tmp;
    *cap = next;
  }

  (*buf)[(*len)++] = ch;
  (*buf)[*len] = '\0';
  return 1;
}

static int parser_parse_string(Parser *p, char **out) {
  parser_skip_ws(p);
  if (p->pos >= p->len || p->s[p->pos] != '"') return 0;
  p->pos++;

  char *buf = NULL;
  size_t len = 0;
  size_t cap = 0;

  while (p->pos < p->len) {
    char c = p->s[p->pos++];
    if (c == '"') {
      if (!buf) {
        *out = xstrdup("");
        return *out ? 1 : 0;
      }
      *out = buf;
      return *out ? 1 : 0;
    }

    if (c == '\\') {
      if (p->pos >= p->len) {
        free(buf);
        return 0;
      }
      char e = p->s[p->pos++];
      switch (e) {
        case '"':
        case '\\':
        case '/':
          if (!parser_append(e, &buf, &len, &cap)) goto fail;
          break;
        case 'b':
          if (!parser_append('\b', &buf, &len, &cap)) goto fail;
          break;
        case 'f':
          if (!parser_append('\f', &buf, &len, &cap)) goto fail;
          break;
        case 'n':
          if (!parser_append('\n', &buf, &len, &cap)) goto fail;
          break;
        case 'r':
          if (!parser_append('\r', &buf, &len, &cap)) goto fail;
          break;
        case 't':
          if (!parser_append('\t', &buf, &len, &cap)) goto fail;
          break;
        case 'u':
          if (p->pos + 4 > p->len) goto fail;
          p->pos += 4;
          if (!parser_append('?', &buf, &len, &cap)) goto fail;
          break;
        default:
          if (!parser_append(e, &buf, &len, &cap)) goto fail;
          break;
      }
      continue;
    }

    if (!parser_append(c, &buf, &len, &cap)) goto fail;
  }

fail:
  free(buf);
  return 0;
}

static int parser_skip_value(Parser *p);

static int parser_skip_array(Parser *p) {
  if (!parser_expect(p, '[')) return 0;
  parser_skip_ws(p);

  if (parser_check(p, ']')) {
    p->pos++;
    return 1;
  }

  while (p->pos < p->len) {
    if (!parser_skip_value(p)) return 0;
    parser_skip_ws(p);
    if (parser_check(p, ',')) {
      p->pos++;
      continue;
    }
    break;
  }

  if (!parser_expect(p, ']')) return 0;
  return 1;
}

static int parser_skip_object(Parser *p) {
  if (!parser_expect(p, '{')) return 0;
  parser_skip_ws(p);
  if (parser_check(p, '}')) {
    p->pos++;
    return 1;
  }

  while (p->pos < p->len) {
    char *key = NULL;
    if (!parser_parse_string(p, &key)) return 0;

    parser_skip_ws(p);
    if (!parser_expect(p, ':')) {
      free(key);
      return 0;
    }

    if (!parser_skip_value(p)) {
      free(key);
      return 0;
    }

    free(key);
    parser_skip_ws(p);
    if (parser_check(p, ',')) {
      p->pos++;
      continue;
    }
    break;
  }

  if (!parser_expect(p, '}')) return 0;
  return 1;
}

static int parser_skip_value(Parser *p) {
  parser_skip_ws(p);
  if (p->pos >= p->len) return 0;

  char c = p->s[p->pos];
  if (c == '{') return parser_skip_object(p);
  if (c == '[') return parser_skip_array(p);
  if (c == '"') {
    char *tmp = NULL;
    if (!parser_parse_string(p, &tmp)) return 0;
    free(tmp);
    return 1;
  }
  if (c == 't' || c == 'f' || c == 'n') {
    const char *lit = (c == 't') ? "true" : (c == 'f' ? "false" : "null");
    size_t l = strlen(lit);
    if (p->pos + l > p->len) return 0;
    if (strncmp(p->s + p->pos, lit, l) != 0) return 0;
    p->pos += l;
    return 1;
  }
  if (c == '-' || isdigit((unsigned char)c)) {
    p->pos++;
    while (p->pos < p->len && isdigit((unsigned char)p->s[p->pos])) p->pos++;
    if (p->pos < p->len && p->s[p->pos] == '.') {
      p->pos++;
      while (p->pos < p->len && isdigit((unsigned char)p->s[p->pos])) p->pos++;
    }
    return 1;
  }

  return 0;
}

static int parser_parse_integer(Parser *p, long long *out) {
  parser_skip_ws(p);
  if (p->pos >= p->len) return 0;

  int sign = 1;
  long long value = 0;

  if (p->s[p->pos] == '-') {
    sign = -1;
    p->pos++;
    if (p->pos >= p->len) return 0;
  }

  if (!isdigit((unsigned char)p->s[p->pos])) return 0;

  while (p->pos < p->len && isdigit((unsigned char)p->s[p->pos])) {
    int d = p->s[p->pos] - '0';
    if (value > (LLONG_MAX - d) / 10) return 0;
    value = value * 10 + d;
    p->pos++;
  }

  if (p->pos < p->len) {
    char next = p->s[p->pos];
    if (next == '.' || next == 'e' || next == 'E') return 0;
  }

  *out = (long long)(sign * value);
  return 1;
}

static int parser_parse_boolean(Parser *p, int *out) {
  parser_skip_ws(p);
  if (p->pos >= p->len) return 0;

  if (p->s[p->pos] == 't') {
    if (p->pos + 4 <= p->len && strncmp(p->s + p->pos, "true", 4) == 0) {
      p->pos += 4;
      *out = 1;
      return 1;
    }
    return 0;
  }

  if (p->s[p->pos] == 'f') {
    if (p->pos + 5 <= p->len && strncmp(p->s + p->pos, "false", 5) == 0) {
      p->pos += 5;
      *out = 0;
      return 1;
    }
    return 0;
  }

  return 0;
}

static int parser_parse_array_strings(Parser *p, StringList *packages) {
  if (!parser_expect(p, '[')) return 0;
  parser_skip_ws(p);

  if (parser_check(p, ']')) {
    p->pos++;
    return 1;
  }

  while (p->pos < p->len) {
    char *value = NULL;
    if (!parser_parse_string(p, &value)) return 0;
    if (!strlist_append(packages, value)) {
      free(value);
      return 0;
    }
    free(value);

    parser_skip_ws(p);
    if (parser_check(p, ',')) {
      p->pos++;
      continue;
    }
    break;
  }

  if (!parser_expect(p, ']')) return 0;
  return 1;
}

static int parser_parse_object(Parser *p, StringList *packages, int pick_packages) {
  if (!parser_expect(p, '{')) return 0;
  parser_skip_ws(p);

  if (parser_check(p, '}')) {
    p->pos++;
    return 1;
  }

  while (p->pos < p->len) {
    char *key = NULL;
    if (!parser_parse_string(p, &key)) return 0;

    parser_skip_ws(p);
    if (!parser_expect(p, ':')) {
      free(key);
      return 0;
    }

    if (pick_packages && strcmp(key, "packages") == 0) {
      if (!parser_parse_array_strings(p, packages)) {
        free(key);
        return 0;
      }
    } else {
      if (!parser_skip_value(p)) {
        free(key);
        return 0;
      }
    }

    free(key);
    parser_skip_ws(p);
    if (parser_check(p, ',')) {
      p->pos++;
      continue;
    }
    break;
  }

  if (!parser_expect(p, '}')) return 0;
  return 1;
}

static int parser_parse_metadata_object(Parser *p, PackageMetadata *meta) {
  if (!parser_expect(p, '{')) return 0;
  parser_skip_ws(p);

  if (parser_check(p, '}')) {
    p->pos++;
    return 1;
  }

  while (p->pos < p->len) {
    char *key = NULL;
    if (!parser_parse_string(p, &key)) return 0;

    parser_skip_ws(p);
    if (!parser_expect(p, ':')) {
      free(key);
      return 0;
    }

    if (strcmp(key, "name") == 0) {
      char *name = NULL;
      if (parser_parse_string(p, &name)) {
        free(meta->declared_name);
        meta->declared_name = name;
      } else if (!parser_skip_value(p)) {
        free(key);
        return 0;
      }
    } else if (strcmp(key, "args") == 0) {
      long long args;
      if (parser_parse_integer(p, &args)) {
        meta->args = args;
        meta->args_set = 1;
      } else if (!parser_skip_value(p)) {
        free(key);
        return 0;
      }
    } else if (strcmp(key, "body") == 0) {
      char *body = NULL;
      if (parser_parse_string(p, &body)) {
        free(meta->body);
        meta->body = body;
      } else if (!parser_skip_value(p)) {
        free(key);
        return 0;
      }
    } else if (strcmp(key, "skipRuntimeShim") == 0) {
      int skip = 0;
      if (parser_parse_boolean(p, &skip)) {
        meta->skip_runtime_shim = skip;
        meta->skip_runtime_shim_set = 1;
      } else if (!parser_skip_value(p)) {
        free(key);
        return 0;
      }
    } else {
      if (!parser_skip_value(p)) {
        free(key);
        return 0;
      }
    }

    free(key);
    parser_skip_ws(p);
    if (parser_check(p, ',')) {
      p->pos++;
      continue;
    }
    break;
  }

  if (!parser_expect(p, '}')) return 0;
  return 1;
}

static int parser_parse_profile_packages(const char *json, StringList *packages) {
  Parser parser = {json, strlen(json), 0};
  int ok = parser_parse_object(&parser, packages, 1);
  if (!ok) return 0;
  parser_skip_ws(&parser);
  if (parser.pos != parser.len) return 0;
  return 1;
}

static int parser_parse_metadata(const char *json, PackageMetadata *meta) {
  Parser parser = {json, strlen(json), 0};
  if (!parser_parse_metadata_object(&parser, meta)) return 0;
  parser_skip_ws(&parser);
  if (parser.pos != parser.len) return 0;
  if (!meta->declared_name) {
    free(meta->declared_name);
    meta->declared_name = xstrdup(meta->package_name);
  }
  if (!meta->body) {
    meta->body = xstrdup("");
  }
  if (!meta->declared_name) return 0;
  if (!meta->body) return 0;
  if (!meta->args_set) meta->args = 1;
  return 1;
}

static int read_file(const char *path, char **out) {
  FILE *in = fopen(path, "rb");
  if (!in) return 0;

  if (fseek(in, 0, SEEK_END) != 0) {
    fclose(in);
    return 0;
  }

  long size = ftell(in);
  if (size < 0) {
    fclose(in);
    return 0;
  }

  rewind(in);

  char *buf = (char *)malloc((size_t)size + 1);
  if (!buf) {
    fclose(in);
    return 0;
  }

  size_t n = fread(buf, 1, (size_t)size, in);
  fclose(in);
  if (n != (size_t)size) {
    free(buf);
    return 0;
  }

  buf[n] = '\0';
  *out = buf;
  return 1;
}

static int collect_profile(const char *script_dir, const char *profile_name, StringList *all_packages) {
  size_t profile_path_len = strlen(script_dir) + strlen(profile_name) + 20;
  char *profile_path = (char *)malloc(profile_path_len);
  if (!profile_path) {
    fprintf(stderr, "  ⚠️  프로파일 처리 실패: %s\n", profile_name);
    return 0;
  }

  if (snprintf(profile_path, profile_path_len, "%s/profiles/%s.json", script_dir, profile_name) >= (int)profile_path_len) {
    fprintf(stderr, "  ⚠️  프로파일 처리 실패: %s\n", profile_name);
    free(profile_path);
    return 0;
  }

  FILE *probe = fopen(profile_path, "rb");
  if (!probe) {
    fprintf(stderr, "  ⚠️  프로파일 없음: %s\n", profile_name);
    free(profile_path);
    return 1;
  }
  fclose(probe);

  char *json = NULL;
  if (!read_file(profile_path, &json)) {
    fprintf(stderr, "  ⚠️  프로파일 읽기 실패: %s\n", profile_name);
    free(profile_path);
    return 0;
  }
  free(profile_path);

  StringList profile_packages;
  strlist_init(&profile_packages);
  if (!parser_parse_profile_packages(json, &profile_packages)) {
    fprintf(stderr, "  ⚠️  프로파일 JSON 파싱 실패: %s\n", profile_name);
    free(json);
    strlist_free(&profile_packages);
    return 0;
  }
  free(json);

  printf("  ✓ :%s → ", profile_name);
  for (size_t i = 0; i < profile_packages.len; i++) {
    if (i > 0) printf(", ");
    printf("%s", profile_packages.items[i]);
  }
  printf("\n");

  size_t seen_before = all_packages->len;
  for (size_t i = 0; i < profile_packages.len; i++) {
    if (!strlist_has_from(all_packages, profile_packages.items[i], seen_before)) {
      if (!strlist_append(all_packages, profile_packages.items[i])) {
        strlist_free(&profile_packages);
        return 0;
      }
    }
  }

  strlist_free(&profile_packages);
  return 1;
}

static int collect_metadata(const char *script_dir, const char *package_name, MetadataList *metas) {
  size_t path_len = strlen(script_dir) + strlen(package_name) + 40;
  char *path = (char *)malloc(path_len);
  if (!path) return 0;

  if (snprintf(path, path_len, "%s/packages/%s/metadata.json", script_dir, package_name) >= (int)path_len) {
    free(path);
    return 0;
  }

  FILE *probe = fopen(path, "rb");
  if (!probe) {
    if (snprintf(path, path_len, "%s/packages/%s.json", script_dir, package_name) >= (int)path_len) {
      free(path);
      return 0;
    }
    probe = fopen(path, "rb");
  }

  if (!probe) {
    fprintf(stderr, "  ⚠️  패키지 없음: %s\n", package_name);
    free(path);
    return 1;
  }
  fclose(probe);

  char *json = NULL;
  if (!read_file(path, &json)) {
    fprintf(stderr, "  ⚠️  패키지 읽기 실패: %s\n", package_name);
    free(path);
    return 0;
  }
  free(path);

  PackageMetadata raw;
  metadata_init(&raw);
  raw.package_name = xstrdup(package_name);
  if (!raw.package_name) {
    free(json);
    metadata_free(&raw);
    return 0;
  }

  if (!parser_parse_metadata(json, &raw)) {
    fprintf(stderr, "  ⚠️  패키지 JSON 파싱 실패: %s\n", package_name);
    free(json);
    metadata_free(&raw);
    return 0;
  }
  free(json);

  if (!metas) {
    metadata_free(&raw);
    return 0;
  }

  if (!raw.args_set) {
    raw.args = 1;
  }
  if (!raw.body) {
    raw.body = xstrdup("");
    if (!raw.body) {
      metadata_free(&raw);
      return 0;
    }
  }

  if (!metas->cap && !metas->len) {
    metallist_init(metas);
  }

  if (!metallist_append(metas, &raw)) {
    metadata_free(&raw);
    return 0;
  }

  metadata_free(&raw);
  return 1;
}

static void json_escape_put(FILE *out, const char *s) {
  while (*s) {
    unsigned char c = (unsigned char)*s;
    switch (c) {
      case '\\':
        fputs("\\\\", out);
        break;
      case '"':
        fputs("\\\"", out);
        break;
      case '\b':
        fputs("\\b", out);
        break;
      case '\f':
        fputs("\\f", out);
        break;
      case '\n':
        fputs("\\n", out);
        break;
      case '\r':
        fputs("\\r", out);
        break;
      case '\t':
        fputs("\\t", out);
        break;
      default:
        if (c < 0x20) {
          fprintf(out, "\\u%04x", c);
        } else {
          fputc(c, out);
        }
        break;
    }
    s++;
  }
}

static void output_metadata_json(const MetadataList *metas) {
  printf("[\n");
  for (size_t i = 0; i < metas->len; i++) {
    const PackageMetadata *m = &metas->items[i];
    if (i > 0) printf(",\n");

    printf("  {\"package\":\"");
    json_escape_put(stdout, m->package_name ? m->package_name : "");
    printf("\",\"name\":\"");
    json_escape_put(stdout, m->declared_name ? m->declared_name : "");
    printf("\",\"args\":%lld,\"body\":\"", m->args);
    json_escape_put(stdout, m->body ? m->body : "");
    printf("\",\"skipRuntimeShim\":%s}", m->skip_runtime_shim ? "true" : "false");
  }
  printf("\n]\n");
}

static int metadata_mode(const char *script_dir, int argc, char **argv) {
  MetadataList metas;
  metallist_init(&metas);

  for (int i = 0; i < argc; i++) {
    const char *package_name = argv[i];
    if (!package_name || !*package_name) continue;

    if (!collect_metadata(script_dir, package_name, &metas)) {
      metallist_free(&metas);
      return 1;
    }
  }

  output_metadata_json(&metas);
  metallist_free(&metas);
  return 0;
}

int main(int argc, char **argv) {
  if (argc < 3) {
    if (argc >= 2 && strcmp(argv[1], "--metadata") == 0) {
      fprintf(stderr, "사용법: fl-resolve-deps-profiles --metadata <script_dir> [package...]");
    } else {
      fprintf(stderr, "사용법: fl-resolve-deps-profiles <script_dir> <profile...>\n");
    }
    return 1;
  }

  if (strcmp(argv[1], "--metadata") == 0) {
    return metadata_mode(argv[2], argc - 3, argv + 3);
  }

  const char *script_dir = argv[1];
  StringList all_packages;
  strlist_init(&all_packages);

  for (int i = 2; i < argc; i++) {
    if (!collect_profile(script_dir, argv[i], &all_packages)) {
      strlist_free(&all_packages);
      return 1;
    }
  }

  for (size_t i = 0; i < all_packages.len; i++) {
    printf("%s\n", all_packages.items[i]);
  }

  strlist_free(&all_packages);
  return 0;
}
