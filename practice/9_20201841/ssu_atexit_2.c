#include <stdio.h>
#include <stdlib.h>

void ssu_out(void);

int main(void){
	if (atexit(ssu_out)) {
	    // atexit() 함수를 사용해 exit() 실행 시 자동으로 호출 될 함수를 등록함
	    // 인자로 ssu_out을 넘겨주었기 때문에 exit() 실행 시 ssu_out 함수를 호출함
		fprintf(stderr, "atexit error\n");
		exit(1);
	}

	exit(0);
}

void ssu_out(void) {
	printf("atexit succeeded!\n");
}
