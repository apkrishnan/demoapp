
CC = g++
CFLAGS = -O2 -c -Wall
LDFLAGS =

SOURCES := main.cpp
SOURCES += gpio.cpp
SOURCES += json.cpp

HEADERS := gpio.h
HEADERS += json.h

EXE = cloudSend

LIBS := -lqpid-proton

OBJS = $(SOURCES:.cpp=.o)

all: $(SOURCES) $(EXE)

$(EXE): $(OBJS) $(HEADERS) Makefile
	@echo LD $(EXE)
	@$(CC) $(LDFLAGS) $(OBJS) -o $@ $(LIBS)

.cpp.o:
	@echo CC $<
	@$(CC) $(CFLAGS) $< -o $@

clean:
	@echo DEL $(EXE) $(OBJS)
	@rm -f $(EXE) $(OBJS)

.PHONY: all clean
