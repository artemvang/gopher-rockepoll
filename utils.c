#include <stdlib.h>
#include <netinet/tcp.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <err.h>

#include "utils.h"


inline void *
xmalloc(const size_t size)
{
    void *ptr = malloc(size);

    if (!ptr) {
        err(1, "malloc(), can't allocate %zu bytes", size);
    }

    return ptr;
}

inline void *
xrealloc(void *ptr, const size_t size)
{
    ptr = realloc(ptr, size);
    if (!ptr) {
        err(1, "realloc(), can't reallocate %zu bytes", size);
    }

    return ptr;
}


inline void
xchdir(const char *dir)
{
    if (chdir(dir) < 0) {
        err(1, "chdir(), `%s'", dir);
    }
}


inline void
xchroot(const char *dir)
{
    if (chroot(dir) < 0) {
        err(1, "chroot(), `%s'", dir);
    }
}


int
create_listen_socket(const char *listen_addr, int port)
{
    int    opt, listenfd;
    struct sockaddr_in addr;

    if ((listenfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) < 0) {
        err(1, "socket()");
    }

    opt = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt))) {
        err(1, "setsockopt(), SOL_SOCKET, SO_REUSEPORT");
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(listen_addr);

    if (bind(listenfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        err(1, "bind(), `%d'", port);
    }

    if (listen(listenfd, -1) < 0) {
        err(1, "listen()");
    }

    return listenfd;
}
