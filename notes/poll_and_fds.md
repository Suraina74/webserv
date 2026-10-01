# poll(), listen fds and client fds

## Two kinds of fds

- **Listen fd**: created by `createSockAddr` (socket → bind → listen). There is one per server block and it stays open for the whole program. It never carries request or response data. `POLLIN` on it only means a new connection is waiting to be `accept()`ed.
- **Client fd**: returned by `accept()`, one per connection. **Both** the request and the response go over this fd. It's the key in `map<int, Client> clients`.

```
listen fd   →  POLLIN  →  accept()                 (connections only)
client fd   →  POLLIN  →  recv()   the request
            →  POLLOUT →  send()   the response
```

## Order of events

`poll()` reacts before `accept()`. There are two separate `poll()` reactions per client:

1. **Listen fd gets `POLLIN`**: call `accept()`, set the new fd non-blocking, add it to the pollfd array with `events = POLLIN`, and create its `Client`. Don't `recv()` yet.
2. **Client fd gets `POLLIN`** (on a later `poll()` call): call `recv()` to read the request.

Never call `recv()` or `send()` without `poll()` saying the fd is ready. The subject requires this, and the data often hasn't arrived yet anyway.

The listen fd and the client are linked only once, at `accept()` time:

```cpp
int cfd = accept(listenFd, NULL, NULL);
clients[cfd] = Client(cfd, &servers[listenFds[listenFd]]);
```

## Reading and sending on the same fd

Switch what `poll()` watches by changing the fd's `events`. A client is only in one phase at a time, so the two phases never overlap:

```
accept()           → events = POLLIN    (reading phase)
recv() ... recv()  → request incomplete, stay on POLLIN
request complete   → build response, events = POLLOUT   (sending phase)
send() ... send()  → partial send, stay on POLLOUT
all bytes sent     → close fd   (or events = POLLIN again for keep-alive)
```

Each `Client` keeps its own progress (bytes read in its `Request`, bytes sent in its `Response`), so many clients can be at different stages at the same time without affecting each other.

## pollfd reference

```cpp
struct pollfd {
    int   fd;       // fd to watch
    short events;   // what you want to know about (you set this)
    short revents;  // what happened (poll overwrites this every call)
};
```

| Flag | Meaning |
|---|---|
| `POLLIN` | Data is ready to read. On a listen fd: a connection is waiting |
| `POLLOUT` | You can write without blocking |
| `POLLERR` | Error on the fd (always reported) |
| `POLLHUP` | The other side hung up (always reported) |
| `POLLNVAL` | The fd isn't open, usually a bug (always reported) |

- Check flags with `&`, not `==`.
- `poll()` writes into the array you pass it, so a copy of a `pollfd` stored elsewhere (e.g. inside `Client`) never gets updated. Store `int fd` in `Client`.
- Handle `POLLERR | POLLHUP | POLLNVAL` before the read and write branches. Otherwise a dropped client stays in the array and `poll()` keeps returning immediately for it.
