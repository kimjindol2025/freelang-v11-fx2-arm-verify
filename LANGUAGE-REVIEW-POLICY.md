# Language Review Policy

언어와 애플리케이션을 리뷰할 때는 아래 기준으로 분류한다.

## 1. LANGUAGE SEMANTICS
언어 명세 자체다.

예시:
- 클로저 지원 여부
- 스코프 규칙
- 함수 호출 규칙
- 타입 시스템
- range 문법

원칙:
- 버그와 혼동하지 않는다.
- 변경 시 Breaking Change 여부를 반드시 표시한다.

## 2. COMPILER BUG
컴파일러가 명세대로 동작하지 않는 경우다.

예시:
- 잘못된 코드 생성
- 잘못된 타입 검사
- 컴파일 오류
- 최적화 오류

우선순위:
- 최우선 수정

## 3. RUNTIME CONSTRAINT
런타임 또는 기본 라이브러리의 현재 제약이다.

예시:
- HTML Escape 미지원
- Safe JSON 출력 미지원
- SQL Prepared API 부족
- 파일 처리 제한

원칙:
- 가능하면 Safe API를 추가한다.

## 4. API CONSTRAINT
기존 API 설계의 제약이다.

예시:
- server-json이 인자 1개만 허용
- 특정 함수의 인자 제한
- 기존 인터페이스의 한계

원칙:
- 호환성을 고려하여 개선한다.

## 5. STDLIB GAP
표준 라이브러리의 기능 부족이다.

예시:
- 문자열 함수 부족
- 컬렉션 함수 부족
- 날짜/시간 API 부족
- JSON 유틸리티 부족

원칙:
- 새 함수를 추가하여 해결한다.

## 6. APPLICATION BUG
언어와 무관한 애플리케이션 구현 문제다.

예시:
- 검색 로직 오류
- XSS
- SQL Injection
- 잘못된 비즈니스 로직

원칙:
- 언어 문제로 분류하지 않는다.

## Change Classification

모든 개선 사항은 아래 중 하나를 반드시 표시한다.

🟢 NON-BREAKING
- 언제든 추가 가능
- 기존 코드 영향 없음

🟡 SOFT BREAKING
- 경고 후 점진적 변경 가능
- 이전 방식과 일정 기간 호환

🔴 BREAKING
- 언어 명세 변경
- 메이저 버전에서만 적용

## Review Rules

리뷰 시 반드시 다음 순서로 판단한다.

1. 언어 명세 문제인가?
2. 컴파일러 버그인가?
3. 런타임 제약인가?
4. API 제약인가?
5. 표준 라이브러리 부족인가?
6. 애플리케이션 버그인가?

언어 문제와 애플리케이션 버그를 혼동하지 않는다.

모든 지적 사항에는 다음을 함께 기록한다.

- Category
- Severity
- Breaking 여부
- 수정 가능 여부
- 권장 해결 방법
