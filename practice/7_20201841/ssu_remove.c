#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]){
    
    // 변경하고자하는 파일, 새이름을 인자로 받아야 함
	if(argc != 3){
		fprintf(stderr, "usage: %s <oldname> <newname>\n", argv[0]);
		exit(1);
	}

    // link로 파일을 생성
	if (link(argv[1], argv[2]) < 0) {
		fprintf(stderr, "link error\n");
		exit(1);
	}
	else {
		printf("step1 passed.\n"); // 에러발생안하면 출력
	}

    // remove함수로 oldname에 지정한 파일을 unlink함
    // 파일의 해당하는 디스크 공간은 해제되지 않고
    // link함수로 생성한 파일이 그 디스크 공간을 가리킴
	if (remove(argv[1]) < 0)
	{
		fprintf(stderr, "remove error\n");
		remove(argv[2]);
		exit(1);
	}
	else{
		printf("step2 passed.\n"); // 에러발생안하면 출력
	}

	printf("Success!\n");
	exit(0);
}
