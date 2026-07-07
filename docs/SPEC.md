# FreeLang fx2 Specification

FreeLang fx2 is a compiled, Lisp-like language for function composition.

## Form

- S-expression based.
- Prefix calls.
- Variable names use `$` prefixes.
- Function names use kebab-case.
- Output target is a native ELF binary.

## Values

Supported value classes:

- integers
- floats
- booleans
- nil
- strings
- vectors
- maps
- functions

## Evaluation

- Expressions evaluate left to right.
- Calls evaluate callee and arguments before invocation.
- `if` evaluates the condition first, then the selected branch.
- `let` binds sequentially inside its scope.

## Binding

- `define` binds a name at module scope.
- `let` creates local bindings.
- `defn` defines a named function.
- `fn` creates an anonymous function.

## Truthiness

Falsey values:

- `nil`
- `false`
- `0`
- `0.0`

Everything else is truthy.

## Collections

- Vectors and maps are first-class values.
- Empty vectors and empty maps are truthy.
- `get` returns `nil` for a missing key.

## Core operations

Representative built-ins and stdlib forms:

- `str`
- `println`
- `apply`
- `map`
- `filter`
- `reduce`
- `some`
- `doseq`
- `list`
- `length`
- `nth`

## Strings and numbers

Representative helpers:

- `str-indent`
- `str-truncate`
- `str-rpad`
- `str-count`
- `str-lines`
- `str-starts-with`
- `math-clamp`
- `math-lerp`
- `math-round-n`
- `math-sign`

## Modules and I/O

- `(load "...")` includes another `.fl` file.
- File, HTTP, JSON, DB, env, and time helpers are runtime-provided.

## Intentional limitations

These are frozen as intentional limitations unless explicitly removed:

- `str-includes` returns boolean.
- `(range n)` is not the standard path; use `(range 0 n)`.
- `fn` closures capture surrounding parameters and local bindings used by the function.
- Naming violations are caught in verification before build where possible.

## Conformance requirement

A source is conformant if it passes:

- spec conformance checks
- verification rules
- the runtime/binary build path
