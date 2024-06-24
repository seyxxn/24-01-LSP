#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
	int i;

	for (i = 0; i < argc; i++) {
		printf("argv[%d] : %s\n", i, argv[i]);
		// 프로그램 실행 시에 인자로 받는 값들을 출력함
	}
	exit(0);
}
