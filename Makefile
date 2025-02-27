# Name : Huraimah Fatima
# Email : huraimah@uoguelph.ca
# SID: 1305776

CC = gcc
CFLAGS = -Wall -std=c11 -g -fPIC
LDFLAGS= -L.
INC = include
SRC = src/
BIN = bin

TARGET_LIB = $(BIN)/libvcparser.so  # target macro for the shared library
TARGET_EXE = $(BIN)/parser
SRCS = $(SRC)LinkedListAPI.c $(SRC)VCParser.c $(SRC)VCValidate.c $(SRC)VCParsingHelper.c #source files macro 
OBJS = $(SRCS:$(SRC)%.c=$(SRC)%.o) # object files macro 

all: parser lib

lib: $(OBJS) 
	@mkdir -p $(BIN)
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -o $(TARGET_LIB) $(OBJS)

parser: lib 
	@mkdir -p $(BIN)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TARGET_EXE) $(OBJS) $(TARGET_LIB) -Wl,-rpath,$(BIN)

$(SRC)%.o: $(SRC)%.c
	$(CC) $(CFLAGS) -I$(INC) -c $< -o $@

clean:
	rm -rf $(SRC)*.o $(TARGET_LIB) $(TARGET_EXE)