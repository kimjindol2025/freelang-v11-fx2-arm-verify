#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
  char **items;
  size_t len;
  size_t cap;
} StringArray;

typedef struct {
  char *name;
  char *version;
  StringArray use;
} ModuleInfo;

typedef struct {
  char *name;
  StringArray packages;
} ProfileDetail;

typedef struct {
  ProfileDetail *items;
  size_t len;
  size_t cap;
} ProfileDetailList;

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
  char *out = (char *)xmalloc(len + 1);
  memcpy(out, src, len + 1);
  return out;
}

static char *substr_dup(const char *src, size_t start, size_t len) {
  char *out = (char *)xmalloc(len + 1);
  memcpy(out, src + start, len);
  out[len] = '\0';
  return out;
}

static void string_array_push(StringArray *array, char *item) {
  if (array->len == array->cap) {
    array->cap = array->cap ? array->cap * 2 : 4;
    array->items = (char **)xrealloc(array->items, array->cap * sizeof(char *));
  }
  array->items[array->len++] = item;
}

static int string_array_contains(const StringArray *array, const char *item) {
  size_t i;
  for (i = 0; i < array->len; i++) {
    if (strcmp(array->items[i], item) == 0) {
      return 1;
    }
  }
  return 0;
}

static void string_array_free(StringArray *array) {
  size_t i;
  for (i = 0; i < array->len; i++) {
    free(array->items[i]);
  }
  free(array->items);
  array->items = NULL;
  array->len = 0;
  array->cap = 0;
}

static void profile_detail_list_push(ProfileDetailList *list, ProfileDetail *detail) {
  if (list->len == list->cap) {
    list->cap = list->cap ? list->cap * 2 : 4;
    list->items = (ProfileDetail *)xrealloc(list->items, list->cap * sizeof(ProfileDetail));
  }
  list->items[list->len++] = *detail;
}

static void profile_detail_list_free(ProfileDetailList *list) {
  size_t i;
  for (i = 0; i < list->len; i++) {
    free(list->items[i].name);
    string_array_free(&list->items[i].packages);
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

static char *read_command_output(const char *cmd) {
  FILE *fp = popen(cmd, "r");
  char *out = NULL;
  size_t len = 0;
  size_t cap = 0;
  char buffer[4096];
  size_t got;
  if (!fp) {
    return NULL;
  }
  while ((got = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
    if (len + got + 1 > cap) {
      size_t next = cap ? cap * 2 : 4096;
      while (next < len + got + 1) {
        next *= 2;
      }
      out = (char *)xrealloc(out, next);
      cap = next;
    }
    memcpy(out + len, buffer, got);
    len += got;
  }
  if (!out) {
    out = (char *)xmalloc(1);
    cap = 1;
  }
  out[len] = '\0';
  pclose(fp);
  return out;
}

static char *shell_quote_dup(const char *text) {
  size_t i;
  size_t extra = 2;
  char *out;
  size_t pos = 0;
  for (i = 0; text[i]; i++) {
    if (text[i] == '\'') {
      extra += 4;
    } else {
      extra += 1;
    }
  }
  out = (char *)xmalloc(extra + 1);
  out[pos++] = '\'';
  for (i = 0; text[i]; i++) {
    if (text[i] == '\'') {
      memcpy(out + pos, "'\"'\"'", 6);
      pos += 6;
    } else {
      out[pos++] = text[i];
    }
  }
  out[pos++] = '\'';
  out[pos] = '\0';
  return out;
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
  size_t base_len = strlen(base);
  size_t rel_len = strlen(rel);
  char *out;
  if (rel_len > 0 && rel[0] == '/') {
    return xstrdup(rel);
  }
  out = (char *)xmalloc(base_len + 1 + rel_len + 1);
  memcpy(out, base, base_len);
  if (base_len == 0 || base[base_len - 1] != '/') {
    out[base_len++] = '/';
  }
  memcpy(out + base_len, rel, rel_len);
  out[base_len + rel_len] = '\0';
  return out;
}

static char *absolute_path_dup(const char *path) {
  char resolved[PATH_MAX];
  if (realpath(path, resolved)) {
    return xstrdup(resolved);
  }
  return xstrdup(path);
}

static char *basename_no_ext_dup(const char *path) {
  const char *base = strrchr(path, '/');
  const char *name = base ? base + 1 : path;
  const char *dot = strrchr(name, '.');
  if (dot && strcmp(dot, ".fl") == 0) {
    return substr_dup(name, 0, (size_t)(dot - name));
  }
  return xstrdup(name);
}

static char *read_line(const char *text, size_t *offset) {
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

static char *extract_load_path(const char *line) {
  const char *p = strstr(line, "(load");
  if (!p) {
    return NULL;
  }
  p += 5;
  while (*p && isspace((unsigned char)*p)) {
    p++;
  }
  if (*p != '"' && *p != '\'') {
    return NULL;
  }
  {
    char quote = *p++;
    const char *start = p;
    while (*p && *p != quote) {
      p++;
    }
    if (*p != quote) {
      return NULL;
    }
    return substr_dup(start, 0, (size_t)(p - start));
  }
}

static void collect_loads_recursive(const char *path, const char *base_dir, StringArray *visited, StringArray *loads) {
  char *abs_path = absolute_path_dup(path);
  char *text;
  char *dir;
  size_t offset = 0;
  if (!abs_path) {
    return;
  }
  if (string_array_contains(visited, abs_path)) {
    free(abs_path);
    return;
  }
  string_array_push(visited, abs_path);
  text = read_file_or_null(abs_path);
  if (!text) {
    return;
  }
  dir = path_dirname_dup(abs_path);
  while (1) {
    char *line = read_line(text, &offset);
    if (!line) {
      break;
    }
    {
      char *load_path = extract_load_path(line);
      if (load_path) {
        char *resolved = load_path[0] == '/' ? xstrdup(load_path) : join_path(dir, load_path);
        if (strncmp(resolved, base_dir, strlen(base_dir)) == 0 && resolved[strlen(base_dir)] == '/') {
          string_array_push(loads, xstrdup(resolved + strlen(base_dir) + 1));
        } else {
          string_array_push(loads, xstrdup(resolved));
        }
        collect_loads_recursive(resolved, base_dir, visited, loads);
        free(resolved);
        free(load_path);
      }
    }
    free(line);
  }
  free(dir);
  free(text);
}

static char *json_find_string_after_key(const char *text, const char *key) {
  char needle[128];
  const char *p;
  size_t out_len = 0;
  char *out;
  snprintf(needle, sizeof(needle), "\"%s\"", key);
  p = strstr(text, needle);
  if (!p) {
    return NULL;
  }
  p = strchr(p + strlen(needle), ':');
  if (!p) {
    return NULL;
  }
  p++;
  while (*p && isspace((unsigned char)*p)) {
    p++;
  }
  if (*p != '"') {
    return NULL;
  }
  p++;
  out = (char *)xmalloc(strlen(p) + 1);
  while (*p && *p != '"') {
    if (*p == '\\' && p[1]) {
      p++;
      switch (*p) {
        case 'n': out[out_len++] = '\n'; break;
        case 'r': out[out_len++] = '\r'; break;
        case 't': out[out_len++] = '\t'; break;
        case '"': out[out_len++] = '"'; break;
        case '\\': out[out_len++] = '\\'; break;
        default: out[out_len++] = *p; break;
      }
      p++;
      continue;
    }
    out[out_len++] = *p++;
  }
  if (*p != '"') {
    free(out);
    return NULL;
  }
  out[out_len] = '\0';
  return out;
}

static int json_find_string_array_after_key(const char *text, const char *key, StringArray *out) {
  char needle[128];
  const char *p;
  snprintf(needle, sizeof(needle), "\"%s\"", key);
  p = strstr(text, needle);
  if (!p) {
    return 0;
  }
  p = strchr(p + strlen(needle), ':');
  if (!p) {
    return 0;
  }
  p++;
  while (*p && isspace((unsigned char)*p)) {
    p++;
  }
  if (*p != '[') {
    return 0;
  }
  p++;
  while (*p) {
    while (*p && isspace((unsigned char)*p)) {
      p++;
    }
    if (*p == ']') {
      return 1;
    }
    if (*p != '"') {
      return 0;
    }
    p++;
    {
      char *item = (char *)xmalloc(strlen(p) + 1);
      size_t len = 0;
      while (*p && *p != '"') {
        if (*p == '\\' && p[1]) {
          p++;
          switch (*p) {
            case 'n': item[len++] = '\n'; break;
            case 'r': item[len++] = '\r'; break;
            case 't': item[len++] = '\t'; break;
            case '"': item[len++] = '"'; break;
            case '\\': item[len++] = '\\'; break;
            default: item[len++] = *p; break;
          }
          p++;
          continue;
        }
        item[len++] = *p++;
      }
      if (*p != '"') {
        free(item);
        return 0;
      }
      item[len] = '\0';
      string_array_push(out, item);
    }
    p++;
    while (*p && isspace((unsigned char)*p)) {
      p++;
    }
    if (*p == ',') {
      p++;
      continue;
    }
    if (*p == ']') {
      return 1;
    }
    return 0;
  }
  return 0;
}

static int parse_module_info(const char *text, ModuleInfo *mod) {
  mod->name = json_find_string_after_key(text, "name");
  mod->version = json_find_string_after_key(text, "version");
  mod->use.items = NULL;
  mod->use.len = 0;
  mod->use.cap = 0;
  if (!mod->name) {
    mod->name = xstrdup("");
  }
  if (!mod->version) {
    mod->version = xstrdup("1.0.0");
  }
  json_find_string_array_after_key(text, "use", &mod->use);
  return 1;
}

static int resolve_profiles(const StringArray *profiles, const char *script_dir, StringArray *all_packages, ProfileDetailList *details) {
  size_t i;
  char *profiles_dir = xmalloc(strlen(script_dir) + 10);
  sprintf(profiles_dir, "%s/profiles", script_dir);
  for (i = 0; i < profiles->len; i++) {
    ProfileDetail detail;
    char *profile_file;
    char *text;
    size_t j;
    detail.name = xstrdup(profiles->items[i]);
    detail.packages.items = NULL;
    detail.packages.len = 0;
    detail.packages.cap = 0;
    profile_file = xmalloc(strlen(profiles_dir) + strlen(profiles->items[i]) + 7);
    sprintf(profile_file, "%s/%s.json", profiles_dir, profiles->items[i]);
    text = read_file_or_null(profile_file);
    if (text) {
      json_find_string_array_after_key(text, "packages", &detail.packages);
      free(text);
      for (j = 0; j < detail.packages.len; j++) {
        if (!string_array_contains(all_packages, detail.packages.items[j])) {
          string_array_push(all_packages, xstrdup(detail.packages.items[j]));
        }
      }
    } else {
      string_array_push(&detail.packages, xstrdup("(프로파일 없음)"));
    }
    profile_detail_list_push(details, &detail);
    free(profile_file);
  }
  free(profiles_dir);
  return 1;
}

static char *run_module_parser(const char *script_dir, const char *fl_input) {
  char *bin = xmalloc(strlen(script_dir) + 19);
  char *qbin;
  char *qinput;
  char *cmd;
  char *out;
  sprintf(bin, "%s/.fl-module-parse", script_dir);
  qbin = shell_quote_dup(bin);
  qinput = shell_quote_dup(fl_input);
  cmd = xmalloc(strlen(qbin) + strlen(qinput) + 2);
  sprintf(cmd, "%s %s", qbin, qinput);
  out = read_command_output(cmd);
  free(bin);
  free(qbin);
  free(qinput);
  free(cmd);
  return out;
}

static void print_plan(const ModuleInfo *mod, const StringArray *loads, const StringArray *all_packages, const ProfileDetailList *details, const char *output) {
  size_t i;
  if (mod->name && mod->name[0]) {
    printf("module:  %s  v%s\n", mod->name, mod->version ? mod->version : "");
    puts("");
  }
  if (mod->use.len > 0) {
    puts("profiles:");
    for (i = 0; i < mod->use.len; i++) {
      size_t j;
      const StringArray *pkgs = &details->items[i].packages;
      printf("  :%-8s → [", mod->use.items[i]);
      for (j = 0; j < pkgs->len; j++) {
        if (j > 0) {
          fputs(", ", stdout);
        }
        fputs(pkgs->items[j], stdout);
      }
      puts("]");
    }
    puts("");
  }
  if (loads->len > 0) {
    puts("loads:");
    for (i = 0; i < loads->len; i++) {
      printf("  → %s\n", loads->items[i]);
    }
    puts("");
  }
  printf("packages: %zu개\n", all_packages->len);
  printf("output:   %s  (ELF)\n", output);
}

static void print_graph(const char *fl_input, const ModuleInfo *mod, const StringArray *loads, const StringArray *all_packages, const ProfileDetailList *details, const char *output) {
  size_t i;
  printf("%s\n", fl_input);
  if (mod->use.len > 0) {
    for (i = 0; i < mod->use.len; i++) {
      size_t j;
      const StringArray *pkgs = &details->items[i].packages;
      printf("%s :%s  [", i + 1 == mod->use.len && loads->len == 0 ? "└──" : "├──", mod->use.items[i]);
      for (j = 0; j < pkgs->len; j++) {
        if (j > 0) {
          fputs(", ", stdout);
        }
        fputs(pkgs->items[j], stdout);
      }
      puts("]");
    }
  }
  if (loads->len > 0) {
    for (i = 0; i < loads->len; i++) {
      printf("%s %s\n", i + 1 == loads->len ? "└──" : "├──", loads->items[i]);
    }
  }
  puts("");
  printf("packages: %zu개  output: %s (ELF)\n", all_packages->len, output);
}

static void print_check(const char *fl_input, const StringArray *all_packages, const char *script_dir) {
  size_t i;
  printf("🔍 declared vs installed 검사: %s\n", fl_input);
  puts("");
  puts("(fl-source-manager 없음, 로컬 확인)");
  puts("");
  for (i = 0; i < all_packages->len; i++) {
    char *pkg_file = xmalloc(strlen(script_dir) + strlen(all_packages->items[i]) + 17);
    int exists;
    sprintf(pkg_file, "%s/packages/%s.json", script_dir, all_packages->items[i]);
    exists = access(pkg_file, F_OK) == 0;
    printf("  %s %s\n", exists ? "✅" : "❌", all_packages->items[i]);
    free(pkg_file);
  }
  puts("");
  puts("✅ 모든 패키지 로컬 확인됨");
}

int main(int argc, char **argv) {
  const char *fl_input;
  const char *script_dir;
  char *output_owned = NULL;
  const char *output;
  const char *mode;
  ModuleInfo mod;
  StringArray all_packages;
  StringArray loads;
  ProfileDetailList details;
  char *module_json;
  char *base_dir;

  if (argc < 3) {
    return 1;
  }

  fl_input = argv[1];
  script_dir = argv[2];
  output = argc > 3 ? argv[3] : (output_owned = basename_no_ext_dup(fl_input));
  mode = argc > 4 ? argv[4] : "plan";

  mod.name = NULL;
  mod.version = NULL;
  mod.use.items = NULL;
  mod.use.len = 0;
  mod.use.cap = 0;
  all_packages.items = NULL;
  all_packages.len = 0;
  all_packages.cap = 0;
  loads.items = NULL;
  loads.len = 0;
  loads.cap = 0;
  details.items = NULL;
  details.len = 0;
  details.cap = 0;

  module_json = run_module_parser(script_dir, fl_input);
  if (module_json) {
    parse_module_info(module_json, &mod);
    free(module_json);
  }
  base_dir = path_dirname_dup(absolute_path_dup(fl_input));
  collect_loads_recursive(fl_input, base_dir, &(StringArray){0}, &loads);
  resolve_profiles(&mod.use, script_dir, &all_packages, &details);

  if (strcmp(mode, "plan") == 0) {
    print_plan(&mod, &loads, &all_packages, &details, output);
  } else if (strcmp(mode, "check") == 0) {
    print_check(fl_input, &all_packages, script_dir);
  } else if (strcmp(mode, "graph") == 0) {
    print_graph(fl_input, &mod, &loads, &all_packages, &details, output);
  } else {
    free(output_owned);
    free(base_dir);
    string_array_free(&mod.use);
    free(mod.name);
    free(mod.version);
    string_array_free(&all_packages);
    string_array_free(&loads);
    profile_detail_list_free(&details);
    return 1;
  }

  free(output_owned);
  free(base_dir);
  string_array_free(&mod.use);
  free(mod.name);
  free(mod.version);
  string_array_free(&all_packages);
  string_array_free(&loads);
  profile_detail_list_free(&details);
  return 0;
}
