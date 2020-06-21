BASENET_INC = ../somo_server_netbase/include
BASENET_LIB = ../somo_server_netbase/build
LIB3RD = ./3rd

CC = g++
INC = -I include -I src -I $(BASENET_INC)
STDLIB = -lz -lcrypto -lrt -lpthread -ldl
LIB = $(BASENET_LIB)/libnetbase.a $(LIB3RD)/libuv/lib/libuv.a

BUILD_TIME = `date "+%Y-%m-%d_%H:%M:%S"`

FLAG_RELEASE = -std=c++0x -Wall -O2 -Wno-deprecated -DBUILD_TIME=\"$(BUILD_TIME)\"
FLAG_DEBUG = -std=c++0x -Wall -g -Wno-deprecated -DBUILD_TIME=\"$(BUILD_TIME)\"
FLAG = $(FLAG_DEBUG)
#FLAG = $(FLAG_RELEASE)

SRC_DIR = ./src
SRC_RTMP_DIR = ./src/rtmp
SRC_COMMON_DIR = ./src/common
OBJ_DIR = ./build/objs
BIN_DIR = ../../bin/videoproxy
PROCESS = somomcu

all: $(PROCESS)

OBJECT = $(OBJ_DIR)/app.o $(OBJ_DIR)/server.o $(OBJ_DIR)/util.o $(OBJ_DIR)/logger.o $(OBJ_DIR)/buffer.o	\
	$(OBJ_DIR)/session.o $(OBJ_DIR)/protocol.o $(OBJ_DIR)/connection.o $(OBJ_DIR)/rtmpprotocol.o 	\
	$(OBJ_DIR)/rtmpserver.o $(OBJ_DIR)/rtmpconnection.o $(OBJ_DIR)/rtmpsession.o $(OBJ_DIR)/rtmpformat.o $(OBJ_DIR)/rtmpshakehands.o $(OBJ_DIR)/rtmpamf.o	$(OBJ_DIR)/rtmpbuffer.o

$(OBJ_DIR)/app.o : $(SRC_DIR)/app.cpp $(SRC_DIR)/app.h
	$(CC) -c $(FLAG) $(SRC_DIR)/app.cpp -o $(OBJ_DIR)/app.o $(INC)

$(OBJ_DIR)/server.o : $(SRC_DIR)/server.cpp $(SRC_DIR)/server.h
	$(CC) -c $(FLAG) $(SRC_DIR)/server.cpp -o $(OBJ_DIR)/server.o $(INC)

$(OBJ_DIR)/util.o : $(SRC_COMMON_DIR)/util.cpp $(SRC_COMMON_DIR)/util.h
	$(CC) -c $(FLAG) $(SRC_COMMON_DIR)/util.cpp -o $(OBJ_DIR)/util.o $(INC)

$(OBJ_DIR)/logger.o : $(SRC_COMMON_DIR)/logger.cpp $(SRC_COMMON_DIR)/logger.h
	$(CC) -c $(FLAG) $(SRC_COMMON_DIR)/logger.cpp -o $(OBJ_DIR)/logger.o $(INC)	

$(OBJ_DIR)/buffer.o : $(SRC_COMMON_DIR)/buffer.cpp $(SRC_COMMON_DIR)/buffer.h
	$(CC) -c $(FLAG) $(SRC_COMMON_DIR)/buffer.cpp -o $(OBJ_DIR)/buffer.o $(INC)

$(OBJ_DIR)/session.o : $(SRC_DIR)/session.cpp $(SRC_DIR)/session.h
	$(CC) -c $(FLAG) $(SRC_DIR)/session.cpp -o $(OBJ_DIR)/session.o $(INC)

$(OBJ_DIR)/protocol.o : $(SRC_DIR)/protocol.cpp $(SRC_DIR)/protocol.h
	$(CC) -c $(FLAG) $(SRC_DIR)/protocol.cpp -o $(OBJ_DIR)/protocol.o $(INC)

$(OBJ_DIR)/connection.o : $(SRC_DIR)/connection.cpp $(SRC_DIR)/connection.h
	$(CC) -c $(FLAG) $(SRC_DIR)/connection.cpp -o $(OBJ_DIR)/connection.o $(INC)

$(OBJ_DIR)/rtmpprotocol.o : $(SRC_DIR)/rtmpprotocol.cpp $(SRC_DIR)/rtmpprotocol.h
	$(CC) -c $(FLAG) $(SRC_DIR)/rtmpprotocol.cpp -o $(OBJ_DIR)/rtmpprotocol.o $(INC)

$(OBJ_DIR)/rtmpserver.o : $(SRC_RTMP_DIR)/rtmpserver.cpp $(SRC_RTMP_DIR)/rtmpserver.h
	$(CC) -c $(FLAG) $(SRC_RTMP_DIR)/rtmpserver.cpp -o $(OBJ_DIR)/rtmpserver.o $(INC)

$(OBJ_DIR)/rtmpconnection.o : $(SRC_RTMP_DIR)/rtmpconnection.cpp $(SRC_RTMP_DIR)/rtmpconnection.h
	$(CC) -c $(FLAG) $(SRC_RTMP_DIR)/rtmpconnection.cpp -o $(OBJ_DIR)/rtmpconnection.o $(INC)

$(OBJ_DIR)/rtmpsession.o : $(SRC_RTMP_DIR)/rtmpsession.cpp $(SRC_RTMP_DIR)/rtmpsession.h
	$(CC) -c $(FLAG) $(SRC_RTMP_DIR)/rtmpsession.cpp -o $(OBJ_DIR)/rtmpsession.o $(INC)

$(OBJ_DIR)/rtmpformat.o : $(SRC_RTMP_DIR)/rtmpformat.cpp $(SRC_RTMP_DIR)/rtmpformat.h
	$(CC) -c $(FLAG) $(SRC_RTMP_DIR)/rtmpformat.cpp -o $(OBJ_DIR)/rtmpformat.o $(INC)

$(OBJ_DIR)/rtmpshakehands.o : $(SRC_RTMP_DIR)/rtmpshakehands.cpp $(SRC_RTMP_DIR)/rtmpshakehands.h
	$(CC) -c $(FLAG) $(SRC_RTMP_DIR)/rtmpshakehands.cpp -o $(OBJ_DIR)/rtmpshakehands.o $(INC)

$(OBJ_DIR)/rtmpamf.o : $(SRC_RTMP_DIR)/rtmpamf.cpp $(SRC_RTMP_DIR)/rtmpamf.h
	$(CC) -c $(FLAG) $(SRC_RTMP_DIR)/rtmpamf.cpp -o $(OBJ_DIR)/rtmpamf.o $(INC)

$(OBJ_DIR)/rtmpbuffer.o : $(SRC_RTMP_DIR)/rtmpbuffer.cpp $(SRC_RTMP_DIR)/rtmpbuffer.h
	$(CC) -c $(FLAG) $(SRC_RTMP_DIR)/rtmpbuffer.cpp -o $(OBJ_DIR)/rtmpbuffer.o $(INC)

$(PROCESS) : $(OBJECT) $(LIB) main.cpp
	$(CC) $(FLAG) -o $@ main.cpp $(INC) $(OBJECT) $(LIB) $(STDLIB)

.PHONY: deploy
.PHONY: clean

deploy:
	make
	cp $(PROCESS) $(BIN_DIR)
	cp server.xml $(BIN_DIR)
	cp server-dev.xml $(BIN_DIR)

clean :
	rm $(OBJECT) -f
	rm $(PROCESS) -f
