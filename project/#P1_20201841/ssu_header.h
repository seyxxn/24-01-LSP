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
#include <libgen.h>

#define true 1
#define false 0

#define HASH_MD5 33

#define NAMEMAX 255
#define PATHMAX 4096
#define STRMAX 4096

#define CMD_BACKUP 0b0000001
#define CMD_REM 0b0000010
#define CMD_REC 0b0000100
#define CMD_SYS 0b0001000
#define CMD_LIST 0b0010000
#define CMD_EXIT 0b0100000
#define NOT_CMD 0b0000000

#define OPT_D 0b000001
#define OPT_R 0b000010
#define OPT_Y 0b000100
#define OPT_A 0b001000
#define OPT_L 0b010000
#define OPT_N 0b100000
#define NOT_OPT 0b000000

char exeNAME[PATHMAX]; // 실행 파일의 이름(ssu_backup)을 저장할 변수
char exePATH[PATHMAX]; // 실행 경로를 저장할 변수
char homePATH[PATHMAX]; // /home/사용자이름 
char backupPATH[PATHMAX]; // 백업 디렉터리 경로 (/home/backup)
char logfilePATH[PATHMAX]; // 백업 로그파일 경로 (/home/backup/ssubak.log)
int hash = HASH_MD5; // md5의 해시 크기(33)를 담은 변수

// 프로그램에 사용할 명령어를 모두 담고 있는 배열
char *commanddata[10] = {
    "backup",  // 0
    "remove",  // 1
    "recover", // 2
    "list",     // 3
    "rm",      // 4
    "rc",      // 5
    "vi",      // 6
    "vim",     // 7
    "help",    // 8
    "exit",    // 9
};

// 사용자에게 입력받은 명령어와 파라미터들을 담는 구조체
typedef struct command_parameter
{
  char *command; // 명령어
  char *filename; // 원본 파일 경로
  char *tmpname;
  int commandopt; // 입력한 옵션의 정보를 저장
  char *argv[10]; // 옵션 뒤의 인자 등을 저장할 공간
} command_parameter;

// 해시 값을 구할 경로를 target_path로 넘겨, 그에 대한 md5 해시 값을 hash_result에 저장하는 함수
int md5(char *target_path, char *hash_result)
{
  FILE *fp;
  unsigned char hash[MD5_DIGEST_LENGTH];
  unsigned char buffer[SHRT_MAX];
  int bytes = 0;
  MD5_CTX md5;

  if ((fp = fopen(target_path, "rb")) == NULL)
  {
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

// target_path로 해시 값을 구할 경로를 넘겨, hash_result에 저장하는 함수
int ConvertHash(char *target_path, char *hash_result)
{
  md5(target_path, hash_result);
}

// 로그 관련 연결리스트 --------------------------

// 로그파일을 읽어서 백업 파일의 연결리스트를 구축할 노드 구조체
struct LogBackupNode
{
  char timestamp[15];       // 백업 시간 정보
  char original_path[4096]; // 백업되었거나 삭제된 원본 파일 경로
  char backup_path[4096];   // 백업된 파일의 경로
  char action[256];

  struct LogBackupNode *prev;
  struct LogBackupNode *next; // 다음 노드를 가리키는 포인터
};

// 연결 리스트의 헤드 포인터
struct LogBackupNode *logHead = NULL;
struct LogBackupNode *logTail = NULL;

// list에서 출력할 때 사용하는 포인터
struct LogBackupNode *mainHead = NULL;
struct LogBackupNode *mainTail = NULL;

// list 연결리스트 테스트 출력 코드
void printTest(){
  struct LogBackupNode *cur = mainHead;
    while (cur != NULL)
  {
    printf("time : %s\n", cur->timestamp);
    printf("origin : %s\n", cur->original_path);
    printf("backup : %s\n\n", cur->backup_path);

    cur = cur->next;
  }
}

// list 명령어에서 입력받은 디렉터리에 대한 백업 파일들의 노드들을 추가할때 오름차순으로 추가하는 함수
void insertInOrder(struct LogBackupNode **headRef, struct LogBackupNode *newNode) {
    struct LogBackupNode *current = *headRef;
    struct LogBackupNode *prev = NULL;

    while (current != NULL && strcmp(newNode->original_path, current->original_path) > 0) {
        prev = current;
        current = current->next;
    }

    // 중복된 노드가 존재하는 경우 제거
    if (current != NULL && strcmp(newNode->original_path, current->original_path) == 0) {
        // 중복된 노드 처리 (current를 제거)
        if (prev != NULL) {
            prev->next = current->next;
        } else {
            *headRef = current->next; // head가 중복 노드인 경우
        }

        if (current->next != NULL) {
            current->next->prev = prev;
        }

        free(current); // 중복된 노드의 메모리 해제
    }

    // 새 노드 삽입
    if (prev == NULL) { // 리스트의 시작 부분에 삽입
        newNode->next = *headRef;
        if (*headRef != NULL) {
            (*headRef)->prev = newNode;
        }
        *headRef = newNode;
    } else { // 중간이나 끝에 삽입
        newNode->next = prev->next;
        newNode->prev = prev;
        prev->next = newNode;
        if (newNode->next != NULL) {
            newNode->next->prev = newNode;
        }
    }

    // 테일 업데이트: 삽입된 노드가 마지막 노드인 경우 테일을 업데이트합니다.
    if (newNode->next == NULL) {
        mainTail = newNode;
    }
}

// 문자열을 '/'단위로 쪼개 몇개가 나오는지 반환하는 함수
int countWords(char *input) {
    // 입력 문자열의 복사본을 만듭니다.
    char *inputCopy = strdup(input);
    if (inputCopy == NULL) {
        // strdup 실패
        return -1;
    }
    
    int count = 0;
    char *token = strtok(inputCopy, "/");
    
    while (token != NULL) {
        count++;
        token = strtok(NULL, "/");
    }
    
    free(inputCopy); // 동적 할당한 메모리 해제
    return count;
}

// 문자열을 처음 나오는 / 로 쪼개 앞부분을 출력하고 뒷부분을 반환하는 함수
char *splitAndPrint(char *str) {
    // '/' 문자의 위치를 찾습니다.
    char *slashPos = strchr(str, '/');
    
    if (slashPos != NULL) {
        // '/'가 발견된 경우
        
        // 첫 번째 부분을 임시로 저장하기 위해 '/' 위치에 NULL 문자를 삽입합니다.
        *slashPos = '\0';
        
        // 첫 번째 부분을 출력합니다.
        printf("%s/\n", str);
        
        // 원본 문자열에 나머지 부분을 저장합니다.
        // +1은 '/' 다음 문자부터 복사하기 위함입니다.
        char *remaining = strdup(slashPos + 1);
        
        // 원본 문자열을 원래 상태로 복원할 필요가 있다면 여기서 처리
        // *slashPos = '/';
        
        return remaining;
    }
}

// 빈 백업 디렉터리 삭제
void deleteEmptyDirectories(const char *basePath) {
    struct dirent *dp;
    char path[PATHMAX];
    DIR *dir = opendir(basePath);

    if (!dir)
        return;

    while ((dp = readdir(dir)) != NULL) {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0) {
            snprintf(path, sizeof(path), "%s/%s", basePath, dp->d_name);
            struct stat statbuf;
            if (lstat(path, &statbuf) == 0 && S_ISDIR(statbuf.st_mode)) {
                deleteEmptyDirectories(path);
            }
        }
    }

    closedir(dir);
    dir = opendir(basePath);
    if (dir) {
        int isEmpty = 1; // Assume the directory is empty
        while ((dp = readdir(dir)) != NULL) {
            if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0) {
                isEmpty = 0; // Directory is not empty
                break;
            }
        }
        closedir(dir);

        if (isEmpty) {
            rmdir(basePath);
        }
    }
}

// 연결 리스트를 이용하여 중복된 파일을 확인하는 함수
// filepath 인자 -> 백업하려는 원본파일의 경로
// command 인자 -> backup에서 사용하면 0, recover에서 사용하면 1
int isDuplicateFile(char *filepath, int command)
{
  struct LogBackupNode *current = logHead;

  if (current == NULL)
  {
    return 0;
  }

  while (current != NULL)
  {
    if (strcmp(current->original_path, filepath) == 0)
    {

      // 두 파일의 내용을 비교하여 중복 여부 확인
      char *filehash = (char *)malloc(sizeof(char *) * hash);
      char *tmphash = (char *)malloc(sizeof(char *) * hash);

      ConvertHash(filepath, filehash); // 원본파일의 경로 해시값
      ConvertHash(current->backup_path, tmphash); // 그 파일의 백업파일 해시값

      if (!strcmp(filehash, tmphash))
      {
        // 두 파일의 내용이 일치하는 경우 중복으로 처리
        if (command == 0)
          printf("\"%s\" already backuped to \"%s\"\n", filepath, current->backup_path);
        return 1;
      }
    }
    current = current->next;
  }
  // 연결 리스트를 모두 확인했는데 중복되는 파일이 없는 경우
  return 0;
}

// 연결 리스트를 출력하는 함수 (테스트용)
void printBackupList()
{
  struct LogBackupNode *current = logHead;
  while (current != NULL)
  {
    printf("LinkedList\n");
    printf("time : %s\n", current->timestamp);
    printf("action : %s\n", current->action);
    printf("origin : %s\n", current->original_path);
    printf("backup : %s\n\n", current->backup_path);

    current = current->next;
  }
}

// 로그 백업 상태를 파일에서 읽어와 연결 리스트에 추가하거나 삭제하는 함수
// (+ backup 명령어인 경우에는 중복 검사) cmpPath : 중복검사하려는 원본 경로
// command로 명령어 구분 (0: backup, 1: remove, 2:recover, 3: list)
int readBackupLog(char *logfilePath, char *cmpPath, int command)
{

  int fd = open(logfilePath, O_RDONLY);
  if (fd == -1)
  {
    printf("Error opening file!\n");
    exit(1);
  }

  char buf[STRMAX]; // 파일 내용을 읽어올 버퍼
  int bytesRead;
  while ((bytesRead = read(fd, buf, sizeof(buf))) > 0)
  {
    char *line = strtok(buf, "\n");
    while (line != NULL)
    {
      struct LogBackupNode *newNode = (struct LogBackupNode *)malloc(sizeof(struct LogBackupNode));
      if (newNode == NULL)
      {
        printf("Memory allocation failed!\n");
        close(fd);
        return 1;
      }

      // 로그 파일에서 정보를 읽어와 노드에 저장
      sscanf(line, "%14s : \"%255[^\"]\" %9s to \"%255[^\"]\"", newNode->timestamp, newNode->original_path, newNode->action, newNode->backup_path);
      newNode->next = NULL;

      // else if 로 변경함 (로그파일에 recover도 들어가서) + recover하면 그 해당 백업파일 삭제해야함
      // 새로운 노드를 연결 리스트에 추가 또는 삭제
      if (strcmp(newNode->action, "removed") == 0 || strcmp(newNode->action, "recovered") == 0)
      {
        struct LogBackupNode *current = logHead;
        while (current != NULL)
        {
          if (strcmp(current->backup_path, newNode->original_path) == 0)
          {
            // 삭제할 노드를 찾음
            if (current == logHead)
            {
              logHead = current->next;
              if (logHead != NULL)
              {
                logHead->prev = NULL;
              }
            }
            else if (current == logTail)
            {
              logTail = current->prev;
              logTail->next = NULL;
            }
            else
            {
              current->prev->next = current->next;
              current->next->prev = current->prev;
            }
            free(current);
            break; // 삭제 후 바로 종료
          }
          current = current->next;
        }
        free(newNode);
      }
      else if (strcmp(newNode->action, "backuped") == 0)
      {
        if (logHead == NULL)
        {
          logHead = newNode;
          logTail = newNode;
        }
        else
        {
          logTail->next = newNode;
          newNode->prev = logTail;
          logTail = newNode;
        }
      }
      line = strtok(NULL, "\n");
    }
  }

  // 새로운 노드를 연결 리스트에 추가
  if (strcmp(cmpPath, "") && command == 0)
  { // 경로가 지정되어 들어온 경우
    if (isDuplicateFile(cmpPath, 0))
      return 1;
  }

  close(fd);
}

// 로그 관련 ---------------------------------------------------

// 백업 파일들의 리스트를 형성하는 노드 구조체
typedef struct backupNode
{
  char backupPath[PATHMAX];
  char newPath[PATHMAX];
  struct stat statbuf;

  struct backupNode *next;
} backupNode;

// 특정 파일 정보를 저장할 노드
typedef struct fileNode
{
  char path[PATHMAX];
  struct stat statbuf;

  backupNode *head;

  struct fileNode *next;
} fileNode;

// 디렉터리 정보를 저장할 노드
typedef struct dirNode
{
  char path[PATHMAX];
  char backupPath[PATHMAX];
  char newPath[PATHMAX];

  fileNode *head;

  struct dirNode *next;
} dirNode;

// 디렉터리 리스트를 관리할 구조체
typedef struct dirList
{
  struct dirNode *head;
  struct dirNode *tail;
} dirList;

dirList *mainDirList;

// 경로의 목록을 관리하는 구조체
typedef struct pathList_
{
  struct pathList_ *next;
  struct pathList_ *prev;
  char path[NAMEMAX];

} pathList;

// 명령어의 usage를 출력하는 help 명령어 함수
void help(char *command);

// 해시값을 비교해주는 함수
int cmpHash(char *path1, char *path2)
{
  char *hash1 = (char *)malloc(sizeof(char) * hash);
  char *hash2 = (char *)malloc(sizeof(char) * hash);

  ConvertHash(path1, hash1);
  ConvertHash(path2, hash2);

  return strcmp(hash1, hash2);
}

// 정수를 입력받아 문자열로 변환, 세 자리마다 쉼표(,)를 추가하여 반환하는 함수
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

// 현재 일시를 특정한 형식의 문자열로 만들어 반환하는 함수
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

// str에서 del(구분자)를 찾아 그 위치 바로 앞의 문자를 제거하는 함수
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

// str을 del(구분자)에 따라 토큰화 하는 함수
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

// str을 del 구분자로 토큰화하고, 그 결과 얻어진 토큰들의 배열을 반환하는 함수
// 분리된 토큰의 개수는 cnt 포인터를 통해 반환
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

// origin(원본경로)을 표준화된 절대 경로로 변환해 resolved에 저장하는 함수
int ConvertPath(char *origin, char *resolved)
{
  int idx = 0;
  int i;
  char *path = (char *)malloc(sizeof(char *) * PATHMAX);    // 오류 수정
  char *tmppath = (char *)malloc(sizeof(char *) * PATHMAX); // 오류 수정
  char **pathlist;
  int pathcnt;

  if (origin == NULL)
  {
    return -1;
  }

  if (origin[0] == '.') // 상대경로로 들어온 경우
  {
    sprintf(path, "%s%s", homePATH, origin + 1);
  }
  else if (origin[0] == '/') // 절대경로로 들어온 경우
  {
    sprintf(path, "%s", origin);
  }
  else
  { // 그냥 파일
    sprintf(path, "%s/%s", homePATH, origin);
  }

  if (!strcmp(path, "/")) // 완전 루트
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

// 두 경로가 일치하는지 판단하는 함수
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