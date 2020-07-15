DIR_OBJ_client = ./obj
DIR_BIN_client = ./bin

OBJ_C_client = $(wildcard ${DIR_OBJ_client}/*.cpp)
OBJ_O_client = $(patsubst %.cpp,${DIR_BIN_client}/%.o,$(notdir ${OBJ_C_client}))

TARGET_client = control-client
#BIN_TARGET_client = ${DIR_BIN_client}/${TARGET_client}

#CC = gcc -std=c99
CC = g++

DEBUG = -g -O0 -pthread -Wall 
CFLAGS += $(DEBUG)

CXXFLAGS += -D_GNU_SOURCE -lm -pthread -c -Wall $( shell pkg-config --cflags opencv)
LDFLAGS += -lm $(shell pkg-config --libs --static opencv)

${TARGET_client}:${OBJ_O_client}
	$(CC) $(CFLAGS) $(OBJ_O_client) -o $@ $(LDFLAGS)

${DIR_BIN_client}/%.o : $(DIR_OBJ_client)/%.cpp
	$(CC) $(CFLAGS) -c  $< -o $@ $(CXXFLAGS)
	
	
clean: 
	rm $(DIR_BIN_client)/*.* 
	rm $(TARGET_client) 
