SOURCES = ./src/
INCLUDES = ../include/

all:    hvmon.mpod-mmap-all hvmon-read-mmap-all hvmon.mpod-shm-all hvmon-read-shm-all 
all-u3-mmap: hvmon.mpod-mmap hvmon-read-mmap 
all-u3-shm:  hvmon.mpod-shm hvmon-read-shm

hvmon.mpod-mmap-all: $(SOURCES)hvmon.mpod-mmap.c
	gcc -Wall  -lpthread -ldl -lm -o hvmon.mpod-mmap $(SOURCES)hvmon.mpod-mmap.c

hvmon-read-mmap-all: $(SOURCES)hvmon-read-mmap.c
	gcc  -Wall -lm -o hvmon-read-mmap $(SOURCES)hvmon-read-mmap.c

hvmon.mpod-shm-all: $(SOURCES)hvmon.mpod-shm.c
	gcc -Wall  -lpthread -ldl -lm -o hvmon.mpod-shm $(SOURCES)hvmon.mpod-shm.c

hvmon-read-shm-all: $(SOURCES)hvmon-read-shm.c
	gcc  -Wall -lm -o hvmon-read-shm $(SOURCES)hvmon-read-shm.c

##Method to make only one executable.

hvmon.mpod-mmap: $(SOURCES)hvmon.mpod-mmap.c
	gcc -Wall  -lpthread -ldl -lm -o hvmon $(SOURCES)hvmon.mpod-mmap.c

hvmon-read-mmap: $(SOURCES)hvmon-read-mmap.c
	gcc  -Wall -lm -o hvmon-read $(SOURCES)hvmon-read-mmap.c

hvmon.mpod-shm: $(SOURCES)hvmon.mpod-shm.c
	gcc -Wall  -lpthread -ldl -lm -o hvmon $(SOURCES)hvmon.mpod-shm.c

hvmon-read-shm: $(SOURCES)hvmon-read-shm.c
	gcc  -Wall -lm -o hvmon-read $(SOURCES)hvmon-read-shm.c


clean:
	rm hvmon*
