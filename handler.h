
#ifndef HANDLER_H
#define HANDLER_H

#include "io.h"

enum conn_status build_response(struct connection *conn);
void init_handler(const char *conf_root_dir, const char *conf_hostname, int conf_port, int conf_chroot);

#endif
