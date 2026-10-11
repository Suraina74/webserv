# Webserv TODO

## Routing (config ↔ request)
- [ ] Match request path to a location block (longest prefix) — `src/eventLoop.cpp`
- [ ] Check allowed methods per location (405), body size limit (413), redirects (Suraina)
- [ ] Use root / index from config instead of hardcoded `www` — `src/parseRL.cpp` (Suraina)
- [ ] Remove the hardcoded `index.html` / `uploads.html` check — `src/parseRL.cpp` (Suraina)

## Request
- [x] Split query string off the path (`?`) and add a getter — `src/parseRL.cpp` 
- [ ] Only require multipart for uploads, not for CGI POSTs — `src/parseHeaders.cpp` (Suraina)

## Methods
- [ ] Complete GET: full file read, 404, index, autoindex, MIME types — `src/Response.cpp` (Suraina)
- [ ] Complete POST: use upload path from config, return 201 — `src/Request.cpp` (Suraina)
- [ ] Complete DELETE: resolve real path, correct status codes (404/403/204) — `src/Request.cpp` (Suraina)

## CGI
- [ ] Complete `src/CGI.cpp` (env, runScript, write body, read output) (Wenxuan)
- [ ] Add CGI map + pipe fds to the event loop — `src/eventLoop.cpp` (Wenxuan)
- [ ] Turn CGI output into a response (status, headers, Content-Length) — `src/Response.cpp` (Wenxuan)
- [x] Add `src/CGI.cpp` to the Makefile

## Response
- [ ] Use custom error pages from config (Suraina)
- [ ] Support redirect responses (301/302 + Location header) (Suraina)

## Robustness
- [ ] Client and CGI timeouts
- [ ] Ignore SIGPIPE 
- [ ] Clean up CGI children when a client disconnects
- [ ] Stress test (siege / many parallel requests)
