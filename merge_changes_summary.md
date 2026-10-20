Merge of http_parser into WenxuanDev - summary of changes (formatting changes excluded)
=====================================================================================

Merge resolution
----------------
src/ServerInfo.cpp (Validated by Wenxuan)
  - Took the WenxuanDev version.
  - Dropped http_parser's old eventLoop, createSockAddr, server, setResult, setFd,
    getResult and getFd. WenxuanDev already replaced them in src/serverProcess/.

Client
------
src/Client.cpp (Need to be checked by Suraina to see if there were major changes to her code)
  - Added Client::receiveRequest() and Client::sendResponse(), moved from
    http_parser's ServerInfo.cpp. Member names were adapted to the Client class
    (_fd, _request, _response, _responsesString).
  - receiveRequest now takes a ServerInfo &eloop parameter. It calls
    processRoute(*this, eloop, route) after parseBody() and before the response is
    composed, in both the chunked and the Content-Length paths. Otherwise routing
    would happen after the response was already built and would have no effect.
  - Added includes: main.hpp, <sys/socket.h>, <cstdio>.

inc/Client.hpp (Validated by Wenxuan)
  - Added the forward declaration "class ServerInfo;" to avoid a circular include.
  - Changed the declaration to receiveRequest(ServerInfo &eloop).

Server loop
-----------
src/serverProcess/eventLoop.cpp (Validated by Wenxuan, further check need by Suraina and Alisha)
  - Receiving: calls client.receiveRequest(eloop). Removes the client on
    PARSER_ERROR or 0, and switches to POLLOUT on RESPONSE_READY. The parseBody()
    and processRoute calls were removed here; they now run inside the client.
  - Sending: calls client.sendResponse().

src/serverProcess/serverManager.cpp
  - Deleted the old standalone receiveRequest(int, Request&) and
    sendResponse(int, Response&).

inc/main.hpp
  - Removed the declarations of those two functions.

Request
-------
inc/Request.hpp
  - The class declarations now match Request.cpp: getters are const, and getBody()
    and setStatus() are declared (router.cpp and CGI.cpp need them).
  - Kept everything only http_parser had: contentTypeMap, bodyHeaderMap,
    makeMapOfHeader, checkOtherHeaders, resetParserVars, etc.

src/Request.cpp
  - setStatusText now handles Forbidden (403) and has a fallback return after the
    switch.
  - getPath, getMethod, getStatusText, getFullRequest and getRequestTillHeaders are
    now const.

Response
--------
inc/Response.hpp
  - Getters are const. resetResponseVars() is kept.

src/Response.cpp
  - composeResponse now closes the fd when read() fails (it used to leak).
  - getFullResponse() is const.

Header parsing (src/parseHeaders.cpp)
-------------------------------------
  - Fixed the unbalanced braces left by the merge, which stopped the file compiling.
  - actionsOnValue no longer reads out of bounds when a header value is empty or
    all spaces.
  - validateBoundary now checks for an empty boundary before calling
    boundary.back().
  - parseHeaders: the \r\n counting loop no longer reads past the end. The value is
    now taken with line.substr(findColon + 1), which gives the same result as
    before.
  - checkOtherHeaders:
      - The "contentLength < 0" check could never be true (contentLength is
        unsigned). It now checks whether reading the number failed, so a
        Content-Length that overflows returns 400.
      - A redundant "else if (!empty())" became "else".
  - checkIfDoubleHeader now uses map.count() and takes the map by const reference
    instead of copying it.

Still not compiling
-------------------
  - src/serverProcess/routerUtils.cpp: unused parameters in the empty run*
    functions.
  - src/serverProcess/CGI.cpp: two unused parameters and a bad map.insert call.

Behaviour that has not changed
------------------------------
  - The event loop ignores REQUEST_READY. When one read contains a second request,
    its response waits until more data arrives.
  - Clients are still disconnected after one full response.
