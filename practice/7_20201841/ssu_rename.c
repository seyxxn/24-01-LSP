#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char *argv[]){
	int fd;

    // 인자로 파일 이름 2개를 받아야함
	if (argc != 3){
		fprintf(stderr, "usage: %s <oldname> <newname>\n", argv[0]);
		exit(1);
	}

    // oldname에 지정한 파일을 open함
	if ((fd = open(argv[1], O_RDONLY)) < 0) {
		fprintf(stderr, "first open error for %s\n", argv[1]);
		exit(1);
	}
	else
		close(fd);
    
    // rename함수를 이용하여 파일의 이름을 newname에 지정한 이름으로 변경
	if (rename(argv[1], argv[2]) < 0) {
		fprintf(stderr, "rename error\n");
		exit(1);
	}
    
    // 이름이 newname으로 변경되었기 때문에, oldname으로 open하면 에러 발생
	if ((fd = open(argv[1], O_RDONLY)) < 0)
		printf("second open error for %s\n", argv[1]);
	else{
		fprintf(stderr, "it's very strange!\n");
		exit(1);
	}

	printf("Everything is good!\n");
	exit(0);
}

