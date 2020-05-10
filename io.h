
#ifndef IO_H
#define IO_H

#include <time.h>

#define MAX_REQ_SIZE 1024 * 4


enum io_step_status {IO_OK, IO_AGAIN, IO_ERROR};
enum io_step_type {S_READ, S_WRITE, S_SENDFILE};
enum conn_status {C_RUN, C_CLOSE};


struct send_meta {
    char *data;
    size_t size, offset;
};


struct sendfile_meta {
    off_t start_offset, end_offset, size;
    int fd;
};


struct read_meta {
    char data[MAX_REQ_SIZE];
    size_t size;
};


struct connection;

struct io_step {
    void *meta;
    enum conn_status (*handler)(struct connection *conn);
    enum io_step_type type;
    struct io_step *next;
};


struct connection {
    int fd;
    enum conn_status status;
    time_t last_active;
    char ip[16];
    struct io_step *steps;
    struct connection *next;
    struct connection *prev;
};


void cleanup_steps(struct io_step *head);

void process_connection(struct connection *conn);

void setup_read_io_step(struct io_step **steps,
                        enum conn_status (*handler)(struct connection *conn));

void setup_write_io_step(struct io_step **steps,
                         char *data, size_t size,
                         enum conn_status (*handler)(struct connection *conn));

void setup_sendfile_io_step(struct io_step **steps,
                            int fd, off_t lower, off_t upper, off_t size,
                            enum conn_status (*handler)(struct connection *conn));


#endif