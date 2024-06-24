#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <limits.h>
#include <string.h>
#include <sys/stat.h>

#ifdef PATH_MAX
static int pathmax = PATH_MAX;
#else
static int pathmax = 0;
#endif

#define MAX_PATH_GUESSED 1024

#ifndef LINE_MAX
#define LINE_MAX 2048
#endif

char *pathname;
char command[LINE_MAX], grep_cmd[LINE_MAX];

int ssu_do_grep(void){
	struct dirent *dirp;
	struct stat statbuf;
	char *ptr;
	DIR *dp;

    // lstat으로 파일을 검사하고 파일의 정보를 저장함
	if (lstat(pathname, &statbuf) < 0){
		fprintf(stderr,"lstat error for %s\n", pathname);
		return 0;
	}
	
    // 디렉터리가 아닌 경우 실행됨 (일반 파일인 경우 실행)
	if (S_ISDIR(statbuf.st_mode) == 0) {
		sprintf(command, "%s %s", grep_cmd, pathname);
		printf("%s : \n", pathname);
		system(command);
		return 0;
	}
    
    // 경로가 디렉터리인 경우
	ptr = pathname + strlen(pathname); // ptr이 pathname의 마지막을 가리키게 됨
	*ptr++ = '/'; // '/'추가
	*ptr = '\0'; // 문자열의 끝을 나타내는 널문자 추가

	if ((dp = opendir(pathname)) == NULL) { // 해당 디렉터리를 open
		fprintf(stderr, "opendir error for %s\n", pathname);
		return 0;
	}

    // 반복문으로 readdir 함수를 통해 디렉터리 내의 파일들을 읽음
	while ((dirp = readdir(dp)) != NULL)
		if (strcmp(dirp->d_name, ".") && strcmp(dirp->d_name, "..")) {
		    // .이나 ..이 아닌경우 실행
			strcpy(ptr, dirp->d_name); // 파일이름을 경로에 붙임

			if (ssu_do_grep() < 0) // 재귀호출
				break;
		}

	ptr[-1] = 0; // 슬래시 제거
	closedir(dp);
	return 0;
}

void ssu_make_grep(int argc, char *argv[]){
	int i;
	strcpy(grep_cmd, " grep"); 
	// 사용자로부터 받은 grep 옵션들을 grep_cmd 문자열에 추가함

	for (i = 1; i < argc-1; i++){
		strcat(grep_cmd, " ");
		strcat(grep_cmd, argv[i]);
	}
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "usage: %s <-CVbchilnsvwx> <-num> <-A num> <-B num> <-f file> \n""			<-e> expr <directory>\n", argv[0]);
		exit(1);
	}

    // 파일의 최대 길이 설정
	if (pathmax == 0){
		if ((pathmax = pathconf("/", _PC_PATH_MAX)) < 0)
			pathmax = MAX_PATH_GUESSED;
		else
			pathmax++;
	}
    
    // 파일의 이름을 받을 변수를 동적할당
	if ((pathname = (char *)malloc(pathmax+1)) == NULL) {
		fprintf(stderr, "malloc error\n");
		exit(1);
	}

	strcpy(pathname, argv[argc-1]); // 인자로 받은 파일 경로를 복사하여 저장함
	ssu_make_grep(argc, argv);
	ssu_do_grep();
	exit(0);
}
