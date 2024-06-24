#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define TABLE_SIZE 128
#define BUFFER_SIZE 1024

// 주어진 파일에서 특정한 라인을 읽어오는 함수
int main(int argc, char *argv[])
{
	// 파일 내 각 라인의 정보를 저장하는 데 사용하는 구조체
	static struct
	{
		long offset; // 라인의 시작 위치를 저장하는 변수
		int length;	 // 라인의 길이를 저장하는 변수
	} table[TABLE_SIZE];

	char buf[BUFFER_SIZE]; // 파일을 읽을 때 사용할 buf 변수

	long offset; // 현재 문자의 위치를 나타내는 변수
	int entry;	 // 현재 처리 중인 라인(몇번째 줄인지) 변수

	int i;		// for문에 사용할 변수
	int length; // 읽어온 byte수를 저장하기 위한 변수
	int fd;		// 파일 디스크립터

	// 파일명이 주어지지 않은 경우 (명령줄에서 인자가 2개들어와야 정상 작동함) 에러 처리
	if (argc < 2)
	{
		fprintf(stderr, "usage : %s <file>\n", argv[0]); // 에러출력
		exit(1);										 // 비정상종료
	}

	// 인자로 받은 파일을 open 함수를 이용하여 읽기 전용으로 파일 디스크립터를 반환받음
	if ((fd = open(argv[1], O_RDONLY)) < 0) // 에러처리
	{
		fprintf(stderr, "open error for %s\n", argv[1]); // 에러 출력
		exit(1);										 // 비정상 종료
	}

	// 0으로 우선 초기화
	entry = 0;
	offset = 0;

	// while문을 통해 읽으려는 파일을 BUFFER_SIZE만큼 buf로 읽고 저장함
	// 실제로 읽어온 byte수는 length에 저장함
	while ((length = read(fd, buf, BUFFER_SIZE)) > 0)
	{
		for (i = 0; i < length; i++) // 읽은 데이터의 길이만큼 반복문 수행
		{
			table[entry].length++; // 현재 처리중인 라인의 길이를 증가시킴
			offset++;			   // 현재 파일 내에서 offset을 증가시킴

			if (buf[i] == '\n')					// 개행문자를 만나면 새로운 라인이 시작되었다는 의미
				table[++entry].offset = offset; // entry를 증가시켜 다음 라인을 가리키도록 하고, 다음 라인의 정보를 저장 (다음라인의 시작위치는 현재 offset)
		}
	}

// DEBUG 매크로가 정의되어 있는 경우에만 실행
#ifdef DEBUG
	// 테이블에 저장된 각 라인의 offset과 length를 출력
	for (i = 0; i < entry; i++)
		printf("%d : %ld, %d\n", i + 1, table[i].offset, table[i].length);

#endif
	// 사용자로부터 출력할 라인의 번호를 입력받음
	while (1)
	{
		printf("Enter line number : ");
		scanf("%d", &length);

		// 음수를 입력하면 반복문을 탈출하고 프로그램 종료
		if (--length < 0)
			break;

		// 사용자가 올바른 번호를 입력한 경우

		lseek(fd, table[length].offset, 0); // lseek을 통해 그 줄에 해당하는 테이블로 커서를 이동

		if (read(fd, buf, table[length].length) <= 0) // 커서를 이동 후, buf를 통해 테이블의 내용을 length 만큼 읽음
			continue;

		buf[table[length].length] = '\0';
		printf("%s", buf); // 입력한 라인의 내용을 출력
	}

	close(fd); // 파일 닫기
	exit(0);
}
