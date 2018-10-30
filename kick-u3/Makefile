SOURCES = ./src/
INCLUDES = ../include/
OBJ = ./obj/

all: kick-u3 kick-read kick-mon kick-u3-pulse

kick-u3: $(SOURCES)kick-u3.c
	gcc -Wall -o kick-u3 $(INCLUDES)u3.o $(SOURCES)kick-u3.c -lm -llabjackusb

kick-u3-pulse: $(SOURCES)kick-u3-pulse.c
	gcc -Wall -o kick-u3-pulse $(INCLUDES)u3.o $(SOURCES)kick-u3-pulse.c -lm -llabjackusb

kick-read:$(SOURCES)kick-read.c
	gcc -Wall -o kick-read $(SOURCES)kick-read.c

kick-mon:$(SOURCES)kick-mon.c
	gcc -Wall -o kick-mon $(SOURCES)kick-mon.c `pkg-config --cflags --libs gtk+-3.0`

clean:
	rm kick-u3 kick-read kick-mon kick-u3-pulse


