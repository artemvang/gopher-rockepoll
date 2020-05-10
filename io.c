#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <fcntl.h>

#include "io.h"
#include "utils.h"
#include "utlist.h"

#define REQ_BUF_SIZE 1024
#define SENDFILE_CHUNK_SIZE 1024 * 512


#define BUILD_IO_STEP(steps, meta, step_type, handler)                        \
do {                                                                          \
    struct io_step *__step = xmalloc(sizeof(struct io_step));                 \
    __step->meta = meta;                                                      \
    __step->type = step_type;                                                 \
    __step->handler = handler;                                                \
    __step->next = NULL;                                                      \
    LL_APPEND(*steps, __step);                                                \
} while(0);


static enum io_step_status
make_sendfile_step(int fd, struct sendfile_meta *meta)
{
    off_t size;
    ssize_t sent_len;

    do {
        size = MIN(SENDFILE_CHUNK_SIZE, meta->size);
        sent_len = sendfile(fd, meta->fd, &meta->start_offset, size);
        if (sent_len < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return IO_AGAIN;
            }

            return IO_ERROR;
        }

        meta->size -= sent_len;
    } while (meta->start_offset < meta->end_offset);

    return IO_OK;
}


static enum io_step_status
make_write_step(int fd, struct send_meta *meta)
{
    ssize_t write_size;

    do {
        write_size = send(fd, meta->data + meta->offset, meta->size, 0);
        if (write_size < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return IO_AGAIN;
            }

            return IO_ERROR;
        }

        meta->offset += write_size;
    } while (meta->offset < meta->size);

    return IO_OK;
}


static enum io_step_status
make_read_step(int fd, struct read_meta *meta)
{
    ssize_t read_size, size;

    do {
        size = MIN(REQ_BUF_SIZE, MAX_REQ_SIZE - meta->size);
        read_size = read(fd, meta->data + meta->size, size);

        if (read_size < 1) {
            if (read_size < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                return IO_AGAIN;
            }

            return IO_ERROR;
        }

        meta->size += read_size;
    } while (read_size == REQ_BUF_SIZE && meta->size < MAX_REQ_SIZE);

    if (!meta->size || meta->size == MAX_REQ_SIZE) {
        return IO_ERROR;
    }
    meta->data[meta->size] = '\0';

    return IO_OK;
}


static enum io_step_status
make_step(int fd, struct io_step *step)
{
    enum io_step_status s;

    switch (step->type) {
    case S_READ:
        s = make_read_step(fd, step->meta);
        break;
    case S_WRITE:
        s = make_write_step(fd, step->meta);
        break;
    case S_SENDFILE:
        s = make_sendfile_step(fd, step->meta);
        break;
    }

    return s;
}


static void
cleanup_step(struct io_step *step)
{
    struct send_meta *s_meta;
    struct sendfile_meta *sf_meta;

    switch (step->type) {
    case S_READ:
        free(step->meta);
        break;
    case S_WRITE:
        s_meta = step->meta;
        free(s_meta->data);
        free(s_meta);
        break;
    case S_SENDFILE:
        sf_meta = step->meta;
        close(sf_meta->fd);
        free(sf_meta);
        break;
    }

    free(step);
}


inline void
cleanup_steps(struct io_step *head)
{
    struct io_step *step, *tmp_step;

    LL_FOREACH_SAFE(head, step, tmp_step) {
        cleanup_step(step);
    }
}


ALWAYS_INLINE void
setup_sendfile_io_step(struct io_step **steps,
                        int fd, off_t lower, off_t upper, off_t size,
                        enum conn_status (*handler)(struct connection *conn))
{
    struct sendfile_meta *meta = xmalloc(sizeof(struct sendfile_meta));
    meta->fd = fd;
    meta->start_offset = lower;
    meta->end_offset = upper;
    meta->size = size;

    BUILD_IO_STEP(steps, meta, S_SENDFILE, handler)
}


ALWAYS_INLINE void
setup_write_io_step(struct io_step **steps,
                    char *data, size_t size,
                    enum conn_status (*handler)(struct connection *conn))
{
    struct send_meta *meta = xmalloc(sizeof(struct send_meta));
    meta->data = data;
    meta->size = size;
    meta->offset = 0;

    BUILD_IO_STEP(steps, meta, S_WRITE, handler)
}


ALWAYS_INLINE void
setup_read_io_step(struct io_step **steps,
                   enum conn_status (*handler)(struct connection *conn))
{
    struct read_meta *meta = xmalloc(sizeof(struct read_meta));
    meta->size = 0;

    BUILD_IO_STEP(steps, meta, S_READ, handler)
}


void
process_connection(struct connection *conn)
{
    int run = 1;
    enum io_step_status s;
    struct io_step *steps_head, *step;

    while (run && conn->steps) {
        steps_head = step = conn->steps;
        s = make_step(conn->fd, steps_head);

        switch(s) {
        case IO_OK:
            if (*steps_head->handler &&
                (*steps_head->handler)(conn) == C_CLOSE)
            {
                run = 0;
            }

            /* cleanup IO step */
            cleanup_step(step);
            LL_DELETE(steps_head, step);

            if (!steps_head) {
                conn->status = C_CLOSE;
                run = 0;
            }
            break;
        case IO_AGAIN:
            run = 0;
            break;
        case IO_ERROR:
            conn->status = C_CLOSE;
            run = 0;
            break;
        }

        conn->steps = steps_head;
    }
}