# 패치: 버그 #3 — reduce 맵 키가 변수로 오인 (클로저 env 충돌)

**대상**: `/root/freelang-v11/self/codegen-c.fl`
**상태**: 소스 근원 확정 · 검증 대기 (aarch64 노드 빌드 불가 — [[bug2-defn-implicit-do]] 동일 블로커)
**증상**(CLAUDE.md 함정 2-b): reduce 람다 안 맵 리터럴의 키가 람다 내 `let`
변수명과 같으면 → cgc-bin이 키를 자유변수로 분류·env 추가 → gcc `'acc' undeclared`

---

## 근원 — `cgc-collect-vars` (codegen-c.fl:307)

람다 캡처 분석은 `cgc-collect-vars`로 본문의 변수 참조를 수집한다. block 분기:

```lisp
;; 315-316행 — 현재
[(= (get $node :kind) "block")
 (cgc-collect-vars-loop (get-block-items $node) 0 $acc)]
```

`get-block-items` (21-25행)는 **Array 블록만** `:fields.items`를 펼치고,
**Map 블록은 노드 자체를 반환**(25행 fallback `$node`). 결과:

1. Map 블록이 리스트가 아닌 노드로 `cgc-collect-vars-loop`에 넘어가
   `(get $node $i)` / `(length $node)`가 오작동 → 맵 **값** 위치의 진짜 변수
   참조(`(str $acc ...)`의 `$acc`)는 정상 스캔 안 됨.
2. 동시에 키 토큰이 자유변수로 잘못 분류되는 경로가 생겨, `let` 변수명과
   동명일 때 env-decl `FLValue acc = _self->env[i]`가 let-decl `FLValue acc`와
   **재선언 충돌**.

Map 노드 구조(cgc-map-block, 906행 기준): `:fields.items` =
평탄 배열 `[key0 val0 key1 val1 ...]`. 키는 `keyword`/문자열 노드, 값은 임의 표현식.

---

## 수정 — Map 블록 전용 분기 추가 (값만 스캔, 키 건너뜀)

`cgc-collect-vars`의 cond에서 **일반 block 분기보다 먼저** Map 분기를 둔다.

```lisp
;; BEFORE (315-316)
    [(= (get $node :kind) "block")
     (cgc-collect-vars-loop (get-block-items $node) 0 $acc)]

;; AFTER — Map 분기 선행, 값(홀수 인덱스)만 스캔
    [(and (= (get $node :kind) "block") (= (get $node :type) "Map"))
     (cgc-collect-vars-map-vals (get (get $node :fields) :items) 1 $acc)]
    [(= (get $node :kind) "block")
     (cgc-collect-vars-loop (get-block-items $node) 0 $acc)]
```

새 헬퍼 (cgc-collect-vars-loop 아래, 326행 뒤에 추가):

```lisp
;; 맵 리터럴: 키(짝수 idx) 건너뛰고 값(홀수 idx)만 변수 수집
(defn cgc-collect-vars-map-vals [$items $i $acc]
  (if (or (null? $items) (>= $i (length $items))) $acc
    (cgc-collect-vars-map-vals $items (+ $i 2)
      (cgc-collect-vars (get $items $i) $acc))))
```

`$i`를 1에서 시작해 +2 → 값 위치(1,3,5,...)만, 키 위치(0,2,4,...)는 스킵.

> **확인 필요(x86 노드)**: 라이브 cgc-bin AST가 self-hosted 경로(`:items` 평탄
> 배열)인지. bootstrap.js AST 경로(`map-entries`)면 `:items`가 null이라
> `map-entries`에서 값만 추출하는 분기를 추가해야 함 (cgc-map-block 909행 분기와 대칭).

---

## 최소 재현 (수정 후 PASS)

```lisp
(defn collide [$xs]
  (reduce (fn [$st $i]
            (let [[$acc (get $st "acc")]]   ;; 변수 $acc
              {"acc" (+ $acc $i)}))         ;; 키 "acc" — 동명
          {"acc" 0} $xs))
(println (get (collide (list 1 2 3)) "acc"))   ;; 기대: 6 (gcc 오류 없이)
```

수정 전: gcc `'acc' undeclared` 또는 재선언 충돌로 빌드 실패.
수정 후: 키 "acc"는 스캔 제외 → 캡처는 값 안의 `$acc`(=let 바인딩)만 → 충돌 소멸.

---

## 검증 절차 (x86 노드 또는 aarch64 cgc-bin 확보 후)

```bash
# 1. codegen-c.fl 수정 → cgc-bin 재빌드
# 2. 위 재현 컴파일
bash fl-build.sh /tmp/collide.fl /tmp/collide && /tmp/collide   # 기대: 6
# 3. 고정점 검증 + conformance 회귀 (특히 맵 리터럴 쓰는 앱 회귀 주의)
```

**회귀 주의**: 이 변경은 모든 맵 리터럴의 캡처 분석에 영향. 맵 **값** 안에서
캡처하던 정상 케이스가 누락되지 않는지 conformance에 맵-값-캡처 케이스 추가 권장:

```lisp
(defn wrap [$tag $xs]
  (map (fn [$x] {"t" $tag "v" $x}) $xs))   ;; 값 위치 $tag/$x 캡처 유지되어야
(wrap "k" (list 1 2))   ;; 기대: [{"t" "k" "v" 1} {"t" "k" "v" 2}]
```

---

## ⛔ 환경 블로커 — [[bug2-defn-implicit-do]] 와 동일

cgc-bin x86-64 → aarch64 노드 실행/빌드/검증 불가, verify-fixpoint.sh 부재.
정식 적용·검증은 x86 노드(kimjin) 핸드오프 또는 aarch64 부트스트랩 선행.
"verify PASS 없이 완료 금지" → 미검증 커밋 금지.

**기록이 증명이다.**
