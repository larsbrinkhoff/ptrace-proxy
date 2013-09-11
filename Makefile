CC = gcc
CFLAGS = -g -Wall

OBJS = main.o proxy.o ptrace.o sysdep.o wait.o

all: ptproxy child

clean:
	rm -f *.o core child ptproxy

ptproxy: $(OBJS)
	gcc $(OBJS) -o ptproxy

child: child.o
