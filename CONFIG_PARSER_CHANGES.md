# Config parser & routing fixes

_2026-10-14_

This file lists the fixes to the config parser and one security fix in the router. For each change it gives the reason, the location, and any functions that were added or changed.

All config parser files and `router.cpp` compile with `-Wall -Wextra -Werror`. The parser was run on 42 edge-case configs with AddressSanitizer and UBSan: there were no memory errors and no undefined behaviour. All configs in `config/` still parse, except `test.config`, which contains `proxy_pass` and looks deliberately invalid.

## New and changed prototypes

All of them are declared in `inc/Config.hpp`.

```cpp
// Config class: new member function
void validateServer(ServerConfig& server, const set<string>& seen, int serverLine);

// Config class: signature changed, lineNum is now passed by reference
void parseLocation(ifstream& configFile, string& line, ServerConfig& server, int& lineNum);

// free helper functions, defined in src/configParser/ConfigUtils.cpp
void verifyDirPath(string& val, int lineNum, const string& name);
void checkDuplicate(const string& line, set<string>& seen, int lineNum);
bool hasDotDotSegment(const string& path);
```

| Function | What it does |
|---|---|
| `verifyDirPath()` | Removes trailing `/`, checks the characters, and checks that the path exists and is a directory. Used for server `root`, location `root` and `upload_path`. |
| `checkDuplicate()` | Throws if a directive appears twice in one block. `error_page` and `cgi` are exempt, because they repeat once per code or extension. |
| `validateServer()` | Runs after a whole `server { }` block has been read. Requires `listen` and `root`, defaults `host` to `0.0.0.0`, and checks that the index file exists. |
| `hasDotDotSegment()` | Returns true if any `/`-separated part of a path is exactly `..`. `/a/../b` is true; `/a..b` is false. Used by the config parser and the router. |

---

## 1. Relative `root` and `upload_path` in locations

**Reason:** location `root` and `upload_path` had to start with `/`. The configs therefore used `root /www;`, which points at `/www` on the filesystem root rather than the project's `www` folder, so every request would return 404.

**Where:**
- `src/configParser/ConfigLocationAttri.cpp`: `parseLocRoot()` and `parseUploadPath()` now call `verifyDirPath()` instead of requiring a leading `/`.
- `src/configParser/ConfigServerAttri.cpp`: `parseRoot()` uses `verifyDirPath()` too, so all three directives are validated the same way.
- `config/standard.config`, `config/mult_ports.config` and `config/port_already_in_use.config` now use `www`, `www/images`, `www/uploads` and so on instead of `/www…`.
- Created `www/images`, `www/files`, `www/cgi-bin` and `www/second`, because the configs refer to them. Each contains an empty `.gitkeep` so git tracks the folder.

Relative paths are resolved from the directory you start `./webserv` in, so run it from the project root.

## 2. Uninitialized `Location` booleans

**Reason:** `_uploadEnabled` and `_directoryListing` were never initialized, so a location that didn't set them could randomly have uploads or directory listing turned on.

**Where:** `inc/Location.hpp`. All members now use `{}` initializers, so the booleans start as `false`. You made this change yourself; it's listed here so the record is complete.

## 3. Default `client_max_body_size` was 0

**Reason:** without `client_max_body_size`, the limit was 0, so every POST with a body got 413.

**Where:** `src/configParser/ServerConfig.cpp`, `ServerConfig::ServerConfig()`. The default is now `1024 * 1024` (1M), the same as nginx.

## 4. Missing or duplicate directives

**Reason:** a second `listen` silently replaced the first. A server block without `listen` got port 0, and one without `host` got an empty host.

**Where:**
- `src/configParser/Config.cpp`, `parseServer()` and `parseLocation()`: each block keeps a `set<string> seen` and calls `checkDuplicate()` for every directive.
- `src/configParser/Config.cpp`, `validateServer()`, called at the end of `parseServer()`: requires `listen` and `root`, and defaults `host` to `0.0.0.0`.

## 5. `index` only worked when written after `root`

**Reason:** `parseIndex()` built the index file path from `server.getRoot()` while parsing, so `index` written before `root` failed with "index file does not exist".

**Where:**
- `src/configParser/ConfigServerAttri.cpp`, `parseIndex()`: now only checks the file name.
- The existence check moved to `validateServer()`, which runs after the whole block has been read, so the order of directives no longer matters.

## 6. Compile error from mixed indentation

**Reason:** a mix of tabs and spaces triggered `-Wmisleading-indentation`, and `-Werror` turned that into a build failure.

**Where:** `src/configParser/ConfigLocationAttri.cpp`, `parseLocPath()`, the trailing-`/` loop. It's now indented with spaces like the lines around it.

## 7. Wrong line numbers after a location block

**Reason:** `parseLocation()` received `lineNum` by value, so lines read inside a location block were never added to the server's count. A later error was reported several lines too early.

**Where:** `src/configParser/Config.cpp` and `inc/Config.hpp`. `parseLocation()` now takes `int& lineNum`.

## 8. Nested `location` silently changed the path

**Reason:** `location /b` inside `location /a { }` was treated as a normal directive and replaced the path, so `/a` disappeared.

**Where:** `src/configParser/Config.cpp`.
- `parseLocation()` now reads the `location <path>` header line itself.
- `parseLocLine()` throws "nested location blocks are not allowed".

## 9. Location `index` could escape the root

**Reason:** `index ../../etc/passwd;` was accepted. The router builds `fsPath + '/' + index`, so this could serve files outside the root.

**Where:** `src/configParser/ConfigLocationAttri.cpp`, `parseLocIndex()`. It now rejects any `/` or `..`, matching the server-level `index`.

## 10. `error_page` with two paths

**Reason:** in `error_page 404 /a.html /b.html;` the second path was silently ignored.

**Where:** `src/configParser/ConfigServerAttri.cpp`, `parseErr()`. It now throws "error_page takes only one path" if anything follows the path.

## 11. `..` in location paths

**Reason:** `location /../a` was accepted.

**Where:** `src/configParser/ConfigLocationAttri.cpp`, `parseLocPath()`. It now rejects any path for which `hasDotDotSegment()` is true. `/a..b` is still allowed, because `..` there isn't a separate segment.

## 12. Duplicate location paths

**Reason:** two `location /a` blocks were both accepted, but `findLocation()` would only ever use the first.

**Where:** `src/configParser/Config.cpp`, `parseLocation()`. After parsing the header, it compares the path with the locations already added and throws "duplicate location".

## 13. `upload on` without `upload_path`

**Reason:** uploads could be enabled without anywhere to store the files.

**Where:** `src/configParser/Config.cpp`, end of `parseLocation()`. It throws "'upload on' requires 'upload_path'".

## 14. `redirect` accepted anything

**Reason:** `redirect ???;` was accepted and would have produced a broken `Location:` header.

**Where:** `src/configParser/ConfigLocationAttri.cpp`, `parseRedirect()`. The value must start with `/`, `http://` or `https://` and may only contain URL characters.

## 15. Misleading error for `location a`

**Reason:** `parseServer()` only recognised a location block if the line started with exactly `"location /"`. As a result:
- `location a` and `location<TAB>/a` were treated as server directives and reported "Unknown directive 'location'".
- `location<TAB>/a` was rejected even though it's valid.

**Where:** `src/configParser/Config.cpp`, `parseServer()`. It now checks whether the line's first word is `location`. Bad paths get "location path must start with '/'", and tabs work.

## 16. Path traversal in request URLs (security)

**Reason:** nothing rejected `..` in the request path. `GET /../../etc/passwd` became `fsPath = www/../../etc/passwd` and could serve files outside `www`. `curl` removes `..` itself, but a raw request sent with `nc` does not.

**Where:** `src/serverProcess/router.cpp`, start of `validateRouteStat()`. It returns `BadRequest` (400) if the path is empty, doesn't start with `/`, or contains a `..` segment (`hasDotDotSegment()`).

**Notes:**
- `%2e%2e` (an encoded `..`) is currently safe, because the server never decodes `%xx` sequences. If URL decoding is added later, this check must run after the decoding.
- You won't see the 400 in practice until the hardcoded `www/index.html` / `www/uploads.html` check in `validateRequestLine()` (`src/parseRL.cpp`) is removed, because that check runs first.

---

## Still open

The whole project doesn't build yet, because of errors in `src/serverProcess/CGI.cpp`. None of these changes caused them; see `TODO.md`.
