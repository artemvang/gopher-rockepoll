# server version

VERSION = 0.1

CPPFLAGS = -DVERSION=\"$(VERSION)\" -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE
CFLAGS   = -std=c99 -pedantic -Wall -Wno-deprecated-declarations -Wextra -Os ${CPPFLAGS}

CC       = gcc
