
CC := gcc
CFLAGS := -O3

all: othello_player
	./othello_player

debug: othello_player.c
	$(CC) -o $@ $< -g

othello_player: othello_player.o
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.cu
	$(CC) -o $@ $< -c $(CFLAGS)

clean:
	rm -rf *.o
	rm -rf othello_player debug
