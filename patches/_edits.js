#!/usr/bin/env node
const fs = require('fs');

const EDITS = {
  '2': [[
`      (let [[$body (cgc (get $args 2))]]`,
`      (let [[$bodies (slice $args 2 (length $args))]
            [$body (if (= (length $bodies) 1)
                     (cgc (get $bodies 0))
                     (cgc-do $bodies))]]`,
  ]],
  '3': [[
`    [(= (get $node :kind) "block")
     (cgc-collect-vars-loop (get-block-items $node) 0 $acc)]`,
`    [(and (= (get $node :kind) "block") (= (get $node :type) "Map"))
     (cgc-collect-vars-map-vals (get (get $node :fields) :items) 1 $acc)]
    [(= (get $node :kind) "block")
     (cgc-collect-vars-loop (get-block-items $node) 0 $acc)]`,
  ], [
`(defn cgc-collect-vars-loop [$args $i $acc]
  (loop [$i $i $acc $acc]
    (if (or (null? $args) (>= $i (length $args))) $acc
    (recur (+ $i 1) (cgc-collect-vars (get $args $i) $acc)))))`,
`(defn cgc-collect-vars-loop [$args $i $acc]
  (loop [$i $i $acc $acc]
    (if (or (null? $args) (>= $i (length $args))) $acc
    (recur (+ $i 1) (cgc-collect-vars (get $args $i) $acc)))))

;; 맵 리터럴: 키(짝수 idx) 건너뛰고 값(홀수 idx)만 변수 수집 (버그 #3)
(defn cgc-collect-vars-map-vals [$items $i $acc]
  (if (or (null? $items) (>= $i (length $items))) $acc
    (cgc-collect-vars-map-vals $items (+ $i 2)
      (cgc-collect-vars (get $items $i) $acc))))`,
  ]],
  '1': [[
`(defn cgc-func-block [$n]
  (let [[$name (c-name (get $n :name))]
      [$f (get $n :fields)]
      [$params-block (get $f :params)]
      [$body-node (get $f :body)]
      [$ps (cgc-params (get-block-items $params-block))]
      [$body (cgc $body-node)]]
  (str "FLValue " $name "(" $ps ") {\n    return " $body ";\n}")))`,
`(defn cgc-func-block [$n]
  (let [[$name (c-name (get $n :name))]
        [$f (get $n :fields)]
        [$params-block (get $f :params)]
        [$body-node (get $f :body)]
        [$param-items (get-block-items $params-block)]
        [$ps (cgc-params $param-items)]
        [$pnames (cgc-fn-param-names $param-items 0 [])]]
    (reset! outer-params-atom $pnames)
    (let [[$body (cgc $body-node)]]
      (str "FLValue " $name "(" $ps ") {\n    return " $body ";\n}"))))`,
  ]],
  '5': [[
`    (if (c-reserved? $raw) (str "fl_" $raw) $raw)))`,
`    (if (c-reserved? $raw) (str "fl_" $raw) $raw)))

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
        (str "fl_fn_new(__fl_op_" $sfx ", 0, NULL)")))))`,
  ], [
`    [(= (get $n :kind) "variable")
  (let [[$cn (c-name (get $n :name))]]
    (if (includes-item @known-defns-atom $cn)
      (str "fl_fn_new(__fl_wrap_" $cn ", 0, NULL)")
      $cn))]`,
`    [(= (get $n :kind) "variable")
  (let [[$nm (get $n :name)]
        [$op (cgc-op-value-ref $nm)]]
    (if (not (null? $op)) $op
      (let [[$cn (c-name $nm)]]
        (if (includes-item @known-defns-atom $cn)
          (str "fl_fn_new(__fl_wrap_" $cn ", 0, NULL)")
          $cn))))]`,
  ], [
`                        [true (let [[$cn (c-name $v)]]
                                (if (includes-item @known-defns-atom $cn)
                                  (str "fl_fn_new(__fl_wrap_" $cn ", 0, NULL)")
                                  $cn)))]`,
`                        [true (let [[$op (cgc-op-value-ref $v)]]
                                (if (not (null? $op)) $op
                                  (let [[$cn (c-name $v)]]
                                    (if (includes-item @known-defns-atom $cn)
                                      (str "fl_fn_new(__fl_wrap_" $cn ", 0, NULL)")
                                      $cn)))))]`,
  ],
],
};

const REPRO = {
  '2': ['(define _log (atom []))\n(defn multi [$x] (swap! _log push "a") (swap! _log push "b") $x)\n(multi 1)\n(println (length (deref _log)))\n', '2'],
  '3': ['(defn collide [$xs]\n  (reduce (fn [$st $i] (let [[$acc (get $st "acc")]] {"acc": (+ $acc $i)})) {"acc" 0} $xs))\n(println (get (collide (list 1 2 3)) "acc"))\n', '6'],
  '1': ['(defn tag-all [$p $xs] (reduce (fn [$a $x] (str $a $p $x)) "" $xs))\n(println (tag-all "n" (list 1 2)))\n', 'n1n2'],
  '5': ['(println (reduce + 0 (list 1 2 3 4)))\n', '10'],
};

function run(cmd, path, bug) {
  let src = fs.readFileSync(path, 'utf8');
  const edits = EDITS[bug];
  for (let i = 0; i < edits.length; i++) {
    const [oldText, newText] = edits[i];
    const matches = src.split(oldText).length - 1;
    if (matches !== 1) {
      process.stderr.write(`[edits] 버그 #${bug} edit#${i}: 앵커 매칭 ${matches}회 (1회 기대) — 중단.\n` +
        `  codegen-c.fl이 패치 문서와 다를 수 있음. patches/bug${bug}-*.patch.md 수동 적용 요망.\n`);
      return 3;
    }
    src = src.replace(oldText, newText);
  }
  if (cmd === 'apply') {
    fs.writeFileSync(path, src, 'utf8');
    process.stderr.write(`[edits] 버그 #${bug} 적용 완료 (${edits.length}개 편집).\n`);
  } else {
    process.stderr.write(`[edits] 버그 #${bug} 앵커 ${edits.length}개 모두 1회 매칭 OK.\n`);
  }
  return 0;
}

function main() {
  if (process.argv.length < 3) {
    process.stderr.write(`usage: ${process.argv[1]} check|apply <codegen-c.fl> <bug> | repro <bug> | expect <bug>\n`);
    return 2;
  }
  const cmd = process.argv[2];
  if (cmd === 'check' || cmd === 'apply') {
    const path = process.argv[3];
    const bug = process.argv[4];
    if (!EDITS[bug]) {
      process.stderr.write(`unknown bug ${bug}\n`);
      return 2;
    }
    return run(cmd, path, bug);
  }
  if (cmd === 'repro') {
    process.stdout.write(REPRO[process.argv[3]][0]);
    return 0;
  }
  if (cmd === 'expect') {
    process.stdout.write(REPRO[process.argv[3]][1]);
    return 0;
  }
  process.stderr.write(`usage: ${process.argv[1]} check|apply <codegen-c.fl> <bug> | repro <bug> | expect <bug>\n`);
  return 2;
}

process.exitCode = main();
