CC=g++48
CPPS=$(wildcard *.cpp)
OBJS=$(CPPS:.cpp=.o)
CC_FLAGS=-libstd=libstdc++ -std=c++11 -lpthread -Wl,--no-as-needed -O3
LD_FLAGS=-lc -lpthread -Wl,--no-as-needed


all: $(EXEC)
	cp ./$(EXEC) ../bin

$(EXEC): $(OBJS)
	$(CC) $(LD_FLAGS) $(OBJS) -o $(EXEC)

.cpp.o:
	$(CC) $(CC_FLAGS) -c $< -o $@

clean:
	-rm -f $(OBJS) $(EXEC)
