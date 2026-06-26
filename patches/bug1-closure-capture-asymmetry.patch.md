# 패치: 버그 #1 — 람다가 defn 파라미터를 캡처 못 함 (경로 비대칭)

**대상**: `/root/freelang-v11/self/codegen-c.fl`
**상태**: 소스 근원 확정 · 검증 대기 ([[bug2-defn-implicit-do]] 동일 블로커)
**증상**(CLAUDE.md 함정 2): defn 본문의 람다가 outer defn 파라미터를 참조하면
C에서 `undeclared` — env 미등록.

---

## 근원 — 두 defn 코드생성 경로의 비대칭

람다 캡처는 `cgc-fn`(356행)이 `cgc-fn-caps-filter`로 결정하며, 필터 조건은
**`(includes-item $outer $v)`** — 즉 `@outer-params-atom`에 있어야만 캡처된다.

| 경로 | 함수 | outer-params-atom 설정 | 결과 |
|------|------|----------------------|------|
| SEXPR | `cgc-defn` (476행) | ✅ `(reset! ... $pnames)` (481행) | 캡처 OK |
| FUNC 블록 | `cgc-func-block` (111행) | ❌ 없음 — 117행 바로 `(cgc $body-node)` | **캡처 실패** |

`cgc-func-block`은 파라미터 이름을 outer에 넣지 않으므로, 그 본문의 람다는
defn 파라미터를 "outer에 없는 자유변수"로 보고 env에서 제외 → C `undeclared`.

---

## 수정 — `cgc-func-block`을 `cgc-defn`과 대칭으로

본문 컴파일 **전에** 파라미터 이름을 outer-params-atom에 reset.

```lisp
;; BEFORE (111-118)
(defn cgc-func-block [$n]
  (let [[$name (c-name (get $n :name))]
      [$f (get $n :fields)]
      [$params-block (get $f :params)]
      [$body-node (get $f :body)]
      [$ps (cgc-params (get-block-items $params-block))]
      [$body (cgc $body-node)]]
  (str "FLValue " $name "(" $ps ") {\n    return " $body ";\n}")))

;; AFTER — outer-params 설정 후 본문 컴파일 (cgc-defn 481행과 동일 패턴)
(defn cgc-func-block [$n]
  (let [[$name (c-name (get $n :name))]
        [$f (get $n :fields)]
        [$params-block (get $f :params)]
        [$body-node (get $f :body)]
        [$param-items (get-block-items $params-block)]
        [$ps (cgc-params $param-items)]
        [$pnames (cgc-fn-param-names $param-items 0 [])]]
    (reset! outer-params-atom $pnames)        ;; ← 추가: 본문 전에 outer 등록
    (let [[$body (cgc $body-node)]]
      (str "FLValue " $name "(" $ps ") {\n    return " $body ";\n}"))))
```

`cgc-fn-param-names`(301행)·`reset!`·`outer-params-atom`(8행)는 기존 정의 그대로 재사용.

> **확인 필요(x86 노드)**: 최상위 `(defn …)`가 실제로 FUNC 블록 경로
> (`cgc-func-block`)로 가는지 sexpr 경로(`cgc-defn`)로 가는지 파서 기준 확인.
> - FUNC 블록 경로면 본 패치가 직접 해결.
> - sexpr 경로뿐이면 버그가 다른 곳 → `cgc-fn`의 outer 스냅샷 시점(중첩 defn/
>   nested scope에서 `reset!`이 outer를 덮어써 상위 파라미터 소실 가능성) 점검.
>   이 경우 `reset!` → 저장·복원(push/pop) 방식으로 바꿔야 중첩에서 안전.

---

## 중첩 스코프 주의 (reset! vs 복원)

`outer-params-atom`을 `reset!`로 덮으면 **중첩 함수**에서 상위 outer가 사라진다.
현재 cgc-defn도 `reset!`을 쓰므로 동일 한계 보유. 더 견고한 형태:

```lisp
;; 진입 시 이전 outer 저장 → 본문 컴파일 → 종료 시 복원
(let [[$saved @outer-params-atom]]
  (reset! outer-params-atom (concat $saved $pnames))   ;; 상위+현재 누적
  (let [[$body (cgc $body-node)]]
    (reset! outer-params-atom $saved)                  ;; 복원
    ...))
```

> 단, 이 누적/복원 방식은 cgc-defn과 동시 적용해야 일관됨. 회귀 위험이 더 크므로
> **1차로는 단순 reset 대칭화**만 적용해 #1 핵심을 닫고, 누적/복원은 별도 검증.

---

## 최소 재현 (수정 후 PASS)

```lisp
(defn tag-all [$prefix $xs]
  (map (fn [$x] (str $prefix $x)) $xs))   ;; 람다가 outer 파라미터 $prefix 캡처
(println (tag-all "n" (list 1 2)))         ;; 기대: ["n1" "n2"]
```

수정 전: gcc `'prefix' undeclared in __fl_anon_N`.
수정 후: $prefix가 outer에 등록 → env 캡처 → 정상.

---

## 검증 절차 (x86 노드 또는 aarch64 cgc-bin 확보 후)

```bash
bash fl-build.sh /tmp/tag.fl /tmp/tag && /tmp/tag    # 기대: ["n1" "n2"]
# 고정점 검증 + conformance 회귀 (람다 캡처 케이스 추가)
# 회귀 주의: outer-params 변경은 모든 람다 캡처에 영향 → 과다 캡처/누락 양방향 점검
```

회귀 케이스 (conformance 편입 권장):

```lisp
;; (a) 캡처 정상 유지
(defn add-n [$n] (map (fn [$x] (+ $x $n)) (list 1 2 3)))
(add-n 10)   ;; [11 12 13]
;; (b) 과다 캡처 없음 — 파라미터 아닌 전역은 env에 안 들어가야
(define G 100)
(defn use-g [$x] (+ $x G))
(use-g 1)    ;; 101
```

---

## ⛔ 환경 블로커 — [[bug2-defn-implicit-do]] 와 동일

cgc-bin x86-64 → aarch64 빌드/검증 불가, verify-fixpoint.sh 부재.
x86 노드 핸드오프 또는 aarch64 부트스트랩 선행. 미검증 커밋 금지.

**기록이 증명이다.**
