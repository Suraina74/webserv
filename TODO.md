# Webserv TODO

## Routing (config ↔ request)
- [ ] Match request path to a location block (longest prefix) — `src/eventLoop.cpp`
- [ ] Check allowed methods per location (405), body size limit (413), redirects
- [ ] Use root / index from config instead of hardcoded `www` — `src/parseRL.cpp`
- [ ] Remove the hardcoded `index.html` / `uploads.html` check — `src/parseRL.cpp`

## Request
- [x] Split query string off the path (`?`) and add a getter — `src/parseRL.cpp`
- [ ] Only require multipart for uploads, not for CGI POSTs — `src/parseHeaders.cpp`

## Methods
- [ ] Complete GET: full file read, 404, index, autoindex, MIME types — `src/Response.cpp`
- [ ] Complete POST: use upload path from config, return 201 — `src/Request.cpp`
- [ ] Complete DELETE: resolve real path, correct status codes (404/403/204) — `src/Request.cpp`

## CGI
- [ ] Complete `src/CGI.cpp` (env, runScript, write body, read output)
- [ ] Add CGI map + pipe fds to the event loop — `src/eventLoop.cpp`
- [ ] Turn CGI output into a response (status, headers, Content-Length) — `src/Response.cpp`
- [x] Add `src/CGI.cpp` to the Makefile

## Response
- [ ] Use custom error pages from config
- [ ] Support redirect responses (301/302 + Location header)

## Robustness
- [ ] Client and CGI timeouts
- [ ] Ignore SIGPIPE
- [ ] Clean up CGI children when a client disconnects
- [ ] Stress test (siege / many parallel requests)
