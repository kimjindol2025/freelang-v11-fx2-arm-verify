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

typedef struct {
  const char *s;
  size_t len;
  size_t pos;
} Parser;

static char *xstrdup(const char *s) {
  if (!s) {
    char *empty = (char *)malloc(1);
    if (!empty) return NULL;
    empty[0] = '\0';
    return empty;
  }

  size_t len = strlen(s) + 1;
  char *copy = (char *)malloc(len);
  if (!copy) return NULL;
  memcpy(copy, s, len);
  return copy;
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

static void metadata_list_init(MetadataList *list) {
  list->items = NULL;
  list->len = 0;
  list->cap = 0;
}

static void metadata_list_free(MetadataList *list) {
  if (!list) return;
  for (size_t i = 0; i < list->len; i++) metadata_free(&list->items[i]);
  free(list->items);
  list->items = NULL;
  list->len = 0;
  list->cap = 0;
}

static int metadata_list_append(MetadataList *list, const PackageMetadata *meta) {
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
  copy.skip_runtime_shim_set = meta->skip_runtime_shim_set;
  copy.package_name = meta->package_name ? xstrdup(meta->package_name) : xstrdup("");
  copy.declared_name = meta->declared_name ? xstrdup(meta->declared_name) : NULL;
  copy.body = meta->body ? xstrdup(meta->body) : xstrdup("");
  if ((meta->package_name && !copy.package_name) ||
      (meta->declared_name && !copy.declared_name) ||
      (meta->body && !copy.body)) {
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
      *out = buf ? buf : xstrdup("");
      return *out ? 1 : 0;
    }

    if (c == '\\') {
      if (p->pos >= p->len) {
        free(buf);
        return 0;
      }
      char e = p->s[p->pos++];
      switch (e) {
        case '\"':
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

static int parser_skip_object(Parser *p);
static int parser_skip_array(Parser *p);

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
    free(key);

    if (!parser_expect(p, ':')) return 0;
    if (!parser_skip_value(p)) return 0;

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
    const char *lit = (c == 't') ? "true" : ((c == 'f') ? "false" : "null");
    size_t l = strlen(lit);
    if (p->pos + l > p->len) return 0;
    if (strncmp(p->s + p->pos, lit, l) != 0) return 0;
    p->pos += l;
    return 1;
  }

  if (c == '-' || isdigit((unsigned char)c)) {
    p->pos++;
    while (p->pos < p->len && isdigit((unsigned char)p->s[p->pos])) p->pos++;
    if (p->pos < p->len && (p->s[p->pos] == '.' || p->s[p->pos] == 'e' || p->s[p->pos] == 'E')) return 0;
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
    value = value * 10 + d;
    p->pos++;
  }

  if (p->pos < p->len && (p->s[p->pos] == '.' || p->s[p->pos] == 'e' || p->s[p->pos] == 'E')) return 0;

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
    } else if (strcmp(key, "package") == 0) {
      char *pkg = NULL;
      if (parser_parse_string(p, &pkg)) {
        free(meta->package_name);
        meta->package_name = pkg;
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

static int parser_parse_metadata_list(const char *json, MetadataList *metas) {
  Parser parser = {json, strlen(json), 0};

  if (!parser_expect(&parser, '[')) return 0;
  parser_skip_ws(&parser);

  if (parser_check(&parser, ']')) {
    parser.pos++;
    return 1;
  }

  while (parser.pos < parser.len) {
    PackageMetadata meta;
    metadata_init(&meta);

    if (!parser_parse_metadata_object(&parser, &meta)) {
      metadata_free(&meta);
      metadata_list_free(metas);
      return 0;
    }

    int emit = meta.package_name ? 1 : 0;

    if (emit) {
      if (!meta.declared_name) {
        meta.declared_name = xstrdup(meta.package_name);
        if (!meta.declared_name) {
          metadata_free(&meta);
          metadata_list_free(metas);
          return 0;
        }
      }
      if (!meta.body) {
        meta.body = xstrdup("");
        if (!meta.body) {
          metadata_free(&meta);
          metadata_list_free(metas);
          return 0;
        }
      }
      if (!meta.args_set) meta.args = 1;

      if (!metadata_list_append(metas, &meta)) {
        metadata_free(&meta);
        metadata_list_free(metas);
        return 0;
      }
    }

    metadata_free(&meta);

    parser_skip_ws(&parser);
    if (parser_check(&parser, ',') ) {
      parser.pos++;
      continue;
    }
    break;
  }


  if (!parser_expect(&parser, ']')) return 0;
  parser_skip_ws(&parser);
  if (parser.pos != parser.len) return 0;
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

static char *trim_whitespace_copy(const char *s) {
  if (!s) return xstrdup("");

  const char *start = s;
  while (*start && isspace((unsigned char)*start)) start++;

  const char *end = s + strlen(s);
  while (end > start && isspace((unsigned char)*(end - 1))) end--;

  size_t len = (size_t)(end - start);
  char *out = (char *)malloc(len + 1);
  if (!out) return NULL;

  memcpy(out, start, len);
  out[len] = '\0';
  return out;
}

static char *normalize_name_copy(const char *name) {
  char *out = xstrdup(name ? name : "");
  if (!out) return NULL;

  for (char *p = out; *p; p++) {
    if (*p == '-') *p = '_';
  }
  return out;
}

static int emit_user_fns(const char *script_dir, const MetadataList *metas) {
  size_t path_len = strlen(script_dir) + strlen("/runtime/user-fns.c") + 1;
  char *path = (char *)malloc(path_len);
  if (!path) return 0;
  if (snprintf(path, path_len, "%s/runtime/user-fns.c", script_dir) >= (int)path_len) {
    free(path);
    return 0;
  }

  FILE *out = fopen(path, "w");
  free(path);
  if (!out) return 0;

  fputs("/**\n", out);
  fputs(" * freelang-v11-fx2 — 사용자 패키지 함수\n", out);
  fputs(" * fl-resolve-deps.py가 module :use 선언에서 자동 생성합니다.\n", out);
  fputs(" * 직접 수정하지 마세요.\n", out);
  fputs(" */\n", out);
  fputs("#include \"runtime.h\"\n", out);
  fputs("#include <string.h>\n", out);
  fputs("#include <stdlib.h>\n", out);
  fputs("#include <stdio.h>\n", out);
  fputs("#include <math.h>\n\n\n", out);

  fputs("/* FL:USER_SECTION:BEGIN */\n", out);
  for (size_t i = 0; i < metas->len; i++) {
    const PackageMetadata *meta = &metas->items[i];
    const char *name = meta->declared_name ? meta->declared_name : meta->package_name;
    char *trimmed_body = trim_whitespace_copy(meta->body ? meta->body : "");
    if (!trimmed_body) {
      fclose(out);
      return 0;
    }

    fprintf(out, "/* FL:FN:%s */\n", name);
    fputs(trimmed_body, out);
    fputs("\n", out);
    fputs("/* FL:FN_END */\n", out);
    fputs("\n", out);
    free(trimmed_body);
  }
  fputs("/* FL:USER_SECTION:END */\n\n", out);

  fputs("/* FL:SHIM_SECTION:BEGIN */\n", out);
  for (size_t i = 0; i < metas->len; i++) {
    const PackageMetadata *meta = &metas->items[i];
    if (meta->skip_runtime_shim) continue;

    const char *name = meta->declared_name ? meta->declared_name : meta->package_name;
    char *shim = normalize_name_copy(name);
    if (!shim) {
      fclose(out);
      return 0;
    }

    size_t cname_len = strlen(shim) + 5;
    char *c_name = (char *)malloc(cname_len);
    if (!c_name) {
      free(shim);
      fclose(out);
      return 0;
    }
    snprintf(c_name, cname_len, "ufl_%s", shim);

    long long arity = meta->args_set ? meta->args : 1;
    if (arity < 0) arity = 0;

    fprintf(out, "/* FL:SHIM_FN:%s */\n", name);
    if (arity == 0) {
      fprintf(out, "FLValue %s(void) { return %s(); }\n", shim, c_name);
    } else {
      fprintf(out, "FLValue %s(", shim);
      for (long long a = 0; a < arity; a++) {
        if (a > 0) fputs(", ", out);
        fprintf(out, "FLValue a%lld", a);
      }
      fprintf(out, ") { return %s(", c_name);
      for (long long a = 0; a < arity; a++) {
        if (a > 0) fputs(", ", out);
        fprintf(out, "a%lld", a);
      }
      fprintf(out, "); }\n");
    }
    fputs("/* FL:SHIM_FN_END */\n", out);
    free(shim);
    free(c_name);
  }
  fputs("/* FL:SHIM_SECTION:END */\n\n", out);

  if (fclose(out) != 0) return 0;
  return 1;
}

static void dump_missing_msg(const MetadataList *metas) {
  (void)metas;
}

int main(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr, "사용법: fl-generate-user-fns <fx2_dir> <metadata-json>\n");
    return 1;
  }

  const char *script_dir = argv[1];
  const char *metadata_path = argv[2];

  char *json = NULL;
  if (!read_file(metadata_path, &json)) {
    fprintf(stderr, "  ⚠️  메타데이터 읽기 실패: %s\n", metadata_path);
    return 1;
  }

  MetadataList metas;
  metadata_list_init(&metas);
  if (!parser_parse_metadata_list(json, &metas)) {
    fprintf(stderr, "  ⚠️  메타데이터 JSON 파싱 실패: %s\n", metadata_path);
    free(json);
    metadata_list_free(&metas);
    return 1;
  }
  free(json);

  if (!emit_user_fns(script_dir, &metas)) {
    fprintf(stderr, "  ⚠️  user-fns.c 생성 실패\n");
    metadata_list_free(&metas);
    return 1;
  }

  printf("  → user-fns.c 생성 (%zu개 함수)\n", metas.len);

  metadata_list_free(&metas);
  dump_missing_msg(&metas);
  return 0;
}
