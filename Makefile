# GG Makefile
# This Makefile assumes you've installed boost in /usr/local/boost_1_30_0, and that
# you've built it using bjam (see boost build docs for explanation of bjam).
# If you've put boost somewhere else, change BOOST_DIR accordingly.

###############################
## Variables ##################
###############################
# Directories
BOOST_DIR = /usr/local/boost_1_30_0

# Includes
SDL_INC = /usr/include/SDL
FREETYPE_INC = /usr/include/freetype2
BOOST_INC = $(BOOST_DIR)

# Libs
BOOST_SIG_LIB = $(BOOST_DIR)/libs/signals/build/bin/libboost_signals.a/gcc/debug/runtime-link-dynamic
BOOST_FS_LIB = $(BOOST_DIR)/libs/filesystem/build/bin/libboost_filesystem.a/gcc/debug/runtime-link-dynamic
BOOST_REGEX_LIB = $(BOOST_DIR)/libs/regex/build/bin/libboost_regex.a/gcc/debug/runtime-link-dynamic

# Library name
LIBNAME  = GG.so

# Build commands
CC   = gcc -D__DEBUG__
CPP  = g++ -D__DEBUG__

# Lists of sources, objects, includes, and libs
CPP_SRCS = $(wildcard src/*.cpp) $(wildcard src/dialogs/*.cpp)
CC_SRCS = $(wildcard src/*.c) $(wildcard src/net/*.c)
ALL_SRCS = $(CPP_SRCS) $(CC_SRCS) $(NET_SRCS)

OBJS = $(addprefix objs/,$(notdir $(patsubst %.cpp,%.o,$(CPP_SRCS)) $(patsubst %.c,%.o,$(CC_SRCS))))

INCS =  -Iinclude  -Iinclude/net  -I/usr/include  -I/usr/include/GL  -I$(SDL_INC)  -I$(FREETYPE_INC)  -I$(BOOST_INC)
LIBS =  -L/usr/lib -L/usr/X11R6/lib -L$(BOOST_SIG_LIB) -L$(BOOST_FS_LIB) -L$(BOOST_REGEX_LIB) -lSDL -lSDL_image -lGL -lGLU -lfreetype -lboost_signals -lboost_filesystem -lboost_regex -lexpat -llog4cpp

# Build Flags
CFLAGS = $(INCS) -g3 -Wall
CXXFLAGS = $(INCS) -g3 -Wall


###############################
## Targets and Rules ##########
###############################
.PHONY: all clean install

all : library

clean : 
	/bin/rm -f $(OBJS) $(LIBNAME)

install :
	install $(LIBNAME) /usr/lib
	install -d /usr/include/GG
	install -d /usr/include/GG/net
	install include/*.h /usr/include/GG
	install include/net/*.h /usr/include/GG/net

library : libprep $(OBJS)
	$(CPP) -shared -o $(LIBNAME) $(OBJS) $(LIBS)

libprep :
	/bin/mkdir -p objs

objs/%.o : src/%.c
	$(CC) $< -c $(CFLAGS) -o $@

objs/%.o : src/net/%.c
	$(CC) $< -c $(CFLAGS) -o $@

objs/%.o : src/%.cpp
	$(CPP) $< -c $(CXXFLAGS) -o $@

objs/%.o : src/dialogs/%.cpp
	$(CPP) $< -c $(CXXFLAGS) -o $@

