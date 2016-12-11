EXECUTABLE_NAME=Sorts
CPP=g++
INC=
CPPFLAGS=-Wall -Wextra -Werror -Wshadow -pedantic -Ofast -std=gnu++14 -fomit-frame-pointer -mavx2 -march=native -mfma -fpeel-loops -ftracer -ftree-vectorize
LIBS=
CPPSOURCES=$(wildcard *.cpp)

OBJECTS=$(CPPSOURCES:.cpp=.o)

.PHONY : all
all: $(CPPSOURCES) $(EXECUTABLE_NAME)

$(EXECUTABLE_NAME) : $(OBJECTS)
	$(CPP) $(CPPFLAGS) $(OBJECTS) $(PROFILE) -o $@ $(LIBS)

%.o:%.cpp
	$(CPP) -c $(INC) $(CPPFLAGS) $(PROFILE) $< -o $@

.PHONY : clean
clean:
	rm -rf *.o $(EXECUTABLE_NAME)
