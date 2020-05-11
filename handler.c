#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "io.h"
#include "log.h"
#include "utils.h"
#include "handler.h"
#include "config.h"


#define LOG_MESSAGE_FORMAT "%s \"%s\" %d\n"


enum file_status {F_EXISTS, F_FORBIDDEN, F_NOT_FOUND, F_INTERNAL_ERROR};
enum gopher_status {G_OK, G_FILE_NOT_FOUND, G_BAD_REQUEST, G_FORBIDDEN, G_INTERNAL_ERROR};

char *gopher_status_str[] = {
    [G_OK]              = "OK",
    [G_FILE_NOT_FOUND]  = "Not found",
    [G_BAD_REQUEST]     = "Bad request",
    [G_FORBIDDEN]       = "Forbidden",
    [G_INTERNAL_ERROR]  = "Internal error"
};


struct file_meta {
    int fd;
    int is_directory;
    size_t size;
};


static const char *hostname;
static int port;


static void
log_new_connection(const struct connection *conn,
                   const char *request,
                   enum gopher_status status)
{
    log_log(&conn->last_active, LOG_MESSAGE_FORMAT, conn->ip, request, status);
}


static char
get_file_mimetype(const char *filename)
{
    size_t i;
    char mimetype = DEFAULT_MIMETYPE;
    char *extension = strrchr(filename, '.');

    if (!extension || extension == filename) {
        return mimetype;
    }

    extension++;

    i = sizeof(mimes) / sizeof(*mimes) - 1;
    while (i--) {
        if (!strcasecmp(extension, mimes[i].ext)) {
            mimetype = mimes[i].type;
            break;
        }
    }

    return mimetype;
}


static enum file_status
gather_file_meta(const char *target, struct file_meta *file_meta)
{
    int fd, gophermap_fd, is_dir;
    struct stat st_buf, gophermap_st_buf;
    char gophermap[1024 + sizeof("/gophermap")] = {0};

    fd = open(target, O_LARGEFILE | O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        return (errno == EACCES) ? F_FORBIDDEN : F_NOT_FOUND;
    }

    if (fstat(fd, &st_buf) < 0) {
        return F_INTERNAL_ERROR;
    }

    is_dir = S_ISDIR(st_buf.st_mode);

    if (!S_ISREG(st_buf.st_mode) && !is_dir) {
        return F_FORBIDDEN;
    }

    if (is_dir) {
        sprintf(gophermap, "%s/gophermap", target);

        gophermap_fd = open(gophermap, O_LARGEFILE | O_RDONLY | O_NONBLOCK);
        if (gophermap_fd > 0 &&
                !fstat(gophermap_fd, &gophermap_st_buf) &&
                S_ISREG(gophermap_st_buf.st_mode))
        {
            fd = gophermap_fd;
            st_buf = gophermap_st_buf;
            is_dir = 0;
        }
    }

    file_meta->fd = fd;
    file_meta->is_directory = is_dir;
    file_meta->size = st_buf.st_size;

    return F_EXISTS;
}


static void
build_file_list(const char *dir_name, struct connection *conn)
{
    char mime;
    struct dirent *de;
    DIR *dir;
    char *data;
    size_t size = 0;
    size_t total_size = 256;

    data = xmalloc(total_size);

    dir = opendir(dir_name);

    while ((de = readdir(dir))) {
        if (*(de->d_name) == '.') {
            continue;
        }
        while ((strlen(de->d_name) << 1) + size + 64 > total_size) {
            total_size <<= 1;
            data = xrealloc(data, total_size);
        }

        switch (de->d_type) {
        case DT_DIR:
            size += sprintf(data + size, "1%s\t/%s/%s\t%s\t%d\n", de->d_name, dir_name, de->d_name, hostname, port);
            break;
        case DT_REG:
            mime = get_file_mimetype(de->d_name);
            size += sprintf(data + size, "%c%s\t/%s/%s\t%s\t%d\n", mime, de->d_name, dir_name, de->d_name, hostname, port);
            break;
        }
    }

    setup_write_io_step(&conn->steps, data, size, NULL);
    closedir(dir);
}


static void
build_status_step(enum gopher_status st, struct connection *conn)
{
    char *data;
    size_t size;

    data = xmalloc(64);
    size = sprintf(data, "3%s\t\t\t\n", gopher_status_str[st]);

    setup_write_io_step(&conn->steps, data, size, NULL);
}


static char *
strip_request(char *raw_request)
{   
    char *request_end;
    char *request = raw_request;

    for (; *request == '/' || isspace(*request); request++);

    request_end = request + strlen(request) - 1;
    for (; *request_end == '/' || isspace(*request_end); request_end--);
    request_end[1] = '\0';

    if (*request == '\0') {
        request[0] = '.';
        request[1] = '\0';
    }

    return request;
}


void
init_handler(const char *conf_root_dir, const char *conf_hostname, int conf_port, int conf_chroot)
{
    xchdir(conf_root_dir);
    if (conf_chroot) {
        xchroot(conf_root_dir);
    }
    hostname = conf_hostname;
    port = conf_port;
}


enum conn_status
build_response(struct connection *conn)
{
    char *target;
    struct file_meta file_meta = {0};
    enum gopher_status gopher_status = G_OK;

    target = strip_request(((struct read_meta *)conn->steps->meta)->data);

    switch (gather_file_meta(target, &file_meta)) {
    case F_EXISTS:
        if (file_meta.is_directory) {
            build_file_list(target, conn);
            close(file_meta.fd);
        } else {
            setup_sendfile_io_step(
                &conn->steps,
                file_meta.fd, 0, file_meta.size, file_meta.size,
                NULL);
        }
        gopher_status = G_OK;
        break;
    case F_FORBIDDEN:
        gopher_status = G_FORBIDDEN;
        break;
    case F_INTERNAL_ERROR:
        gopher_status = G_INTERNAL_ERROR;
        break;
    case F_NOT_FOUND:
        gopher_status = G_FILE_NOT_FOUND;
        break;
    }

    if (gopher_status != G_OK) {
        build_status_step(gopher_status, conn);
    }

    log_new_connection(conn, target, gopher_status);

    return C_RUN;
}