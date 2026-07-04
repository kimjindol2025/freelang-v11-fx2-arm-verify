#!/usr/bin/env python3
"""fx2 컴파일러 패치 — 앵커 기반 정확 문자열 편집 (codegen-c.fl).

라인번호는 패치 누적 시 밀리므로 문자열 앵커로 매칭한다.
각 (old,new)는 파일에 정확히 1회만 매칭되어야 하며, 아니면 중단(파일 보존).

사용:
  python3 _edits.py check <codegen-c.fl> <bug>     # 앵커 매칭만 확인 (수정 X)
  python3 _edits.py apply <codegen-c.fl> <bug>     # 적용 (덮어쓰기)
  python3 _edits.py repro <bug>                     # 재현 .fl 소스를 stdout
  python3 _edits.py expect <bug>                    # 기대 출력값을 stdout
bug ∈ {2,3,1,5}
"""
import sys

# 주의: FL 소스의 "\n"은 파일에 '백슬래시+n' 2글자로 존재 → raw 문자열로 매칭.
EDITS = {
    # ── 버그 #2: cgc-defn 다중 본문 (implicit do) ──
    "2": [(
r"""      (let [[$body (cgc (get $args 2))]]""",
r"""      (let [[$bodies (slice $args 2 (length $args))]
            [$body (if (= (length $bodies) 1)
                     (cgc (get $bodies 0))
                     (cgc-do $bodies))]]""",
    )],

    # ── 버그 #3: cgc-collect-vars 맵 키 충돌 ──
    "3": [
        (
r"""    [(= (get $node :kind) "block")
     (cgc-collect-vars-loop (get-block-items $node) 0 $acc)]""",
r"""    [(and (= (get $node :kind) "block") (= (get $node :type) "Map"))
     (cgc-collect-vars-map-vals (get (get $node :fields) :items) 1 $acc)]
    [(= (get $node :kind) "block")
     (cgc-collect-vars-loop (get-block-items $node) 0 $acc)]""",
        ),
        (
r"""(defn cgc-collect-vars-loop [$args $i $acc]
  (loop [$i $i $acc $acc]
    (if (or (null? $args) (>= $i (length $args))) $acc
    (recur (+ $i 1) (cgc-collect-vars (get $args $i) $acc)))))""",
r"""(defn cgc-collect-vars-loop [$args $i $acc]
  (loop [$i $i $acc $acc]
    (if (or (null? $args) (>= $i (length $args))) $acc
    (recur (+ $i 1) (cgc-collect-vars (get $args $i) $acc)))))

;; 맵 리터럴: 키(짝수 idx) 건너뛰고 값(홀수 idx)만 변수 수집 (버그 #3)
(defn cgc-collect-vars-map-vals [$items $i $acc]
  (if (or (null? $items) (>= $i (length $items))) $acc
    (cgc-collect-vars-map-vals $items (+ $i 2)
      (cgc-collect-vars (get $items $i) $acc))))""",
        ),
    ],

    # ── 버그 #1: cgc-func-block outer-params 대칭화 ──
    "1": [(
r"""(defn cgc-func-block [$n]
  (let [[$name (c-name (get $n :name))]
      [$f (get $n :fields)]
      [$params-block (get $f :params)]
      [$body-node (get $f :body)]
      [$ps (cgc-params (get-block-items $params-block))]
      [$body (cgc $body-node)]]
  (str "FLValue " $name "(" $ps ") {\n    return " $body ";\n}")))""",
r"""(defn cgc-func-block [$n]
  (let [[$name (c-name (get $n :name))]
        [$f (get $n :fields)]
        [$params-block (get $f :params)]
        [$body-node (get $f :body)]
        [$param-items (get-block-items $params-block)]
        [$ps (cgc-params $param-items)]
        [$pnames (cgc-fn-param-names $param-items 0 [])]]
    (reset! outer-params-atom $pnames)
    (let [[$body (cgc $body-node)]]
      (str "FLValue " $name "(" $ps ") {\n    return " $body ";\n}"))))""",
    )],

    # ── 버그 #5: 연산자 first-class (합성 래퍼) ──
    "5": [
        # (a) c-name 뒤에 op 헬퍼 삽입
        (
r"""    (if (c-reserved? $raw) (str "fl_" $raw) $raw)))""",
r"""    (if (c-reserved? $raw) (str "fl_" $raw) $raw)))

;; ── 버그 #5: 값 위치 연산자 → 합성 함수값 ──
(define op-emitted-atom (atom []))
(defn op-spec [$name]
  (cond
    [(= $name "+")  {"sfx" "add" "fn" "fl_add" "kind" "chain" "id" "fl_int(0)"}]
    [(= $name "*")  {"sfx" "mul" "fn" "fl_mul" "kind" "chain" "id" "fl_int(1)"}]
    [(= $name "-")  {"sfx" "sub" "fn" "fl_sub" "kind" "chain" "id" "fl_int(0)"}]
    [(= $name "/")  {"sfx" "div" "fn" "fl_div" "kind" "bin"}]
    [(= $name "%")  {"sfx" "mod" "fn" "fl_mod" "kind" "bin"}]
    [(= $name "=")  {"sfx" "eq"  "fn" "fl_eq"  "kind" "bin"}]
    [(= $name "!=") {"sfx" "neq" "fn" "fl_neq" "kind" "bin"}]
    [(= $name "<")  {"sfx" "lt"  "fn" "fl_lt"  "kind" "bin"}]
    [(= $name ">")  {"sfx" "gt"  "fn" "fl_gt"  "kind" "bin"}]
    [(= $name "<=") {"sfx" "lte" "fn" "fl_lte" "kind" "bin"}]
    [(= $name ">=") {"sfx" "gte" "fn" "fl_gte" "kind" "bin"}]
    [true nil]))
(defn op-wrapper-c [$spec]
  (let [[$sfx (get $spec "sfx")] [$fn (get $spec "fn")] [$kind (get $spec "kind")]]
    (if (= $kind "chain")
      (str "static FLValue __fl_op_" $sfx "(FLClosure* _s, int _ac, FLValue* argv) {\n"
           "    (void)_s;\n"
           "    if (_ac == 0) return " (get $spec "id") ";\n"
           "    FLValue acc = argv[0];\n"
           "    for (int i = 1; i < _ac; i++) acc = " $fn "(acc, argv[i]);\n"
           "    return acc;\n}")
      (str "static FLValue __fl_op_" $sfx "(FLClosure* _s, int _ac, FLValue* argv) {\n"
           "    (void)_s; (void)_ac;\n"
           "    return " $fn "(argv[0], argv[1]);\n}"))))
(defn cgc-op-value-ref [$name]
  (let [[$spec (op-spec $name)]]
    (if (null? $spec) nil
      (let [[$sfx (get $spec "sfx")]]
        (if (not (includes-item @op-emitted-atom $sfx))
          (do (swap! op-emitted-atom push $sfx)
              (swap! wrapper-defs-atom push (op-wrapper-c $spec)))
          nil)
        (str "fl_fn_new(__fl_op_" $sfx ", 0, NULL)")))))""",
        ),
        # (b) cgc variable 분기
        (
r"""    [(= (get $n :kind) "variable")
      (let [[$cn (c-name (get $n :name))]]
        (if (includes-item @known-defns-atom $cn)
          (str "fl_fn_new(__fl_wrap_" $cn ", 0, NULL)")
          $cn))]""",
r"""    [(= (get $n :kind) "variable")
      (let [[$nm (get $n :name)]
            [$op (cgc-op-value-ref $nm)]]
        (if (not (null? $op)) $op
          (let [[$cn (c-name $nm)]]
            (if (includes-item @known-defns-atom $cn)
              (str "fl_fn_new(__fl_wrap_" $cn ", 0, NULL)")
              $cn))))]""",
        ),
        # (c) cgc-literal symbol 분기
        (
r"""                        [true (let [[$cn (c-name $v)]]
                                (if (includes-item @known-defns-atom $cn)
                                  (str "fl_fn_new(__fl_wrap_" $cn ", 0, NULL)")
                                  $cn))])]""",
r"""                        [true (let [[$op (cgc-op-value-ref $v)]]
                                (if (not (null? $op)) $op
                                  (let [[$cn (c-name $v)]]
                                    (if (includes-item @known-defns-atom $cn)
                                      (str "fl_fn_new(__fl_wrap_" $cn ", 0, NULL)")
                                      $cn))))])]""",
        ),
    ],
}

REPRO = {
    "2": (
'(define _log (atom []))\n'
'(defn multi [$x] (swap! _log push "a") (swap! _log push "b") $x)\n'
'(multi 1)\n'
'(println (length (deref _log)))\n', "2"),
    "3": (
'(defn collide [$xs]\n'
'  (reduce (fn [$st $i] (let [[$acc (get $st "acc")]] {"acc" (+ $acc $i)})) {"acc" 0} $xs))\n'
'(println (get (collide (list 1 2 3)) "acc"))\n', "6"),
    "1": (
'(defn tag-all [$p $xs] (reduce (fn [$a $x] (str $a $p $x)) "" $xs))\n'
'(println (tag-all "n" (list 1 2)))\n', "n1n2"),
    "5": (
'(println (reduce + 0 (list 1 2 3 4)))\n', "10"),
}


def run(node, path, bug):
    src = open(path, encoding="utf-8").read()
    edits = EDITS[bug]
    for i, (old, new) in enumerate(edits):
        n = src.count(old)
        if n != 1:
            sys.stderr.write(
                f"[edits] 버그 #{bug} edit#{i}: 앵커 매칭 {n}회 (1회 기대) — 중단.\n"
                f"  codegen-c.fl이 패치 문서와 다를 수 있음. patches/bug{bug}-*.patch.md 수동 적용 요망.\n")
            return 3
        src = src.replace(old, new, 1)
    if node == "apply":
        open(path, "w", encoding="utf-8").write(src)
        sys.stderr.write(f"[edits] 버그 #{bug} 적용 완료 ({len(edits)}개 편집).\n")
    else:
        sys.stderr.write(f"[edits] 버그 #{bug} 앵커 {len(edits)}개 모두 1회 매칭 OK.\n")
    return 0


def main():
    if len(sys.argv) < 2:
        sys.stderr.write(__doc__); return 2
    cmd = sys.argv[1]
    if cmd in ("check", "apply"):
        _, _, path, bug = sys.argv[:4]
        if bug not in EDITS:
            sys.stderr.write(f"unknown bug {bug}\n"); return 2
        return run(cmd, path, bug)
    if cmd == "repro":
        sys.stdout.write(REPRO[sys.argv[2]][0]); return 0
    if cmd == "expect":
        sys.stdout.write(REPRO[sys.argv[2]][1]); return 0
    sys.stderr.write(__doc__); return 2


if __name__ == "__main__":
    sys.exit(main())
