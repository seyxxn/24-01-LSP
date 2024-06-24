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
#include <stdbool.h>
#include <libgen.h>

#define true 1
#define false 0

#define HASH_MD5 33 // MD5만 사용

#define NAMEMAX 255
#define PATHMAX 4096
#define STRMAX 4096

#define CMD_ADD 0b00000001
#define CMD_REM 0b00000010
#define CMD_STA 0b00000100
#define CMD_COM 0b00001000
#define CMD_REV 0b00010000
#define CMD_LOG 0b00100000
#define CMD_HELP 0b01000000
#define CMD_EXIT 0b10000000
#define NOT_CMD 0b00000000

char exeNAME[PATHMAX];
char exePATH[PATHMAX];
char homePATH[PATHMAX];
char repoPATH[PATHMAX];

char commitlogPATH[PATHMAX];  // 커밋 로그 파일 경로
char staginglogPATH[PATHMAX]; // 스테이징 구역 파일 경로

int hash;

// 명령어 이름 저장한 배열
char *commanddata[10] = {
    "add",
    "remove",
    "status",
    "commit",
    "revert",
    "log",
    "help",
    "exit"};

// 받아온 명령어들을 나누어 저장할 구조체
typedef struct command_parameter
{
  char *command;
  char *filename;
  char *tmpname;
  int commandopt;
  char *argv[256];
} command_parameter;

typedef struct _fileNode
{
  struct _dirNode *root_dir; // 파일노드가 속한 상위 디렉터리를 가리킴

  char file_path[PATHMAX]; // 파일 경로 저장

  struct _fileNode *prev_file; // 이전 파일 노드
  struct _fileNode *next_file; // 다음 파일 노드
} fileNode;

typedef struct _dirNode
{
  struct _dirNode *root_dir; // 디렉터리 노드가 속한 상위 디렉터리

  char dir_path[PATHMAX];       // 디렉터리 경로
  fileNode *file_head;          // 디렉터리에 속한 파일들을 관리하는 연결리스트의 헤드를 가리킴
  struct _dirNode *subdir_head; // 하위 디렉터리들을 관리하는 연결 리스트의 헤드를 가리킴

  struct _dirNode *prev_dir; // 이전 디렉터리 노드
  struct _dirNode *next_dir; // 다음 디렉터리 노드
} dirNode;

dirNode *staging_dir_list;  // staging된 파일 및 디렉터리들을 관리하기 위한 최상위 디렉터리 역할
dirNode *managing_dir_list; // add나 remove를 한번이라도 하면 추가되는 리스트 노드 (삭제되지 않음)
dirNode *commit_dir_list;   // commit하여 백업한 노드들을 저장할 리스트 노드

// 디렉터리 경로를 저장하기 위한 연결리스트 노드
typedef struct pathList_
{
  struct pathList_ *next;
  struct pathList_ *prev;
  char path[NAMEMAX];

} pathList;

// 커밋 로그 저장 구조체
typedef struct CommitLogEntry
{
  char commitMessage[256];
  char changeType[32]; // "new file", "modified", "removed"
  char filePath[PATHMAX];
  char backupPath[PATHMAX];
  struct CommitLogEntry *next; // 다음 노드를 가리키는 포인터
} CommitLogEntry;

CommitLogEntry *commitHead = NULL;


CommitLogEntry *appendEntry(CommitLogEntry **head, CommitLogEntry *newEntry);            // 커밋 로그 노드 삽입 함수
int parseCommitLog(const char *line, CommitLogEntry *entry);                             // 커밋로그에서 파싱하는 함수
void printCommitLogEntry(const CommitLogEntry *entry);                                   // 커밋 로그 구조체 값 확인 출력 함수
void printCommitLogEntryGrouped(CommitLogEntry *head);                                   // 로그를 커밋메세지 별로 출력하는 함수
void printSpecificCommitLog(CommitLogEntry *head, const char *commitMessage);            // 특정 커밋 메세지에 대한 로그만 출력하고 없으면 에러 처리
void testStagingList(dirNode *stagingList);                                              // 연결리스트 구축 확인용 코드
int md5(char *target_path, char *hash_result);                                           // 파일 내용을 읽어 MD5 해시 값을 계산하고 hash_result에 저장
int ConvertHash(char *target_path, char *hash_result);                                   // 해시 실행
int cmpHash(char *path1, char *path2);                                                   // path1과 path2의 해시 값을 비교하는 함수
char *cvtNumComma(int a);                                                                // 3자리 수씩 끊어서 문자열로 변환하는 함수
char *GetFileName(char file_path[]);                                                     // 경로에서 파일 이름만을 추출하는 함수
char *strToHex(char *str);                                                               // str을 입력받아 16진수 형태의 문자열로 변환 후 반환
char *getDate();                                                                         // 일정한 형식으로 일시를 받아오는 함수
char *QuoteCheck(char **str, char del);                                                  // 문자열에서 구분자를 찾아서 구분자를 제거하는 함수
char *Tokenize(char *str, char *del);                                                    // 구분자를 기준으로 토큰을 분리하는 함수
char **GetSubstring(char *str, int *cnt, char *del);                                     // Tokenize 호출함
int ConvertPath(char *origin, char *resolved);                                           // 절대 경로로 변환하는 함수
int cmpPath(char *path1, char *path2);                                                   // 두 경로를 비교하여 정렬 순서를 결정
char *substr(char *str, int beg, int end);                                               // 주어진 문자열인 str에서 특정 범위의 문자열을 추출
char *c_str(char *str);                                                                  // 주어진 문자열을 복사하여 새로운 문자열 생성(입력된 문자열의 복사본 반환)
void fileNode_init(fileNode **file_node);                                                // 파일 노드 초기화 함수
void dirNode_init(dirNode **node);                                                       // 디렉터리 노드 초기화 함수
fileNode *find_file_node(fileNode *head, const char *path);                              // 연결리스트 구축 과정에서 : 중복된 파일 노드를 거르기 위한 함수
dirNode *find_directory_node(dirNode *head, const char *path);                           // 연결리스트 구축 과정에서 : 중복된 디렉터리 노드를 거르기 위한 함수
void insert_directory_node(dirNode **head, dirNode *parent, const char *path);           // 디렉터리 노드 삽입 함수
void insert_file_node(dirNode *parent, const char *path);                                // 파일 노드 삽입 함수
void remove_file_node(dirNode *parent, const char *path);                                // 파일 노드 삭제 함수
void remove_dir_node(dirNode **head, dirNode *nodeToDelete);                             // 연결리스트에서 디렉터리 노드를 삭제하는 함수
void recursiveDeleteEmptyDirs(dirNode **head);                                           // 루트 디렉터리부터 하위 디렉터리를 재귀적으로 탐색하며 비어있는 디렉터리 삭제
void add_staging(char *path, dirNode *root);                                             // add된 로그 파일의 경로를 읽어 연결리스트 구축 (경로들을 한 줄씩 인자로 받음)
void remove_staging(char *path, dirNode *root);                                          // remove된 로그 파일의 경로를 읽어 연결리스트 구축 (경로들을 한 줄씩 인자로 받음)
bool is_fully_staged_directory(dirNode *root, const char *dirPath);                      // 경로로 디렉터리가 들어왔을때 디렉터리 내의 모든 파일이나 디렉터리들이 staging 되었는지 확인하는 함수
fileNode *find_file_recursively(dirNode *dir, const char *path);                         // 재귀적으로 탐색하며 해당 파일이 staging 되었는지 확인하는 함수
bool is_staged_file_or_directory(dirNode *root, const char *path, struct stat *statbuf); // 경로로 들어온 파일 혹은 디렉터리가 이미 staging되어있는지 확인하기 위한 함수
bool is_none_staged_in_directory(dirNode *root, const char *dirPath);                    // 하나라도 있으면 false반환, 모두 없어야 true 반환
bool is_none_file_or_directory(dirNode *root, const char *path, struct stat *statbuf);   // 경로로 디렉터리가 들어온 경우 하나라도 있으면 false 반환, 모두 존재하지 않는 경우 true 반환 / 파일의 경우 존재하면 false 반환
int isInStagingList(const char *path);                                                   // 주어진 경로가 staging 리스트에 존재하지 않으면 false 반환, 존재하면 true 반환
int isInManagingList(const char *path);                                                  // 주어진 경로가 managing 리스트에 존재하지 않으면 false 반환, 존재하면 true 반환
int statusUntrackedCheck(const char *basePath, int *firstPrint);                         // 재귀적으로 디렉토리를 탐색하며 파일 상태를 출력하는 함수




// 커밋 로그 노드 삽입 함수
CommitLogEntry *appendEntry(CommitLogEntry **head, CommitLogEntry *newEntry)
{
  if (*head == NULL)
  {
    *head = newEntry;
  }
  else
  {
    CommitLogEntry *current = *head;
    while (current->next != NULL)
    {
      current = current->next;
    }
    current->next = newEntry;
  }
  return *head;
}

// 커밋로그에서 파싱하는 함수
int parseCommitLog(const char *line, CommitLogEntry *entry)
{
  const char *commitPrefix = "commit: \"";
  const char *typePrefix = "- ";
  const char *filePrefix = ": \"";

  // 커밋 메시지 파싱
  const char *start = strstr(line, commitPrefix);
  if (start == NULL)
    return -1;
  start += strlen(commitPrefix);
  const char *end = strchr(start, '"');
  if (end == NULL)
    return -1;
  int length = end - start;
  if (length >= sizeof(entry->commitMessage))
    return -1;
  strncpy(entry->commitMessage, start, length);
  entry->commitMessage[length] = '\0';

  // 변경 유형 파싱
  start = strstr(end, typePrefix);
  if (start == NULL)
    return -1;
  start += strlen(typePrefix);
  end = strchr(start, ':');
  if (end == NULL)
    return -1;
  length = end - start;
  if (length >= sizeof(entry->changeType))
    return -1;
  strncpy(entry->changeType, start, length);
  entry->changeType[length] = '\0';

  // 파일 경로 파싱
  start = strstr(end, filePrefix);
  if (start == NULL)
    return -1;
  start += strlen(filePrefix);
  end = strchr(start, '"');
  if (end == NULL)
    return -1;
  length = end - start;
  if (length >= sizeof(entry->filePath))
    return -1;
  strncpy(entry->filePath, start, length);
  entry->filePath[length] = '\0';

  return 0;
}

// 커밋 로그 구조체 값 확인 출력 함수
void printCommitLogEntry(const CommitLogEntry *entry)
{
  printf("Commit Message: %s\n", entry->commitMessage);
  printf("Change Type: %s\n", entry->changeType);
  printf("File Path: %s\n", entry->filePath);
  // printf("backup Path : %s\n\n", entry->backupPath);
}

// 로그를 커밋메세지 별로 출력하는 함수
void printCommitLogEntryGrouped(CommitLogEntry *head)
{
  CommitLogEntry *current = head;
  char lastCommitMsg[256] = "";

  while (current != NULL)
  {
    if (strcmp(lastCommitMsg, current->commitMessage) != 0)
    {
      if (strlen(lastCommitMsg) > 0)
        printf("\n");
      printf("commit: \"%s\"\n", current->commitMessage);
      strcpy(lastCommitMsg, current->commitMessage);
    }
    printf(" - %s: \"%s\"\n", current->changeType, current->filePath);
    current = current->next;
  }
}

// 특정 커밋 메세지에 대한 로그만 출력하고 없으면 에러 처리
void printSpecificCommitLog(CommitLogEntry *head, const char *commitMessage)
{
  CommitLogEntry *current = head;
  int found = 0;

  while (current != NULL)
  {
    if (strcmp(current->commitMessage, commitMessage) == 0)
    {
      if (!found)
      {
        printf("commit: \"%s\"\n", commitMessage);
        found = 1;
      }
      printf(" - %s: \"%s\"\n", current->changeType, current->filePath);
    }
    current = current->next;
  }

  if (!found)
  {
    fprintf(stderr, "ERROR: No logs found for the commit message '%s'\n", commitMessage);
  }
}

// 연결리스트 구축 확인용 코드
void testStagingList(dirNode *stagingList)
{
  dirNode *currDir = stagingList;
  int i;

  while (currDir != NULL)
  {
    i = 0;

    printf("--------------\n");
    if (currDir->root_dir == NULL)
    {
      printf("최상위 디렉터리\n");
    }
    else
    {
      printf("root dir: %s\n", currDir->root_dir->dir_path);
    }
    // printf("root dir: %s\n", currDir->root_dir->dir_path);
    printf("Directory path : %s\n", currDir->dir_path);

    // 파일 출력
    fileNode *currFile = currDir->file_head;
    while (currFile != NULL)
    {
      printf("\t[%d]\n", ++i);
      printf("\tFile path: %s\n", currFile->file_path);
      currFile = currFile->next_file;
    }

    // 하위 디렉토리 검사
    if (currDir->subdir_head != NULL)
    {
      printf("> Sub Directories of \"%s\":\n", currDir->dir_path);
      testStagingList(currDir->subdir_head);
    }

    currDir = currDir->next_dir;
  }
}

// help 명령어의 결과를 출력하는 함수
void help();

// 파일 내용을 읽어 MD5 해시 값을 계산하고 hash_result에 저장
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

// 해시 실행
int ConvertHash(char *target_path, char *hash_result)
{
  if (hash == HASH_MD5)
  {
    md5(target_path, hash_result);
  }
}

// path1과 path2의 해시 값을 비교하는 함수
int cmpHash(char *path1, char *path2)
{
  char *hash1 = (char *)malloc(sizeof(char) * hash);
  char *hash2 = (char *)malloc(sizeof(char) * hash);

  ConvertHash(path1, hash1);
  ConvertHash(path2, hash2);

  return strcmp(hash1, hash2);
}

// 3자리 수씩 끊어서 문자열로 변환하는 함수
char *cvtNumComma(int a)
{
  char *str = (char *)malloc(sizeof(char) * STRMAX);
  char *ret = (char *)malloc(sizeof(char) * STRMAX);
  int i;
  for (i = 0; a > 0; i++)
  {
    str[i] = a % 10 + '0';
    a /= 10;
    if (i % 4 == 2)
    {
      i++;
      str[i] = ',';
    }
  }
  str[i] = '\0';

  for (i = 0; i < strlen(str); i++)
  {
    ret[i] = str[strlen(str) - i - 1];
  }
  ret[i] = '\0';

  return ret;
}

// 경로에서 파일 이름만을 추출하는 함수
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

// str을 입력받아 16진수 형태의 문자열로 변환 후 반환
char *strToHex(char *str)
{
  char *result = (char *)malloc(sizeof(char) * PATHMAX);
  for (int i = 0; i < strlen(str); i++)
  {
    sprintf(result + (i * 2), "%02X", str[i]);
  }
  result[strlen(str) * 2] = '\0';

  return result;
}

// 일정한 형식으로 일시를 받아오는 함수
char *getDate()
{
  char *date = (char *)malloc(sizeof(char) * 14);
  time_t timer;
  struct tm *t;

  timer = time(NULL);
  t = localtime(&timer);

  sprintf(date, "%02d%02d%02d%02d%02d%02d", t->tm_year % 100, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);

  return date;
}

// 문자열에서 구분자를 찾아서 구분자를 제거하는 함수
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

// 구분자를 기준으로 토큰을 분리하는 함수
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

// Tokenize 호출함
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

// 절대 경로로 변환하는 함수
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

// 두 경로를 비교하여 정렬 순서를 결정
int cmpPath(char *path1, char *path2)
{
  int i;
  int cnt1, cnt2;
  char tmp1[PATHMAX], tmp2[PATHMAX];
  strcpy(tmp1, path1);
  strcpy(tmp2, path2);
  char **pathlist1 = GetSubstring(tmp1, &cnt1, "/");
  char **pathlist2 = GetSubstring(tmp2, &cnt2, "/");

  if (cnt1 == cnt2)
  {
    for (i = 0; i < cnt1; i++)
    {
      if (!strcmp(pathlist1[i], pathlist2[i]))
        continue;
      return -strcmp(pathlist1[i], pathlist2[i]);
    }
  }
  else
  {
    return cnt1 < cnt2;
  }
  return 1;
}

// 주어진 문자열인 str에서 특정 범위의 문자열을 추출
// beg : 추출 시작할 인덱스
// end : 추출 끝 인덱스
char *substr(char *str, int beg, int end)
{
  char *ret = (char *)malloc(sizeof(char) * (end - beg + 1));

  for (int i = beg; i < end && *(str + i) != '\0'; i++)
  {
    ret[i - beg] = str[i];
  }
  ret[end - beg] = '\0';

  return ret;
}

// 주어진 문자열을 복사하여 새로운 문자열 생성(입력된 문자열의 복사본 반환)
char *c_str(char *str)
{
  return substr(str, 0, strlen(str));
}

// 파일 노드 초기화 함수
void fileNode_init(fileNode **file_node)
{
  (*file_node)->root_dir = NULL;
  (*file_node)->prev_file = NULL;
  (*file_node)->next_file = NULL;
}

// 디렉터리 노드 초기화 함수
void dirNode_init(dirNode **node)
{
  (*node)->root_dir = NULL;
  (*node)->file_head = NULL;
  (*node)->subdir_head = NULL;
  (*node)->prev_dir = NULL;
  (*node)->next_dir = NULL;
}

// 연결리스트 구축 과정에서 : 중복된 파일 노드를 거르기 위한 함수
fileNode *find_file_node(fileNode *head, const char *path)
{
  fileNode *current = head;
  while (current != NULL)
  {
    if (strcmp(current->file_path, path) == 0)
    {
      return current;
    }
    current = current->next_file;
  }
  return NULL;
}

// 연결리스트 구축 과정에서 : 중복된 디렉터리 노드를 거르기 위한 함수
dirNode *find_directory_node(dirNode *head, const char *path)
{
  dirNode *current = head;
  while (current != NULL)
  {
    if (strcmp(current->dir_path, path) == 0)
    {
      return current;
    }
    dirNode *found = find_directory_node(current->subdir_head, path);
    if (found)
      return found;
    current = current->next_dir;
  }
  return NULL;
}

// 디렉터리 노드 삽입 함수
void insert_directory_node(dirNode **head, dirNode *parent, const char *path)
{
  if (find_directory_node(parent ? parent->subdir_head : *head, path))
  {
    // printf("Directory already exists: %s\n", path);
    return; // 경로 중복 확인
  }

  dirNode *newNode = (dirNode *)malloc(sizeof(dirNode));
  if (!newNode)
  {
    perror("Memory allocation failed");
    return;
  }

  strcpy(newNode->dir_path, path);
  newNode->file_head = NULL;
  newNode->subdir_head = NULL;
  newNode->root_dir = parent; // 부모 노드 설정
  newNode->prev_dir = NULL;
  newNode->next_dir = NULL;

  if (parent)
  {
    if (parent->subdir_head == NULL)
    {
      parent->subdir_head = newNode;
    }
    else
    {
      dirNode *temp = parent->subdir_head;
      while (temp->next_dir)
      {
        temp = temp->next_dir;
      }
      temp->next_dir = newNode;
      newNode->prev_dir = temp;
    }
  }
  else
  {
    if (*head == NULL)
    {
      *head = newNode;
    }
    else
    {
      dirNode *temp = *head;
      while (temp->next_dir)
      {
        temp = temp->next_dir;
      }
      temp->next_dir = newNode;
      newNode->prev_dir = temp;
    }
  }
}

// 파일 노드 삽입 함수
void insert_file_node(dirNode *parent, const char *path)
{
  if (!parent)
    return;

  if (find_file_node(parent->file_head, path))
  {
    // printf("File already exists: %s\n", path);
    return; // 파일 경로 중복 확인
  }

  fileNode *newNode = (fileNode *)malloc(sizeof(fileNode));
  if (!newNode)
  {
    perror("Memory allocation failed");
    return;
  }

  strcpy(newNode->file_path, path);
  newNode->next_file = NULL;
  newNode->prev_file = NULL;

  if (parent->file_head == NULL)
  {
    parent->file_head = newNode;
  }
  else
  {
    fileNode *temp = parent->file_head;
    while (temp->next_file)
    {
      temp = temp->next_file;
    }
    temp->next_file = newNode;
    newNode->prev_file = temp;
  }
}

// 파일 노드 삭제 함수
void remove_file_node(dirNode *parent, const char *path)
{
  if (!parent)
    return;

  // printf("파일 부모 노드 제대로 찾아왔니? %s\n", parent->dir_path);
  // printf("삭제할 파일 노드의 경로 %s\n", path);

  // path에 해당하는 파일 노드를 먼저 찾기
  fileNode *findRemoveNode = parent->file_head;
  // printf("file_head 찾기 : %s\n", parent->file_head->file_path);

  while (findRemoveNode != NULL)
  {
    if (!strcmp(findRemoveNode->file_path, path))
    {
      // 삭제하려는 파일 노드가 맨 앞에 있는 경우
      if (parent->file_head == findRemoveNode)
      {
        parent->file_head = findRemoveNode->next_file;
        if (parent->file_head != NULL)
        {
          parent->file_head->prev_file = NULL;
        }
      }
      else
      {
        // 삭제하려는 파일 노드가 중간 또는 마지막 노드인 경우
        if (findRemoveNode->next_file != NULL)
        {
          findRemoveNode->next_file->prev_file = findRemoveNode->prev_file;
        }
        if (findRemoveNode->prev_file != NULL)
        {
          findRemoveNode->prev_file->next_file = findRemoveNode->next_file;
        }
      }

      // 메모리 해제
      free(findRemoveNode);
      break;
    }
    findRemoveNode = findRemoveNode->next_file;
  }
  // 만약 디렉터리가 비었으면 디렉터리 삭제 함수 여기 추가해도 될 듯
}

// 연결리스트에서 디렉터리 노드를 삭제하는 함수
void remove_dir_node(dirNode **head, dirNode *nodeToDelete)
{
  if (!nodeToDelete || !head || !*head)
    return;

  if (strcmp(nodeToDelete->dir_path, exePATH) == 0)
  {
    // 현재 작업 디렉터리는 삭제하지 않음
    return;
  }

  if (nodeToDelete->prev_dir)
  {
    nodeToDelete->prev_dir->next_dir = nodeToDelete->next_dir;
  }
  else
  {
    *head = nodeToDelete->next_dir; // 노드가 헤드인 경우, 헤드를 업데이트
  }
  if (nodeToDelete->next_dir)
  {
    nodeToDelete->next_dir->prev_dir = nodeToDelete->prev_dir;
  }

  free(nodeToDelete);
}

// 루트 디렉터리부터 하위 디렉터리를 재귀적으로 탐색하며 비어있는 디렉터리 삭제
void recursiveDeleteEmptyDirs(dirNode **head)
{
  if (!head || !*head)
    return;

  dirNode *current = *head;
  while (current)
  {
    // 하위 디렉터리 먼저 재귀적으로 처리
    if (current->subdir_head)
    {
      recursiveDeleteEmptyDirs(&(current->subdir_head));
    }

    // 삭제할 디렉터리를 찾은 후 다음 디렉터리로 이동
    dirNode *next = current->next_dir;

    // 현재 디렉터리가 비어있는지 확인
    if (current->file_head == NULL && current->subdir_head == NULL && current->root_dir != NULL)
    {
      remove_dir_node(head, current);
    }

    current = next;
  }
}

// add된 로그 파일의 경로를 읽어 연결리스트 구축 (경로들을 한 줄씩 인자로 받음)
void add_staging(char *path, dirNode *root)
{
  struct stat statbuf, tmpbuf, subbuf;
  char tmpPath[PATHMAX];
  char exceptExePath[PATHMAX]; // 현재 작업 디렉터리를 제외한 경로를 저장할 배열 선언
  char **dirList = NULL;
  int pathDepth = 0;
  struct dirent **namelist;
  int exePathFlag = 0;

  // printf("인자로 받은 전체 절대 경로 : %s\n", path);

  // 전체 경로에 대한 정보를 statbuf에 담아오기
  if (stat(path, &statbuf) != 0)
  {
    // perror("Failed to get file status");
    return;
  }

  strcpy(exceptExePath, path + strlen(exePATH)); // 현재 작업 디렉터리 경로를 제외하여 저장
  // printf("작업 디렉터리 경로 제외 경로 : %s\n", exceptExePath);

  // 현재 작업디렉터리가 경로로 들어온 경우
  if (!strcmp(exceptExePath, ""))
  {
    // printf("<<<<< null\n");
    exePathFlag = 1;
  }

  dirList = GetSubstring(exceptExePath, &pathDepth, "/");
  // 경로를 /로 쪼개서 리스트에 저장

  for (int i = 0; i < pathDepth; i++)
  {
    // printf("dirList[%d] : %s\n", i, dirList[i]);
  }

  strcpy(tmpPath, exePATH); // 경로에 리스트 값을 하나씩 붙여주기 위해

  dirNode *current_dir = root; // 현재 디렉터리 노드 포인터

  if (exePathFlag == 1)
  {
    int cnt;
    struct dirent **namelist;
    struct stat statbuf;

    if ((cnt = scandir(exePATH, &namelist, NULL, alphasort)) == -1)
    {
      fprintf(stderr, "ERROR: scandir error for %s\n", exePATH);
      return;
    }

    for (int i = 0; i < cnt; i++)
    {
      if (!strcmp(namelist[i]->d_name, ".") || !strcmp(namelist[i]->d_name, "..") || !strcmp(namelist[i]->d_name, ".repo"))
        continue;

      char *full_path = (char *)malloc(sizeof(char) * (PATHMAX + 1));
      sprintf(full_path, "%s/%s", exePATH, namelist[i]->d_name);
      // printf("!!! 디렉터리 내 경로 : %s\n", full_path);

      // 디렉터리 내 경로에 대한 파일 검사
      if (lstat(full_path, &statbuf) != 0)
      {
        // perror("Failed to get file status");
        return;
      }

      if (S_ISDIR(statbuf.st_mode))
      {
        add_staging(full_path, current_dir);
      }
      else if (S_ISREG(statbuf.st_mode))
      {
        // 파일 추가
        insert_file_node(current_dir, full_path); // 파일 노드 추가
      }
    }
  }
  else
  {
    for (int i = 0; i < pathDepth; i++)
    {
      strcat(tmpPath, "/");
      strcat(tmpPath, dirList[i]);

      if (stat(tmpPath, &tmpbuf) != 0)
      {
        // perror("Failed to get file status");
        return;
      }

      // 전체 경로가 디렉터리 였던 경우
      if (S_ISDIR(statbuf.st_mode))
      {
        if (S_ISDIR(tmpbuf.st_mode))
        {
          // tmpbuf로 만들어지는 경로에 있는 모든 디렉터리 노드를 만들고
          // 연결리스트에 디렉터리 노드 추가
          if (current_dir == NULL)
          {
            insert_directory_node(&root, NULL, tmpPath);      // 최상위 디렉터리
            current_dir = find_directory_node(root, tmpPath); // 추가한 노드를 현재 디렉터리로 설정
          }
          else
          {
            insert_directory_node(&current_dir->subdir_head, current_dir, tmpPath); // 하위 디렉터리
            current_dir = find_directory_node(current_dir->subdir_head, tmpPath);   // 추가한 노드를 현재 디렉터리로 설정
          }
        }
        // 마지막엔 그 디렉터리 노드 내의 파일들을 모두 add
        if (i == pathDepth - 1)
        {
          int cnt; // 디렉터리 내의 파일이나 디렉터리 갯수
          if ((cnt = scandir(tmpPath, &namelist, NULL, alphasort)) == -1)
          {
            fprintf(stderr, "ERROR: scandir error for %s\n", tmpPath);
            return;
          }

          for (int i = 0; i < cnt; i++)
          {
            if (!strcmp(namelist[i]->d_name, ".") || !strcmp(namelist[i]->d_name, ".."))
              continue;

            char *full_path = (char *)malloc(sizeof(char) * (PATHMAX + 1));
            sprintf(full_path, "%s/%s", tmpPath, namelist[i]->d_name);
            // printf("디렉터리 내 경로 : %s\n", full_path);

            // 디렉터리 내 경로에 대한 파일 검사
            if (lstat(full_path, &subbuf) != 0)
            {
              // perror("Failed to get file status");
              return;
            }

            if (S_ISDIR(subbuf.st_mode))
            {
              add_staging(full_path, current_dir);
            }
            else if (S_ISREG(subbuf.st_mode))
            {
              // 파일 추가
              insert_file_node(current_dir, full_path); // 파일 노드 추가
            }
          }
        }
      }
      else if (S_ISREG(statbuf.st_mode)) // 전체 경로가 파일이였던 경우
      {
        if (S_ISDIR(tmpbuf.st_mode))
        {
          // tmpbuf로 만들어지는 모든 디렉터리 노드는 생성
          // 연결리스트에 디렉터리 노드 추가
          if (current_dir == NULL)
          {
            insert_directory_node(&root, NULL, tmpPath);      // 최상위 디렉터리
            current_dir = find_directory_node(root, tmpPath); // 추가한 노드를 현재 디렉터리로 설정
          }
          else
          {
            insert_directory_node(&current_dir->subdir_head, current_dir, tmpPath); // 하위 디렉터리
            current_dir = find_directory_node(current_dir->subdir_head, tmpPath);   // 추가한 노드를 현재 디렉터리로 설정
          }
        }
        else if (S_ISREG(tmpbuf.st_mode))
        {
          // 최종 파일의 경로가 나오면 파일 노드를 생성
          insert_file_node(current_dir, path);
        }
      }
    }
  }
}

// remove된 로그 파일의 경로를 읽어 연결리스트 구축 (경로들을 한 줄씩 인자로 받음)
void remove_staging(char *path, dirNode *root)
{
  struct stat statbuf, tmpbuf, subbuf;
  char tmpPath[PATHMAX];
  char exceptExePath[PATHMAX]; // 현재 작업 디렉터리를 제외한 경로
  char **dirList = NULL;
  int pathDepth = 0;
  struct dirent **namelist;
  int exePathFlag = 0; // 현재 작업 디렉터리가 들어올 경우 플래그

  // printf("remove_staging > %s\n", path);
  // 전체 경로에 대한 정보를 statbuf에 담아오기
  if (stat(path, &statbuf) != 0)
  {
    // perror("Failed to get file status");
    return;
  }

  strcpy(exceptExePath, path + strlen(exePATH)); // 현재 작업 디렉터리 경로를 제외하여 저장
  // printf("작업 디렉터리 경로 제외 경로 : %s\n", exceptExePath);

  // 현재 작업 디렉터리가 경로로 들어온 경우 (잘 들어가는지 확인 필요)
  if (!strcmp(exceptExePath, ""))
  {
    // printf("path is exePath ! \n");
    exePathFlag = 1;
  }

  dirList = GetSubstring(exceptExePath, &pathDepth, "/");
  // 경로를 /로 쪼개서 리스트에 저장

  for (int i = 0; i < pathDepth; i++)
  {
    // printf("dirList[%d] : %s\n", i, dirList[i]);
  }

  strcpy(tmpPath, exePATH); // 경로에 리스트 값을 하나씩 붙여주기 위해

  dirNode *current_dir = root; // 현재 디렉터리 노드 포인터

  if (exePathFlag == 1) // 경로로 현재 작업 디렉터리가 들어온 경우
  {
    // printf("exePathFlag == 1 인 경우\n");
    int cnt;
    struct dirent **namelist;
    struct stat statbuf;

    if ((cnt = scandir(exePATH, &namelist, NULL, alphasort)) == -1)
    {
      fprintf(stderr, "ERROR: scandir error for %s\n", exePATH);
      return;
    }

    for (int i = 0; i < cnt; i++)
    {
      if (!strcmp(namelist[i]->d_name, ".") || !strcmp(namelist[i]->d_name, "..") || !strcmp(namelist[i]->d_name, ".reop"))
        continue;

      char *full_path = (char *)malloc(sizeof(char) * (PATHMAX + 1));
      sprintf(full_path, "%s/%s", exePATH, namelist[i]->d_name);
      // printf(">> full_path : %s\n", full_path);

      if (lstat(full_path, &statbuf) != 0)
      {
        // perror("Failed to get file status");
        return;
      }

      if (S_ISDIR(statbuf.st_mode))
      {
        remove_staging(full_path, current_dir);
      }
      else if (S_ISREG(statbuf.st_mode))
      {
        remove_file_node(current_dir, full_path);
      }
    }
  }
  else
  {
    for (int i = 0; i < pathDepth; i++)
    {
      strcat(tmpPath, "/");
      strcat(tmpPath, dirList[i]);

      if (stat(tmpPath, &tmpbuf) != 0)
      {
        // perror("Failed to get file status");
        return;
      }

      // 전체 경로가 디렉터리 였던 경우
      if (S_ISDIR(statbuf.st_mode))
      {
        if (S_ISDIR(tmpbuf.st_mode))
        {
          if (current_dir == NULL)
          {
            current_dir = find_directory_node(root, tmpPath);
            // printf("처음 dir : %s\n", current_dir->dir_path);
          }
          else
          {
            current_dir = find_directory_node(current_dir->subdir_head, tmpPath);
            // printf("다음 dir : %s\n", current_dir->dir_path);
          }
        }

        if (i == pathDepth - 1)
        {
          int cnt; // 디렉터리 내의 파일이나 디렉터리 갯수
          if ((cnt = scandir(tmpPath, &namelist, NULL, alphasort)) == -1)
          {
            fprintf(stderr, "ERROR: scandir error for %s\n", tmpPath);
            return;
          }

          for (int i = 0; i < cnt; i++)
          {
            if (!strcmp(namelist[i]->d_name, ".") || !strcmp(namelist[i]->d_name, ".."))
              continue;

            char *full_path = (char *)malloc(sizeof(char) * (PATHMAX + 1));
            sprintf(full_path, "%s/%s", tmpPath, namelist[i]->d_name);
            // printf("디렉터리 내 경로 full_path : %s\n", full_path);

            // 디렉터리 내 경로에 대한 파일 검사
            if (lstat(full_path, &subbuf) != 0)
            {
              // perror("Failed to get file status");
              return;
            }

            if (S_ISDIR(subbuf.st_mode))
            {
              remove_staging(full_path, current_dir);
            }
            else if (S_ISREG(subbuf.st_mode))
            {
              // 파일 노드 삭제
              remove_file_node(current_dir, full_path);
            }
          }
        }
      }
      else if (S_ISREG(statbuf.st_mode)) // 전체 경로가 파일이였던 경우
      {
        if (S_ISDIR(tmpbuf.st_mode))
        {
          if (current_dir == NULL)
          {
            current_dir = find_directory_node(root, tmpPath);
            // printf("처음 dir : %s\n", current_dir->dir_path);
          }
          else
          {
            current_dir = find_directory_node(current_dir->subdir_head, tmpPath);
            // printf("다음 dir : %s\n", current_dir->dir_path);
          }
        }
        else if (S_ISREG(tmpbuf.st_mode))
        {
          remove_file_node(current_dir, path);
        }
      }
    }
  }
}

// 경로로 디렉터리가 들어왔을때 디렉터리 내의 모든 파일이나 디렉터리들이 staging 되었는지 확인하는 함수
bool is_fully_staged_directory(dirNode *root, const char *dirPath)
{
  // printf(">>>>> dirPath : %s\n", dirPath);
  dirNode *dir = find_directory_node(root, dirPath);
  if (!dir)
  {
    return false;
  }

  // 디렉터리 내의 모든 파일이나 서브 디렉터리들을 탐색해야 함
  struct dirent **namelist;
  int n = scandir(dirPath, &namelist, NULL, alphasort);
  if (n < 0)
  {
    perror("Failed to scan directory");
    return false;
  }

  bool allStaged = true; // 모두 다 구축되어있는지 확인할 변수
  for (int i = 0; i < n; i++)
  {
    struct dirent *entry = namelist[i];
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 || strcmp(entry->d_name, ".repo") == 0)
    {
      free(namelist[i]);
      continue;
    }

    char fullPath[PATHMAX];
    snprintf(fullPath, sizeof(fullPath), "%s/%s", dirPath, entry->d_name);
    // printf("fullPath : %s\n", fullPath);

    if (entry->d_type == DT_DIR)
    {
      if (!is_fully_staged_directory(root, fullPath))
      {
        free(namelist[i]);
        while (++i < n)
          free(namelist[i]); // 남은 namelist 정리
        free(namelist);
        return false; // 스테이징되지 않은 하위 디렉터리 발견 시 바로 false 반환
      }
    }
    else if (entry->d_type == DT_REG)
    {
      // 파일 스테이징 상태 확인
      if (!find_file_node(dir->file_head, fullPath))
      {
        // printf("%s > %s 존재하지 않습니다.\n", root->dir_path, fullPath);
        free(namelist[i]);
        while (++i < n)
          free(namelist[i]); // 남은 namelist 정리
        free(namelist);
        return false; // 스테이징되지 않은 파일 발견 시 바로 false 반환
      }
    }

    free(namelist[i]);
  }
  free(namelist);

  return allStaged;
}

// 재귀적으로 탐색하며 해당 파일이 staging 되었는지 확인하는 함수
fileNode *find_file_recursively(dirNode *dir, const char *path)
{
  if (dir == NULL)
  {
    return NULL;
  }

  // 현재 디렉터리의 파일 노드 검색
  fileNode *file = find_file_node(dir->file_head, path);
  if (file != NULL)
  {
    return file;
  }

  // 재귀적으로 하위 디렉터리 검색
  dirNode *subdir = dir->subdir_head;
  while (subdir != NULL)
  {
    file = find_file_recursively(subdir, path);
    if (file != NULL)
    {
      return file;
    }
    subdir = subdir->next_dir;
  }

  return NULL;
}

// add에서 사용
// 경로로 들어온 파일 혹은 디렉터리가 이미 staging되어있는지 확인하기 위한 함수
bool is_staged_file_or_directory(dirNode *root, const char *path, struct stat *statbuf)
{
  if (S_ISREG(statbuf->st_mode)) // 경로로 파일이 들어온 경우
  {
    return find_file_recursively(root, path) != NULL;
  }
  else if (S_ISDIR(statbuf->st_mode)) // 경로로 디렉터리가 들어온 경우
  {
    return is_fully_staged_directory(root, path);
  }
  return false;
}

// 하나라도 있으면 false반환, 모두 없어야 true 반환
bool is_none_staged_in_directory(dirNode *root, const char *dirPath)
{
  // printf(">>>>> dirPath : %s\n", dirPath);
  dirNode *dir = find_directory_node(root, dirPath);
  if (!dir)
  {
    return true; // 디렉터리 노드 자체가 없으면 아무 것도 스테이지되지 않은 것으로 간주
  }

  struct dirent **namelist;
  int n = scandir(dirPath, &namelist, NULL, alphasort);
  if (n < 0)
  {
    perror("Failed to scan directory");
    return true; // 디렉터리 스캔 실패는 스테이징되지 않았다고 간주
  }

  for (int i = 0; i < n; i++)
  {
    struct dirent *entry = namelist[i];
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 || strcmp(entry->d_name, ".repo") == 0)
    {
      free(namelist[i]);
      continue;
    }

    char fullPath[PATHMAX];
    snprintf(fullPath, sizeof(fullPath), "%s/%s", dirPath, entry->d_name);

    if (entry->d_type == DT_DIR)
    {
      // 하위 디렉터리에 대한 재귀적 확인
      if (!is_none_staged_in_directory(root, fullPath))
      {
        free(namelist[i]);
        while (++i < n)
          free(namelist[i]); // 남은 namelist 정리
        free(namelist);
        return false; // 스테이징된 하위 디렉터리가 하나라도 있으면 false 반환
      }
    }
    else if (entry->d_type == DT_REG)
    {
      // 파일 스테이징 상태 확인
      if (find_file_node(dir->file_head, fullPath))
      {
        // printf("%s > %s 스테이징 되었습니다.\n", root->dir_path, fullPath);
        free(namelist[i]);
        while (++i < n)
          free(namelist[i]); // 남은 namelist 정리
        free(namelist);
        return false; // 스테이징된 파일 발견 시 바로 false 반환
      }
    }
    free(namelist[i]);
  }
  free(namelist);

  return true; // 디렉터리 내 모든 항목이 스테이징되지 않은 경우
}

// remove에서 사용
// 경로로 디렉터리가 들어온 경우 하나라도 있으면 false 반환
// 모두 존재하지 않는 경우 true 반환
// 파일의 경우 존재하면 false 반환됨 -> 바로 리무브
bool is_none_file_or_directory(dirNode *root, const char *path, struct stat *statbuf)
{
  if (S_ISREG(statbuf->st_mode)) // 경로로 파일이 들어온 경우
  {
    return find_file_recursively(root, path) == NULL;
  }
  else if (S_ISDIR(statbuf->st_mode)) // 경로로 디렉터리가 들어온 경우
  {
    return is_none_staged_in_directory(root, path);
  }
}

// 주어진 경로가 staging 리스트에 존재하지 않으면 false 반환, 존재하면 true 반환
int isInStagingList(const char *path)
{
  return find_file_recursively(staging_dir_list, path) != NULL;
}

// 주어진 경로가 managing 리스트에 존재하지 않으면 false 반환, 존재하면 true 반환
int isInManagingList(const char *path)
{
  return find_file_recursively(managing_dir_list, path) != NULL;
}

// 재귀적으로 디렉토리를 탐색하며 파일 상태를 출력하는 함수
// untracked 파일이 발견되면 1을 반환하고, 없으면 0을 반환
int statusUntrackedCheck(const char *basePath, int *firstPrint)
{
  struct dirent **namelist;
  int n = scandir(basePath, &namelist, NULL, alphasort);
  if (n < 0)
  {
    perror("Failed to scan directory");
    return 0;
  }

  char fullPath[PATHMAX];
  int foundUntracked = 0;
  for (int i = 0; i < n; i++)
  {
    struct dirent *entry = namelist[i];
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 || strcmp(entry->d_name, ".repo") == 0)
    {
      free(namelist[i]);
      continue;
    }

    snprintf(fullPath, sizeof(fullPath), "%s/%s", basePath, entry->d_name);
    if (entry->d_type == DT_DIR)
    {
      // 하위 디렉터리에 대해 재귀적 호출
      foundUntracked |= statusUntrackedCheck(fullPath, firstPrint);
    }
    else if (entry->d_type == DT_REG)
    {
      // 일반 파일 처리
      if (!isInStagingList(fullPath) && !isInManagingList(fullPath))
      {
        if (!*firstPrint)
        {
          printf("Untracked files:\n");
          *firstPrint = 1;
        }
        printf("  new file: \".%s\"\n", fullPath + strlen(exePATH));
        foundUntracked = 1;
      }
    }
    free(namelist[i]);
  }
  free(namelist);
  return foundUntracked;
}