# FreeLang fx2 언어 사양

**상태**: Formal Specification v2.0  
**기준일**: 2026-06-25  
**적용 범위**: 런타임 + 컴파일러 + stdlib

---

## 1. 핵심 타입 시스템

### 1.1 Truthiness

| 값 | 타입 | 평가 |
|---|------|-----|
| `nil` | nil | **false** |
| `false` | boolean | **false** |
| `0` | integer | **false** ⚠️ |
| `0.0` | float | **false** ⚠️ |
| `""` | string | **true** |
| `[]` | list | **true** |
| `{}` | map | **true** |

**핵심**: `nil`, `false`, 숫자 `0`만 falsy. 공 컬렉션은 truthy.

---

## 2. 함수와 클로저

### 2.1 정의

```lisp
(define $name $value)          ;; 전역 바인딩
(defn $name [$args] $body)     ;; 함수 (단일 표현식)
(fn [$args] $body)             ;; 익명 함수
(let [[$v $e]] $body...)       ;; 지역 바인딩 (다중 표현식)
```

**제약**: `defn` 본문은 **단일 표현식** 필수. `do`로 감싸서 다중 표현식 가능.

### 2.2 클로저

- 외부 `define` 캡처 가능
- `let` 바인딩 캡처 가능
- lambda 내부 `defn` 파라미터 캡처 시 주의 필요

### 2.3 Reduce 규칙

상태 맵의 키 이름은 lambda 내 변수명과 겹치지 않게 지을 것.

---

## 3. 컬렉션

```lisp
(list 1 2 3)                    ;; [1 2 3]
(length $list)                  ;; 3
(get $list 0)                   ;; 1
(push $list 4)                  ;; [1 2 3 4]
(map (fn [$x] (* $x 2)) $list) ;; [2 4 6]
(filter (fn [$x] (> $x 1)) $list)
(reduce (fn [$a $x] (+ $a $x)) 0 $list)

{"key" "value"}                 ;; 맵
(get $map "key")                ;; "value"
(assoc $map "new" "val")        ;; 새 맵
```

---

## 4. 문자열과 수학

```lisp
(str "Hello" " " "World")       ;; 문자열 합치기
(str-count $s "sub")            ;; 부분문자열 개수
(str-truncate $s 10 "...")      ;; 길이 제한
(str-rpad $s 20 ".")            ;; 오른쪽 패딩
(str-includes $s "sub")         ;; boolean

(+ 1 2)
(- 5 2)
(* 3 4)
(/ 10 2)
(% 10 3)
(> $a $b)
(math-clamp $val 0 10)
(math-sign -5)                  ;; -1
```

---

## 5. 제어 흐름

```lisp
(if $cond $then $else)
(if $cond $then)                ;; $else 없으면 nil

(cond
  ($c1 $r1)
  ($c2 $r2)
  :else $default)

(try
  $expr
  (catch $e $handler))
```

---

## 6. 고급 제어

### Threading Macros

```lisp
(-> 3 (+ 10) (* 2))            ;; 첫 인자에 삽입
(->> (list 1 2 3)
     (map (fn [$x] (* $x 2)))  ;; 마지막 인자에 삽입
     (filter (fn [$x] (> $x 2))))
```

### Atoms

```lisp
(define $counter (atom 0))
(deref $counter)                ;; 0
(reset! $counter 5)             ;; 5
(swap! $counter (fn [$x] (+ $x 1)))  ;; atomic 증가
```

---

## 7. 네이밍 규칙

| 범주 | 형식 | 예 |
|------|------|-----|
| 변수 | `$name` | `$x`, `$result` |
| 함수 | `name-hyphens` | `add-one` |
| 상수 | `$CONST` | `$PI`, `$MAX` |
| 심볼 | `:keyword` | `:else` |

**예약어**: `define`, `defn`, `fn`, `let`, `if`, `cond`, `try`, `catch`, `do`, `apply`, `atom`, `deref`, `reset!`, `swap!`, `map`, `filter`, `reduce`, `push`, `get`, `assoc`, `keys`, `+`, `-`, `*`, `/`, `%`, `=`, `<`, `>`, `<=`, `>=`

---

## 8. 런타임 제약

- **정수**: int64
- **실수**: IEEE 754 double (15자리)
- **문자열**: UTF-8, 무제한
- **컬렉션**: 무제한
- **재귀**: ~10,000 깊이

---

## 9. 빌드

```toml
[project]
name = "app"
entry = "main.fl"
output = "app-bin"

[runtime]
packages = ["str-truncate", "math-clamp"]
```

빌드:

```bash
bash fl-build.sh main.fl app-bin
```

검증 (컴파일 없이):

```bash
bash verify.sh main.fl
```

---

## 10. 버전

| 버전 | 릴리즈 | 변경 |
|------|--------|------|
| v2.0 | 2026-06-25 | 정식 사양 |
| v1.1 | 2026-06-14 | `->` / `->>` |
| v1.0 | 2026-06-01 | 초기 |

---

**기록이 증명이다.**
