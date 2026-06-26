# 패치: 버그 #5 — 연산자 first-class 참조 시 깨진 C 생성

**대상**: `/root/freelang-v11/self/codegen-c.fl`
**상태**: 소스 근원 확정 · 검증 대기 ([[bug2-defn-implicit-do]] 동일 블로커)
**난이도**: 4개 중 최고 (코드생성 + 합성 래퍼). 회귀 위험도 가장 큼.
**증상**(cgc-main.fl:5 기록): `(reduce + 0 …)` / `(apply * …)`처럼 연산자를
값으로 넘기면 `-`→`_`, `+`/`*`→그대로 → C에 깨진/모호한 토큰.

---

## 근원 — 값 위치 연산자가 변수 경로로 빠짐

`cgc`(60행)의 variable 분기와 `cgc-literal`(80행)의 symbol 분기:

```lisp
;; cgc 64-68행
[(= (get $n :kind) "variable")
  (let [[$cn (c-name (get $n :name))]]
    (if (includes-item @known-defns-atom $cn)
      (str "fl_fn_new(__fl_wrap_" $cn ", 0, NULL)")   ;; 사용자 defn → 함수값
      $cn))]                                           ;; 그 외 → bare 토큰
```

연산자 `+`는 `known-defns`에 없으므로 `$cn`(=`c-name "+"` → `"+"`)이 **bare로 방출**.
`-`는 `c-name`(53행 `"-"`→`"_"`)으로 `"_"`가 되어 C에 `_` 식별자 생성. 어느 쪽도
호출 가능한 함수값이 아니라 깨진 C.

핵심: 빌트인 연산자를 **값 위치**에서 만나면 호출 가능한 `FLValue`(closure)로
lowering하는 분기가 없다. (연산자 **호출** 위치는 `cgc-dispatch` 162행+에서
`fl_add` 등으로 정상 처리됨 — 값 위치만 누락.)

---

## 수정 — 합성 래퍼 + 값 위치 분기 (런타임 무수정)

기존 `wrapper-defs-atom`(13행, `__fl_wrap_` 사용자 래퍼 방출 경로)을 재사용해
`__fl_op_<name>` 합성 래퍼를 등록하면 `generate-c`(615행) 수정 불필요
(이미 `cgc-join-wrappers`로 자동 합쳐짐, $fns보다 앞).

### (1) 연산자 → (C접미사, 런타임함수, 종류) 테이블 + 헬퍼

```lisp
;; codegen-c.fl 상단(util 구역, c-name 근처)에 추가
(define op-emitted-atom (atom []))   ;; 중복 방출 방지(연산자별 1회)

;; 값으로 참조 가능한 빌트인 연산자 → {suffix, fn, kind}
;; kind: "chain"(가변 reduce) | "bin"(2항)
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

;; 합성 래퍼 C 본문 (closure 시그니처: FLValue (*)(FLClosure*, int, FLValue*))
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

;; 값 위치 연산자 → 함수값 표현식 (래퍼 1회 등록)
(defn cgc-op-value-ref [$name]
  (let [[$spec (op-spec $name)]]
    (if (null? $spec) nil
      (let [[$sfx (get $spec "sfx")]]
        (if (not (includes-item @op-emitted-atom $sfx))
          (do (swap! op-emitted-atom push $sfx)
              (swap! wrapper-defs-atom push (op-wrapper-c $spec)))
          nil)
        (str "fl_fn_new(__fl_op_" $sfx ", 0, NULL)")))))
```

### (2) `cgc` variable 분기에 연산자 우선 처리 (64-68행)

```lisp
;; AFTER
[(= (get $n :kind) "variable")
  (let [[$nm (get $n :name)]
        [$op (cgc-op-value-ref $nm)]]      ;; 연산자면 함수값, 아니면 nil
    (if (not (null? $op)) $op
      (let [[$cn (c-name $nm)]]
        (if (includes-item @known-defns-atom $cn)
          (str "fl_fn_new(__fl_wrap_" $cn ", 0, NULL)")
          $cn))))]
```

### (3) `cgc-literal` symbol 분기에도 동일 (91-94행)

```lisp
;; symbol 케이스의 [true ...] 직전에 op 체크 추가
[true (let [[$op (cgc-op-value-ref $v)]]
        (if (not (null? $op)) $op
          (let [[$cn (c-name $v)]]
            (if (includes-item @known-defns-atom $cn)
              (str "fl_fn_new(__fl_wrap_" $cn ", 0, NULL)")
              $cn))))]
```

> **주의**: 연산자가 **호출 위치**(sexpr op)일 때는 `cgc-sexpr`(148행)에서
> `(string? $op)` → `cgc-dispatch` 경로로 빠지므로 본 패치의 값-위치 분기와
> 충돌하지 않는다. 본 패치는 op가 **인자/값**으로 등장하는 경우(variable/symbol
> 노드)에만 발동.

---

## 최소 재현 (수정 후 PASS)

```lisp
(println (reduce + 0 (list 1 2 3 4)))      ;; 기대: 10
(println (reduce * 1 (list 1 2 3 4)))      ;; 기대: 24
(println (apply - (list 10 3 2)))          ;; 기대: 5
(println (map (fn [$p] (apply < $p)) (list (list 1 2) (list 3 1))))  ;; [true false]
```

수정 전: gcc에서 `_`/`+` 토큰 오류로 빌드 실패.
수정 후: 각 연산자가 `__fl_op_*` 합성 래퍼의 함수값으로 lowering → 정상.

---

## 검증 절차 (x86 노드 또는 aarch64 cgc-bin 확보 후)

```bash
bash fl-build.sh /tmp/op.fl /tmp/op && /tmp/op     # 기대: 10 / 24 / 5 / [true false]
# 고정점 검증 — 이 패치는 codegen-c.fl 자체에도 연산자 값-참조가 있으면 영향
# → 부트스트랩 self-compile 결과 변동 점검 필수
# conformance 회귀 + 연산자-값 케이스 신규 편입
```

> **회귀 위험(최고)**: ① 호출/값 위치 구분이 어긋나면 정상 산술이 깨질 수 있음.
> ② 합성 래퍼 시그니처가 런타임 `FLClosure`/`fl_fn_new`/`fl_fn_call` ABI와
>    정확히 일치해야 함 (runtime.h 확인). ③ `=`는 fl_eq 2항 — 가변 인자 비교가
>    필요하면 별도 설계. ④ self-host 고정점이 가장 흔들리기 쉬운 패치이므로
>    #2·#3·#1 안정화 후 **마지막**에 적용 권장.

---

## ⛔ 환경 블로커 — [[bug2-defn-implicit-do]] 와 동일

cgc-bin x86-64 → aarch64 빌드/검증 불가, verify-fixpoint.sh 부재.
x86 노드 핸드오프 또는 aarch64 부트스트랩 선행. 미검증 커밋 금지.

**기록이 증명이다.**
