PROGNAME = md5model

CXX = g++

INCLUDES = -I/usr/X11/include -I/usr/X11/include/freetype2 -I/System/Library/Frameworks/AGL.framework/Versions/A/Headers

LIBS = -lboost_filesystem -lboost_system -lboost_thread -lboost_date_time -lGL -lGLU -lGLEW -lalut -lfreetype -lpng
#LIBS += -lX11 -framework AGL

BASE_CFLAGS = -std=c++11 -Wall -Woverloaded-virtual -pipe `sdl-config --cflags`
CFLAGS = $(BASE_CFLAGS) -g -gstabs+ -fno-default-inline -DDEBUG
#CFLAGS = $(BASE_CFLAGS) -O3 -finline-functions -ffast-math -fno-common -funroll-loops -DNDEBUG #-fomit-frame-pointer

CFLAGS += -msse3 -DUSE_SSE

LD_FLAGS = `sdl-config --libs`
LD_FLAGS += -L/usr/X11R6/lib

#CFLAGS += -pg
#LD_FLAGS += -pg

all: $(PROGNAME)

.PHONY: $(PROGNAME)
$(PROGNAME):
	$(MAKE) -C src PROGNAME="$(PROGNAME)" CXX="$(CXX)" INCLUDES="$(INCLUDES)" LIBS="$(LIBS)" CFLAGS="$(CFLAGS)" LD_FLAGS="$(LD_FLAGS)"
	mv -f src/$(PROGNAME) .

.PHONY: clean
clean:
	$(MAKE) -C src PROGNAME="$(PROGNAME)" $@
	rm -f $(PROGNAME)
	rm -f gmon.out

.PHONY: reallyclean
reallyclean: clean
	$(MAKE) -C src PROGNAME="$(PROGNAME)" $@
