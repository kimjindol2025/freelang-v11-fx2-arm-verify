# FreeLang fx2 — 정식 컴파일형 언어

> **Lisp 문법으로 쓰고, C 컴파일로 실행하는 네이티브 바이너리 언어.**

[![정식 사양](https://img.shields.io/badge/SPEC-v2.0-brightgreen)](./SPEC.md)
[![릴리즈](https://img.shields.io/badge/릴리즈-2026--06--22-blue)](./RELEASE-GATE.md)

---

## 🚀 시작하기

### 설치

```bash
# fx2 바이너리 확인
which fl
```

### 첫 프로그램

```lisp
;; hello.fl
(define msg "Hello, fx2!")
(println msg)
```

빌드 및 실행:

```bash
bash fl-build.sh hello.fl hello-bin
./hello-bin
```

---

## 📚 문서

| 문서 | 내용 |
|------|------|
| **[INTRODUCTION.md](./INTRODUCTION.md)** | 언어 소개 + 파이프라인 + 장단점 |
| **[SPEC.md](./SPEC.md)** | 완전한 언어 사양 (truthiness, 함수, 제약) |
| **[RELEASE-GATE.md](./RELEASE-GATE.md)** | 정식 릴리즈 체크리스트 |
| **[spec/README.md](./spec/README.md)** | Conformance 테스트 가이드 |

---

## ✅ 핵심 기능

### 1. 함수 조립형

```lisp
;; 고차함수
(define nums (list 1 2 3 4 5))

(filter (fn [$x] (> $x 2)) nums)
;; [3 4 5]

(map (fn [$x] (* $x 2)) nums)
;; [2 4 6]
```

### 2. 컴파일 파이프라인

```
.fl → cgc-bin → C → gcc → ELF 바이너리
```

### 3. 검증 우회

컴파일 없이 의존성만 검사:

```bash
bash verify.sh program.fl
```

---

## 🧪 테스트

Conformance 테스트 실행:

```bash
cd spec
bash ../fl-run.sh build
```

---

## 📋 구조

```
freelang-v11-fx2/
├── INTRODUCTION.md      — 언어 소개
├── SPEC.md             — 정식 사양
├── RELEASE-GATE.md     — 릴리즈 체크리스트
├── README.md           — 이 파일
├── verify.sh           — 검증 우회 경로
├── fl-build.sh         — 빌드 스크립트
├── fl                  — 런처
├── spec/
│   ├── README.md
│   ├── conformance.fl  — 기능 테스트
│   ├── conformance.md
│   └── fx.toml
└── runtime/            — C 런타임
```

---

## 🎯 다음 단계

- [x] SPEC.md 정식화
- [x] INTRODUCTION.md 작성
- [x] verify.sh (검증 우회)
- [x] conformance.fl (테스트 스캐폴드)
- [ ] stdlib 완성
- [ ] 추가 에러 패턴 검증

---

## 📝 버전

**v2.0** (2026-06-22) — 정식 사양 공개

- 완전한 언어 사양 (SPEC.md)
- 정식 소개 문서 (INTRODUCTION.md)
- 검증 우회 경로 (verify.sh)
- Conformance 테스트 (spec/conformance.fl)

---

## 💡 특징

| 특징 | 설명 |
|------|------|
| **파이프라인** | 선형적·명확함 |
| **배포** | 네이티브 바이너리 단일 파일 |
| **성능** | C 레벨 속도 |
| **문법** | Lisp 스타일 |
| **타입** | 동적 타입 |

---

**기록이 증명이다.**
