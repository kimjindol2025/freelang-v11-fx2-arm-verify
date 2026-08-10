#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ARRAY_INIT_CAP 8

typedef struct {
  char *data;
  size_t len;
  size_t cap;
} StrBuf;

typedef struct {
  char **items;
  size_t len;
  size_t cap;
} StrList;

typedef struct {
  int line;
  char *msg_prefix;
} SeenError;

typedef struct {
  SeenError *items;
  size_t len;
  size_t cap;
} SeenList;

static void *xmalloc(size_t size) {
  void *ptr = malloc(size ? size : 1);
  if (!ptr) {
    fprintf(stderr, "out of memory\n");
    exit(1);
  }
  return ptr;
}

static void *xrealloc(void *ptr, size_t size) {
  void *next = realloc(ptr, size ? size : 1);
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

static void sb_init(StrBuf *buf) {
  buf->data = NULL;
  buf->len = 0;
  buf->cap = 0;
}

static void sb_reserve(StrBuf *buf, size_t extra) {
  size_t need = buf->len + extra + 1;
  if (need <= buf->cap) {
    return;
  }
  while (buf->cap < need) {
    buf->cap = buf->cap ? buf->cap * 2 : 128;
  }
  buf->data = (char *)xrealloc(buf->data, buf->cap);
}

static void sb_append_n(StrBuf *buf, const char *text, size_t n) {
  sb_reserve(buf, n);
  memcpy(buf->data + buf->len, text, n);
  buf->len += n;
  buf->data[buf->len] = '\0';
}

static void sb_append(StrBuf *buf, const char *text) {
  sb_append_n(buf, text, strlen(text));
}

static void sb_append_char(StrBuf *buf, char ch) {
  sb_reserve(buf, 1);
  buf->data[buf->len++] = ch;
  buf->data[buf->len] = '\0';
}

static char *sb_take(StrBuf *buf) {
  char *out;
  if (!buf->data) {
    out = xstrdup("");
  } else {
    out = buf->data;
  }
  buf->data = NULL;
  buf->len = 0;
  buf->cap = 0;
  return out;
}

static void list_push(StrList *list, char *item) {
  if (list->len == list->cap) {
    list->cap = list->cap ? list->cap * 2 : ARRAY_INIT_CAP;
    list->items = (char **)xrealloc(list->items, list->cap * sizeof(char *));
  }
  list->items[list->len++] = item;
}

static int list_contains(const StrList *list, const char *item) {
  size_t i;
  for (i = 0; i < list->len; i++) {
    if (strcmp(list->items[i], item) == 0) {
      return 1;
    }
  }
  return 0;
}

static void list_pop(StrList *list) {
  if (list->len > 0) {
    list->len--;
  }
}

static void list_free(StrList *list) {
  size_t i;
  for (i = 0; i < list->len; i++) {
    free(list->items[i]);
  }
  free(list->items);
  list->items = NULL;
  list->len = 0;
  list->cap = 0;
}

static char *read_file_or_null(const char *path) {
  FILE *fp = fopen(path, "rb");
  long size;
  char *buf;
  size_t got;
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
  buf = (char *)xmalloc((size_t)size + 1);
  got = fread(buf, 1, (size_t)size, fp);
  fclose(fp);
  buf[got] = '\0';
  return buf;
}

static int write_file(const char *path, const char *text) {
  FILE *fp = fopen(path, "wb");
  size_t len;
  if (!fp) {
    return 0;
  }
  len = strlen(text);
  if (len > 0 && fwrite(text, 1, len, fp) != len) {
    fclose(fp);
    return 0;
  }
  fclose(fp);
  return 1;
}

static char *path_dirname_dup(const char *path) {
  const char *slash = strrchr(path, '/');
  if (!slash) {
    return xstrdup(".");
  }
  if (slash == path) {
    return xstrdup("/");
  }
  return substr_dup(path, 0, (size_t)(slash - path));
}

static char *join_path(const char *base, const char *rel) {
  StrBuf buf;
  sb_init(&buf);
  if (rel[0] == '/') {
    sb_append(&buf, rel);
    return sb_take(&buf);
  }
  sb_append(&buf, base);
  if (buf.len == 0 || buf.data[buf.len - 1] != '/') {
    sb_append_char(&buf, '/');
  }
  sb_append(&buf, rel);
  return sb_take(&buf);
}

static char *normalize_path_dup(const char *path) {
  char *tmp = xstrdup(path);
  char *save = NULL;
  char *part;
  StrList comps = {0};
  int absolute = path[0] == '/';
  char *result;
  StrBuf buf;
  for (part = strtok_r(tmp, "/", &save); part; part = strtok_r(NULL, "/", &save)) {
    if (strcmp(part, ".") == 0 || part[0] == '\0') {
      continue;
    }
    if (strcmp(part, "..") == 0) {
      if (comps.len > 0 && strcmp(comps.items[comps.len - 1], "..") != 0) {
        free(comps.items[--comps.len]);
      } else if (!absolute) {
        list_push(&comps, xstrdup(".."));
      }
      continue;
    }
    list_push(&comps, xstrdup(part));
  }
  sb_init(&buf);
  if (absolute) {
    sb_append_char(&buf, '/');
  }
  for (size_t i = 0; i < comps.len; i++) {
    if (i > 0) {
      sb_append_char(&buf, '/');
    }
    sb_append(&buf, comps.items[i]);
  }
  if (!absolute && comps.len == 0) {
    sb_append_char(&buf, '.');
  }
  result = sb_take(&buf);
  list_free(&comps);
  free(tmp);
  return result;
}

static char *absolute_path_dup(const char *path) {
  if (path[0] == '/') {
    return normalize_path_dup(path);
  }
  {
    char cwd[4096];
    char *joined;
    if (!getcwd(cwd, sizeof(cwd))) {
      return normalize_path_dup(path);
    }
    joined = join_path(cwd, path);
    {
      char *norm = normalize_path_dup(joined);
      free(joined);
      return norm;
    }
  }
}

static char *strip_module_form(const char *src) {
  const char *start = strstr(src, "(module");
  const char *marker = "; [module form stripped by fl-build]\n";
  const char *suffix;
  char *out;
  size_t prefix_len;
  size_t marker_len;
  size_t suffix_len;
  int depth = 0;
  size_t i;
  size_t end = 0;
  if (!start) {
    return xstrdup(src);
  }
  for (i = (size_t)(start - src); src[i]; i++) {
    if (src[i] == '(') {
      depth++;
    } else if (src[i] == ')') {
      depth--;
      if (depth == 0) {
        end = i;
        break;
      }
    }
  }
  if (depth != 0) {
    return xstrdup(src);
  }
  prefix_len = (size_t)(start - src);
  suffix = src + end + 1;
  marker_len = strlen(marker);
  suffix_len = strlen(suffix);
  out = (char *)xmalloc(prefix_len + marker_len + suffix_len + 1);
  memcpy(out, src, prefix_len);
  memcpy(out + prefix_len, marker, marker_len);
  memcpy(out + prefix_len + marker_len, suffix, suffix_len + 1);
  return out;
}

static int is_debug_line(const char *line, size_t len) {
  size_t i = 0;
  while (i < len && isspace((unsigned char)line[i])) {
    i++;
  }
  if (i + 8 > len || strncmp(line + i, "(println", 8) != 0) {
    return 0;
  }
  i += 8;
  while (i < len && isspace((unsigned char)line[i])) {
    i++;
  }
  if (i < len && line[i] == '"') {
    i++;
  }
  return i + 7 <= len && strncmp(line + i, "[DEBUG]", 7) == 0;
}

static int parse_load_path(const char *line, size_t len, char **out_path) {
  size_t i = 0;
  char quote;
  size_t start;
  while (i < len && isspace((unsigned char)line[i])) {
    i++;
  }
  if (i + 5 > len || strncmp(line + i, "(load", 5) != 0) {
    return 0;
  }
  i += 5;
  while (i < len && isspace((unsigned char)line[i])) {
    i++;
  }
  if (i >= len || (line[i] != '"' && line[i] != '\'')) {
    return 0;
  }
  quote = line[i++];
  start = i;
  while (i < len && line[i] != quote) {
    i++;
  }
  if (i >= len) {
    return 0;
  }
  *out_path = substr_dup(line, start, i - start);
  i++;
  while (i < len && isspace((unsigned char)line[i])) {
    i++;
  }
  if (i >= len || line[i] != ')') {
    free(*out_path);
    *out_path = NULL;
    return 0;
  }
  i++;
  while (i < len && isspace((unsigned char)line[i])) {
    i++;
  }
  if (i != len) {
    free(*out_path);
    *out_path = NULL;
    return 0;
  }
  return 1;
}

static char *inline_loads(const char *path, StrList *visited, StrList *stack, int is_root) {
  char *abs_path = absolute_path_dup(path);
  char *content;
  char *base_dir;
  char *working;
  char *save = NULL;
  char *line;
  StrBuf out;
  if (list_contains(stack, abs_path)) {
    fprintf(stderr, "[fl-build] 순환 의존 감지: %s\n", path);
    free(abs_path);
    return xstrdup("; [fl-build] CYCLE DETECTED");
  }
  if (list_contains(visited, abs_path)) {
    free(abs_path);
    return xstrdup("");
  }
  list_push(visited, xstrdup(abs_path));
  list_push(stack, xstrdup(abs_path));
  content = read_file_or_null(abs_path);
  if (!content) {
    fprintf(stderr, "; [fl-build] 경고: %s 읽기 실패\n", path);
    free(abs_path);
    list_pop(stack);
    return xstrdup("");
  }
  if (is_root) {
    char *tmp = strip_module_form(content);
    free(content);
    content = tmp;
  }
  base_dir = path_dirname_dup(abs_path);
  sb_init(&out);
  working = content;
  for (line = strtok_r(working, "\n", &save); line; line = strtok_r(NULL, "\n", &save)) {
    char *load_path = NULL;
    size_t len = strlen(line);
    if (is_debug_line(line, len)) {
      continue;
    }
    if (parse_load_path(line, len, &load_path)) {
      char *full_path = join_path(base_dir, load_path);
      char *nested;
      fprintf(stderr, "; [fl-build] 인라인: %s\n", full_path);
      sb_append(&out, "; --- inlined: ");
      sb_append(&out, full_path);
      sb_append(&out, " ---\n");
      nested = inline_loads(full_path, visited, stack, 0);
      sb_append(&out, nested);
      if (nested[0] != '\0' && nested[strlen(nested) - 1] != '\n') {
        sb_append_char(&out, '\n');
      }
      sb_append(&out, "; --- end inlined: ");
      sb_append(&out, full_path);
      sb_append(&out, " ---\n");
      free(nested);
      free(full_path);
      free(load_path);
    } else {
      sb_append(&out, line);
      sb_append_char(&out, '\n');
    }
  }
  free(base_dir);
  free(content);
  free(abs_path);
  list_pop(stack);
  return sb_take(&out);
}

static int line_number_at(const char *src, size_t pos) {
  int line = 1;
  for (size_t i = 0; i < pos && src[i]; i++) {
    if (src[i] == '\n') {
      line++;
    }
  }
  return line;
}

static void sb_append_preview(StrBuf *buf, const char *text, size_t limit) {
  size_t len = strlen(text);
  if (len <= limit) {
    sb_append(buf, text);
  } else {
    sb_append_n(buf, text, limit);
    sb_append(buf, "...");
  }
}

static size_t skip_ws_text(const char *src, size_t pos) {
  while (src[pos] && isspace((unsigned char)src[pos])) {
    pos++;
  }
  return pos;
}

static size_t skip_token(const char *src, size_t pos) {
  while (src[pos] && !isspace((unsigned char)src[pos]) && src[pos] != ')' && src[pos] != ']' && src[pos] != '[') {
    pos++;
  }
  return pos;
}

static int parse_string_literal(const char *src, size_t *pos, char **out) {
  StrBuf buf;
  char quote;
  if (src[*pos] != '"') {
    return 0;
  }
  quote = src[*pos];
  (*pos)++;
  sb_init(&buf);
  while (src[*pos]) {
    char ch = src[*pos];
    if (ch == '\\' && src[*pos + 1]) {
      sb_append_char(&buf, src[*pos + 1]);
      *pos += 2;
      continue;
    }
    if (ch == quote) {
      (*pos)++;
      *out = sb_take(&buf);
      return 1;
    }
    sb_append_char(&buf, ch);
    (*pos)++;
  }
  free(buf.data);
  return 0;
}

static int parse_bracket_block(const char *src, size_t *pos, char **out) {
  int depth = 0;
  size_t start = *pos;
  if (src[*pos] != '[') {
    return 0;
  }
  while (src[*pos]) {
    if (src[*pos] == '[') {
      depth++;
    } else if (src[*pos] == ']') {
      depth--;
      if (depth == 0) {
        (*pos)++;
        *out = substr_dup(src, start, *pos - start);
        return 1;
      }
    }
    (*pos)++;
  }
  return 0;
}

static int count_sql_args(const char *args) {
  int count = 0;
  size_t i = 0;
  while (args[i]) {
    if (args[i] == '$' && (isalpha((unsigned char)args[i + 1]) || args[i + 1] == '_')) {
      count++;
      i += 2;
      while (isalnum((unsigned char)args[i]) || args[i] == '_') {
        i++;
      }
      continue;
    }
    if (args[i] == '"') {
      i++;
      while (args[i] && args[i] != '"') {
        if (args[i] == '\\' && args[i + 1]) {
          i += 2;
        } else {
          i++;
        }
      }
      if (args[i] == '"') {
        i++;
      }
      count++;
      continue;
    }
    if (isdigit((unsigned char)args[i])) {
      count++;
      i++;
      while (isdigit((unsigned char)args[i])) {
        i++;
      }
      continue;
    }
    i++;
  }
  return count;
}

static int run_semantic(const char *path) {
  char *src = read_file_or_null(path);
  size_t pos = 0;
  int error_count = 0;
  int warn_count = 0;
  StrBuf errors;
  StrBuf warnings;
  if (!src) {
    fprintf(stderr, "failed to read %s\n", path);
    return 1;
  }
  sb_init(&errors);
  sb_init(&warnings);
  while (src[pos]) {
    if (strncmp(src + pos, "(fxb-sqlite-query-p", 19) == 0 || strncmp(src + pos, "(fxb-sqlite-exec-p", 18) == 0) {
      size_t cur = pos;
      char *sql = NULL;
      char *args = NULL;
      int q_count;
      int arg_count;
      cur = skip_token(src, cur);
      cur = skip_ws_text(src, cur);
      cur = skip_token(src, cur);
      cur = skip_ws_text(src, cur);
      if (parse_string_literal(src, &cur, &sql)) {
        cur = skip_ws_text(src, cur);
        if (parse_bracket_block(src, &cur, &args)) {
          q_count = 0;
          for (size_t i = 0; sql[i]; i++) {
            if (sql[i] == '?') {
              q_count++;
            }
          }
          arg_count = count_sql_args(args);
          if (q_count != arg_count) {
            int line_no = line_number_at(src, pos);
            char numbuf[160];
            snprintf(numbuf, sizeof(numbuf), "  📍 줄 %d: SQL ? 개수(%d) ≠ 인자 개수(%d)\n", line_no, q_count, arg_count);
            sb_append(&errors, numbuf);
            sb_append(&errors, "     SQL: ");
            sb_append_preview(&errors, sql, 60);
            sb_append(&errors, "\n     💡 배열 원소를 ");
            snprintf(numbuf, sizeof(numbuf), "%d", q_count);
            sb_append(&errors, numbuf);
            sb_append(&errors, "개로 맞추세요\n");
            error_count++;
          }
        }
      }
      free(sql);
      free(args);
    } else if (strncmp(src + pos, "(fxb-sqlite-query", 17) == 0 && src[pos + 17] != '-') {
      size_t cur = pos;
      char *sql = NULL;
      cur = skip_token(src, cur);
      cur = skip_ws_text(src, cur);
      cur = skip_token(src, cur);
      cur = skip_ws_text(src, cur);
      if (parse_string_literal(src, &cur, &sql) && strchr(sql, '?')) {
        char numbuf[160];
        int line_no = line_number_at(src, pos);
        snprintf(numbuf, sizeof(numbuf), "  ⚠️  줄 %d: fxb-sqlite-query 에 ? 포함 → SQL 인젝션 위험\n", line_no);
        sb_append(&warnings, numbuf);
        sb_append(&warnings, "     💡 (fxb-sqlite-query-p DB sql [param ...]) 로 교체하세요\n");
        warn_count++;
      }
      free(sql);
    } else if (strncmp(src + pos, "(or nil ", 8) == 0 || strncmp(src + pos, "(or nil\n", 8) == 0 || strncmp(src + pos, "(or nil\t", 8) == 0) {
      char numbuf[160];
      int line_no = line_number_at(src, pos);
      snprintf(numbuf, sizeof(numbuf), "  ⚠️  줄 %d: (or nil ...) — 첫 인자가 nil 리터럴\n", line_no);
      sb_append(&warnings, numbuf);
      sb_append(&warnings, "     💡 (or $var fallback) 패턴을 확인하세요\n");
      warn_count++;
    }
    pos++;
  }
  if (error_count > 0) {
    printf("❌ 시맨틱 오류 %d개:\n", error_count);
    fputs(errors.data ? errors.data : "", stdout);
  }
  if (warn_count > 0) {
    printf("⚠️  시맨틱 경고 %d개:\n", warn_count);
    fputs(warnings.data ? warnings.data : "", stdout);
  }
  if (error_count == 0 && warn_count == 0) {
    puts("   ✅ 시맨틱 OK");
  }
  free(errors.data);
  free(warnings.data);
  free(src);
  return error_count > 0 ? 1 : 0;
}

static int seen_contains(const SeenList *list, int line, const char *prefix) {
  size_t i;
  for (i = 0; i < list->len; i++) {
    if (list->items[i].line == line && strcmp(list->items[i].msg_prefix, prefix) == 0) {
      return 1;
    }
  }
  return 0;
}

static void seen_add(SeenList *list, int line, const char *prefix) {
  if (list->len == list->cap) {
    list->cap = list->cap ? list->cap * 2 : ARRAY_INIT_CAP;
    list->items = (SeenError *)xrealloc(list->items, list->cap * sizeof(SeenError));
  }
  list->items[list->len].line = line;
  list->items[list->len].msg_prefix = xstrdup(prefix);
  list->len++;
}

static void seen_free(SeenList *list) {
  size_t i;
  for (i = 0; i < list->len; i++) {
    free(list->items[i].msg_prefix);
  }
  free(list->items);
}

static char *extract_quoted_after(const char *msg, const char *needle) {
  const char *p = strstr(msg, needle);
  const char *start;
  const char *end;
  if (!p) {
    return NULL;
  }
  start = strchr(p, '\'');
  if (!start) {
    return NULL;
  }
  end = strchr(start + 1, '\'');
  if (!end) {
    return NULL;
  }
  return substr_dup(start + 1, 0, (size_t)(end - start - 1));
}

static void c_to_fl_name(const char *name, char *out, size_t out_size) {
  char tmp[256];
  size_t i;
  size_t j = 0;
  for (i = 0; name[i] && i + 1 < sizeof(tmp); i++) {
    tmp[i] = name[i] == '_' ? '-' : name[i];
  }
  tmp[i] = '\0';
  i = strncmp(tmp, "fl-", 3) == 0 ? 3 : 0;
  for (; tmp[i] && j + 1 < out_size; i++) {
    out[j++] = tmp[i];
  }
  out[j] = '\0';
}

static char *get_line_text(char **lines, size_t line_count, int line_no) {
  if (line_no < 1 || (size_t)line_no > line_count) {
    return NULL;
  }
  return lines[line_no - 1];
}

static char **split_lines(char *text, size_t *count) {
  StrList lines = {0};
  char *save = NULL;
  char *line = strtok_r(text, "\n", &save);
  while (line) {
    list_push(&lines, xstrdup(line));
    line = strtok_r(NULL, "\n", &save);
  }
  *count = lines.len;
  return lines.items;
}

static int run_syntax_map(const char *syntax_log_path, const char *fl_path) {
  char *syntax_log = read_file_or_null(syntax_log_path);
  char *fl_text = read_file_or_null(fl_path);
  char *fl_text_copy;
  char **lines;
  size_t line_count = 0;
  SeenList seen = {0};
  char *cursor;
  if (!syntax_log || !fl_text) {
    free(syntax_log);
    free(fl_text);
    return 1;
  }
  fl_text_copy = xstrdup(fl_text);
  lines = split_lines(fl_text_copy, &line_count);
  cursor = syntax_log;
  while (*cursor) {
    char *line_end = strchr(cursor, '\n');
    size_t len = line_end ? (size_t)(line_end - cursor) : strlen(cursor);
    char *line = substr_dup(cursor, 0, len);
    char *marker = strstr(line, "<fl>:");
    char *msg = strstr(line, ": error: ");
    if (marker && msg) {
      int fl_line = atoi(marker + 5);
      char *message = msg + 9;
      char prefix[61];
      char *fl_line_text;
      memset(prefix, 0, sizeof(prefix));
      strncpy(prefix, message, 60);
      if (!seen_contains(&seen, fl_line, prefix)) {
        printf("  📍 FL 줄 %d: %s\n", fl_line, message);
        fl_line_text = get_line_text(lines, line_count, fl_line);
        if (fl_line_text && fl_line_text[0]) {
          printf("     코드: %.120s\n", fl_line_text);
        }
        if (strstr(message, "called object") && strstr(message, "is not a function")) {
          char *name = extract_quoted_after(message, "called object");
          if (name) {
            char mapped[256];
            c_to_fl_name(name, mapped, sizeof(mapped));
            printf("     💡 %s 는 함수가 아닙니다 → (fxb-%s) 로 교체하세요\n", name, mapped);
            free(name);
          }
        }
        if (strstr(message, "too few arguments to function") || strstr(message, "too many arguments to function")) {
          char *name = extract_quoted_after(message, "arguments to function");
          if (name) {
            char mapped[256];
            c_to_fl_name(name, mapped, sizeof(mapped));
            printf("     💡 (%s) 인자 수 %s — 함수 정의 확인\n", mapped, strstr(message, "too few") ? "부족" : "초과");
            free(name);
          }
        }
        if (strstr(message, " undeclared")) {
          char *name = extract_quoted_after(message, "");
          if (name) {
            printf("     💡 %s 미선언 → runtime.h 확인 또는 (fxb-...) 패턴 사용\n", name);
            free(name);
          }
        }
        if (strstr(message, "incompatible type")) {
          puts("     💡 타입 불일치 — FLValue 필요 위치에 int/char* 전달 여부 확인");
          puts("        (없는 함수 호출 시 GCC가 int 반환으로 추론 → 이 에러 발생)");
        }
        putchar('\n');
        seen_add(&seen, fl_line, prefix);
      }
    }
    free(line);
    if (!line_end) {
      break;
    }
    cursor = line_end + 1;
  }
  for (size_t i = 0; i < line_count; i++) {
    free(lines[i]);
  }
  free(lines);
  seen_free(&seen);
  free(fl_text_copy);
  free(fl_text);
  free(syntax_log);
  return 0;
}

static int run_preprocess(const char *input, const char *output) {
  StrList visited = {0};
  StrList stack = {0};
  char *out = inline_loads(input, &visited, &stack, 1);
  int ok = write_file(output, out);
  if (ok) {
    fprintf(stderr, "[fl-build] 전처리 완료\n");
  }
  free(out);
  list_free(&visited);
  list_free(&stack);
  return ok ? 0 : 1;
}

int main(int argc, char **argv) {
  if (argc >= 2 && strcmp(argv[1], "--preprocess") == 0) {
    if (argc != 4) return 1;
    return run_preprocess(argv[2], argv[3]);
  }
  if (argc >= 2 && strcmp(argv[1], "--semantic") == 0) {
    if (argc != 3) return 1;
    return run_semantic(argv[2]);
  }
  if (argc >= 2 && strcmp(argv[1], "--syntax-map") == 0) {
    if (argc != 4) return 1;
    return run_syntax_map(argv[2], argv[3]);
  }
  fprintf(stderr, "usage: %s --preprocess <in> <out> | --semantic <file> | --syntax-map <gcc-log> <fl-file>\n", argv[0]);
  return 1;
}
