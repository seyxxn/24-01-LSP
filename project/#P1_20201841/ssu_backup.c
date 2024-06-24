#include "ssu_header.h"

void SystemExec(char **arglist); // list 명령어를 실행 후, 사용자 명령을 받을 때 주어진 명령어를 사용하여 외부 프로그램을 실행시키는 함수
int Prompt(char *path, char **backupFileList, int backupFileCnt); // list 명령어를 실행 후, 사용자 명령을 받을 때 해당 명령어를 처리하는 인터페이스 역할
int ListFile(char *path); // list의 명령어 인자로 파일이 들어온 경우를 처리하는 함수
int ListDir(char *basePath); // list의 명령어 인자로 디렉터리가 들어온 경우를 처리하는 함수
int ListCommand(command_parameter *parameter); // list를 수행하기 전 사용자의 명령 조건에 알맞게 처리하기 위해 실행하는 함수

int RecoverFile(char *path, char *newPath, int optDR, int optL); // 파일을 복구하는 함수
int RecoverDir(char *dirPath, char *newPath, int optDR, int optL); // 디렉터리를 복구하는 함수
int RecoverCommand(command_parameter *parameter); // recover를 수행하기 전 사용자의 명령 조건에 알맞게 처리하기 위해 실행하는 함수

int RemoveFile(char *path, int optA, int optL, int optDR); // 백업 파일을 삭제하는 함수
int RemoveDir(char *dirPath, int optDR, int optA, int optL); // 백업 디렉터리를 삭제하는 함수
int RemoveCommand(command_parameter *parameter); // remove를 수행하기 전 사용자의 명령 조건에 알맞게 처리하기 위해 실행하는 함수

int BackupFile(char *path, char *date, char *basedir, int optY); // 파일을 백업하는 함수
int BackupDir(char *path, char *date, char *basedir, int optY); // 디렉터리를 백업하는 함수
int AddCommand(command_parameter *parameter); // backup을 수행하기 전 사용자의 명령 조건에 알맞게 처리하기 위해 실행하는 함수

void CommandFun(char **arglist); // 인자 배열 arglist를 기반으로 특정 명령을 실행하는 함수를 호출하는 역할
void CommandExec(command_parameter parameter); // parameter 구조체로 명령을 실행하는 프로세스를 생성하고 실행하는 함수
void ParameterInit(command_parameter *parameter); // parameter 구조체 멤버 초기화
int ParameterProcessing(int argcnt, char **arglist, int command, command_parameter *parameter); // 명령어에 따라 인자를 처리하고 처리 결과를 parameter 구조체에 저장하는 함수
void Init(); // 함수 실행을 위한 초기화 함수

void SystemExec(char **arglist)
{
  pid_t pid;
  char whichPath[PATHMAX];

  sprintf(whichPath, "/usr/bin/%s", arglist[0]);
  if ((pid = fork()) < 0)
  {
    fprintf(stderr, "ERROR: fork error\n");
    exit(1);
  }
  else if (pid == 0)
  {
    execv(whichPath, arglist);
    exit(0);
  }
  else
  {
    pid = wait(NULL);
  }
}

// path로는 원본 경로를 받음
// 인자로 해당하는 백업 파일 리스트의 경로를 모두 받음
int Prompt(char *path, char **backupFileList, int backupFileCnt)
{
  char input[STRMAX];
  int argcnt = 0;
  char **arglist = NULL;
  int command;
  command_parameter parameter = {(char *)0, (char *)0, (char *)0, 0};

  while (true)
  {
    printf(">> ");
    fgets(input, sizeof(input), stdin);
    input[strlen(input) - 1] = '\0';

    if ((arglist = GetSubstring(input, &argcnt, " \t")) == NULL)
    {
      continue;
    }

    if (argcnt == 0)
      continue;

    // arglist[0] 에는 명령어를 받음. exit, rm, rv, vi, vim 등
    // arglist[1]은 파일 경로를 받아야하는데 여기서는 인덱스를 넘겨줌

    if (!strcmp(arglist[0], commanddata[4]))
    { // rm
      arglist[0] = "remove";
      command = CMD_REM;
    }
    else if (!strcmp(arglist[0], commanddata[5]))
    { // rc
      arglist[0] = "recover";
      command = CMD_REC;
    }
    else if (!strcmp(arglist[0], commanddata[6]))
    { // vi
      command = CMD_SYS;
    }
    else if (!strcmp(arglist[0], commanddata[7]))
    { // vim
      command = CMD_SYS;
    }
    else if (!strcmp(arglist[0], commanddata[9]))
    { // exit
      command = CMD_EXIT;
      exit(0);
    }
    else
    {
      command = NOT_CMD;
    }

    if (command & (CMD_REM | CMD_REC))
    {
      ParameterInit(&parameter);
      parameter.command = arglist[0];
      arglist[1] = path;
      // parameter.filename = path;

      if (ParameterProcessing(argcnt, arglist, command, &parameter) == -1)
      {
        continue;
      }
      parameter.commandopt |= OPT_L; // 파일이 들어올경우 무조건 최신 파일에 대해 적용 시키기 위해

      CommandExec(parameter);
    }
    else if (command & CMD_SYS)
    {
      arglist[1] = path;
      SystemExec(arglist);
    }
  }
}

void printListDir(struct LogBackupNode **cur, int level, int *idx) {
  struct stat statbuf;
    while (*cur != NULL) {
        char *path;
        int cnt = countWords((*cur)->original_path);

        printf("%d. ", (*idx)++);
        for (int i = 0; i < level; i++) printf("  "); // 두 칸 공백으로 들여쓰기

        lstat((*cur)->backup_path, &statbuf);
        
        if (cnt == 1) { // 마지막 파일 또는 폴더
            printf("├─ %s\t\t%s\t%sbytes\n", (*cur)->original_path, (*cur)->timestamp, cvtNumComma(statbuf.st_size));
            *cur = (*cur)->next; // 다음 노드로 이동
        } else { // 폴더
            printf("└─ ");
            path = splitAndPrint((*cur)->original_path); // 첫 부분 출력 및 나머지 저장
            if (path) {
                strcpy((*cur)->original_path, path); // 나머지 부분을 현재 노드에 복사
                free(path); // splitAndPrint에서 할당된 메모리 해제

                printListDir(cur, level + 1, idx); // 재귀 호출로 하위 항목 처리
            }
        }
    }
    // level -= 1; // 이전 레벨로 복귀
}

// basePath : 받아온 원본 디렉터리 경로(/home/ubuntu/a)
int ListDir(char *basePath)
{
  // printf("basePath : %s\n", basePath);

  char **backupFileList = (char **)malloc(sizeof(char *) * PATHMAX);
  // 백업 파일들의 경로를 담을 배열

  char *dirPath = (char *)malloc(sizeof(char *) * STRMAX);
  strcpy(dirPath, basePath);
  strcat(dirPath, "/");

  readBackupLog(logfilePATH, "", 3); // 모든 백업 파일에 대한 정보를 저장한 연결리스트를 구축

  struct LogBackupNode *current = logHead;

  while(current != NULL) {
    // 인자로 받은 디렉터리 경로와 원본 파일의 경로가 같은 것만 또 다른 연결리스트에 저장
    if (strncmp(current->original_path, dirPath, strlen(dirPath)) == 0)
    {
      struct LogBackupNode *newNode = (struct LogBackupNode *)malloc(sizeof(struct LogBackupNode));
      
      strcpy(newNode->original_path, current->original_path + strlen(dirPath));
      // origin_path에는 기준으로하는 경로를 자른 뒤쪽 경로만 들어감
      strcpy(newNode->action, current->action);
      strcpy(newNode->timestamp, current->timestamp);
      strcpy(newNode->backup_path, current->backup_path);

      insertInOrder(&mainHead, newNode);      
    }
    current = current->next;
  }

  // printTest();
  // printf("----------------------\n");

  if (!strcmp(basePath, homePATH)) {
    printf("0. %s\n", homePATH);
  } else {
    printf("0. %s/\n", basePath + strlen(homePATH) + 1);
  }
  int level = 0;
  int idx = 1;
  
  struct LogBackupNode *printCur = mainHead;
  printListDir(&printCur, level, &idx);

}

// path : 받아온 원본 경로
int ListFile(char *path)
{
  char *filepath = (char *)malloc(sizeof(char *) * PATHMAX);

  readBackupLog(logfilePATH, "", 3);
  // printBackupList();

  strcpy(filepath, path); // filepath에 받아온 원본 경로를 저장

  struct LogBackupNode *current = logHead; // 연결리스트를 읽으면서 원본 파일이 같은 것 가져오기
  int backupFileCnt = 0;                   // 원본파일에 대한 같은 백업 파일들 갯수 세기

  struct stat tmpbuf;
  char **backupFileList = (char **)malloc(sizeof(char *) * PATHMAX);
  // 백업 파일들의 경로를 담을 배열

  while (current != NULL)
  {
    if (!strcmp(current->original_path, path))
    {
      backupFileList[backupFileCnt] = current->backup_path;
      backupFileCnt++;
    }
    current = current->next;
  }

  // 백업 디렉토리를 돌았는데도 해당 파일이 존재하지 않는 경우는 잘못된 경로가 들어온 경우, 에러처리
  if (backupFileCnt == 0)
  {
    // // 원본에는 있는데 백업파일이 존재하지 않는경우
    // if (stat(path, &tmpbuf) == 0){
    //   // fprintf(stderr, "ERROR: %s backup file does not exist\n", path); 
    // }
    // else // 원본에도 없고 에러파일도 존재하지 않는 경우
    // {
      fprintf(stderr, "ERROR: %s is invalid filepath\n", path);
    // }
    return 1;
  }

  // 파일일 경우 최근 백업시간을 출력함
  struct stat statbuf;
  if (lstat(backupFileList[backupFileCnt - 1], &statbuf) < 0)
  {
    fprintf(stderr, "lstat error\n");
    return 1;
  }

  char *fileName;
  fileName = strrchr(backupFileList[backupFileCnt - 1], '/');
  if (fileName != NULL)
  {
    fileName++;
  }

  int offset = strlen(backupPATH) + 1;
  char *dateDirName = malloc(strlen(backupFileList[backupFileCnt - 1]));
  strcpy(dateDirName, backupFileList[backupFileCnt - 1] + offset);

  dateDirName[12] = '\0';

  printf("0. %s\t\t%s\t%sbytes\n", fileName, dateDirName, cvtNumComma(statbuf.st_size));

  Prompt(path, backupFileList, backupFileCnt);

  return 0;
}

int ListCommand(command_parameter *parameter)
{
  struct stat statbuf;
  char *tmpPath = (char *)malloc(sizeof(char *) * PATHMAX);
  char *originPath = (char *)malloc(sizeof(char *) * PATHMAX);
  char *backupPath = (char *)malloc(sizeof(char *) * PATHMAX);

  strcpy(originPath, parameter->filename);
  // 인자로 받은 원본 파일의 경로를 저장

  if (stat(originPath, &statbuf) == 0)
  {
    if (lstat(originPath, &statbuf) < 0)
    {
      fprintf(stderr, "ERROR: stat error for %s\n", originPath);
      return 1;
    }
  }

  if (S_ISREG(statbuf.st_mode)) // 경로로 파일이 주어진 경우
  {
    ListFile(parameter->filename);
  }
  else if (S_ISDIR(statbuf.st_mode))
  {
    ListDir(parameter->filename);
  }
}

// path : 원본 경로, newPath : 저장할 이름 경로
int RecoverFile(char *path, char *newPath, int optDR, int optL)
{
  int len;
  int fd1, fd2;
  char *buf = (char *)malloc(sizeof(char *) * STRMAX);
  struct stat statbuf, tmpbuf;

  struct LogBackupNode *current = logHead; // 연결리스트를 읽으면서 원본 파일이 같은 것 가져오기
  int backupFileCnt = 0;
  char **backupFileList = (char **)malloc(sizeof(char *) * PATHMAX);
  char *backupFileSource; // 백업할 파일의 소스 경로

  // 파일의 내용을 비교하여 중복 여부를 확인
  char *backupFilehash = (char *)malloc(sizeof(char *) * hash);
  char *tmphash = (char *)malloc(sizeof(char *) * hash);
  char *tmpPath = (char *)malloc(sizeof(char *) * PATHMAX);
  char **newPathList = NULL;
  int newPathDepth = 0;
  int i;

  while (current != NULL)
  {
    if (!strcmp(current->original_path, path)) // 연결리스트에 담긴 원본 경로와 지금 받아온 원본 경로 비교
    {
      backupFileList[backupFileCnt] = current->backup_path; // 원본경로가 같은 것이 있으면, 그 백업파일의 경로를 저장
      backupFileCnt++;
    }
    current = current->next;
  }

  char *logfileWrite = (char *)malloc(sizeof(char *) * STRMAX);

  int logfd;
  if ((logfd = open(logfilePATH, O_RDWR | O_CREAT | O_APPEND, 0666)) < 0)
  {
    fprintf(stderr, "open error for %s\n", logfilePATH);
    exit(1);
  }

  // 백업 디렉토리를 돌았는데도 해당 파일이 존재하지 않는 경우는 잘못된 경로가 들어온 경우, 에러처리
  if (backupFileCnt == 0)
  {
    if (optDR == 0) {
        fprintf(stderr, "ERROR: %s is invalid filepath\n", path); 
    }
    return 1;
  }

  // 만약 백업 파일이 한개거나, 가장 최신것을 백업하는 경우
  if ((backupFileCnt == 1) || (optL == 1))
  {
    backupFileSource = backupFileList[backupFileCnt - 1]; // 백업할 파일 소스

    // 중복 처리 함수
    ConvertHash(path, tmphash);                    // 원본 경로 파일의 해시값
    ConvertHash(backupFileSource, backupFilehash); // 백업할 파일 소스의 해시값

    if (!strcmp(backupFilehash, tmphash))
    {
      printf("\"%s\" not changed with \"%s\"\n", backupFileSource, path);
      return 1;
    }

    char tmpDir[PATHMAX]; // 새로운 디렉터리경로를 저장할 변수
    strcpy(tmpDir, newPath);

    if ((newPathList = GetSubstring(tmpDir, &newPathDepth, "/")) == NULL)
    {
      fprintf(stderr, "ERROR: %s can't be backuped\n", newPath);
      return -1;
    }

    // 생성할 파일의 경로를 '/'으로 잘라서 newPathList에 저장
    // 그 경로가 존재하지 않을 경우 디렉터리를 생성해야함
    strcpy(tmpPath, "");
    for (i = 0; i < newPathDepth - 1; i++)
    {
      strcat(tmpPath, "/");
      strcat(tmpPath, newPathList[i]);

      // 디렉토리가 존재하지 않을 때만 새로운 디렉토리 생성
      if (access(tmpPath, F_OK) != 0)
      {
        if (mkdir(tmpPath, 0777) == -1)
        {
          perror("mkdir");
          fprintf(stderr, "ERROR: Failed to create directory %s\n", tmpPath);
          return -1;
        }
      }
    }

    // 중복이 아니라면
    if ((fd1 = open(backupFileSource, O_RDONLY)) < 0) // 백업할 파일의 소스에서 읽어서
    {
      fprintf(stderr, "ERROR: open error for %s\n", backupFileSource);
      return 1;
    }

    if ((fd2 = open(newPath, O_CREAT | O_TRUNC | O_WRONLY, 0777)) < 0) // 저장할 경로에 쓴다.
    {
      perror("open");
      fprintf(stderr, "ERROR: open error for %s\n", newPath);
      return 1;
    }

    while ((len = read(fd1, buf, sizeof(buf))) > 0)
    {
      write(fd2, buf, len);
    }

    close(fd1);
    close(fd2);

    // 복구 성공시 출력
    printf("\"%s\" recovered to \"%s\"\n", backupFileSource, newPath);
    remove(backupFileSource);

    // 로그파일에 출력할 것 추가하기
    sprintf(logfileWrite, "%s : \"%s\" recovered to \"%s\"\n", getDate(), backupFileSource, newPath);
  }
  else if ((backupFileCnt >= 2) && (optL == 0)) // 백업파일이 여러개인 경우 (사용자가 선택해야함)
  {
    int backupNum; // 복구할 파일 선택
    printf("backup files of %s\n", path);
    printf("0. exit\n");

    for (int i = 0; i < backupFileCnt; i++) // 선택가능한 백업 파일들 출력
    {
      struct stat statbuf;

      if (lstat(backupFileList[i], &statbuf) < 0)
      {
        fprintf(stderr, "lstat error\n");
        return 1;
      }

      int offset = strlen(backupPATH) + 1;
      char *dateDirName = malloc(strlen(backupFileList[i]));
      strcpy(dateDirName, backupFileList[i] + offset);

      dateDirName[12] = '\0';

      printf("%d. %s\t\t%sbytes\n", i + 1, dateDirName, cvtNumComma(statbuf.st_size));
    }

    printf(">> ");
    scanf("%d", &backupNum); // 백업할 파일의 번호 선택

    // 선택할수 없는 번호를 입력한 경우
    if (backupNum > backupFileCnt || backupNum < 0){
      return 1;
    }

    if (backupNum == 0) // 0 선택시 백업 X
      return 1;
    else
    {                                                   // 번호 선택한 경우
      backupFileSource = backupFileList[backupNum - 1]; // 백업할 파일 소스

      ConvertHash(path, tmphash);                    // 원본 경로 파일의 해시값
      ConvertHash(backupFileSource, backupFilehash); // 백업할 파일 소스의 해시값

      if (!strcmp(backupFilehash, tmphash))
      {
        printf("\"%s\" not changed with \"%s\"\n", backupFileSource, path);
        return 1;
      }

      char tmpDir[PATHMAX]; // 새로운 디렉터리경로를 저장할 변수
      strcpy(tmpDir, newPath);

      if ((newPathList = GetSubstring(tmpDir, &newPathDepth, "/")) == NULL)
      {
        fprintf(stderr, "ERROR: %s can't be recovered\n", newPath);
        return -1;
      }

      // 생성할 파일의 경로를 '/'으로 잘라서 newPathList에 저장
      // 그 경로가 존재하지 않을 경우 디렉터리를 생성해야함
      strcpy(tmpPath, "");
      for (i = 0; i < newPathDepth - 1; i++)
      {
        strcat(tmpPath, "/");
        strcat(tmpPath, newPathList[i]);

        // 디렉토리가 존재하지 않을 때만 새로운 디렉토리 생성
        if (access(tmpPath, F_OK) != 0)
        {
          if (mkdir(tmpPath, 0777) == -1)
          {
            perror("mkdir");
            fprintf(stderr, "ERROR: Failed to create directory %s\n", tmpPath);
            return -1;
          }
        }
      }

      // 중복이 아니라면
      if ((fd1 = open(backupFileSource, O_RDONLY)) < 0)
      {
        fprintf(stderr, "ERROR1: open error for %s\n", path);
        return 1;
      }

      // printf("newPath(RecoverFile) : %s\n", newPath);
      if ((fd2 = open(newPath, O_CREAT | O_TRUNC | O_WRONLY, 0777)) < 0)
      {
        fprintf(stderr, "ERROR: open error for %s\n", newPath);
        return 1;
      }
      while ((len = read(fd1, buf, sizeof(buf))) > 0)
      {
        write(fd2, buf, len);
      }

      close(fd1);
      close(fd2);

      // 복구 성공시 출력
      printf("\"%s\" recovered to \"%s\"\n", backupFileSource, newPath);
      remove(backupFileSource);

      // 로그파일에 출력할 것 추가하기
      sprintf(logfileWrite, "%s : \"%s\" recovered to \"%s\"\n", getDate(), backupFileSource, newPath);
    }
  }

  while ((len = read(fd1, buf, sizeof(buf))) > 0)
  {
    write(fd2, buf, len);
  }

  write(logfd, logfileWrite, strlen(logfileWrite));
  close(logfd);

  return 0;
}

// 디렉터리의 경우, newPath로 디렉터리의 경로가 들어올 것이다.
int RecoverDir(char *dirPath, char *newPath, int optDR, int optL)
{
  DIR *dir = opendir(dirPath);
  struct dirent *entry;

  char *newFullPath = (char *)malloc(sizeof(char *) * PATHMAX);
  // 새로운 완전한 경로를 저장할 변수

  if (!dir)
  {
    fprintf(stderr, "Error: Failed to open directory %s\n", dirPath);
    return 1;
  }

  int dirfd = open(dirPath, O_RDONLY | __O_DIRECTORY);
  if (dirfd == -1)
  {
    perror("Error in open");
    closedir(dir);
    return 1;
  }

  while ((entry = readdir(dir)) != NULL)
  {
    // 현재 항목의 전체 경로 생성
    char fullPath[PATHMAX];
    strcpy(fullPath, dirPath);       // fullPath에 dirPath 복사
    strcat(fullPath, "/");           // fullPath에 "/" 추가
    strcat(fullPath, entry->d_name); // fullPath에 entry->d_name 추가

    // printf("dirPath : %s\n", dirPath);
    // printf("fullPath : %s\n", fullPath); // 디렉터리 내 파일의 전체 경로를 가져온다.
    // printf("newPath : %s\n\n", newPath);

    struct stat statbuf; // 파일 상태 가져오기

    if (stat(fullPath, &statbuf) == 0)
    {
      if (optDR == 1) // 옵션 d인경우에는 파일만 복구
      {
        if (S_ISREG(statbuf.st_mode))
        {
          sprintf(newFullPath, "%s%s", newPath, fullPath + strlen(dirPath));
          RecoverFile(fullPath, newFullPath, optDR, optL);
          continue;
        }
      }
      else if (optDR == 2) // 옵션 r인경우에는 서브디렉터리 내의 파일까지 복구
      {
        if (S_ISREG(statbuf.st_mode))
        {
          sprintf(newFullPath, "%s%s", newPath, fullPath + strlen(dirPath));
          RecoverFile(fullPath, newFullPath, optDR, optL);
        }
        else if (S_ISDIR(statbuf.st_mode))
        {
          // 디렉토리인 경우
          if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
          {

            struct dirent *sub_entry;
            DIR *sub_dir = opendir(fullPath);
            if (!sub_dir)
            {
              fprintf(stderr, "Error: Failed to open directory %s\n", fullPath);
              continue;
            }
            while ((sub_entry = readdir(sub_dir)) != NULL)
            {
              char sub_fullPath[PATHMAX];

              strcpy(sub_fullPath, fullPath);
              strcat(sub_fullPath, "/");
              strcat(sub_fullPath, sub_entry->d_name);

              struct stat sub_statbuf;
              if (lstat(sub_fullPath, &sub_statbuf) == 0)
              {
                if (S_ISREG(sub_statbuf.st_mode))
                {
                  sprintf(newFullPath, "%s%s", newPath, sub_fullPath + strlen(dirPath));
                  RecoverFile(sub_fullPath, newFullPath, optDR, optL);
                }
              }
            }
            closedir(sub_dir);
          }
        }
      }
    }
    else
    {
      fprintf(stderr, "Error: Failed to get file status for %s\n", fullPath);
    }
  }

  // 디렉토리 파일 디스크립터와 디렉토리 닫기
  close(dirfd);
  closedir(dir);
  return 0;
}

int RecoverCommand(command_parameter *parameter)
{
  // recover : 원본 경로를 입력받아 해당 경로와 일치하는 백업 파일 또는 디렉터리를 복구
  struct stat statbuf;
  char *tmpPath = (char *)malloc(sizeof(char *) * PATHMAX);
  char *originPath = (char *)malloc(sizeof(char *) * PATHMAX);
  char *backupPath = (char *)malloc(sizeof(char *) * PATHMAX);
  char *newPath = (char *)malloc(sizeof(char *) * PATHMAX);
  char **newPathList = NULL;
  int newPathDepth = 0;
  int i;

  int flag1 = 0; //-d : 1(파일들만 복구), -r : 2(서브디렉터리 내 파일들까지 복구)
  int flag2 = 0; // -l 사용 x : 0, -l 사용 o : 1 -> 여러개의 파일있으면 가장 최근 것 백업

  strcpy(originPath, parameter->filename); // 원본파일 경로

  readBackupLog(logfilePATH, "", 2); // 로그파일 읽어와서 연결리스트 구축

  if (stat(originPath, &statbuf) == 0)
  {
    if (lstat(originPath, &statbuf) < 0)
    {
      fprintf(stderr, "ERROR: stat error for %s\n", originPath);
      return 1;
    }
  }

  if (parameter->commandopt & OPT_N)
  {                                      // 옵션 n을 적용한 경우
    strcpy(newPath, parameter->tmpname); // 새로운 파일 이름 인자를 newPath에 저장
  }
  else
  {
    strcpy(newPath, originPath); // 옵션 n으로 새로운 이름을 지정하지 않은 경우는 그냥 원본 경로와 같은 곳에 복구
  }
  // recover하여 생성할 파일의 이름을 newPath에 저장

  if (parameter->commandopt & OPT_D)
  {
    flag1 = 1;
  }

  if (parameter->commandopt & OPT_R)
  {
    flag1 = 2;
  }

  // -d나 -r을 사용하지 않았는데 경로가 디렉터리인 경우
  if (S_ISDIR(statbuf.st_mode) && (flag1 == 0))
  {
    fprintf(stderr, "ERROR: \'%s\' is directory.\n", originPath);
    printf(" - use '-d' or '-r' option or input file path.\n");
    exit(1);
  }

  // -d나 -r을 사용하였는데 인자로 받은 경로가 존재하고 파일인 경우 에러처리
  if ((flag1 != 0) && S_ISREG(statbuf.st_mode))
  {
    fprintf(stderr, "ERROR: \'%s\' is a file.\n", originPath);
    exit(1);
  }

  if (parameter->commandopt & OPT_L)
  {
    flag2 = 1;
  }

  strcpy(tmpPath, newPath); // tmpPath에 recover하여 생성할 파일의 이름 저장

  if ((newPathList = GetSubstring(tmpPath, &newPathDepth, "/")) == NULL)
  {
    fprintf(stderr, "ERROR: %s can't be recovered\n", newPath);
    return -1;
  }

  // 생성할 파일의 경로를 '/'으로 잘라서 newPathList에 저장
  // 그 경로가 존재하지 않을 경우 디렉터리를 생성해야함
  strcpy(tmpPath, "");
  for (i = 0; i < newPathDepth - 1; i++)
  {
    strcat(tmpPath, "/");
    strcat(tmpPath, newPathList[i]);
    // printf("tmpPath[%d] : %s\n", i, tmpPath);

    if (access(tmpPath, F_OK))
    {
      mkdir(tmpPath, 0777);
      // printf("만들어진 디렉터리 : %s\n", tmpPath);
    }
  }

  if (parameter->commandopt & OPT_D || parameter->commandopt & OPT_R)
  {
    RecoverDir(originPath, newPath, flag1, flag2);
  }
  else
  {
    RecoverFile(originPath, newPath, flag1, flag2);
  }
}

int RemoveFile(char *path, int optA, int optL, int optDR)
{ // path로 삭제하려는 파일의 경로를 받음
  fileNode *head = (fileNode *)malloc(sizeof(fileNode));
  fileNode *curr = head;

  struct stat statbuf, tmpbuf;
  char *originPath = (char *)malloc(sizeof(char *) * PATHMAX);
  char *filepath = (char *)malloc(sizeof(char *) * PATHMAX);
  char *filename = (char *)malloc(sizeof(char *) * PATHMAX);
  char *tmpPath = (char *)malloc(sizeof(char *) * PATHMAX);
  char *date = (char *)malloc(sizeof(char *) * PATHMAX);

  struct dirent **namelist;
  int cnt;
  int idx;
  int i;
  char input[STRMAX];
  int num;

  strcpy(filepath, path); // filepath에 삭제하려는 파일의 경로 넣음

  struct LogBackupNode *current = logHead; // 연결리스트를 읽으면서 원본 파일이 같은 것 가져오기
  int backupFileCnt = 0;                   // 원본파일에 대한 같은 백업 파일들 갯수 세기
  char **removeBackupFileList = (char **)malloc(sizeof(char *) * PATHMAX);
  // int n = 0;

  while (current != NULL)
  {
    if (!strcmp(current->original_path, path))
    {
      removeBackupFileList[backupFileCnt] = current->backup_path;
      backupFileCnt++;
    }
    current = current->next;
  }

  char *logfileWrite = (char *)malloc(sizeof(char *) * STRMAX);
  int logfd;
  if ((logfd = open(logfilePATH, O_RDWR | O_CREAT | O_APPEND, 0666)) < 0)
  {
    fprintf(stderr, "open error for %s\n", logfilePATH);
    exit(1);
  }

  // 백업 디렉토리를 돌았는데도 해당 파일이 존재하지 않는 경우는 잘못된 경로가 들어온 경우, 에러처리
  if (backupFileCnt == 0)
  {
    if (optDR == 0) {
        fprintf(stderr, "ERROR: %s is invalid filepath\n", path);
    }
    return 1;
  }

  if (backupFileCnt == 1)
  { // 만약 삭제할 백업 파일이 한개라면 바로 삭제
    remove(removeBackupFileList[0]);
    printf("\"%s\" removed by \"%s\"\n", removeBackupFileList[0], path);
    sprintf(logfileWrite, "%s : \"%s\" removed to \"%s\"\n", getDate(), removeBackupFileList[0], path);

    write(logfd, logfileWrite, strlen(logfileWrite));
    close(logfd);
    return 0;
  }
  else if (optA == 1)
  { // 삭제할 백업 파일이 여러개 인데 옵션 A인 경우 모두 삭제
    for (int i = 0; i < backupFileCnt; i++)
    {
      remove(removeBackupFileList[i]);
      printf("\"%s\" removed by \"%s\"\n", removeBackupFileList[i], path);
      sprintf(logfileWrite, "%s : \"%s\" removed to \"%s\"\n", getDate(), removeBackupFileList[i], path);

      write(logfd, logfileWrite, strlen(logfileWrite));
    }
    close(logfd);
    return 0;
  }
  else if (optL == 1) // 추가된 부분
  {
    remove(removeBackupFileList[backupFileCnt - 1]);
    printf("\"%s\" removed by \"%s\"\n", removeBackupFileList[backupFileCnt - 1], path);
    sprintf(logfileWrite, "%s : \"%s\" removed to \"%s\"\n", getDate(), removeBackupFileList[backupFileCnt - 1], path);

    write(logfd, logfileWrite, strlen(logfileWrite));
    close(logfd);
    return 0;
  }
  else
  {                // 삭제할 백업 파일이 여러개라서 선택 후에 삭제해야 하는 경우
    int deleteNum; // 삭제할 파일 선택
    printf("backup files of %s\n", path);
    printf("0. exit\n");

    for (int i = 0; i < backupFileCnt; i++) // 선택가능한 백업 파일들 출력
    {
      struct stat statbuf;
      if (lstat(removeBackupFileList[i], &statbuf) < 0)
      {
        fprintf(stderr, "lstat error\n");
        return 1;
      }

      int offset = strlen(backupPATH) + 1;
      char *dateDirName = malloc(strlen(removeBackupFileList[i]));
      strcpy(dateDirName, removeBackupFileList[i] + offset);

      dateDirName[12] = '\0';
      printf("%d. %s\t\t%sbytes\n", i + 1, dateDirName, cvtNumComma(statbuf.st_size));
    }

    printf(">> ");
    scanf("%d", &deleteNum); // 삭제할 파일의 번호 선택

    // 선택할 수 없는 번호를 입력한 경우
    if ((deleteNum > backupFileCnt) || (deleteNum < 0))
    {
      return 1;
    }

    if (deleteNum == 0) // 0 선택시 삭제 X (exit)
    {
      return 1;
    }
    else // 숫자 선택 시
    {
      remove(removeBackupFileList[deleteNum - 1]);
      printf("\"%s\" removed by \"%s\"\n", removeBackupFileList[deleteNum - 1], path);
      sprintf(logfileWrite, "%s : \"%s\" removed to \"%s\"\n", getDate(), removeBackupFileList[deleteNum - 1], path);

      write(logfd, logfileWrite, strlen(logfileWrite));
    }
    close(logfd);
  }
}

int RemoveDir(char *dirPath, int optDR, int optA, int optL)
{
  DIR *dir = opendir(dirPath);
  struct dirent *entry;

  if (!dir)
  {
    fprintf(stderr, "Error: Failed to open directory %s\n", dirPath);
    return 1;
  }

  // 디렉토리 파일 디스크립터를 얻기 위해 open을 사용
  int dirfd = open(dirPath, O_RDONLY | __O_DIRECTORY);
  if (dirfd == -1)
  {
    perror("Error in open");
    closedir(dir);
    return 1;
  }

  while ((entry = readdir(dir)) != NULL)
  {
    // 현재 항목의 전체 경로 생성
    char fullPath[PATHMAX];
    strcpy(fullPath, dirPath);       // fullPath에 dirPath 복사
    strcat(fullPath, "/");           // fullPath에 "/" 추가
    strcat(fullPath, entry->d_name); // fullPath에 entry->d_name 추가

    struct stat statbuf; // 파일 상태 가져오기

    if (stat(fullPath, &statbuf) == 0)
    {
      if (optDR == 1)
      {
        if (S_ISREG(statbuf.st_mode))
        {
          RemoveFile(fullPath, optA, optL, optDR);
          continue;
        }
      }
      else if (optDR == 2)
      {
        if (S_ISREG(statbuf.st_mode))
        {
          RemoveFile(fullPath, optA, optL, optDR);
        }
        else if (S_ISDIR(statbuf.st_mode))
        {
          // 디렉토리인 경우
          if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
          {

            // optDR이 1이면 현재 디렉토리의 파일에 대한 삭제만 수행
            struct dirent *sub_entry;
            DIR *sub_dir = opendir(fullPath);
            if (!sub_dir)
            {
              fprintf(stderr, "Error: Failed to open directory %s\n", fullPath);
              continue;
            }
            while ((sub_entry = readdir(sub_dir)) != NULL)
            {
              char sub_fullPath[PATHMAX];

              strcpy(sub_fullPath, fullPath);
              strcat(sub_fullPath, "/");
              strcat(sub_fullPath, sub_entry->d_name);

              struct stat sub_statbuf;
              if (lstat(sub_fullPath, &sub_statbuf) == 0)
              {
                if (S_ISREG(sub_statbuf.st_mode))
                {
                  RemoveFile(sub_fullPath, optA, optL, optDR);
                }
              }
            }
            closedir(sub_dir);
          }
        }
      }
    }
    else
    {
      fprintf(stderr, "Error: Failed to get file status for %s\n", fullPath);
    }
  }

  // 디렉토리 파일 디스크립터와 디렉토리 닫기
  close(dirfd);
  closedir(dir);
  return 0;
}

int RemoveCommand(command_parameter *parameter)
{
  struct stat statbuf;
  int i;
  char *backuptime;
  char *originPath = (char *)malloc(sizeof(char *) * PATHMAX);
  char *backupPath = (char *)malloc(sizeof(char *) * PATHMAX);
  char *tmpPath = (char *)malloc(sizeof(char *) * PATHMAX);
  char **backupPathList = NULL;
  int backupPathDepth = 0;

  int flag1 = 0; // -d : 1, -r : 2
  int flag2 = 0; // -a 사용 x : 0, -a 사용 o : 1

  int flag3 = 0;
  // 무조건 최신 파일을 삭제시키기 위한 플래그 (optL)
  // 옵션 조건은 없지만 list 명령어에서 사용하기 위함

  if (parameter->commandopt & OPT_L)
  {
    flag3 = 1;
  }

  int filecnt = 0;
  int dircnt = 0;

  strcpy(originPath, parameter->filename); // 원본 파일 경로

  readBackupLog(logfilePATH, "", 1);

  if (stat(originPath, &statbuf) == 0)
  {
    if (lstat(originPath, &statbuf) < 0)
    {
      fprintf(stderr, "ERROR: stat error for %s\n", originPath);
      return 1;
    }
  }

  // -a : 백업본이 2개 이상인 파일에 대한 목록 출력 안하고 모두 삭제
  if (parameter->commandopt & OPT_A)
  {
    // printf("option a\n");
    flag2 = 1;
  }

  // -d : path가 디렉터리일때 해당 경로 아래의 백업 '파일'들에 대한 삭제
  if (parameter->commandopt & OPT_D)
  {
    flag1 = 1;
  }

  // -r : path가 디렉터리일때 해당 경로 아래의 '모든' 백업 파일들에 대한 삭제
  if (parameter->commandopt & OPT_R)
  {
    flag1 = 2;
  }

  // -d 나 -r을 사용하지 않았는데 경로가 디렉터리인 경우
  if (S_ISDIR(statbuf.st_mode) && (flag1 == 0))
  {
    fprintf(stderr, "ERROR: \'%s\' is directory.\n", originPath);
    printf(" - use '-d' or '-r' option or input file path.\n");
    exit(1);
  }

  if (flag1 == 0)
  {                                       // 수정 필요
    RemoveFile(originPath, flag2, flag3, flag1); // 삭제하려는 파일의 경로를 보냄, -a의 유무(0이면 선택 삭제, 1이면 모두 삭제)
  }
  else
  {                                             // flag1 이 0이 아닌 경우는 -> 삭제 경로로 디렉터리가 들어온 경우에 해당
    RemoveDir(originPath, flag1, flag2, flag3); // -d : flag1(1), -r : flag1(2)
  }

  return 0;
}

int BackupFile(char *path, char *date, char *basedir, int optY)
{
  int len;
  int fd1, fd2;
  char *buf = (char *)malloc(sizeof(char *) * STRMAX);
  struct stat statbuf, tmpbuf;
  struct dirent **namelist;
  int cnt;

  char *filename = (char *)malloc(sizeof(char *) * PATHMAX);
  char *filepath = (char *)malloc(sizeof(char *) * PATHMAX);
  char *tmpPath = (char *)malloc(sizeof(char *) * PATHMAX);
  char *tmpName = (char *)malloc(sizeof(char *) * PATHMAX);
  char *tmpdir = (char *)malloc(sizeof(char *) * PATHMAX);
  char *newPath = (char *)malloc(sizeof(char *) * PATHMAX);
  int idx;
  int i;

  char *filehash = (char *)malloc(sizeof(char *) * hash);
  char *tmphash = (char *)malloc(sizeof(char *) * hash);

  strcpy(filepath, path); // filepath에는 백업할 파일의 경로가 저장되어있음

  for (idx = strlen(filepath) - 1; filepath[idx] != '/'; idx--)
    ;

  strcpy(filename, filepath + idx + 1);
  filepath[idx] = '\0';
  // 위의 과정을 지나면 filepath에는 파일 이름 전까지의 디렉터리까지만 담김

  if (lstat(path, &statbuf) < 0)
  {
    fprintf(stderr, "ERROR: lstat error for %s\n", path);
    return 1;
  }

  ConvertHash(path, filehash);

  if (!optY && readBackupLog(logfilePATH, path, 0))
  {
    return 1;
  }; // path로 백업하려는 파일의 경로를 넘겨주고 중복 검사

  // 추가
  if (!strcmp(basedir, backupPATH))
  {
    sprintf(tmpdir, "%s", basedir); // 백업 파일을 담을 일자 디렉터리 경로 저장
    sprintf(tmpPath, "%s/%s", backupPATH, date);
    sprintf(newPath, "%s/%s/%s", backupPATH, date, filename);
  }
  else
  {
    sprintf(tmpdir, "%s", basedir); // 백업 파일을 담을 일자 디렉터리 경로 저장
    sprintf(newPath, "%s/%s", basedir, filename);
  }
  // 추가 여기까지

  if ((fd1 = open(path, O_RDONLY)) < 0)
  {
    fprintf(stderr, "ERROR: open error for %s\n", path);
    return 1;
  }

  if ((fd2 = open(newPath, O_CREAT | O_TRUNC | O_WRONLY, 777)) < 0)
  {
    fprintf(stderr, "ERROR: open error for %s\n", newPath);
    return 1;
  }

  while ((len = read(fd1, buf, statbuf.st_size)) > 0)
  {
    write(fd2, buf, len);
  }

  // 백업 성공시 출력
  printf("\"%s\" backuped to \"%s\"\n", path, newPath);

  char *logfileWrite = (char *)malloc(sizeof(char *) * STRMAX);
  sprintf(logfileWrite, "%s : \"%s\" backuped to \"%s\"\n", date, path, newPath);

  // 백업 파일에 출력하기
  int logfd;
  if ((logfd = open(logfilePATH, O_RDWR | O_CREAT | O_APPEND, 0666)) < 0)
  {
    fprintf(stderr, "open error for %s\n", logfilePATH);
    exit(1);
  }
  else
  {
    write(logfd, logfileWrite, strlen(logfileWrite));
    close(logfd);
  }

  readBackupLog(logfilePATH, "", 0);
}

int BackupDir(char *path, char *date, char *basedir, int optY)
{
  DIR *dir;
  struct dirent *entry;
  struct stat statbuf;
  char tmpdir[PATHMAX];

  if ((dir = opendir(path)) == NULL)
  {
    fprintf(stderr, "Cannot open directory: %s\n", path);
    return 1;
  }

  while ((entry = readdir(dir)) != NULL)
  {
    // 현재 항목이 디렉터리인지 확인
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
    {
      continue; // 현재 디렉터리나 상위 디렉터리일 경우 건너뜀
    }

    sprintf(tmpdir, "%s/%s", path, entry->d_name); // 백업할 원본 파일의 경로

    if (lstat(tmpdir, &statbuf) < 0)
    {
      fprintf(stderr, "ERROR: lstat error for %s\n", tmpdir);
      continue;
    }

    if (S_ISDIR(statbuf.st_mode))
    {
      // 디렉터리일 경우 재귀적으로 백업 함수 호출
      char subdir_backup_path[PATHMAX];
      sprintf(subdir_backup_path, "%s/%s/%s", basedir, date, entry->d_name);
      if (access(subdir_backup_path, F_OK))
      {
        mkdir(subdir_backup_path, 0777); // 서브 디렉터리 생성
      }
      BackupDir(tmpdir, date, subdir_backup_path, optY); // 재귀 호출
    }
    else if (S_ISREG(statbuf.st_mode))
    {
      // 일반 파일일 경우 백업 함수 호출
      BackupFile(tmpdir, date, basedir, optY);
    }
  }

  closedir(dir);
}

int AddCommand(command_parameter *parameter)
{
  struct stat statbuf;
  char *backuptime;
  char *tmpPath = (char *)malloc(sizeof(char *) * PATHMAX);
  char *originPath = (char *)malloc(sizeof(char *) * PATHMAX);
  char *newBackupPath = (char *)malloc(sizeof(char *) * PATHMAX);
  char **backupPathList = NULL;
  int backupPathDepth = 0;
  int i;

  int flag1 = 0; // 옵션 d = 1, 옵션 r =2
  int flag2 = 0; // 옵션 y 적용하면 1

  strcpy(originPath, parameter->filename); // originPath에 백업하려는 파일의 경로를 저장

  if (lstat(originPath, &statbuf) < 0)
  {
    fprintf(stderr, "ERROR: lstat error for %s\n", originPath);
    return 1;
  }

  if (!S_ISREG(statbuf.st_mode) && !S_ISDIR(statbuf.st_mode))
  {
    fprintf(stderr, "ERROR: %s is not directory or regular file\n", originPath);
    return -1;
  }

  if (parameter->commandopt & OPT_D)
    flag1 = 1;

  if (parameter->commandopt & OPT_R)
    flag1 = 2;

  // -d, -r을 사용하지 않았는데 디렉터리를 입력받은 경우
  if (S_ISDIR(statbuf.st_mode) && (flag1 == 0))
  {
    fprintf(stderr, "ERROR: \'%s\' is directory.\n", originPath);
    printf(" - use '-d' option or input file path.\n");
    exit(1);
  }

  if (parameter->commandopt & OPT_Y)
    flag2 = 1;

  // 추가한 부분
  char *fileName = (char *)malloc(sizeof(char *) * PATHMAX); // 파일이름만 담을 변수
  int idx = 0;
  for (idx = strlen(originPath) - 1; originPath[idx] != '/'; idx--)
    ;
  strcpy(fileName, originPath + idx + 1);

  // 추가한 부분
  // newBackupPath에 백업 파일을 넣을 경로를 저장
  sprintf(newBackupPath, "%s/%s/%s", backupPATH, getDate(), fileName);

  if ((backupPathList = GetSubstring(newBackupPath, &backupPathDepth, "/")) == NULL)
  {
    fprintf(stderr, "ERROR: %s can't be backuped\n", originPath);
    return -1;
  }

  struct dirent **optdlist;
  struct stat optdbuf;
  int optdcnt;
  char *optdPath = (char *)malloc(sizeof(char *) * PATHMAX);

  if (S_ISDIR(statbuf.st_mode))
  {
    if ((optdcnt = scandir(parameter->filename, &optdlist, NULL, alphasort)) == -1)
    {
      fprintf(stderr, "ERROR: scandir error for %s\n", parameter->tmpname);
      return -1;
    }

    if (optdcnt == 2)
    {
      fprintf(stderr, "not file in %s directory\n", parameter->filename);
      exit(1);
    }
  }

  strcpy(tmpPath, "");
  for (i = 0; i < backupPathDepth - 1; i++)
  {
    strcat(tmpPath, "/");
    strcat(tmpPath, backupPathList[i]);

    if (access(tmpPath, F_OK))
    {
      mkdir(tmpPath, 0777);
    }
  }

  if (S_ISREG(statbuf.st_mode))
  {
    BackupFile(originPath, getDate(), backupPATH, flag2);
  }
  else if (S_ISDIR(statbuf.st_mode) && flag1 == 2) // -r 재귀적으로 모두 백업
  {
    char *baseBackupDir = (char *)malloc(sizeof(char) * PATHMAX);
    baseBackupDir = backupPATH;

    mainDirList = (dirList *)malloc(sizeof(dirList));
    dirNode *head = (dirNode *)malloc(sizeof(dirNode));
    mainDirList->head = head;
    dirNode *curr = head->next;
    dirNode *new = (dirNode *)malloc(sizeof(dirNode));
    strcpy(new->path, originPath);
    curr = new;
    mainDirList->tail = curr;

    while (curr != NULL)
    {
      BackupDir(curr->path, getDate(), baseBackupDir, flag2);
      curr = curr->next;
    }
  }
  else if (S_ISDIR(statbuf.st_mode) && flag1 == 1) // -d 파일들만 백업
  {

    for (int i = 0; i < optdcnt; i++)
    {
      strcpy(optdPath, "");
      if (!strcmp(optdlist[i]->d_name, ".") || !strcmp(optdlist[i]->d_name, ".."))
        continue;

      strcat(optdPath, parameter->filename);
      strcat(optdPath, "/");
      strcat(optdPath, optdlist[i]->d_name);

      if (lstat(optdPath, &optdbuf) < 0)
      {
        fprintf(stderr, "ERROR: lstat error for %s\n", optdPath);
        return 1;
      }

      if (S_ISDIR(optdbuf.st_mode))
        continue;
      else if (S_ISREG(optdbuf.st_mode))
      {
        BackupFile(optdPath, getDate(), backupPATH, flag2);
      }
    }
  }

  return 0;
}

void CommandFun(char **arglist)
{
  int (*commandFun)(command_parameter *parameter);
  command_parameter parameter = {
      arglist[0], arglist[1], arglist[2], atoi(arglist[3])};

  if (!strcmp(parameter.command, commanddata[0]))
  {
    commandFun = AddCommand;
  }
  else if (!strcmp(parameter.command, commanddata[1]))
  {
    commandFun = RemoveCommand;
  }
  else if (!strcmp(parameter.command, commanddata[2]))
  {
    commandFun = RecoverCommand;
  }
  else if (!strcmp(parameter.command, commanddata[3]))
  {
    commandFun = ListCommand;
  }

  if (commandFun(&parameter) != 0)
  {
    exit(1);
  }
}

void CommandExec(command_parameter parameter)
{
  pid_t pid;

  parameter.argv[0] = "command";
  parameter.argv[1] = (char *)malloc(sizeof(char *) * 32);
  sprintf(parameter.argv[1], "%d", hash);

  parameter.argv[2] = parameter.command;
  parameter.argv[3] = parameter.filename;
  parameter.argv[4] = parameter.tmpname;
  parameter.argv[5] = (char *)malloc(sizeof(char *) * 32);
  sprintf(parameter.argv[5], "%d", parameter.commandopt);
  parameter.argv[6] = (char *)0;

  if ((pid = fork()) < 0)
  {
    fprintf(stderr, "ERROR: fork error\n");
    exit(1);
  }
  else if (pid == 0)
  {
    execv(exeNAME, parameter.argv);
    exit(0);
  }
  else
  {
    pid = wait(NULL);
  }
}

void ParameterInit(command_parameter *parameter)
{
  parameter->command = (char *)malloc(sizeof(char *) * PATHMAX);
  parameter->filename = (char *)malloc(sizeof(char *) * PATHMAX);
  parameter->tmpname = (char *)malloc(sizeof(char *) * PATHMAX);
  parameter->commandopt = 0;
}

int ParameterProcessing(int argcnt, char **arglist, int command, command_parameter *parameter)
{
  struct stat buf;
  optind = 0;
  opterr = 0;
  int lastind;
  int option;
  int optcnt = 0;

  switch (command)
  {
  case CMD_BACKUP:
  {
    if (argcnt < 2)
    { // 경로 입력이 없을 경우
      fprintf(stderr, "ERROR: missing operand <PATH>\n");
      exit(1);
    }

    if (ConvertPath(arglist[1], parameter->filename) != 0)
    {
      fprintf(stderr, "ERROR: %s is invalid filepath\n", parameter->filename);
      return -1;
    }

    // 경로가 사용자 디렉토리를 벗어난 경우 예외처리, 경로가 백업 디렉터리인 경우, 백업 디렉터리를 포함하는 경우 예외처리
    if (strncmp(parameter->filename, homePATH, strlen(homePATH)) ||
        !strcmp(parameter->filename, backupPATH) ||
        !strncmp(parameter->filename, backupPATH, strlen(backupPATH)))
    {
      fprintf(stderr, "ERROR: path must be in user directory\n");
      fprintf(stderr, " - \'%s\' is not in user directory\n", parameter->filename);
      exit(1);
    }

    if (lstat(parameter->filename, &buf) < 0)
    { // 경로 없을 시 예외처리
      fprintf(stderr, "ERROR: \'%s\' is not exist\n", parameter->filename);
      exit(1);
    }

    if (!S_ISREG(buf.st_mode) && !S_ISDIR(buf.st_mode))
    {
      fprintf(stderr, "ERROR: %s is not regular file\n", parameter->filename);
      return -1;
    }

    lastind = 2;

    // backup에 옵션 3개 존재 (d,r,y)
    while ((option = getopt(argcnt, arglist, "dry")) != -1)
    {
      if (option != 'd' && option != 'r' && option != 'y')
      { // 인자로 받은 옵션이 올바르지 않을 경우
        help("backup");
        exit(1);
      }

      if (optind == lastind)
      {
        fprintf(stderr, "ERROR: wrong option input\n");
        exit(1);
      }

      // 옵션 중 -d나 -r을 사용하였는데 인자로 받은 경로가 파일인 경우
      if ((option == 'd' || option == 'r') && !S_ISDIR(buf.st_mode))
      {
        fprintf(stderr, "ERROR: %s is not directory\n", parameter->filename);
        exit(1);
      }

      // 옵션 d를 입력한 경우
      if (option == 'd')
      {
        if (parameter->commandopt & OPT_D)
        {
          fprintf(stderr, "ERROR: duplicate option -%c\n", option);
          exit(1);
        }
        parameter->commandopt |= OPT_D;
      }

      // 옵션 R를 입력한 경우
      if (option == 'r')
      {
        if (parameter->commandopt & OPT_R)
        {
          fprintf(stderr, "ERROR: duplicate option -%c\n", option);
          exit(1);
        }
        parameter->commandopt |= OPT_R;
      }

      // 옵션 y를 입력한 경우
      if (option == 'y')
      {
        if (parameter->commandopt & OPT_Y)
        {
          fprintf(stderr, "ERROR: duplicate option -%c\n", option);
          exit(1);
        }
        parameter->commandopt |= OPT_Y;
      }

      optcnt++;
      lastind = optind;
    }

    if (argcnt - optcnt != 2)
    {
      fprintf(stderr, "ERROR: argument error\n");
      exit(1);
    }
    break;
  }
  case CMD_REM:
  {
    if (argcnt < 2)
    { // 경로입력이 없는 경우
      help("remove");
      exit(1);
    }

    if (ConvertPath(arglist[1], parameter->filename) != 0)
    {
      fprintf(stderr, "ERROR: %s is invalid filepath\n", parameter->filename);
      return -1;
    }

    // stat 함수를 사용하면 파일이 존재하지않아도 에러처리가 나지 않게 방지 가능
    if (stat(parameter->filename, &buf) == 0)
    {
      if (lstat(parameter->filename, &buf) < 0)
      {
        fprintf(stderr, "ERROR: \'%s\' is not exist\n", parameter->filename);
        exit(1);
      }
    }

    lastind = 1;

    // -d, -r, -a 가 아닌 옵션을 입력한 경우
    while ((option = getopt(argcnt, arglist, "dra")) != -1)
    {
      if (option != 'd' && option != 'r' && option != 'a')
      {
        help("remove");
        exit(1);
      }

      if (optind == lastind)
      {
        fprintf(stderr, "ERROR: wrong option input\n");
        exit(1);
      }

      if (option == 'a') // 백업파일 여러개 존재해도 모두 삭제
      {
        if (parameter->commandopt & OPT_A)
        {
          fprintf(stderr, "ERROR: duplicate option -%c\n", option);
          exit(1);
        }
        parameter->commandopt |= OPT_A;
      }

      if (option == 'r') // path로 디렉터리를 받을 때, 디렉터리내의 서브 디렉터리 내의 파일들도 모두 삭제
      {
        if (parameter->commandopt & OPT_R)
        {
          fprintf(stderr, "ERROR: duplicate option -%c\n", option);
          exit(1);
        }
        if (!S_ISDIR(buf.st_mode))
        {
          fprintf(stderr, "ERROR: %s is not directory\n", parameter->filename);
          exit(1);
        }
        parameter->commandopt |= OPT_R;
      }

      if (option == 'd') // path로 디렉터리를 받을 때, 디렉터리 내의 파일들 삭제
      {
        if (parameter->commandopt & OPT_D)
        {
          fprintf(stderr, "ERROR: duplicate option -%c\n", option);
          exit(1);
        }
        if (!S_ISDIR(buf.st_mode))
        {
          fprintf(stderr, "ERROR: %s is not directory\n", parameter->filename);
          exit(1);
        }
        parameter->commandopt |= OPT_D;
      }

      optcnt++;
      lastind = optind;
    }

    if ((((parameter->commandopt & OPT_A) || (parameter->commandopt & OPT_R) || (parameter->commandopt & OPT_Y)) && argcnt - optcnt != 2))
    {
      fprintf(stderr, "ERROR: argument error\n");
      return -1;
    }

    if (ConvertPath(parameter->filename, parameter->filename) != 0)
    {
      fprintf(stderr, "ERROR: %s is invalid filepath\n", parameter->filename);
      exit(1);
    }

    if (strncmp(parameter->filename, homePATH, strlen(homePATH)) || !strncmp(parameter->filename, backupPATH, strlen(backupPATH)) || !strcmp(parameter->filename, homePATH))
    {
      fprintf(stderr, "ERROR: %s can't be removed\n", parameter->filename);
      exit(1);
    }

    break;
  }
  case CMD_REC:
  {
    if (argcnt < 2) // 경로를 입력하지 않은 경우
    {
      help("recover");
      exit(1);
    }

    if (ConvertPath(arglist[1], parameter->filename) != 0)
    {
      fprintf(stderr, "ERROR: %s is invalid filepath\n", parameter->filename);
      exit(1);
    }

    if (stat(parameter->filename, &buf) == 0)
    {
      if (lstat(parameter->filename, &buf) < 0) // 존재하지 않는 경로 입력시 에러처리
      {
        fprintf(stderr, "ERROR: \'%s\' is not exist\n", parameter->filename);
        exit(1);
      }
    }

    // 경로가 홈 디렉터리를 벗어난 경우
    if (strncmp(parameter->filename, homePATH, strlen(homePATH)) || !strncmp(parameter->filename, backupPATH, strlen(backupPATH)) || !strcmp(parameter->filename, homePATH))
    {
      fprintf(stderr, "ERROR: %s can't be recovered\n", parameter->filename);
      exit(1);
    }

    lastind = 2;

    while ((option = getopt(argcnt, arglist, "drln:")) != -1) // 옵션 d, r, l, n (arglist + 1)
    {
      // n 옵션인데 뒤에 인자를 넣지 않은 경우도 여기에서 에러 처리가 된다.
      if (option != 'd' && option != 'r' && option != 'l' && option != 'n')
      {
        help("recover");
        exit(1);
      }

      if (optind == lastind)
      {
        fprintf(stderr, "ERROR: wrong option input\n");
        exit(1);
      }

      if (option == 'd') // 옵션 d인 경우
      {
        if (parameter->commandopt & OPT_D)
        {
          fprintf(stderr, "ERROR: duplicate option -%c\n", option);
          exit(1);
        }
        parameter->commandopt |= OPT_D;
      }

      if (option == 'r') // 옵션 r인 경우
      {
        if (parameter->commandopt & OPT_R)
        {
          fprintf(stderr, "ERROR: duplicate option -%c\n", option);
          exit(1);
        }

        parameter->commandopt |= OPT_R;
        // optcnt++;
      }

      if (option == 'l') // 옵션 l인 경우
      {
        if (parameter->commandopt & OPT_L)
        {
          fprintf(stderr, "ERROR: duplicate option -%c\n", option);
          exit(1);
        }

        parameter->commandopt |= OPT_L;
        // optcnt++;
      }

      if (option == 'n')
      { // 옵션 n인 경우
        if (parameter->commandopt & OPT_N)
        {
          fprintf(stderr, "ERROR: duplicate option -%c\n", option);
          exit(1);
        }

        if (optarg == NULL)
        { // 옵션 n은 뒤에 <NEWNAME> 인자를 더 받아야함
          fprintf(stderr, "ERROR: <NEWNAME> is null\n");
          exit(1);
        }

        if (ConvertPath(optarg, parameter->tmpname) != 0)
        {
          fprintf(stderr, "ERROR: %s is invalid filepath\n", parameter->tmpname);
          exit(1);
        }

        if (strncmp(parameter->tmpname, homePATH, strlen(homePATH)) || !strncmp(parameter->tmpname, backupPATH, strlen(backupPATH)) || !strcmp(parameter->tmpname, homePATH))
        {
          fprintf(stderr, "ERROR: %s can't be backuped\n", parameter->tmpname);
          exit(1);
        }
        parameter->commandopt |= OPT_N;
        // optcnt++;
      }

      optcnt++;
      lastind = optind;
    }

    break;
  }
  case CMD_LIST:
  {
    if (argcnt == 1) // 경로 입력을 안한경우
    {
      // printf("모든 백업 파일 출력\n");
      // ListDir(homePATH);
      return 0;
    }

    if (ConvertPath(arglist[1], parameter->filename) != 0)
    {
      fprintf(stderr, "ERROR: %s is invalid filepath\n", parameter->filename);
      return 1;
    }

    // 경로가 사용자 디렉토리를 벗어난 경우 예외처리, 경로가 백업 디렉터리인 경우, 백업 디렉터리를 포함하는 경우 예외처리
    if (strncmp(parameter->filename, homePATH, strlen(homePATH)) ||
        !strcmp(parameter->filename, backupPATH) ||
        !strncmp(parameter->filename, backupPATH, strlen(backupPATH)))
    {
      fprintf(stderr, "ERROR: path must be in user directory\n");
      fprintf(stderr, " - \'%s\' is not in user directory\n", parameter->filename);
      exit(1);
    }

    if (stat(parameter->filename, &buf) == 0)
    {
      if (lstat(parameter->filename, &buf) < 0) // 존재하지 않는 경로 입력시 에러처리
      {
        fprintf(stderr, "ERROR: \'%s\' is not exist\n", parameter->filename);
        exit(1);
      }
    }

    if (!S_ISREG(buf.st_mode) && !S_ISDIR(buf.st_mode))
    {
      fprintf(stderr, "ERROR: %s is not regular file\n", parameter->filename);
      return -1;
    }
    break;
  }
  }
      return 0;
}

void Init()
{ // 수정
  getcwd(exePATH, PATHMAX);

  strcat(homePATH, exePATH); // homePATH : /home/ubuntu

  char *home = dirname(exePATH); // /home (exePATH가 /home으로 바뀜)

  sprintf(backupPATH, "%s/backup", home); // backupPATH : /home/backup

  strcat(logfilePATH, backupPATH);
  strcat(logfilePATH, "/ssubak.log"); // logfilePATH : /home/backup/ssubak.log

  if (access(backupPATH, F_OK))
  {
    mkdir(backupPATH, 0777);
  }

  int logfd;
  if ((logfd = open(logfilePATH, O_RDWR | O_CREAT | O_APPEND, 0666)) < 0)
  {
    fprintf(stderr, "open error for %s\n", logfilePATH);
    exit(1);
  }
}

int main(int argc, char *argv[])
{
  Init();

  if (argc == 1) { // 첫번째 인자를 입력하지 않을 경우 에러처리 후 비정상 종료
    printf("ERROR: wrong input\n");
    printf("%s help : show commands for program\n", argv[0]);
    exit(1);
  }

  if (!strcmp(argv[0], "command"))
  {
    CommandFun(argv + 2);
  }
  // help 명령어 입력
  else if (!strcmp(argv[1], "help"))
  {
    if (argc == 2)
    {                    // 인자 입력없이 help 명령어만 입력한 경우
      char *all = "all"; // 문자열 all을 help 함수의 인자로 넘김
      help(all);
    }
    else if (argc == 3)
    {                // help 함수의 인자로 다른 내장 명령어를 입력한 경우
      help(argv[2]); // 입력한 내장 명령어 문자열이 help 함수의 인자로 넘어감
    }
  }
  else // help 명령어 이외의 명령어를 실행시킨 경우
  {
    // 올바르지 않은 명령어를 입력한 경우
    if (strcmp(argv[1], "backup") && strcmp(argv[1], "remove") && strcmp(argv[1], "recover") && strcmp(argv[1], "list") && strcmp(argv[1], "help"))
    {
      fprintf(stderr, "ERROR: invalid command -- %s\n", argv[1]);
      printf("%s help : show commands for program\n", argv[0]);
      exit(1);
    }

    strcpy(exeNAME, argv[0]); // 현재 실행하고 있는 파일이름 저장 (./ssu_backup)
    hash = HASH_MD5;

    int argcnt = argc - 1; // ./ssu_backup backup a.txt => argc = 3
    char **arglist = NULL;
    int command;
    command_parameter parameter = {(char *)0, (char *)0, (char *)0, 0};

    // 전달된 모든 인자를 하나의 문자열로 조합
    int totalLength = 0;
    for (int i = 1; i < argc; i++)
    {                                     // 프로그램 이름을 제외하고 인자를 조합
      totalLength += strlen(argv[i]) + 1; // 각 인자와 공백을 더함
    }

    // 공백을 추가할 공간도 포함하여 메모리를 할당
    char *input = (char *)malloc(totalLength);
    input[0] = '\0'; // 문자열을 초기화

    // 각 인자를 하나의 문자열로
    for (int i = 1; i < argc; i++)
    { // 프로그램 이름을 제외하고 인자를 조합
      strcat(input, argv[i]);
      if (i != argc - 1)
      {
        strcat(input, " "); // 공백 추가
      }
    }

    arglist = GetSubstring(input, &argcnt, " \t");

    if (!strcmp(arglist[0], commanddata[0]))
    {
      command = CMD_BACKUP;
    }
    else if (!strcmp(arglist[0], commanddata[1]))
    {
      command = CMD_REM;
    }
    else if (!strcmp(arglist[0], commanddata[2]))
    {
      command = CMD_REC;
    }
    else if (!strcmp(arglist[0], commanddata[3]))
    {
      command = CMD_LIST;
    }

    if (command & (CMD_BACKUP | CMD_REM | CMD_REC | CMD_LIST))
    {
      ParameterInit(&parameter);
      parameter.command = arglist[0];
      if (ParameterProcessing(argcnt, arglist, command, &parameter) == -1)
      {
        // printf("parameterProcessing error\n");
        return 1;
      }
      CommandExec(parameter);
    }
  }
  deleteEmptyDirectories(backupPATH);
  exit(0);
}