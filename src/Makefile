INCLUDES += -I. -I..

SOURCES = $(wildcard *.cc)
OBJECTS = $(patsubst %.cc, %.o, \
            $(SOURCES))

.SUFFIXES: .cc .o

.cc.o:
	$(CXX) $(CFLAGS) $(INCLUDES) -c $^ -o $@

all: pch.h.gch $(PROGNAME)

$(PROGNAME): $(OBJECTS)
	$(CXX) -o $@ $^ $(LD_FLAGS) $(LIBS)

pch.h.gch: pch.h
	$(CXX) $(CFLAGS) $(INCLUDES) pch.h

.PHONY: clean
clean:
	rm -f *.o
	rm -f $(PROGNAME)

.PHONY: reallyclean
reallyclean: clean
	rm -f *.gch
