# Security

## HTML
- Escape all user-controlled content before writing HTML.
- `server-html` does not protect callers automatically.

## Inline Script
- Never embed raw JSON into `<script>` without escaping `</script>`.
- Prefer `\u003C/script>` or `<\/script>` in serialized payloads.

## SQL
- Use prepared APIs for any SQL that touches external input.
- Do not concatenate user input into SQL strings.

## Input Validation
- Validate route params, query params, and request bodies before use.
- Treat all external data as untrusted, even in internal tools.
