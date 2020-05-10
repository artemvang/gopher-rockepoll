
#define MAXFDS 128


#define DEFAULT_CONF_PORT         7000
#define DEFAULT_CONF_QUIET        0
#define DEFAULT_CONF_CHROOT       0
#define DEFAULT_CONF_HOSTNAME     "127.0.0.1"
#define DEFAULT_CONF_LISTEN_ADDR  "127.0.0.1"
#define DEFAULT_CONF_ROOT_DIR     "."


#define DEFAULT_MIMETYPE '9'


/* mime-types */
static const struct {
    char *ext;
    char type;
} mimes[] = {
    { "xhtml", 'h' },
    { "html",  'h' },
    { "htm",   'h' },
    { "xml",   '0' },
    { "js",    '0' },
    { "css",   '0' },
    { "txt",   '0' },
    { "md",    '0' },
    { "rst",   '0' },
    { "c",     '0' },
    { "svg",   '0' },
    { "py",    '0' },
    { "h",     '0' },
    { "png",   'I' },
    { "jpeg",  'I' },
    { "jpg",   'I' },
    { "bmp",   'I' },
    { "gif",   'g' },
    { "pdf",   'd' },
};
