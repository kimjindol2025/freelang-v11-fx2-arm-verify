# 패치: 버그 #2 — defn 다중 본문 (implicit-do)

**대상**: `/root/freelang-v11/self/codegen-c.fl`
**상태**: 소스 근원 확정 · 검증 대기 (이 노드 빌드 불가 — 하단 블로커 참조)
**근거**: 생성 C에서 다중 본문 defn의 함수 정의·본문 통째 소실 재현 확인

---

## 수정 1 — `cgc-defn` (sexpr 경로, codegen-c.fl:476)

args 레이아웃: `[name, params, body0, body1, ...]`. 현재 `(get $args 2)`로
body0만 컴파일하고 body1+ 폐기.

```lisp
;; BEFORE (488행)
(let [[$body (cgc (get $args 2))]]
  (str "FLValue " $name "(" $ps ") {\n    return " $body ";\n}"))

;; AFTER — args[2..]를 암묵적 do로
(let [[$bodies (slice $args 2 (length $args))]
      [$body (if (= (length $bodies) 1)
               (cgc (get $bodies 0))
               (cgc-do $bodies))]]      ;; cgc-do(438행): (__extension__ ({ ... }))
  (str "FLValue " $name "(" $ps ") {\n    return " $body ";\n}"))
```

`cgc-do`는 이미 `(__extension__ ({ stmt0; stmt1; ...; last }))`를 생성하므로
`return <그것>;` 형태로 그대로 유효 (GCC statement-expression, 마지막 값 반환).

---

## 수정 2 — `cgc-func-block` (FUNC 블록 경로, codegen-c.fl:111)

`:body`가 파서에서 단일 노드 슬롯. **선행 확인 필요**: FUNC 블록 파서가
다중 본문 폼을 `:body`(또는 :fields)에 배열로 보존하는가?
- 보존 O → 수정 1과 동일하게 implicit-do 적용
- 보존 X (단일만) → 다중 본문 defn은 sexpr 경로(cgc-defn)로만 들어오므로 수정 1로 충분

> 참고: cgc-func-block은 버그 #1(outer-params-atom 미설정)도 함께 가지므로,
> 그 수정 시 본 패치와 동반 적용 권장.

---

## 검증 절차 (x86 노드 또는 aarch64 cgc-bin 확보 후)

```bash
# 1. codegen-c.fl 수정 → cgc-bin 재빌드
# 2. 최소 재현 컴파일
cat > /tmp/t.fl <<'F'
(define _log (atom []))
(defn multi [$x] (swap! _log push "a") (swap! _log push "b") $x)
(multi 1)
(println (length (deref _log)))   ;; 수정 후 2 (현재: 1)
F
bash fl-build.sh /tmp/t.fl /tmp/t && /tmp/t   # 기대 출력: 2
# 3. 고정점 검증 + conformance 회귀
```

---

## ⛔ 환경 블로커 (2026-06-26 확인)

이 aarch64 노드에서 **fx2 생산 컴파일러 빌드/검증 불가**:

| 항목 | 상태 | 증거 |
|------|------|------|
| `cgc-bin` 아키텍처 | **x86-64** | ELF machine `3e 00`, 노드는 `aarch64` → `cannot execute` |
| aarch64 cgc-bin | **없음** | bin/ 전체 x86-64 |
| 빌드 스크립트 경로 | **불일치** | fl-build.sh `/home/kimjin/...` 하드코딩 |
| `verify-fixpoint.sh` | **부재** | 어디에도 없음 (CLAUDE.md만 참조) |
| bootstrap 경유 빌드 | **무한루프** | codegen-c.fl 전체 체인 인터프리터 구동 시 timeout(TCO 부재) |
| 인터프리터(`fl`) | ✅ 정상 | 단, 버그 #2 **없음** — 컴파일 경로 전용 버그 |

**결론**: 수정 자체는 작고 명확하나, 이 노드에선 빌드·고정점 검증이 불가.
정식 적용은 (A) x86 노드(kimjin) 또는 (B) aarch64 cgc-bin 부트스트랩 선행 필요.
"verify PASS 없이 완료 금지" 원칙상 미검증 커밋 금지.

**기록이 증명이다.**
