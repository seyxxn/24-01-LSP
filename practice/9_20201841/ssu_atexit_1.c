#include <stdio.h>
#include <stdlib.h>

static void ssu_exit1(void);
static void ssu_exit2(void);

int main(void){
	if (atexit(ssu_exit2) != 0) { // ssu_exit2를 종료 핸들러로 등록
		fprintf(stderr, "atexit error for ssu_exit2");
		exit(1);
	}

	if (atexit(ssu_exit1) != 0) { // ssu_exit1을 종료 핸들러로 등록
		fprintf(stderr, "atexit error for ssu_exit1");
		exit(1);
	}

	if (atexit(ssu_exit1) != 0) { // ssu_exit1을 종료 핸들러로 등록
		fprintf(stderr, "atexit error for ssu_exit1");
		exit(1);
	}

	printf("done\n"); // 함수 종료 전 done 출력
	exit(0); // 정상 종료하며 atexit()에 등록된 함수들이 호출됨
	// 프로그램이 종료될 때 atexit()에 의해 등록된 함수들이 역순으로 호출 됨
}

static void ssu_exit1(void) {
	printf("ssu_exit1 handler\n");
}

static void ssu_exit2(void) {
	printf("ssu_exit2 handler\n");
}
