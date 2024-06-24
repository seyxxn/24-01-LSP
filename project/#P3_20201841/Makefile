CC = gcc

HEADER = ssu_header
SYNC = ssu_sync
HELP = ssu_help

ssu_sync : $(SYNC).o $(HELP).o
	$(CC) -o ssu_sync $(SYNC).o $(HELP).o -lcrypto

$(SYNC).o : $(HEADER).h $(SYNC).c
	$(CC) -c -o $@ $(SYNC).c -lcrypto

$(HELP).o : $(HELP).c
	$(CC) -c -o $@ $(HELP).c -lcrypto

clean :
	rm -rf *.o
	rm -rf backup