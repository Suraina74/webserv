*This project has been created as part of the 42 curriculum by ksoedama, schabboe, wxi*

# Description
Webserve project builds a basic HTTP web server from scratch in C++. It is a recreation of a simplified version of servers like Nginx or Apache. 
A general order of client-server communication process:
```
HTTP request from a client ─ ─ > TCP socket ─ ─ > Server
         ꓥ                                          |
         |                                          |
         |                                          V
Send back to client                        Process HTTP request
         |                                          |
         |                                          |
         |                                          V
         ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ Parse into configs
```

Client sends HTTP request over TCP
A browser or curl opens a TCP connection to your server's IP:port and writes raw bytes onto the socket — the HTTP request line, headers, and (if present) a body. At this stage it's just a stream of bytes; your server hasn't looked at any of it yet. This is the client side of the socket() -> connect() pair — your server is the one that already called socket(), bind(), and listen() on that port earlier, during startup.

Server accepts the connection and reads it
Your server's main loop is watching its listening socket(s) with something like poll()/select()/epoll(). When a client connects, accept() hands back a new file descriptor dedicated to that one connection. The server then calls recv() (or read()) on that fd to pull the bytes the client sent into a buffer. Because HTTP can arrive in chunks across multiple recv() calls, most webserv implementations keep reading (often buffering per-client) until they see the end of the headers (a blank line, \r\n\r\n) and, if needed, the full body length indicated by Content-Length or a chunked encoding terminator.

Server processes the HTTP request
Once the raw request is fully buffered, it gets parsed into a structured request object: method (GET/POST/DELETE), path, HTTP version, headers (as a map), and body. This is the equivalent of your Location::parseLocLine-style directive parsing, but applied to an incoming request instead of a config file — you're tokenizing a line, validating it, and populating an object's members. From there the server does the actual work: matching the request path against the config it parsed at startup, checking allowed methods, serving a static file, running CGI, handling an upload, or returning a redirect/error page.

Parse into configs (this happens at startup, feeding step 3)
This is where your Config::parse(), Config::parseServer(), Config::parseLocation(), and Config::parseLocLine() come in — but importantly, this happens once, when the server boots, not per-request. parse() opens the config file and loops line by line; parseServer() consumes one server{} block, building a ServerConfig; parseLocation() consumes one location{} block inside it, building a Location; parseLocLine()/parseLine() dispatch each directive line (methods, root, cgi, etc.) to the right setter. The result — your vector<ServerConfig> _servers — is what step 3 consults on every incoming request to decide how to handle it (which root, which methods are allowed, whether to redirect, etc.).

Server sends the response back to the client
After processing, the server builds an HTTP response (status line, headers like Content-Length and Content-Type, and body) and writes it back on the same connection's fd using send() (or write()). Depending on the Connection header and HTTP version, the socket is then either kept open for further requests (keep-alive) or closed. This is the arrow going back up to the client in your diagram — same TCP socket, just traffic flowing the other direction.

# Instructions

To start the program: send terminal input
______________________
make
./webserv filename.config
______________________

# What does a server structure look like in its config
Here is a complete example that covers essentially all the functionality your Webserv subject asks for.
Config
│
├── ServerConfig
│   ├── listen
│   ├── host
│   ├── server_name
│   ├── root
│   ├── index
│   ├── error_page
│   ├── client_max_body_size
│   │
│   ├── Location
│   ├── Location
│   └── Location
│
└── ServerConfig
    ├── ...
    └── Locations
In ./config/standard.config we included a complete server config example.
the outer server{...} represents one website/server configuration. There could be multiple servers in a config file. 
- Listen tells your server which port to listen on. 
- Host specifies the interface/address the server should bind to.
- Server_name identifies the website. This becomes important if you have multiple servers.
- Root is the default filesystem location for this server.
- The index directive tells your server: If the requested resource is a directory, try this file.
- Client_max_body_size is specifically required so that an HTTP requestion wont exceed the configured limit.
- Error_page tells the server what page to return when an error occurs.
- Locations allows different URL paths of the same server to have different behavior/configuration.

# Understanding different functions
socket()  →  outlet installed, no wiring yet
bind()    →  wiring given a specific address
listen()  →  outlet is "live" and waiting for a plug
accept()  →  something plugs in, connection now active

# Resources
[NGINX](https://nginx.org/en/docs/beginners_guide.html)
[Beej's Guide to Network Programming](https://beej.us/guide/bgnet/html/#system-calls-or-bust)
[The Linux Programming inTerface](https://broman.dev/download/The%20Linux%20Programming%20Interface.pdf)
[rfc9110](https://datatracker.ietf.org/doc/html/rfc9110)
[rfc9112](https://datatracker.ietf.org/doc/html/rfc9112)
[HTTP:The Definitive Guide](https://github.com/oxidation99/MyBooks-1/blob/master/HTTP%20The%20%20Definitive%20Guide.pdf)

# How is AI used
