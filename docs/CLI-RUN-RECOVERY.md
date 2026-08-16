# `fl run` 실행 복구

## 변경

`fl run`은 프로젝트의 `fx.toml`에 선언된 output을 실행한다. 기존 output이 현재 호스트에서 실행되지 않으면 실패 원인과 재빌드 명령을 함께 출력한다.

```bash
fl run
fl run --help
fl run --rebuild
```

## 진단

실행 결과가 `126` 또는 `127`이면 다음 안내를 출력한다.

```text
실행 파일을 현재 환경에서 시작할 수 없습니다: <output>
fl run --rebuild 로 다시 빌드하세요
```

이 변경은 실행 경로의 진단만 개선한다. 바이너리 ABI·호스트 아키텍처 자체를 자동 변환하지는 않는다.
