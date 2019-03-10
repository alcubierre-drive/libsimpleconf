SRC=libsimpleconf.c
OBJ=libsimpleconf.o
SHARED=libsimpleconf.so
STATIC=libsimpleconf.a
HEADER=libsimpleconf.h

.PHONY: all clean install

all: $(STATIC) $(SHARED)

$(OBJ):
	$(CC) -Wall -Wextra -fpic -c $(SRC)

$(STATIC): $(OBJ)
	$(AR) r $(STATIC) $(OBJ)

$(SHARED): $(OBJ)
	$(CC) -shared -o $(SHARED) $(OBJ)

clean:
	$(RM) $(STATIC) $(OBJ) $(SHARED)

#PREFIX is environment variable, but if it is not set, then set default value
ifeq ($(PREFIX),)
    PREFIX := /usr/
endif

install: $(SHARED) $(STATIC) $(HEADER)
	install -d $(PREFIX)/lib/
	install -m 644 $(STATIC) $(PREFIX)/lib/
	install -m 644 $(SHARED) $(PREFIX)/lib/
	install -d $(PREFIX)/include/
	install -m 644 $(HEADER) $(PREFIX)/include/
