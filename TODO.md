# Webserv TODO

_Last reviewed: 2026-10-19 (after merging `http_parser` into `WenxuanDev`)_

## Build (blocking: `make` currently fails)
- [ ] `src/serverProcess/CGI.cpp`: `_env("","")` can't initialize a `map`; `CGI();` inside the constructor creates a temporary instead of delegating; unused params in `populateEnv()`; destructor, `runScript()` and getters are declared but not defined (will fail at link time) (Wenxuan)
- [ ] `src/serverProcess/routerUtils.cpp`: empty handlers have unused `client`/`route` params, `(void)` them until implemented (Wenxuan)
- [x] `src/Request.cpp` `setStatusText()`: missing `case Forbidden` and no return after the switch (Suraina)
- [x] `src/configParser/ConfigLocationAttri.cpp:17`: mixed tabs/spaces trigger `-Wmisleading-indentation`
- [ ] `inc/main.hpp`: add `#pragma once`

## Routing (config ↔ request), `src/serverProcess/router.cpp`
- [x] Match request path to a location block (longest prefix, whole segments): `findLocation()`
- [x] Check allowed methods (405) and body size limit (413): `validateRouteStat()`
- [x] Resolve `route.fsPath` from location root, falling back to server root
- [x] Decide route action (redirect / CGI / upload / delete / static / index / autoindex): `setRouteAction()`
- [x] Fall back to the server `index` when a location has none
- [x] Config roots are relative (`root www;`)
- [x] `processRoute()` is now called from `Client::receiveRequest()` before the response is composed, so routing affects the response
- [ ] **Blocking all requests:** `src/parseRL.cpp` prefixes the path with `www` (`Path = "www" + Path`) and only allows `www/index.html` / `www/uploads.html`. The router rejects any path not starting with `/`, so every request currently returns 400 or 404 (smoke test 2026-10-19: `GET /` → 400, `GET /images/icon.png` → 404). Remove the prefix and the hardcoded page check (Suraina)
- [ ] Skip `processRoute()` when parsing already failed (`request.getStatusCode() != OK`): `src/Client.cpp` `receiveRequest()`
- [ ] `processRoute()`: if action is `R_ERROR` but status is still `OK`, set 500
- [ ] Call each handler in the `processRoute()` switch once it is implemented
- [ ] `Request::setStatus()` overwrites `Path` with `www/404.html`; remove once error pages come from config
- [ ] Return `const string&` from `Request` getters instead of copies

## Route handlers, `src/serverProcess/routerUtils.cpp`
- [ ] `runStatic()`: read the whole file at `route.fsPath` (not just 2048 bytes), set Content-Type from the extension (Suraina)
- [ ] `runAutoIndex()`: HTML listing with links to files and subdirectories (Suraina)
- [ ] `runUpload()`: save to `loc->getUploadPath()`, check the dir exists and is writable, return 201; replaces the hardcoded `www/uploads/` in `Request::addFile()` (Suraina)
- [ ] `runDelete()`: remove `route.fsPath`, return 204 / 403 / 404; replaces `Request::postAndDelete()` (Suraina)
- [ ] `runRedirect()`: 301/302 + `Location:` header from `route.redirectTo` (Suraina)
- [ ] `runCGI()`: see CGI (Wenxuan)

## Request
- [x] Split query string off the path (`?`) and add a getter: `src/parseRL.cpp`
- [x] Multiple requests in one read: leftover data is kept and the parser is reset (`Client::receiveRequest()`)
- [x] Use `>=` for the Content-Length completion check
- [x] Content-Length overflow is rejected with 400: `src/parseHeaders.cpp`
- [ ] Only require multipart for uploads, not for CGI POSTs: `src/parseHeaders.cpp` (Suraina)
- [ ] `Client::receiveRequest()`: `find("0\r\n\r\n")` can match inside chunk data; enforce `client_max_body_size` while reading instead of after; avoid copying `fullRequest` on every `recv()`: `src/Client.cpp`

## Response
- [x] Compose the response once instead of on every POLLOUT (`Client::sendResponse()` sends from `_responsesString`)
- [ ] Build the response from the `Route` (fsPath, status, headers), not `request.getPath()`; `Route` is a local in `Client::receiveRequest()` and is discarded after routing
- [x] `Client::sendResponse()` copies `_responsesString` on every POLLOUT; send directly from it
- [ ] Content-Type per file instead of always `text/html`
- [ ] Use custom error pages from config (Suraina)
- [ ] Support redirect responses (301/302 + Location header) (Suraina)

## Event loop / connections, `src/serverProcess/eventLoop.cpp`
- [x] Use `Client::receiveRequest()` / `Client::sendResponse()` instead of the old free functions
- [ ] Handle `REQUEST_READY`: a second request in the same read is only answered after more data arrives or the client closes
- [ ] Keep-alive: the client is removed after one response and `Connection: close` is always sent
- [ ] Set client fds to non-blocking (`fcntl(fd, F_SETFL, O_NONBLOCK)` in `ServerInfo::addClient()`)

## CGI (Wenxuan)
- [x] Add `src/serverProcess/CGI.cpp` to the Makefile
- [x] Interpreter lookup and executable check: done in `setRouteAction()` (`route.interpreter`)
- [ ] Fix `CGI.cpp` so it compiles (see Build)
- [ ] Construct `CGI` from the `Route` (interpreter + fsPath already resolved) instead of scanning locations again
- [ ] `populateEnv()`: REQUEST_METHOD, QUERY_STRING, CONTENT_LENGTH, CONTENT_TYPE, SCRIPT_FILENAME, SERVER_PROTOCOL, ...
- [ ] `runScript()`: pipes, fork, execve, non-blocking fds
- [ ] Add CGI map + pipe fds to the event loop: `src/serverProcess/eventLoop.cpp`
- [ ] Turn CGI output into a response (status, headers, Content-Length): `src/Response.cpp`

## Robustness
- [ ] Client and CGI timeouts (`poll()` TIMEOUT is 60 ms; there is no per-client timeout)
- [ ] Ignore SIGPIPE (`signal(SIGPIPE, SIG_IGN)` or `MSG_NOSIGNAL` on `send()`)
- [ ] Handle POLLHUP / POLLERR on client fds
- [ ] Clean up CGI children when a client disconnects
- [ ] Stress test (siege / many parallel requests)
