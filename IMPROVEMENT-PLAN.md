# FreeLang fx2 컴파일러 개선 계획

**상태**: 개선 준비 (근본 원인 소스 확인 완료)
**기준일**: 2026-06-25
**대상 컴파일러**: `/root/freelang-v11/self/codegen-c.fl` → `cgc-bin`

> ⚠️ **이 컴파일러는 self-hosting이다.** 수정 후 반드시 `cgc-bin` 재빌드 +
> `verify-fixpoint.sh` 고정점 검증 + `spec/conformance.fl` 회귀 통과해야 커밋 가능.

---

## 근본 원인 — 소스 위치 확정

### 버그 #1 — 클로저가 defn 파라미터를 캡처 못 함 🔴

**원인**: 두 defn 코드생성 경로의 비대칭.

| 경로 | 함수 | `outer-params-atom` 설정 | 결과 |
|------|------|------------------------|------|
| SEXPR | `cgc-defn` (codegen-c.fl:476) | ✅ `(reset! ... $pnames)` (481행) | 람다 캡처 OK |
| FUNC 블록 | `cgc-func-block` (codegen-c.fl:111) | ❌ 설정 안 함 (117행 바로 `cgc`) | 람다 캡처 실패 |

람다 캡처는 `cgc-fn` (356행)에서 `cgc-fn-caps-filter`로 결정되는데, 필터 조건이
`(includes-item $outer $v)` — 즉 **`outer-params-atom`에 있어야만 캡처**된다.
FUNC 블록 경로는 이걸 안 채우므로, 그 본문의 람다는 defn 파라미터를 자유변수로
오인 → env 미등록 → C에서 `undeclared`.

**수정**: `cgc-func-block`에서 본문 컴파일 전 `(reset! outer-params-atom $pnames)`
추가 (cgc-defn과 동일하게).

---

### 버그 #2 — defn 본문 단일 표현식 (나머지 조용히 소실) 🔴 최우선

**원인**: 두 경로 모두 본문을 **단일 AST 노드**로 받아 `return <body>;` 생성.

```lisp
;; cgc-func-block (codegen-c.fl:115,118)
[$body-node (get $f :body)]                    ;; 단일 노드
(str "FLValue " $name "(...) {\n    return " $body ";\n}")

;; cgc-defn (codegen-c.fl:488-489)
[$body (cgc (get $args 2))]                     ;; args[2]만, args[3+] 무시
(str "FLValue " $name "(...) {\n    return " $body ";\n}")
```

`(defn f [x] a b c)` → `a`만 컴파일, `b` `c`는 `slice`조차 안 되고 버려짐.
**에러 없이** 첫 표현식만 실행되는 이유.

**수정**: 본문이 여러 폼이면 암묵적 `do`로 감싸기 — 이미 있는 `cgc-do`(438행) /
`cgc-stmts` 패턴 재사용. `cgc-defn`은 `(slice $args 2 ...)`, `cgc-func-block`은
FUNC 블록이 다중 본문 노드를 보존하는지 파서 확인 필요.

---

### 버그 #3 — reduce 맵 키가 변수로 오인 🟠

**원인**: `cgc-collect-vars` (codegen-c.fl:307)가 Map 블록을
`get-block-items`로 순회 (315-316행). Map 블록의 items는 **키+값 평탄 배열**이라
키 문자열까지 변수 이름으로 수집됨.

```lisp
[(= (get $node :kind) "block")
 (cgc-collect-vars-loop (get-block-items $node) 0 $acc)]   ;; 키도 포함
```

키 이름이 람다 내 `let` 변수명과 겹치면 → env-decl `FLValue acc = _self->env[0]`가
let의 `FLValue acc`와 충돌 → gcc `redeclaration` / `undeclared`.

**수정**: `cgc-collect-vars`에서 Map 블록은 **값 위치(홀수 인덱스)만** 스캔, 키 건너뛰기.

---

### 버그 #5 — 연산자 first-class 참조 시 `_` 생성 🟠

**원인**: `+` `-` `*`를 값으로 넘기면 (`(apply + ...)`, `(reduce + 0 ...)`)
변수로 취급 → `cgc-extract-name`/`c-name "+"` → `_`. cgc-main.fl:5에 이미 기록된 회피책.

**수정**: 연산자 심볼을 값 위치에서 만나면 빌트인 함수 참조
(`fl_fn_new(fl_add_wrap, ...)` 등)로 lowering하는 매핑 테이블 추가.

---

## 우선순위 (영향 / 위험 비)

| # | 버그 | 영향 | 수정 위치 | 위험 | 순위 | 패치 |
|---|------|------|----------|------|------|------|
| 2 | defn 단일 표현식 | 🔴 데이터 무손실 실패 | 2개 함수 | 낮음 | 1 | [patches/bug2-defn-implicit-do](patches/bug2-defn-implicit-do.patch.md) ✅ |
| 3 | 맵 키 충돌 | 🟠 컴파일 실패 | 1개 함수 | 낮음 | 2 | [patches/bug3-map-key-collision](patches/bug3-map-key-collision.patch.md) ✅ |
| 1 | 클로저 캡처 | 🔴 런타임 실패 | 1개 함수 (대칭화) | 중간 | 3 | [patches/bug1-closure-capture-asymmetry](patches/bug1-closure-capture-asymmetry.patch.md) ✅ |
| 5 | 연산자 first-class | 🟠 표현력 제약 | 합성 래퍼 | 높음 | 4 | [patches/bug5-operator-first-class](patches/bug5-operator-first-class.patch.md) ✅ |

> **패치 4종 모두 준비 완료** (2026-06-26). 적용 순서 = 위험 오름차순 2→3→1→5.
> 각 패치는 환경 블로커로 이 노드에서 미검증 — x86 노드(kimjin) 핸드오프 대기.

**근거**: #2·#3은 국소 수정 + 낮은 위험이면서 영향이 크다. #1은 캡처 분석
전반에 영향을 줘 회귀 위험이 더 크므로 #2·#3 안정화 후 착수.

---

## 실행 절차 (각 수정 공통)

```bash
# 1. codegen-c.fl 수정
# 2. cgc-bin 재빌드
cd /root/freelang-v11 && node scripts/build.js   # 또는 self 빌드 경로

# 3. 고정점 검증 (필수)
bash /root/freelang-v11-fx2/verify-fixpoint.sh 2>/dev/null \
  || bash /root/freelang-v11-fx/verify-fixpoint.sh
# gen-a == gen-b == gen-c 여야 커밋 OK

# 4. conformance 회귀
fl /root/freelang-v11-fx2/spec/conformance.fl

# 5. 각 버그별 최소 재현 테스트 추가 → conformance.fl 편입
```

---

## 검증용 최소 재현 (수정 후 PASS 되어야 함)

```lisp
;; 버그 #2 — 다중 본문이 모두 실행되는가
(define _log (atom []))
(defn multi [$x]
  (swap! _log push "a")
  (swap! _log push "b")
  $x)
(multi 1)
;; 기대: (length @_log) == 2

;; 버그 #3 — 맵 키와 let 변수 동명
(defn collide [$xs]
  (reduce (fn [$st $i]
            (let [[$acc (get $st "acc")]]
              {"acc" (+ $acc $i)}))
          {"acc" 0} $xs))
(collide (list 1 2 3))   ;; 기대: {"acc" 6} (gcc 오류 없이)

;; 버그 #1 — 람다가 defn 파라미터 캡처
(defn tag-all [$prefix $xs]
  (map (fn [$x] (str $prefix $x)) $xs))
(tag-all "n" (list 1 2))  ;; 기대: ["n1" "n2"]
```

---

**기록이 증명이다.**
