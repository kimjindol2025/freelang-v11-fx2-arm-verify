# FreeLang Load Balancer 1E-B socket path inventory

| Path | Creation | Kind | Owner | Close responsibility |
|---|---|---|---|---|
| fxb_net_listen | socket + bind/listen | LISTENER | returned listener fd | fl_net_close_once on errors or public close |
| fxb_net_accept | accept | INBOUND_CONNECTION | returned map fd | public fxb_net_close |
| fxb_net_pasv_open | socket + bind/listen | LISTENER | returned map fd | fl_net_close_once on failure or pasv accept |
| fxb_net_pasv_accept | accept | INBOUND_CONNECTION | returned data fd | public close; listener close-once |
| fxb_net_connect | socket + connect | OUTBOUND_CONNECTION | returned fd | fl_net_close_once on refusal or public close |
| fxb_net_local_ip | temporary UDP socket | OUTBOUND_CONNECTION | function-local | fl_net_close_once before return |

## Close paths

All returned network fds preserve the public fd API. Registered fds enter OPEN. The first close caller claims CLOSING; later callers are no-ops. The claimant waits for active I/O references to drain with a condition variable, performs OS close without the registry lock, then marks CLOSED and reaps only at refcount zero.

EOF, timeout, and error paths in existing blocking APIs do not implicitly close user-owned fds, preserving legacy behavior. Active lookup accepts only OPEN entries. A CLOSING entry remains retired until close completes; fd reuse receives a new monotonic generation and cannot inherit old state. No public generation, statistics, socket-option, or buffer API is added in 1E-B.
