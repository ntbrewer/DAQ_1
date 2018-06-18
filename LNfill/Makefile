SOURCES = ./src/
INCLUDES = ../include/

all:    lnfill-u3-mmap-all lnfill-u6-mmap-all lnfill-read-mmap-all lnfill-log-mmap-all lnfill-u3-shm-all lnfill-u6-shm-all lnfill-log-shm-all lnfill-read-shm-all 
all-u6-mmap: lnfill-u6-mmap lnfill-read-mmap lnfill-log-mmap
all-u3-mmap: lnfill-u3-mmap lnfill-read-mmap lnfill-log-mmap
all-u6-shm:  lnfill-u6-shm lnfill-read-shm lnfill-log-shm
all-u3-shm:  lnfill-u3-shm lnfill-read-shm lnfill-log-shm

lnfill-u6-mmap-all: $(SOURCES)lnfill-u6-mmap.c
	gcc $(INCLUDES)u6.o -o lnfill-u6-mmap $(SOURCES)lnfill-u6-mmap.c -lm -llabjackusb 

lnfill-u3-mmap-all: $(SOURCES)lnfill-u3-mmap.c
	gcc $(INCLUDES)u3.o -o lnfill-u3-mmap $(SOURCES)lnfill-u3-mmap.c -lm -llabjackusb

lnfill-read-mmap-all: $(SOURCES)lnfill-read-mmap.c
	gcc  -o lnfill-read-mmap $(SOURCES)lnfill-read-mmap.c

lnfill-log-mmap-all: $(SOURCES)lnfill-log-mmap.c
	gcc  -o lnfill-log-mmap $(SOURCES)lnfill-log-mmap.c

lnfill-u6-shm-all: $(SOURCES)lnfill-u6-shm.c
	gcc $(INCLUDES)u6.o -o lnfill-u6-shm $(SOURCES)lnfill-u6-shm.c -lm -llabjackusb

lnfill-u3-shm-all: $(SOURCES)lnfill-u3-shm.c
	gcc $(INCLUDES)u3.o -o lnfill-u3-shm $(SOURCES)lnfill-u3-shm.c -lm -llabjackusb

lnfill-read-shm-all: $(SOURCES)lnfill-read-shm.c
	gcc -o lnfill-read-shm $(SOURCES)lnfill-read-shm.c

lnfill-log-shm-all: $(SOURCES)lnfill-log-shm.c
	gcc -o lnfill-log-shm $(SOURCES)lnfill-log-shm.c

##Method to make only one executable.

lnfill-u6-mmap: $(SOURCES)lnfill-u6-mmap.c
	gcc $(INCLUDES)u6.o -o lnfill $(SOURCES)lnfill-u6-mmap.c -lm -llabjackusb

lnfill-u3-mmap: $(SOURCES)lnfill-u3-mmap.c
	gcc $(INCLUDES)u3.o -o lnfill $(SOURCES)lnfill-u3-mmap.c -lm -llabjackusb

lnfill-read-mmap: $(SOURCES)lnfill-read-mmap.c
	gcc  -o lnfill-read $(SOURCES)lnfill-read-mmap.c

lnfill-log-mmap: $(SOURCES)lnfill-log-mmap.c
	gcc  -o lnfill-log $(SOURCES)lnfill-log-mmap.c

lnfill-u6-shm: $(SOURCES)lnfill-u6-shm.c
	gcc $(INCLUDES)u6.o -o lnfill $(SOURCES)lnfill-u6-shm.c -lm -llabjackusb

lnfill-u3-shm: $(SOURCES)lnfill-u3-shm.c
	gcc $(INCLUDES)u3.o -o lnfill $(SOURCES)lnfill-u3-shm.c -lm -llabjackusb

lnfill-read-shm: $(SOURCES)lnfill-read-shm.c
	gcc -o lnfill-read $(SOURCES)lnfill-read-shm.c

lnfill-log-shm: $(SOURCES)lnfill-log-shm.c
	gcc -o lnfill-log $(SOURCES)lnfill-log-shm.c

clean:
	rm lnfill*
