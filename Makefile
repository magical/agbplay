CXX = g++
CXXFLAGS = -Wall -Wextra -Wconversion -Wunreachable-code -std=c++0x -D NDEBUG -O3
#CXXFLAGS = -Wall -Wextra -Wconversion -Wunreachable-code -std=c++0x -Og -g
BINARY = agbplay
BASE_LIBS = -lm -lncursesw -lboost_system -lboost_thread -lboost_filesystem -pthread -lsndfile

SYS = $(shell $(CXX) -dumpmachine)

ifneq (, $(findstring linux, $(SYS)))
	# clang doesn't seem to compile correctly on windows but on linux it works
	# use g++ for now instead of clang++, OpenMP seems to be a bit tricky with clang
	CXX = g++
	LIBS = -lportaudio $(BASE_LIBS) -lasound
else ifneq (, $(findstring cygwin, $(SYS))$(findstring windows, $(SYS)))
	LIBS = ../portaudio/lib/.libs/libportaudio.dll.a $(BASE_LIBS)
	CXXFLAGS += -D_GLIBCXX_USE_C99=1 -D_BSD_SOURCE
else
	# put something else here
endif

IMPORT = 

GREEN = \033[1;32m
RED = \033[1;31m
BROWN = \033[1;33m
WHITE = 
NCOL = \033[0m

SRC_FILES = $(wildcard src/*.cpp)
OBJ_FILES = $(addprefix obj/,$(notdir $(SRC_FILES:.cpp=.o)))

.PHONY: all
all: $(BINARY)

.PHONY: clean
clean:
	@printf "[$(BROWN)Cleaning$(NCOL)] $(WHITE)$(OBJ_FILES)$(NCOL)\n"
	@rm -f $(OBJ_FILES)

$(BINARY): $(OBJ_FILES)
	@printf "[$(RED)Linking$(NCOL)] $(WHITE)$(BINARY)$(NCOL)\n"
	@gcc -o $@ $(CXXFLAGS) $^ $(LIBS) -lstdc++

obj/%.o: src/%.cpp src/*.h
	@printf "[$(GREEN)Compiling$(NCOL)] $(WHITE)$@$(NCOL)\n"
	@$(CXX) -c -o $@ $< $(CXXFLAGS) $(IMPORT)

