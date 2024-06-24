#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define TABLE_SIZE (sizeof(table)/sizeof(*table))

int main(int argc, int *argv[])
{
	struct {
		char *text;
		int mode;
	} table [] = {
		{"exists", 0},
		{"execute", 1},
		{"write", 2},
		{"read", 4}
	};
	int i;

	if (argc < 2) {
		fprintf(stderr, "usage : %s <file>\n", argv[0]);
		exit(1);
	}

	for (i = 0; i < TABLE_SIZE; i++){ 
		if (access(argv[1], table[i].mode) != -1) // 차례대로 파일의 존재 여부, 실행 권한, 쓰기 권한, 읽기 권한을 access 함수로 확인함
			printf("%s -ok\n", table[i].text); // 권한이 존재하면 ok 출력
		else
			printf("%s\n", table[i].text);
	}

	exit(0);
}
