CC = gcc
CFLAGS = -Wall -pthread -Wunused-function

SRC = main.c \
      pos_sockets/passive_socket.c \
      pos_sockets/char_buffer.c \
      pos_sockets/active_socket.c \
      connect4/connect4.c \
      connect4/structures/hra.c \
      connect4/structures/thread_data.c \

OBJ = $(SRC:.c=.o)
TARGET = SemestralnaPracaServer

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJ) $(TARGET)
