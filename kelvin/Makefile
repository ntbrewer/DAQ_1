SOURCES = ./src/
INCLUDES = ../include/
OBJ = ./obj/

all: kelvin-u3 kelvin-read kelvin-log

kelvin-u3: $(SOURCES)kelvin-u3.c
	gcc -Wall -o kelvin-u3 $(INCLUDES)u3.o $(SOURCES)kelvin-u3.c -lm -llabjackusb

kelvin-read:$(SOURCES)kelvin-read.c
	gcc -Wall -o kelvin-read $(SOURCES)kelvin-read.c

kelvin-log:$(SOURCES)kelvin-log.c
	gcc -Wall -o kelvin-log $(SOURCES)kelvin-log.c

clean:
	rm kelvin-u3 kelvin-read kelvin-log 


