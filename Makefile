include config.mk


SRC = server.c utils.c io.c log.c handler.c
OBJ = ${SRC:.c=.o}


all: options grockepoll


options:
	@echo server build options
	@echo "CFLAGS    = ${CFLAGS}"
	@echo "CPPFLAGS  = ${CPPFLAGS}"
	@echo "LDLAGS    = ${LDLAGS}"
	@echo "CC        = ${CC}"


config.h:
	cp config.def.h config.h

.c.o:
	${CC} -o $@ -c ${CFLAGS} $<


${OBJ}: config.mk config.h


grockepoll: ${OBJ}
	${CC} -static -o $@ ${OBJ}


clean:
	rm -f server ${OBJ}


.PHONY: all options
