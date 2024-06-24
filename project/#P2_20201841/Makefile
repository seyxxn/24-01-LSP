CC = gcc

HEADER = ssu_header
BACKUP = ssu_backup
ADD = ssu_add
HELP = ssu_help

# 실행 파일 이름을 ssu_repo로 변경
ssu_repo : $(BACKUP).o $(HELP).o
	$(CC) -o ssu_repo $(BACKUP).o $(HELP).o -lcrypto

$(BACKUP).o : $(HEADER).h $(BACKUP).c
	$(CC) -c -o $@ $(BACKUP).c -lcrypto

$(HELP).o : $(HELP).c
	$(CC) -c -o $@ $(HELP).c -lcrypto

clean :
	rm -rf ssu_repo
	rm -rf *.o
	rm -rf .repo
