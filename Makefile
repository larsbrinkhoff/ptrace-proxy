CC = gcc
CFLAGS = -g -Wall
LDFLAGS =

OBJS = main.o proxy.o ptrace.o sysdep.o wait.o

all: ptproxy child

clean:
	rm -f *.o core child ptproxy

ptproxy: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o ptproxy

child: child.o
