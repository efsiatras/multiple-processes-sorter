CC=gcc
CFLAGS=-Wall -g

MYSORT_OBJS=coordinator.o records/record.o
COACH0_OBJS=coaches/coach0.o records/record.o
COACH1_OBJS=coaches/coach1.o records/record.o
COACH2_OBJS=coaches/coach2.o records/record.o
COACH3_OBJS=coaches/coach3.o records/record.o
HEAPSORTER_OBJS=sorters/heapsorter.o records/record.o
QUICKSORTER_OBJS=sorters/quicksorter.o records/record.o
TOTAL_OBJS=coordinator.o coaches/coach0.o coaches/coach1.o coaches/coach2.o coaches/coach3.o sorters/heapsorter.o sorters/quicksorter.o records/record.o

all: mysort coach0 coach1 coach2 coach3 heapsorter quicksorter

mysort: $(MYSORT_OBJS)
	$(CC) $(MYSORT_OBJS) -o mysort $(CFLAGS)

coach0: $(COACH0_OBJS)
	$(CC) $(COACH0_OBJS) -o coach0 $(CFLAGS)

coach1: $(COACH1_OBJS)
	$(CC) $(COACH1_OBJS) -o coach1 $(CFLAGS)

coach2: $(COACH2_OBJS)
	$(CC) $(COACH2_OBJS) -o coach2 $(CFLAGS)

coach3: $(COACH3_OBJS)
	$(CC) $(COACH3_OBJS) -o coach3 $(CFLAGS)

heapsorter: $(HEAPSORTER_OBJS)
	$(CC) $(HEAPSORTER_OBJS) -o heapsorter $(CFLAGS)

quicksorter: $(QUICKSORTER_OBJS)
	$(CC) $(QUICKSORTER_OBJS) -o quicksorter $(CFLAGS)

coordinator.o: coordinator.c
	$(CC) -c coordinator.c -o coordinator.o $(CFLAGS)

coaches/coach0.o: coaches/coach0.c
	$(CC) -c coaches/coach0.c -o coaches/coach0.o $(CFLAGS)

coaches/coach1.o: coaches/coach1.c
	$(CC) -c coaches/coach1.c -o coaches/coach1.o $(CFLAGS)

coaches/coach2.o: coaches/coach2.c
	$(CC) -c coaches/coach2.c -o coaches/coach2.o $(CFLAGS)

coaches/coach3.o: coaches/coach3.c
	$(CC) -c coaches/coach3.c -o coaches/coach3.o $(CFLAGS)

sorters/heapsorter.o: sorters/heapsorter.c
	$(CC) -c sorters/heapsorter.c -o sorters/heapsorter.o $(CFLAGS)

sorters/quicksorter.o: sorters/quicksorter.c
	$(CC) -c sorters/quicksorter.c -o sorters/quicksorter.o $(CFLAGS)

records/record.o: records/record.c
	$(CC) -c records/record.c -o records/record.o $(CFLAGS)

.PHONY: clean

clean:
	rm -f mysort coach0 coach1 coach2 coach3 heapsorter quicksorter $(TOTAL_OBJS)