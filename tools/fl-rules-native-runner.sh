#!/usr/bin/env bash

# Shared native rules runner used by fl-verify.sh and fl-build.sh.
run_rules_native() {
  local repo_dir="$1"
  local rules_file="$2"
  local input_file="$3"
  local build_script="${4:-$repo_dir/fl-build.sh}"

  if [ -z "$repo_dir" ] || [ -z "$rules_file" ] || [ -z "$input_file" ]; then
    echo "[FAIL] run_rules_native: missing arguments" >&2
    return 1
  fi

  local tmpdir
  local native_rules
  local native_runner
  local native_bin
  local build_log
  local run_log
  local -a cleanup_args=()

  tmpdir="$(mktemp -d)"
  native_rules="$tmpdir/fx-rules-native.fl"
  native_runner="$tmpdir/fx-rules-native-runner.fl"
  native_bin="$tmpdir/fx-rules-native-bin"
  cleanup_args=("$tmpdir")

  sed -E 's/\bsome\b/_native_some/g' "$rules_file" > "$native_rules"

  cat > "$native_runner" <<'EOF'
(defn _native_some [$pred $vec]
  (if (nil? (first $vec))
    false
    (let [$x (first $vec)
          $rest (rest $vec)]
      (if ($pred $x)
        true
        (_native_some $pred $rest)))))

(define $src (file-read "__FL_VERIFY_INPUT__"))
(define $lines (str-split $src "\\n"))

(define $checks (atom (list)))
(define $has-error (atom false))

(defn check-error [$desc $fn]
  (swap! $checks append {"is_error" true "desc" $desc "fn" $fn}))

(defn check-warn [$desc $fn]
  (swap! $checks append {"is_error" false "desc" $desc "fn" $fn}))

(defn _emit-result [$check $result]
  (println (str (if (get $check "is_error") "error" "warn") ":" (get $check "desc") ":" (if (string? $result) $result "규칙 위반"))))

(defn _run-checks [$items]
  (if (nil? (first $items))
    nil
    (let [$check (first $items)
          $fn (get $check "fn")
          $result (if (nil? $fn) nil ($fn $src $lines))
          $is-error (get $check "is_error")]
      (if (not (nil? $result))
        (if $is-error (reset! $has-error true) nil)
        nil)
      (if (not (nil? $result))
        (_emit-result $check $result)
        nil)
      (_run-checks (rest $items)))))

EOF
  cat "$native_rules" >> "$native_runner"

  cat >> "$native_runner" <<'EOF'

(_run-checks (deref $checks))
(println (if (deref $has-error) "RULE_ERROR" "RULE_OK"))
EOF

  sed -i "s|__FL_VERIFY_INPUT__|$input_file|g" "$native_runner"

  if ! build_log=$(FL_SKIP_RULES=1 "$build_script" "$native_runner" "$native_bin" --no-net 2>&1); then
    echo "$build_log" >&2
    echo "[FAIL] native rules 빌드 실패" >&2
    rm -rf "${cleanup_args[@]}"
    return 1
  fi

  if ! run_log=$("$native_bin" 2>&1); then
    echo "$run_log" >&2
    echo "[FAIL] native rules 실행 실패" >&2
    rm -rf "${cleanup_args[@]}"
    return 1
  fi

  echo "$run_log"

  if printf '%s\n' "$run_log" | grep -Fxq "RULE_ERROR"; then
    rm -rf "${cleanup_args[@]}"
    return 1
  fi

  if printf '%s\n' "$run_log" | grep -Fxq "RULE_OK"; then
    rm -rf "${cleanup_args[@]}"
    return 0
  fi

  echo "[FAIL] canonical rules marker 미검증: RULE_OK/RULE_ERROR 둘 다 없음" >&2
  rm -rf "${cleanup_args[@]}"
  return 1
}
