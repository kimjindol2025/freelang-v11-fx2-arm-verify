# fx2 Playground

fx2 코드 입력을 받아 `fl check`를 실행하는 최소 SSR/API 예제다.

## 실행

```bash
bash run.sh build
./fx2-playground
```

접속 주소: `http://localhost:8488/`

## API

`POST /api/check`에 `code` 필드를 보내면 다음 형태의 JSON을 반환한다.

```json
{
  "ok": false,
  "source": "...",
  "output": "...",
  "file": "/tmp/fx2-playground-check.fl"
}
```

현재는 단일 임시 파일을 사용하는 최소 구현이다. 다중 사용자 운영 전에는 요청별 임시 경로, 실행 시간 제한, 소스 크기 제한, 샌드박스 실행을 추가해야 한다.
