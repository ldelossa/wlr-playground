LIBS = \
	   -lwayland-client

main: main.o
	gcc -g3 -o main main.o $(LIBS)
