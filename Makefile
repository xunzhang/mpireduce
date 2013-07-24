SRC_PATH=./src
TEST_PATH=./test

AR=ar
CC=mpicc
CFLAGS=-fPIC -Wall
CPPFLAGS=-I./include

main: 
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $(SRC_PATH)/treereduce.c -o treereduce.o
	$(AR) rcs treereduce.a treereduce.o

test: ex1.o ex2.o

ex1.o:
	$(CC) -o ex1 $(CPPFLAGS) $(TEST_PATH)/ex1.c treereduce.a	

ex2.o:
	$(CC) -o ex2 $(CPPFLAGS) $(TEST_PATH)/ex2.c treereduce.a

.PHONY:
	clean

clean:
	-rm ex* *.a *.o 
