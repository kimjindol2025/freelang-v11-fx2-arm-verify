#define _POSIX_C_SOURCE 200809L
/**
 * fl-str-split.c — fl-str-split.py 대체 구현
 * - 900B 초과 문자열을 (str "...") 형태로 분할
 * - 이미 (str "...") 라인 형식은 건드리지 않음
 * - 입력 파일을 제자리(in-place)로 갱신
 */

#include <ctype.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define LIMIT 900

typedef struct {
  char *buf;
  size_t len;
  size_t cap;
} StringBuf;

static void sb_init(StringBuf *sb) {
  sb->buf = NULL;
  sb->len = 0;
  sb->cap = 0;
}

static void sb_free(StringBuf *sb) {
  free(sb->buf);
  sb_init(sb);
}

static void sb_reserve(StringBuf *sb, size_t need) {
  if (sb->len + need + 1 <= sb->cap) return;
  size_t next = sb->cap ? sb->cap : 128;
  while (next < sb->len + need + 1) {
    next *= 2;
  }
  sb->buf = (char *)realloc(sb->buf, next);
  sb->cap = next;
}

static void sb_push(StringBuf *sb, char c) {
  sb_reserve(sb, 1);
  sb->buf[sb->len++] = c;
  sb->buf[sb->len] = '\0';
}

static void sb_append(StringBuf *sb, const char *s) {
  if (!s) return;
  size_t n = strlen(s);
  sb_reserve(sb, n);
  memcpy(sb->buf + sb->len, s, n);
  sb->len += n;
  sb->buf[sb->len] = '\0';
}

static void sb_append_n(StringBuf *sb, const char *s, size_t n) {
  if (!s || n == 0) return;
  sb_reserve(sb, n);
  memcpy(sb->buf + sb->len, s, n);
  sb->len += n;
  sb->buf[sb->len] = '\0';
}

static int is_existing_str_line(const char *line) {
  const unsigned char *p = (const unsigned char *)line;
  while (*p && isspace(*p)) p++;
  if (*p != '(') return 0;
  p++;
  while (*p && isspace(*p)) p++;
  if (strncmp((const char *)p, "str", 3) != 0) return 0;
  p += 3;
  if (!*p || !isspace(*p)) return 0;
  while (*p && isspace(*p)) p++;
  return *p == '"';
}

static void transform_line(const char *line, StringBuf *out, size_t *line_changed) {
  if (is_existing_str_line(line)) {
    sb_append(out, line);
    return;
  }

  size_t len = strlen(line);
  for (size_t i = 0; i < len;) {
    if (line[i] != '"') {
      sb_push(out, line[i]);
      i++;
      continue;
    }

    // 문자열 토큰 후보 파싱
    size_t j = i + 1;
    int closed = 0;
    StringBuf body;
    sb_init(&body);
    while (j < len) {
      if (line[j] == '\\' && j + 1 < len) {
        sb_push(&body, line[j]);
        sb_push(&body, line[j + 1]);
        j += 2;
        continue;
      }
      if (line[j] == '"') {
        j++;
        closed = 1;
        break;
      }
      sb_push(&body, line[j]);
      j++;
    }


    if (!closed) {
      sb_append_n(out, line + i, len - i);
      sb_free(&body);
      return;
    }

    if (body.len > LIMIT) {
      *line_changed = 1;
      sb_append(out, "(str ");
      for (size_t pos = 0; pos < body.len;) {
        size_t part_len = body.len - pos;
        if (part_len > LIMIT) part_len = LIMIT;
        sb_push(out, '"');
        sb_append_n(out, body.buf + pos, part_len);
        sb_push(out, '"');
        pos += part_len;
        if (pos < body.len) sb_push(out, ' ');
      }
      sb_append(out, ")");
    } else {
      sb_push(out, '"');
      sb_append_n(out, body.buf, body.len);
      sb_push(out, '"');
    }
    sb_free(&body);
    i = j;
  }
}

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "usage: fl-str-split <file>\n");
    return 1;
  }

  const char *path = argv[1];
  FILE *in = fopen(path, "r");
  if (!in) return 1;

  char tmp_path[PATH_MAX];
  if (snprintf(tmp_path, sizeof(tmp_path), "%s.XXXXXX", path) >= (int)sizeof(tmp_path)) {
    fclose(in);
    return 1;
  }

  int tmp_fd = mkstemp(tmp_path);
  if (tmp_fd < 0) {
    fclose(in);
    return 1;
  }
  FILE *out = fdopen(tmp_fd, "w");
  if (!out) {
    close(tmp_fd);
    fclose(in);
    return 1;
  }

  char *line = NULL;
  size_t cap = 0;
  ssize_t n = 0;
  size_t split_lines = 0;
  int last_line_had_newline = 0;
  while ((n = getline(&line, &cap, in)) != -1) {
    int has_newline = (n > 0 && line[n - 1] == '\n');
    if (has_newline) {
      line[n - 1] = '\0';
    }

    size_t changed = 0;
    StringBuf out_line;
    sb_init(&out_line);
    transform_line(line, &out_line, &changed);
    if (changed) split_lines++;
    fwrite(out_line.buf, 1, out_line.len, out);
    if (has_newline) {
      fputc('\n', out);
      last_line_had_newline = 1;
    } else {
      last_line_had_newline = 0;
    }
    sb_free(&out_line);
  }
  free(line);

  if (last_line_had_newline) {
    if (fseek(out, -1, SEEK_END) == 0) {
      off_t out_pos = ftell(out);
      if (out_pos < 0 || ftruncate(fileno(out), out_pos) != 0) {
        remove(tmp_path);
        return 1;
      }
    }
  }

  fclose(in);
  fclose(out);

  if (rename(tmp_path, path) != 0) {
    remove(tmp_path);
    return 1;
  }

  if (split_lines) {
    fprintf(stderr, "[fl-build] 긴 문자열 %zu개 자동 분할 (>900B)\n", split_lines);
  }
  return 0;
}
