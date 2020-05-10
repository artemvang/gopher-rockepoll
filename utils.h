
#ifndef UTILS_H
#define UTILS_H


#define ALWAYS_INLINE inline __attribute__((always_inline))
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))


void *xmalloc(const size_t size);
void *xrealloc(void *ptr, const size_t size);
void xchdir(const char *dir);
void xchroot(const char *dir);
int create_listen_socket(const char *listen_addr, int port);

#endif