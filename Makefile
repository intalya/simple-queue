BIN_DIR = bin

default: all

qslib:
	g++ -c -fpic QueueServerLibrary.cpp

clean:
	rm -rf *.o bin/*
	mkdir -p bin

main:
	g++ QueueServerLibrary.o QueueServerMain.cpp -o $(BIN_DIR)/qs

test:
	g++ QueueServerLibrary.o test_producer.cpp -o $(BIN_DIR)/producer
	g++ QueueServerLibrary.o test_consumer.cpp -o $(BIN_DIR)/consumer

cs:
	g++ -pthread QueueServerLibrary.o Server.cpp -o $(BIN_DIR)/server
	g++ QueueServerLibrary.o Client.cpp -o $(BIN_DIR)/client


all: clean qslib main test cs

