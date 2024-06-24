#define OPENSSL_API_COMPAT 0x10100000L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <wait.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <signal.h>
#include <syslog.h>
#include <stdbool.h>

#define true 1
#define false 0

#define HASH_MD5 33

#define NAMEMAX 255
#define PATHMAX 4096
#define STRMAX 4096

#define CMD_ADD 0b0000001
#define CMD_REM 0b0000010
#define CMD_LIST 0b0000100
#define CMD_HELP 0b0001000
#define CMD_EXIT 0b0010000
#define NOT_CMD 0b0000000

#define OPT_D 0b00001
#define OPT_R 0b00010
#define OPT_T 0b00100
#define NOT_OPT 0b00000

char exeNAME[PATHMAX];
char exePATH[PATHMAX];
char homePATH[PATHMAX];
char backupPATH[PATHMAX];
char monitorListLogPATH[PATHMAX];
int hash;

// 실행가능한 명령어를 담고있는 문자열 배열
char *commanddata[10] = {
    "add",
    "remove",
    "list",
    "help",
    "exit"};

// 입력받은 명령어와 인자를 저장할 구조체
typedef struct command_parameter
{
  char *command; // 명령어
  char *filename;
  char *tmpname;
  int commandopt;
  char *argv[10];
} command_parameter;

// 파일 노드 구조체
typedef struct fileNode
{
  char originPath[PATHMAX]; // 파일의 원본 경로
  char backupPath[PATHMAX]; // 파일의 백업 경로
  char *fileName; // 파일의 이름 추출
  time_t modTime; // 변경 시간
  struct fileNode *prevFile; // 이전 파일노드를 가리키는 포인터
  struct fileNode *nextFile; // 다음 파일노드를 가리키는 포인터
} fileNode;

// 디렉터리 리스트 구조체
typedef struct dirList
{
  struct dirNode *head; // 디렉터리 노드 리스트의 시작 지점을 가리키는 포인터
  struct dirNode *tail; // 디렉터리 노드 리스트의 끝 지점을 가리키는 포인터
} dirList;

// 디렉터리 노드 구조체
typedef struct dirNode
{
  char dirPath[PATHMAX]; // 디렉터리 경로
  struct fileNode *headFile; // 디렉터리에 속한 파일들의 시작 파일을 가리키는 포인터
  struct fileNode *tailFile; // 디렉터리에 속한 파일들의 끝 파일을 가리키는 포인터
  struct dirList *subDirs; // 하위 디렉터리 리스트를 가리키는 포인터
  struct dirNode *prevDir; // 이전 디렉터리 리스트를 가리키는 포인터
  struct dirNode *nextDir; // 다음 디렉터리 리스트를 가리키는 포인터
} dirNode;

typedef struct pathList_
{
  struct pathList_ *next;
  struct pathList_ *prev;
  char path[NAMEMAX];

} pathList;

void help(); // help 명령어 실행 함수
int md5(char *target_path, char *hash_result); // md5 해시 값 계산 함수
int ConvertHash(char *target_path, char *hash_result); // 해시 계산 함수 
char *GetFileName(char file_path[]); // 파일명만 추출하는 함수
char *getDate(); // 현재 날짜와 시간을 문자열로 반환하는 함수
char *convertDateTime(char *date); // 날짜를 일정 형식으로 반환해주는 함수
char *QuoteCheck(char **str, char del); // 따옴표 안에 있는 문자열을 확인하고 해당 문자열의 끝을 찾아서 반환하는 함수
char *Tokenize(char *str, char *del); // 문자열을 구분자로 토큰화하여 토큰을 반환하는 함수
char **GetSubstring(char *str, int *cnt, char *del); // 문자열을 구분자로 나누어서 부분 문자열을 배열에 저장하는 함수
int ConvertPath(char *origin, char *resolved); // 전체 경로로 변환해주는 함수
void printToLogFile(char *logfilePath, char *date, char *path, int flag); // 파일의 상태를 로그 파일에 출력하는 함수

// md5 해시 값 계산 함수
int md5(char *target_path, char *hash_result)
{
  FILE *fp;
  unsigned char hash[MD5_DIGEST_LENGTH];
  unsigned char buffer[SHRT_MAX];
  int bytes = 0;
  MD5_CTX md5;

  if ((fp = fopen(target_path, "rb")) == NULL)
  {
    printf("ERROR: fopen error for %s\n", target_path);
    return 1;
  }

  MD5_Init(&md5);

  while ((bytes = fread(buffer, 1, SHRT_MAX, fp)) != 0)
    MD5_Update(&md5, buffer, bytes);

  MD5_Final(hash, &md5);

  for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
    sprintf(hash_result + (i * 2), "%02x", hash[i]);
  hash_result[HASH_MD5 - 1] = 0;

  fclose(fp);

  return 0;
}

// 해시 계산 함수
int ConvertHash(char *target_path, char *hash_result)
{
  if (hash == HASH_MD5)
  {
    md5(target_path, hash_result);
  }
}

// 파일명만 추출하는 함수
char *GetFileName(char file_path[])
{
  char *file_name;

  while (*file_path)
  {
    if (*file_path == '/' && (file_path + 1) != NULL)
    {
      file_name = file_path + 1;
    }
    file_path++;
  }
  return file_name;
}

// 현재 날짜와 시간을 문자열로 반환하는 함수
char *getDate()
{
  char *date = (char *)malloc(sizeof(char) * STRMAX);
  time_t timer;
  struct tm *t;

  timer = time(NULL);
  t = localtime(&timer);

  sprintf(date, "%04d%02d%02d%02d%02d%02d", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);

  return date;
}

// 2024-05-03 10:00:00 형식으로 반환해주는 함수
char *convertDateTime(char *date)
{
  char *result = (char *)calloc(sizeof(char), PATHMAX);

  strncpy(result, date, 4); // 연도
  strcat(result, "-");
  strncat(result, date + 4, 2); // 월
  strcat(result, "-");
  strncat(result, date + 6, 2); // 일
  strcat(result, " ");
  strncat(result, date + 8, 2); // 시
  strcat(result, ":");
  strncat(result, date + 10, 2); // 분
  strcat(result, ":");
  strncat(result, date + 12, 2); // 초

  return result;
}

// 따옴표 안에 있는 문자열을 확인하고 해당 문자열의 끝을 찾아서 반환하는 함수
char *QuoteCheck(char **str, char del)
{
  char *tmp = *str + 1;
  int i = 0;

  while (*tmp != '\0' && *tmp != del)
  {
    tmp++;
    i++;
  }
  if (*tmp == '\0')
  {
    *str = tmp;
    return NULL;
  }
  if (*tmp == del)
  {
    for (char *c = *str; *c != '\0'; c++)
    {
      *c = *(c + 1);
    }
    *str += i;
    for (char *c = *str; *c != '\0'; c++)
    {
      *c = *(c + 1);
    }
  }
}

// 문자열을 구분자로 토큰화하여 토큰을 반환하는 함수
char *Tokenize(char *str, char *del)
{
  int i = 0;
  int del_len = strlen(del);
  static char *tmp = NULL;
  char *tmp2 = NULL;

  if (str != NULL && tmp == NULL)
  {
    tmp = str;
  }

  if (str == NULL && tmp == NULL)
  {
    return NULL;
  }

  char *idx = tmp;

  while (i < del_len)
  {
    if (*idx == del[i])
    {
      idx++;
      i = 0;
    }
    else
    {
      i++;
    }
  }
  if (*idx == '\0')
  {
    tmp = NULL;
    return tmp;
  }
  tmp = idx;

  while (*tmp != '\0')
  {
    if (*tmp == '\'' || *tmp == '\"')
    {
      QuoteCheck(&tmp, *tmp);
      continue;
    }
    for (i = 0; i < del_len; i++)
    {
      if (*tmp == del[i])
      {
        *tmp = '\0';
        break;
      }
    }
    tmp++;
    if (i < del_len)
    {
      break;
    }
  }

  return idx;
}

// 문자열을 구분자로 나누어서 부분 문자열을 배열에 저장하는 함수
char **GetSubstring(char *str, int *cnt, char *del)
{
  *cnt = 0;
  int i = 0;
  char *token = NULL;
  char *templist[100] = {
      NULL,
  };
  token = Tokenize(str, del);
  if (token == NULL)
  {
    return NULL;
  }

  while (token != NULL)
  {
    templist[*cnt] = token;
    *cnt += 1;
    token = Tokenize(NULL, del);
  }

  char **temp = (char **)malloc(sizeof(char *) * (*cnt + 1));
  for (i = 0; i < *cnt; i++)
  {
    temp[i] = templist[i];
  }
  return temp;
}

// 전체 경로로 변환해주는 함수
int ConvertPath(char *origin, char *resolved)
{
  int idx = 0;
  int i;
  char *path = (char *)malloc(sizeof(char *) * PATHMAX);
  char *tmppath = (char *)malloc(sizeof(char *) * PATHMAX);
  char **pathlist;
  int pathcnt;

  if (origin == NULL)
  {
    return -1;
  }

  if (origin[0] == '~')
  {
    sprintf(path, "%s%s", homePATH, origin + 1);
  }
  else if (origin[0] != '/')
  {
    sprintf(path, "%s/%s", exePATH, origin);
  }
  else
  {
    sprintf(path, "%s", origin);
  }

  if (!strcmp(path, "/"))
  {
    resolved = "/";
    return 0;
  }

  if ((pathlist = GetSubstring(path, &pathcnt, "/")) == NULL)
  {
    return -1;
  }

  pathList *headpath = (pathList *)malloc(sizeof(pathList));
  pathList *currpath = headpath;

  for (i = 0; i < pathcnt; i++)
  {
    if (!strcmp(pathlist[i], "."))
    {
      continue;
    }
    else if (!strcmp(pathlist[i], ".."))
    {
      currpath = currpath->prev;
      currpath->next = NULL;
      continue;
    }

    pathList *newpath = (pathList *)malloc(sizeof(pathList));
    strcpy(newpath->path, pathlist[i]);
    currpath->next = newpath;
    newpath->prev = currpath;

    currpath = currpath->next;
  }

  currpath = headpath->next;

  strcpy(tmppath, "/");
  while (currpath != NULL)
  {
    strcat(tmppath, currpath->path);
    if (currpath->next != NULL)
    {
      strcat(tmppath, "/");
    }
    currpath = currpath->next;
  }

  strcpy(resolved, tmppath);

  return 0;
}

// 파일의 상태를 로그 파일에 출력하는 함수
// flag : 0 -> 생성, 1 -> 수정, 2 -> 삭제
void printToLogFile(char *logfilePath, char *date, char *path, int flag)
{
  // 백업 성공시 로그파일에 출력
  if (access(logfilePath, F_OK) != 0)
  { // 로그파일 존재하지 않는다면 생성
    int fd = open(logfilePath, O_CREAT | O_WRONLY, 0666);
    if (fd == -1)
    {
      fprintf(stderr, "ERROR: open error for %s\n", logfilePath);
      exit(EXIT_FAILURE);
    }
    close(fd);
  }

  char *logfileWrite = (char *)malloc(sizeof(char *) * STRMAX);
  if (flag == 0)
    sprintf(logfileWrite, "[%s][create][%s]\n", convertDateTime(date), path);
  else if (flag == 1)
    sprintf(logfileWrite, "[%s][modify][%s]\n", convertDateTime(date), path);
  else if (flag == 2)
    sprintf(logfileWrite, "[%s][remove][%s]\n", convertDateTime(date), path);

  // 백업 파일에 출력하기
  int logfd;
  if ((logfd = open(logfilePath, O_RDWR | O_CREAT | O_APPEND, 0666)) < 0)
  {
    fprintf(stderr, "open error for %s\n", logfilePath);
    exit(1);
  }
  else
  {
    write(logfd, logfileWrite, strlen(logfileWrite));
    close(logfd);
  }

}