DIR_OBJ = ./obj
DIR_BIN = ./bin

OBJ_C = $(wildcard ${DIR_OBJ}/*.cpp)
OBJ_O = $(patsubst %.cpp,${DIR_BIN}/%.o,$(notdir ${OBJ_C}))

TARGET = DLP-basic-server
#BIN_TARGET = ${DIR_BIN}/${TARGET}

#CC = gcc -std=c99
CC = g++

DEBUG = -g -O0 -pthread -Wall 
CFLAGS += $(DEBUG)

CXXFLAGS += -D_GNU_SOURCE -lm -pthread -c -Wall $( shell pkg-config --cflags opencv)
LDFLAGS += -lm $(shell pkg-config --libs --static opencv)

${TARGET}:${OBJ_O}
	$(CC) $(CFLAGS) $(OBJ_O) -o $@ $(LDFLAGS)

${DIR_BIN}/%.o : $(DIR_OBJ)/%.cpp
	$(CC) $(CFLAGS) -c  $< -o $@ $(CXXFLAGS)
	

clean:
	rm $(DIR_BIN)/*.* 
	rm $(TARGET) 


