# FreeLang AFJ Backend/API smoke

The server implementation is FreeLang AFJ (app.fl) using the existing fx2 HTTP runtime: server_get, server_post, server_json, server_status, and server_start.

The API invokes the existing AFJ DB v2 AFJ CLI through process-run; the verification adapter also reads the resulting temporary log directly. No SQLite, PostgreSQL, Node/Express, or production port is used.

Run: fl verify

The verifier chooses a test port, starts the compiled FreeLang binary, calls health, POST, GET, direct AFJ DB read, and validation, then stops the child and cleans the temporary database namespace.
