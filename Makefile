SOURCE = jobCommander.c jobExecutorServer.c queue.c main.c
OBJS = jobCommander.o jobExecutorServer.o queue.o main.o
HEADER =myheaders.h queue.h
OUT = jobCommander
CC = gcc
FLAGS = -c -g
LIBS = -pthread

all: $(OBJS)
	$(CC) $(OBJS) -o $(OUT) $(LIBS)  

main.o : main.c $(HEADER)
	$(CC) $(FLAGS) main.c

jobCommander.o : jobCommander.c $(HEADER)
	$(CC) $(FLAGS) jobCommander.c

jobExecutorServer.o : jobExecutorServer.c $(HEADER)
	$(CC) $(FLAGS) jobExecutorServer.c

queue.o : queue.c $(HEADER)
	$(CC) $(FLAGS) queue.c

build : $(SOURCE)
	$(CC) -o $(OUT) $(SOURCE) $(LIBS)  # Include $(LIBS) to link with pthread library

clean : 
	rm -f $(OBJS) $(OUT) $(FILES)

rebuild : clean build