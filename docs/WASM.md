# fx2 WASM 런타임

현재 WASM 단계는 네이티브 런타임의 값·문자열·컬렉션·JSON·기본 I/O를
Emscripten으로 컴파일하는 smoke 경로다. OpenSSL, 네트워크, MariaDB 같은
호스트 의존 기능은 브라우저 이식 범위에서 제외하고 stub을 사용한다.

## 빌드와 실행

```bash
bash tools/build-wasm-runtime.sh
# 또는 출력 디렉터리 지정
bash tools/build-wasm-runtime.sh /tmp/fx2-wasm
```

출력은 `fx2-runtime-smoke.js`, `fx2-runtime-smoke.wasm`,
`fx2-runtime-smoke.wat`이며 Node에서 기존 `runtime/test_runtime.c` 테스트를
실행한다.

필요 도구는 `emcc`, `wasm2wat`, `node`다. 이 smoke 경로는 전체 fx2 언어
컴파일러나 DOM 바인딩을 의미하지 않는다. 다음 단계는 브라우저용 메모리
런타임과 JavaScript DOM 어댑터를 별도 계층으로 추가하는 것이다.
