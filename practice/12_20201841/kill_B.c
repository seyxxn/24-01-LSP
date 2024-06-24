#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void){
	// 무한 루프 시작
	while (1) {
		// 5초마다 "[OSLAB]"을 출력
		printf("\n[OSLAB]");
		sleep(5); // 5초 대기
	}

	exit(0); // 프로그램 종료
}
