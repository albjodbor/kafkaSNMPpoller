CC = gcc
CFLAGS= -g
PROG = poller
HEADERS = poller.h datos.h estructura.h logger.h poller_thread.h productor_kafka.h conf.h
PROGO = poller.o estructura.o logger.o poller_thread.o productor_kafka.o conf.o
LIB = -lpthread -lsnmp -lrdkafka -lz -lrt -ljansson

default: $(PROG)

$(PROG): $(PROGO)
	$(CC) $(CFLAGS) -o $(PROG) $(PROGO) $(LIB)
	
$(PROG).o: $(PROG).c $(HEADERS) 
	$(CC) $(CFLAGS) -c poller.c $(LIB)
	
estructura.o: estructura.c estructura.h datos.h
	$(CC) $(CFLAGS) -c estructura.c $(LIB)
	
logger.o: logger.c logger.h datos.h
	$(CC) $(CFLAGS) -c logger.c $(LIB)
	
poller-thread.o: poller_thread.c poller_thread.h datos.h
	$(CC) $(CFLAGS) -c poller_thread.c $(LIB)
	
productor_kafka.o: productor_kafka.c productor_kafka.h datos.h
	$(CC) $(CFLAGS) -c productor_kafka.c $(LIB)
	
conf.o: conf.c conf.h datos.h
	$(CC) $(CFLAGS) -c conf.c $(LIB)
	
clean:
	$(RM) *.o $(PROG) *~