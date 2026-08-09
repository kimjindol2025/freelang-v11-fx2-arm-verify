#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *xstrdup(const char *s) {
  size_t n = strlen(s);
  char *p = (char *)malloc(n + 1);
  if (!p) return NULL;
  memcpy(p, s, n + 1);
  return p;
}

#define LIMIT_TAG_BUFFER 256

static int is_ident_start(int ch) {
  return isalnum(ch) || ch == '_';
}

static int is_ident_char(int ch) {
  return is_ident_start(ch) || ch == '-';
}

static char *read_text_file(const char *path, size_t *out_len) {
  FILE *in = fopen(path, "rb");
  if (!in) {
    return NULL;
  }

  if (fseek(in, 0, SEEK_END) != 0) {
    fclose(in);
    return NULL;
  }

  long size = ftell(in);
  if (size < 0) {
    fclose(in);
    return NULL;
  }

  rewind(in);

  char *buf = (char *)malloc((size_t)size + 1);
  if (!buf) {
    fclose(in);
    return NULL;
  }

  size_t read_bytes = fread(buf, 1, (size_t)size, in);
  fclose(in);

  if (read_bytes != (size_t)size) {
    free(buf);
    return NULL;
  }

  buf[size] = '\0';
  if (out_len) *out_len = (size_t)size;
  return buf;
}

static char *strip_comment_lines(const char *src, size_t len, size_t *out_len) {
  char *buf = (char *)malloc(len + 1);
  if (!buf) return NULL;

  size_t o = 0;
  for (size_t i = 0; i < len;) {
    size_t line_start = i;
    while (i < len && src[i] != '\n') {
      i++;
    }

    size_t line_len = i - line_start;

    size_t p = line_start;
    while (p < i && isspace((unsigned char)src[p])) {
      p++;
    }

    if (!(p < i && src[p] == ';')) {
      memcpy(buf + o, src + line_start, line_len);
      o += line_len;
      if (i < len) {
        buf[o++] = '\n';
      }
    }

    if (i < len) {
      i++;
    }
  }

  buf[o] = '\0';
  if (out_len) *out_len = o;
  return buf;
}

static char *substring_dup(const char *src, size_t start, size_t end) {
  if (end < start) return NULL;
  size_t len = end - start;
  char *out = (char *)malloc(len + 1);
  if (!out) return NULL;
  memcpy(out, src + start, len);
  out[len] = '\0';
  return out;
}

static void json_escape_append(const char *input, char **out, size_t *len, size_t *cap) {
  for (size_t i = 0; input[i] != '\0'; i++) {
    char c = input[i];
    switch (c) {
      case '\\':
      case '"':
      case '/':
      case '\b':
      case '\f':
      case '\n':
      case '\r':
      case '\t': {
        if (*len + 3 > *cap) {
          size_t next = *cap ? *cap : 16;
          while (next <= *len + 3) next *= 2;
          char *new_out = (char *)realloc(*out, next);
          if (!new_out) return;
          *out = new_out;
          *cap = next;
        }

        (*out)[(*len)++] = '\\';
        (*out)[*len] = '\0';

        switch (c) {
          case '\\':
            (*out)[(*len)++] = '\\';
            break;
          case '"':
            (*out)[(*len)++] = '"';
            break;
          case '/':
            (*out)[(*len)++] = '/';
            break;
          case '\b':
            (*out)[(*len)++] = 'b';
            break;
          case '\f':
            (*out)[(*len)++] = 'f';
            break;
          case '\n':
            (*out)[(*len)++] = 'n';
            break;
          case '\r':
            (*out)[(*len)++] = 'r';
            break;
          case '\t':
            (*out)[(*len)++] = 't';
            break;
        }
        (*out)[*len] = '\0';
        break;
      }
      default: {
        if (*len + 2 > *cap) {
          size_t next = *cap ? *cap : 16;
          while (next <= *len + 1) next *= 2;
          char *new_out = (char *)realloc(*out, next);
          if (!new_out) return;
          *out = new_out;
          *cap = next;
        }
        (*out)[(*len)++] = c;
        (*out)[*len] = '\0';
        break;
      }
    }
  }
}

static char *json_escape_alloc(const char *input) {
  size_t len = 0;
  size_t cap = 16;
  char *out = (char *)malloc(cap);
  if (!out) return NULL;
  out[0] = '\0';
  json_escape_append(input, &out, &len, &cap);
  return out;
}

static int find_substr_in_range(const char *src, size_t start, size_t end, const char *needle, size_t *found) {
  size_t nlen = strlen(needle);
  if (start >= end || nlen == 0 || end - start < nlen) return 0;

  for (size_t i = start; i + nlen <= end; i++) {
    if (strncmp(src + i, needle, nlen) == 0) {
      *found = i;
      return 1;
    }
  }
  return 0;
}

int main(int argc, char **argv) {
  if (argc < 2) {
    return 1;
  }

  size_t src_len = 0;
  char *src = read_text_file(argv[1], &src_len);
  if (!src) {
    fprintf(stderr, "Error: %s\n", strerror(errno));
    return 1;
  }

  size_t cleaned_len = 0;
  char *cleaned = strip_comment_lines(src, src_len, &cleaned_len);
  free(src);
  if (!cleaned) {
    return 1;
  }

  const char *module_kw = "(module";
  char *module_pos = strstr(cleaned, module_kw);
  if (!module_pos) {
    free(cleaned);
    return 1;
  }

  size_t start = (size_t)(module_pos - cleaned);
  size_t depth = 0;
  size_t end = start;
  for (size_t i = start; i < cleaned_len; i++) {
    if (cleaned[i] == '(') {
      depth++;
    } else if (cleaned[i] == ')') {
      if (depth == 0) continue;
      depth--;
      if (depth == 0) {
        end = i + 1;
        break;
      }
    }
  }

  if (end <= start) {
    free(cleaned);
    return 1;
  }

  size_t name_pos = start + strlen(module_kw);
  while (name_pos < end && isspace((unsigned char)cleaned[name_pos])) {
    name_pos++;
  }

  if (name_pos >= end) {
    free(cleaned);
    return 1;
  }

  size_t name_end = name_pos;
  while (name_end < end && !isspace((unsigned char)cleaned[name_end])) {
    name_end++;
  }

  if (name_pos >= name_end) {
    free(cleaned);
    return 1;
  }

  char *name = substring_dup(cleaned, name_pos, name_end);
  if (!name) {
    free(cleaned);
    return 1;
  }

  char *version = xstrdup("1.0.0");
  if (!version) {
    free(name);
    free(cleaned);
    return 1;
  }

  size_t version_pos = 0;
  if (find_substr_in_range(cleaned, start, end, ":version", &version_pos)) {
    size_t cursor = version_pos + 8;
    while (cursor < end && isspace((unsigned char)cleaned[cursor])) {
      cursor++;
    }

    if (cursor < end && cleaned[cursor] == '"') {
      size_t v_end = cursor + 1;
      while (v_end < end && cleaned[v_end] != '"') {
        v_end++;
      }
      if (v_end < end && v_end > cursor + 1) {
        char *ver = substring_dup(cleaned, cursor + 1, v_end);
        if (ver) {
          free(version);
          version = ver;
        }
      }
    }
  }

  size_t use_cap = 128;
  size_t use_len = 0;
  char *use_json = (char *)malloc(use_cap);
  if (!use_json) {
    free(version);
    free(name);
    free(cleaned);
    return 1;
  }
  use_json[0] = '\0';

  size_t use_pos = 0;
  if (find_substr_in_range(cleaned, start, end, ":use", &use_pos)) {
    size_t brace = use_pos + 4;
    while (brace < end && isspace((unsigned char)cleaned[brace])) brace++;

    if (brace < end && cleaned[brace] == '[') {
      size_t open = brace + 1;
      size_t close = open;
      while (close < end && cleaned[close] != ']') close++;
      if (close < end) {
        for (size_t i = open; i < close; i++) {
          if (cleaned[i] != ':') continue;

          if (i + 1 >= close || !is_ident_start((unsigned char)cleaned[i + 1])) {
            continue;
          }

          size_t token_start = i + 1;
          size_t token_end = token_start;
          while (token_end < close && is_ident_char((unsigned char)cleaned[token_end])) {
            token_end++;
          }

          char profile[LIMIT_TAG_BUFFER];
          size_t profile_len = token_end - token_start;
          if (profile_len == 0 || profile_len >= LIMIT_TAG_BUFFER) {
            continue;
          }

          memcpy(profile, cleaned + token_start, profile_len);
          profile[profile_len] = '\0';

          size_t add = profile_len + 4;
          if (use_len + add + 1 >= use_cap) {
            size_t next = use_cap ? use_cap : 16;
            while (next < use_len + add + 2) next *= 2;
            char *tmp = (char *)realloc(use_json, next);
            if (!tmp) {
              free(use_json);
              free(version);
              free(name);
              free(cleaned);
              return 1;
            }
            use_json = tmp;
            use_cap = next;
          }

          if (use_len > 0) {
            use_json[use_len++] = ',';
          }
          use_json[use_len++] = '"';
          memcpy(use_json + use_len, profile, profile_len);
          use_len += profile_len;
          use_json[use_len++] = '"';
          use_json[use_len] = '\0';
        }
      }
    }
  }

  char *esc_name = json_escape_alloc(name);
  char *esc_version = json_escape_alloc(version);
  if (!esc_name || !esc_version) {
    free(use_json);
    free(version);
    free(name);
    free(cleaned);
    free(esc_name);
    free(esc_version);
    return 1;
  }

  size_t out_cap = snprintf(NULL, 0, "{\"name\":\"%s\",\"use\":[%s],\"version\":\"%s\"}", esc_name, use_json, esc_version) + 1;
  char *out = (char *)malloc(out_cap);
  if (!out) {
    free(use_json);
    free(esc_name);
    free(esc_version);
    free(version);
    free(name);
    free(cleaned);
    return 1;
  }

  snprintf(out, out_cap, "{\"name\":\"%s\",\"use\":[%s],\"version\":\"%s\"}", esc_name, use_json, esc_version);
  printf("%s\n", out);

  free(use_json);
  free(esc_name);
  free(esc_version);
  free(version);
  free(name);
  free(cleaned);
  free(out);

  return 0;
}
