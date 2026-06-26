# fx2 컴파일러 개선 패치 — 핸드오프 패키지

**대상 컴파일러**: `/root/freelang-v11/self/codegen-c.fl` → `cgc-bin`
**작성**: 2026-06-26 · 근원 분석은 [`../IMPROVEMENT-PLAN.md`](../IMPROVEMENT-PLAN.md)

fx(C-네이티브) 컴파일 경로의 불안정 버그 4종. **인터프리터(`fl`)에는 없고 컴파일
경로에만** 존재한다. 각 패치는 소스 근원(행 단위) + before/after + 최소 재현 +
검증 절차 + 회귀 케이스를 담는다.

---

## 패치 목록 (적용 순서 = 위험 오름차순)

| 순서 | 버그 | 파일 | 수정 위치 | 위험 |
|------|------|------|----------|------|
| 1 | #2 defn 단일 표현식 (데이터 무손실 실패) | [bug2-defn-implicit-do.patch.md](bug2-defn-implicit-do.patch.md) | `cgc-defn` 488 / `cgc-func-block` 111 | 낮음 |
| 2 | #3 맵 키 충돌 (컴파일 실패) | [bug3-map-key-collision.patch.md](bug3-map-key-collision.patch.md) | `cgc-collect-vars` 315 | 낮음 |
| 3 | #1 클로저 캡처 (런타임 실패) | [bug1-closure-capture-asymmetry.patch.md](bug1-closure-capture-asymmetry.patch.md) | `cgc-func-block` 111 | 중간 |
| 4 | #5 연산자 first-class (표현력 제약) | [bug5-operator-first-class.patch.md](bug5-operator-first-class.patch.md) | `cgc` 64 / `cgc-literal` 91 | 높음 |

> 순서 근거: #2·#3은 국소·저위험. #1은 캡처 분석 전반 영향. #5는 self-host
> 고정점이 가장 흔들리기 쉬워 **마지막**.

---

## ⛔ 적용 전 필수 — 환경 요건

이 aarch64 노드에서는 **빌드·검증 불가** (cgc-bin이 x86-64 → `cannot execute`,
`verify-fixpoint.sh` 부재). 다음 중 하나가 갖춰진 노드에서만 적용:

- **(A) x86 노드 (kimjin)**: cgc-bin 실행 가능 + `fl-build.sh` 정상 경로
- **(B) aarch64 부트스트랩**: cgc-bin을 aarch64로 재생성한 뒤 (별도 선행 작업)

"verify PASS 없이 완료 금지" — 미검증 커밋 금지.

---

## 적용 워크플로우 (패치 1건당)

```bash
# 1. codegen-c.fl 수정 (해당 패치의 before/after 적용)
# 2. cgc-bin 재빌드
cd /root/freelang-v11 && node scripts/build.js     # 또는 self 빌드 경로
# 3. 고정점 검증 (필수) — gen-a == gen-b == gen-c
bash verify-fixpoint.sh
# 4. 패치의 "최소 재현" 컴파일·실행 → 기대값 확인
# 5. conformance 회귀 + 패치의 회귀 케이스 편입
fl /root/freelang-v11-fx2/spec/conformance.fl
# 6. 통과 시에만 커밋
```

각 패치 적용 후 **다음 패치로 넘어가기 전 고정점·conformance 통과 확인**.
한 번에 여러 패치를 묶지 말 것 (회귀 원인 격리).

---

## 검증 체크리스트 (4종 공통)

- [ ] 최소 재현이 수정 후 기대값 출력
- [ ] 고정점 gen-a == gen-b == gen-c
- [ ] conformance.fl 전체 PASS
- [ ] 패치별 회귀 케이스 conformance 편입
- [ ] (#5) 합성 래퍼 시그니처가 runtime.h `FLClosure`/`fl_fn_new` ABI 일치
- [ ] (#1) 과다 캡처/누락 양방향 점검
- [ ] (#3) 맵 값 캡처 정상 케이스 유지

---

**기록이 증명이다.**
