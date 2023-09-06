BIN = hello_thread

OBJ = hello_thread.o

CC = g++

CFLAGS = -g -Wall -MMD -D_REENTRANT

all: $(BIN) 

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -std=c++14 -pthread -lm -o $(BIN)   $(OBJ)

include $(wildcard *.d)

.PHONY : clean

clean:
	rm -f $(BIN) *.o *.d
	#rm -f *~ *.dat *.log \#*\# *.cache 
