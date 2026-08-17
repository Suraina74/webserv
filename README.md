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
