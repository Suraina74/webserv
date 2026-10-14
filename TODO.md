# Webserv TODO

_Last reviewed: 2026-10-14_

## Build (blocking: `make` currently fails)
- [ ] `src/serverProcess/CGI.cpp`: `_env("","")` can't initialize a `map`; `CGI();` inside the constructor creates a temporary instead of delegating; `int i` vs `size()` sign-compare; unused params in `populateEnv()`; destructor, `runScript()` and getters are declared but not defined (will fail at link time) (Wenxuan)
- [ ] `src/serverProcess/routerUtils.cpp`: empty handlers have unused `client`/`route` params, `(void)` them until implemented (Wenxuan)
- [ ] `src/Request.cpp` `setStatusText()`: missing `case Forbidden` and no return after the switch (Suraina)
- [ ] `src/configParser/ConfigLocationAttri.cpp:17`: mixed tabs/spaces trigger `-Wmisleading-indentation`
- [ ] `inc/main.hpp`: add `#pragma once`

## Routing (config ↔ request), `src/serverProcess/router.cpp`
- [x] Match request path to a location block (longest prefix, whole segments): `findLocation()`
- [x] Check allowed methods (405) and body size limit (413): `validateRouteStat()`
- [x] Resolve `route.fsPath` from location root, falling back to server root
- [x] Decide route action (redirect / CGI / upload / delete / static / index / autoindex): `setRouteAction()`
- [ ] Remove the hardcoded `www/index.html` / `www/uploads.html` check in `validateRequestLine()`; it 404s every GET/POST before routing runs: `src/parseRL.cpp` (Suraina)
- [ ] Skip `processRoute()` when parsing already failed (`request.getStatusCode() != OK`): `src/serverProcess/eventLoop.cpp`
- [ ] `processRoute()`: if action is `R_ERROR` but status is still `OK`, set 500
- [ ] Fall back to the server `index` when a location has none (location `/` in `standard.config` would show autoindex instead of `index.html`)
- [ ] Config roots are absolute (`root /www;`) and resolve to `/www` on the filesystem root; make them relative in the configs or resolve them against the project dir
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
- [ ] Only require multipart for uploads, not for CGI POSTs: `src/parseHeaders.cpp` (Suraina)
- [ ] `receiveRequest()`: use `>=` for the Content-Length completion check; `find("0\r\n\r\n")` can match inside chunk data; enforce `client_max_body_size` while reading instead of after; avoid copying `fullRequest` on every `recv()`: `src/serverProcess/serverManager.cpp`

## Response
- [ ] Build the response from the `Route` (fsPath, status, headers), not `request.getPath()`; `Route` is a local in `eventLoop()` and is discarded after routing
- [ ] `sendResponse()` calls `composeResponse()` on every POLLOUT, so the status line and Content-Length are appended again on each partial send; compose once
- [ ] `setCString(fullResponse.c_str())` points into a local string; keep the response string in `Response` and send from it
- [ ] Content-Type per file instead of always `text/html`
- [ ] Use custom error pages from config (Suraina)
- [ ] Support redirect responses (301/302 + Location header) (Suraina)

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
