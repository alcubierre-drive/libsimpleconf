SRC=libsimpleconf.c
OBJ=libsimpleconf.o
SHARED=libsimpleconf.so
STATIC=libsimpleconf.a

all: $(STATIC) $(SHARED)

$(OBJ):
	$(CC) -Wall -Wextra -fpic -c $(SRC)

$(STATIC): $(OBJ)
	$(AR) r $(STATIC) $(OBJ)

$(SHARED): $(OBJ)
	$(CC) -shared -o $(SHARED) $(OBJ)
