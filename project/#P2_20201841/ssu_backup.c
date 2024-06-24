#include "ssu_header.h"
int AddCommand(command_parameter *parameter);                                                   // Add 명령어를 실행하는 함수
int RemoveCommand(command_parameter *parameter);                                                // remove 명령어를 실행하는 함수
int StatusCommand(command_parameter *parameter);                                                // status 명령어를 실행하는 함수
int CommitCommand(command_parameter *parameter);                                                // commit 명령어를 실행하는 함수
int LogCommand(command_parameter *parameter);                                                   // log 명령어를 실행하는 함수
void CommandFun(char **arglist);                                                                // 입력된 명령어에 따라 명령 함수 실행
void CommandExec(command_parameter parameter);                                                  // 명령어 실행 함수
void HelpExec(char *command);                                                                   // help 실행 함수
void ParameterInit(command_parameter *parameter);                                               // 파라미터 초기화 함수
int ParameterProcessing(int argcnt, char **arglist, int command, command_parameter *parameter); // 명령줄로 받은 명령어들을 정리하고 알맞게 저장하는 함수
int Prompt();                                                                                   // 사용자에게 명령어를 계속 입력받기 위해 호출하는 함수
int init_staging_list(int log_fd);                                                              // staging 로그 파일을 읽어서 staging_dir_list와 managing_dir_list 연결리스트를 구축하는 함수
int init_commit_list();                                                                         // commit 로그 파일을 읽어서 커밋 구조체 리스트를 구축하는 함수
void Init();                                                                                    // 초기화 함수

// Add 명령어를 실행하는 함수
int AddCommand(command_parameter *parameter)
{
  struct stat statbuf; // 파일 정보를 저장할 구조체 선언
  char *originPath = (char *)malloc(sizeof(char *) * PATHMAX);
  // char **backupPathList = NULL;
  // int backupPathDepth = 0;
  int i;
  int staging_fd;

  strcpy(originPath, parameter->filename);

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

  if ((staging_fd = open(staginglogPATH, O_RDWR | O_CREAT, 0777)) == -1)
  {
    fprintf(stderr, "ERROR: open error for '%s'\n", staginglogPATH);
    return 1;
  }

  init_staging_list(staging_fd);
  // if (staging_dir_list == NULL)
  //   printf("NULL\n");
  // if (managing_dir_list == NULL)
  //   printf("managing_dir_list is NULL\n");

  // printf("<staging_dir_list>\n");
  // testStagingList(staging_dir_list);

  // printf("<managing_dir_list>\n");
  // testStagingList(managing_dir_list);

  // 모든 파일이 존재하면 이미 존재함을 출력
  if (is_staged_file_or_directory(staging_dir_list, originPath, &statbuf))
  {
    printf("\".%s\" already exist in staging area\n", originPath + strlen(exePATH));
    free(originPath);
    close(staging_fd);
    return 0;
  }

  char *relPath = (char *)malloc(sizeof(char *) * PATHMAX); // 상대경로 저장할 변수 (/부터 저장됨)
  strcpy(relPath, parameter->filename + strlen(exePATH));

  printf("add \".%s\" \n", relPath);

  // 스테이징 로그 파일에 출력할 문자열을 저장할 변수
  char *staginglogWrite = (char *)malloc(sizeof(char *) * STRMAX);
  sprintf(staginglogWrite, "add \"%s\"\n", originPath);

  // 스테이징 로그 파일에 출력하기
  int fd;
  if ((fd = open(staginglogPATH, O_RDWR | O_CREAT | O_APPEND, 0666)) < 0)
  {
    fprintf(stderr, "open error for %s\n", staginglogPATH);
    exit(1);
  }
  else
  {
    write(fd, staginglogWrite, strlen(staginglogWrite));
    close(fd);
  }

  return 0;
}

// remove 명령어를 실행하는 함수
int RemoveCommand(command_parameter *parameter)
{
  struct stat statbuf; // 파일 정보를 저장할 구조체 선언
  char *originPath = (char *)malloc(sizeof(char *) * PATHMAX);
  int i;
  int staging_fd;

  strcpy(originPath, parameter->filename);

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

  if ((staging_fd = open(staginglogPATH, O_RDWR | O_CREAT, 0777)) == -1)
  {
    fprintf(stderr, "ERROR: open error for '%s'\n", staginglogPATH);
    return 1;
  }

  init_staging_list(staging_fd);
  // if (staging_dir_list == NULL)
  //   printf("NULL\n");

  // printf("<staging_dir_list>\n");
  // testStagingList(staging_dir_list);

  // printf("<managing_dir_list>\n");
  // testStagingList(managing_dir_list);

  if (is_none_file_or_directory(staging_dir_list, originPath, &statbuf))
  {
    if (is_staged_file_or_directory(managing_dir_list, originPath, &statbuf))
    {
      printf("\".%s\" already removed from staging area\n", originPath + strlen(exePATH));
      return 0;
    }
  }

  // 경로로 들어온 것이 연결리스트에 있으면 로그파일에 remove 출력
  char *relPath = (char *)malloc(sizeof(char *) * PATHMAX); // 상대경로 저장할 변수 (/부터 저장됨)
  strcpy(relPath, parameter->filename + strlen(exePATH));

  printf("remove \".%s\" \n", relPath);

  // 스테이징 로그 파일에 출력할 문자열을 저장할 변수
  char *staginglogWrite = (char *)malloc(sizeof(char *) * STRMAX);
  sprintf(staginglogWrite, "remove \"%s\"\n", originPath);

  // 스테이징 로그 파일에 출력하기
  int fd;
  if ((fd = open(staginglogPATH, O_RDWR | O_CREAT | O_APPEND, 0666)) < 0)
  {
    fprintf(stderr, "open error for %s\n", staginglogPATH);
    exit(1);
  }
  else
  {
    write(fd, staginglogWrite, strlen(staginglogWrite));
    close(fd);
  }

  return 0;
}

// status 명령어를 실행하는 함수
int StatusCommand(command_parameter *parameter)
{
  int staging_fd;

  if ((staging_fd = open(staginglogPATH, O_RDWR | O_CREAT, 0777)) == -1)
  {
    fprintf(stderr, "ERROR: open error for '%s'\n", staginglogPATH);
    return 1;
  }

  init_staging_list(staging_fd);
  // if (staging_dir_list == NULL)
  //   printf("NULL\n");
  // if (managing_dir_list == NULL)
  //   printf("managing_dir_list is NULL\n");

  // printf("<staging_dir_list>\n");
  // testStagingList(staging_dir_list);

  // printf("<managing_dir_list>\n");
  // testStagingList(managing_dir_list);

  init_commit_list();

  CommitLogEntry *current = commitHead;
  int check = 0;
  printf("Changes to be committed:\n");
  while (current != NULL)
  {
    // printf("Check : %s\n", current->filePath);

    struct stat buf;
    if (stat(current->filePath, &buf) == 0)
    {
      // printf("존재\n");
      // 파일이 변경된 경우
      if (cmpHash(current->filePath, current->backupPath))
      {
        check++;
        printf("  modified: .%s\n", current->filePath + strlen(exePATH));
      }
    }
    else
    {
      // printf("없음\n");
      // 없으면 removed로 출력
      check++;
      printf("  removed: .%s\n", current->filePath + strlen(exePATH));
    }
    current = current->next;
  }

  if (check == 0)
  {
    printf("  Nothing to commit\n");
  }

  // 현재 작업 디렉터리에 있는 모든 파일들을 돌면서
  // staging_dir_list에 포함되어 있지 않고, managing_dir_list에도 포함되어있지 않은 파일들에 대해
  // Untraked와 파일 상태(new files) 출력

  int firstPrint = 0;
  statusUntrackedCheck(exePATH, &firstPrint); // 재귀적으로 디렉터리를 탐색하는 함수 호출

  close(staging_fd);
  return 0;
}

// commit 명령어를 실행하는 함수
int CommitCommand(command_parameter *parameter)
{
  if (!strcmp(parameter->tmpname, "")) // 커밋 이름 입력하지 않은 경우 에러처리
  {
    fprintf(stderr, "ERROR: <NAME> is not include\n");
    help("commit");
    return 1;
  }
  // printf("commit name : %s\n", parameter->tmpname);

  init_commit_list();
}

// log 명령어를 실행하는 함수
int LogCommand(command_parameter *parameter)
{
  FILE *fp = fopen(commitlogPATH, "r");
  if (!fp)
  {
    fprintf(stderr, "fopen error for commitLogFile\n");
    return 1;
  }

  char line[1024];

  // 커밋 로그 파일을 한줄씩 읽으면서 구조체에 값을 저장
  while (fgets(line, sizeof(line), fp))
  {
    CommitLogEntry *newEntry = malloc(sizeof(CommitLogEntry));
    if (newEntry == NULL)
    {
      perror("Failed to allocate memory");
      return EXIT_FAILURE;
    }

    if (parseCommitLog(line, newEntry) == 0)
    {
      commitHead = appendEntry(&commitHead, newEntry);
    }
    else
    {
      free(newEntry);
      fprintf(stderr, "Failed to parse line: %s\n", line);
    }
  }

  if (!strcmp(parameter->tmpname, ""))
  {
    // 모든 로그 출력, 커밋 메시지별로 그룹화하여 출력
    printCommitLogEntryGrouped(commitHead);
  }
  else
  {
    // printf("tmpname : %s\n", parameter->tmpname);
    // 특정 커밋 메시지에 대한 로그만 출력
    printSpecificCommitLog(commitHead, parameter->tmpname);
  }

  // 출력 및 메모리 해제
  CommitLogEntry *current = commitHead;
  while (current != NULL)
  {
    // printCommitLogEntry(current);
    CommitLogEntry *temp = current;
    current = current->next;
    free(temp);
  }

  fclose(fp);
  return 0;
}

// 입력된 명령어에 따라 명령 함수 실행
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
    commandFun = StatusCommand;
  }
  else if (!strcmp(parameter.command, commanddata[3]))
  {
    commandFun = CommitCommand;
  }
  else if (!strcmp(parameter.command, commanddata[5]))
  {
    commandFun = LogCommand;
  }

  if (commandFun(&parameter) != 0)
  {
    exit(1);
  }
}

// 명령어 실행 함수
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

// help 실행 함수
void HelpExec(char *command)
{
  pid_t pid;

  if ((pid = fork()) < 0)
  {
    fprintf(stderr, "ERROR: fork error\n");
    exit(1);
  }
  else if (pid == 0)
  {
    // printf("command : %s\n", command);
    execl(exeNAME, "help", command, (char *)0);
    exit(0);
  }
  else
  {
    pid = wait(NULL);
  }
}

// 파라미터 초기화 함수
void ParameterInit(command_parameter *parameter)
{
  parameter->command = (char *)malloc(sizeof(char *) * PATHMAX);
  parameter->filename = (char *)malloc(sizeof(char *) * PATHMAX);
  parameter->tmpname = (char *)malloc(sizeof(char *) * PATHMAX);
  parameter->commandopt = 0;
}

// 명령줄로 받은 명령어들을 정리하고 알맞게 저장하는 함수
int ParameterProcessing(int argcnt, char **arglist, int command, command_parameter *parameter)
{
  struct stat buf;
  int lastind;
  int option;
  int optcnt = 0;

  switch (command)
  {
  case CMD_ADD:
  {
    if (argcnt < 2)
    { // 경로를 입력하지 않은 경우
      fprintf(stderr, "ERROR: <PATH> is not include\n");
      help(arglist[0]);
      return -1;
    }

    if (ConvertPath(arglist[1], parameter->filename) != 0)
    {
      fprintf(stderr, "ERROR: %s is invalid filepath\n", parameter->filename);
      return -1;
    }

    if (strncmp(parameter->filename, homePATH, strlen(homePATH)) || !strncmp(parameter->filename, repoPATH, strlen(repoPATH)) || !strcmp(parameter->filename, homePATH))
    {
      fprintf(stderr, "ERROR: %s can't be added\n", parameter->filename);
      return -1;
    }

    // 존재하지 않는 파일의 경로를 입력한 경우
    if (lstat(parameter->filename, &buf) < 0)
    {
      fprintf(stderr, "ERROR: '%s' is wrong path\n", parameter->filename + strlen(exePATH) + 1);
      return -1;
    }

    // 일반 파일이나 디렉터리가 아닌 경우
    if (!S_ISREG(buf.st_mode) && !S_ISDIR(buf.st_mode))
    {
      fprintf(stderr, "ERROR: %s is not regular file\n", parameter->filename);
      return -1;
    }
    break;
  }
  case CMD_REM:
  {
    if (argcnt < 2)
    { // 경로를 입력하지 않은 경우
      fprintf(stderr, "ERROR: <PATH> is not include\n");
      help(arglist[0]);
      return -1;
    }

    if (ConvertPath(arglist[1], parameter->filename) != 0)
    {
      fprintf(stderr, "ERROR: %s is invalid filepath\n", parameter->filename);
      return -1;
    }

    if (strncmp(parameter->filename, homePATH, strlen(homePATH)) || !strncmp(parameter->filename, repoPATH, strlen(repoPATH)) || !strcmp(parameter->filename, homePATH))
    {
      fprintf(stderr, "ERROR: %s can't be removed\n", parameter->filename);
      return -1;
    }

    // 존재하지 않는 파일의 경로를 입력한 경우
    if (lstat(parameter->filename, &buf) < 0)
    {
      fprintf(stderr, "ERROR: '%s' is wrong path\n", parameter->filename + strlen(exePATH) + 1);
      return -1;
    }

    // 일반 파일이나 디렉터리가 아닌 경우
    if (!S_ISREG(buf.st_mode) && !S_ISDIR(buf.st_mode))
    {
      fprintf(stderr, "No arguments required\n");
      fprintf(stderr, "ERROR: %s is not regular file\n", parameter->filename);
      return -1;
    }
    break;
  }
  case CMD_STA:
  {
    if (argcnt > 1)
    {
      // status는 인자를 받지 않음
      help(arglist[0]);
      return -1;
    }
  }
  }
}

// 사용자에게 명령어를 계속 입력받기 위해 호출하는 함수
int Prompt()
{
  char input[STRMAX];
  int argcnt = 0;
  char **arglist = NULL;
  int command;
  command_parameter parameter = {(char *)0, (char *)0, (char *)0, 0};

  while (true)
  {
    printf("20201841> ");
    fgets(input, sizeof(input), stdin);
    input[strlen(input) - 1] = '\0';

    if ((arglist = GetSubstring(input, &argcnt, " \t")) == NULL)
    {
      continue;
    }

    if (argcnt == 0)
      continue;

    if (!strcmp(arglist[0], commanddata[0]))
    {
      command = CMD_ADD;
    }
    else if (!strcmp(arglist[0], commanddata[1]))
    {
      command = CMD_REM;
    }
    else if (!strcmp(arglist[0], commanddata[2]))
    {
      command = CMD_STA;
    }
    else if (!strcmp(arglist[0], commanddata[3]))
    {
      command = CMD_COM;
    }
    else if (!strcmp(arglist[0], commanddata[4]))
    {
      command = CMD_REV;
    }
    else if (!strcmp(arglist[0], commanddata[5]))
    {
      command = CMD_LOG;
    }
    else if (!strcmp(arglist[0], commanddata[6]))
    {
      // printf("arglist[0] : %s\n", arglist[0]);
      command = CMD_HELP;
    }
    else if (!strcmp(arglist[0], commanddata[7]))
    {
      command = CMD_EXIT;
      fprintf(stdout, "\n"); // exit 입력 시 종료
      exit(0);
    }
    else
    {
      command = NOT_CMD;
    }

    if (command & (CMD_ADD | CMD_REM | CMD_STA | CMD_COM | CMD_LOG))
    {
      ParameterInit(&parameter);
      parameter.command = arglist[0];
      if (command & CMD_COM || command & CMD_LOG)
      {
        char name[256] = "";
        // printf("argcnt : %d\n", argcnt);
        for (int i = 1; i < argcnt; i++)
        {
          strcat(name, arglist[i]);
          if (i != argcnt - 1)
            strcat(name, " ");
        }
        // printf("name : %s!\n", name);
        parameter.tmpname = name;
      }
      if (ParameterProcessing(argcnt, arglist, command, &parameter) == -1)
      {
        continue;
      }

      CommandExec(parameter);
    }
    else if (command & CMD_HELP || command == NOT_CMD)
    {
      // printf("arglist[1] : %s\n", arglist[1]);
      HelpExec(arglist[1]);
    }
  }
}

// commit 로그 파일을 읽어서 커밋 구조체 리스트를 구축하는 함수
int init_commit_list()
{
  FILE *fp = fopen(commitlogPATH, "r");
  char line[1024];

  if (!fp)
  {
    fprintf(stderr, "fopen error for commitLogFile\n");
    return 1;
  }

  // 커밋 로그 파일을 한줄씩 읽으면서 구조체에 값을 저장
  while (fgets(line, sizeof(line), fp))
  {
    CommitLogEntry *newEntry = malloc(sizeof(CommitLogEntry));
    if (newEntry == NULL)
    {
      perror("Failed to allocate memory");
      return EXIT_FAILURE;
    }

    if (parseCommitLog(line, newEntry) == 0)
    {
      commitHead = appendEntry(&commitHead, newEntry);
    }
    else
    {
      free(newEntry);
      fprintf(stderr, "Failed to parse line: %s\n", line);
    }
  }

  CommitLogEntry *current = commitHead;
  while (current != NULL)
  {
    char backupPath[PATHMAX];
    strcpy(backupPath, repoPATH);
    strcat(backupPath, "/");
    strcat(backupPath, current->commitMessage);
    strcat(backupPath, "/.");
    strcat(backupPath, current->filePath + strlen(exePATH) + 1);

    // printf("back : %s\n", backupPath);
    strcpy(current->backupPath, backupPath);
    // printCommitLogEntry(current);
    current = current->next;
  }

  fclose(fp);
}

// staging 로그 파일을 읽어서 staging_dir_list와 managing_dir_list 연결리스트를 구축하는 함수
int init_staging_list(int log_fd)
{
  int len;
  char buf[PATHMAX];
  staging_dir_list = (dirNode *)malloc(sizeof(dirNode)); // staging 맨 앞 노드 할당
  managing_dir_list = (dirNode *)malloc(sizeof(dirNode));

  dirNode_init(&staging_dir_list);
  dirNode_init(&managing_dir_list);

  strcpy(staging_dir_list->dir_path, exePATH); // 현재작업디렉터리
  strcpy(managing_dir_list->dir_path, exePATH);

  char *path; // 경로를 받을 변수

  while (len = read(log_fd, buf, STRMAX))
  {
    char *ptr = strchr(buf, '\n');
    ptr[0] = '\0';
    // 로그 파일 한줄씩 읽기

    lseek(log_fd, -(len - strlen(buf)) + 1, SEEK_CUR);
    // 그 다음 부분을 읽기 위해 포인터 조정

    if ((ptr = strstr(buf, "add")) != NULL)
    {                       // 문자열에서 add가 찾아진 경우
      ptr += strlen("add"); // 'add' 명령어 다음 위치로 이동
      while (*ptr == ' ')
        ptr++; // 공백 스킵
      ptr++;   // 처음 따옴표 건너뛰기

      // 'ptr'가 이제 파일 경로의 시작을 가리키고 있으므로 경로의 끝을 찾을 때까지 이동
      char *end = strchr(ptr, '"');
      if (end != NULL)
        *end = '\0'; // 마지막 따옴표를 널 문자로 대체하여 문자열 종료

      path = ptr; // 순수한 경로 만을 가지고 있는 ptr를 path에 값을 복사하여 넢음

      // printf("path : %s\n", path);
      add_staging(path, staging_dir_list);
      add_staging(path, managing_dir_list);

      // start_staging(path);  // 로그 파일에 기록되어있는 경로를 추출 받아 넘김
    }
    else if ((ptr = strstr(buf, "remove")) != NULL) // 문자열에서 remove가 찾아진 경우
    {
      ptr += strlen("remove"); // 'remove' 명령어 다음 위치로 이동
      while (*ptr == ' ')
        ptr++; // 공백 스킵
      ptr++;   // 처음 따옴표 건너뛰기

      // 'ptr'가 이제 파일 경로의 시작을 가리키고 있으므로 경로의 끝을 찾을 때까지 이동
      char *end = strchr(ptr, '"');
      if (end != NULL)
        *end = '\0'; // 마지막 따옴표를 널 문자로 대체하여 문자열 종료

      path = ptr; // 순수한 경로 만을 가지고 있는 ptr를 path에 값을 복사하여 넢음

      // printf("init_staging_list > path : %s\n", path);
      remove_staging(path, staging_dir_list);
      add_staging(path, managing_dir_list); // managing_dir_list는 add든 remove든 추가함

      recursiveDeleteEmptyDirs(&staging_dir_list);
    }
  }
  return 0;
}

// 초기화 함수
void Init()
{
  getcwd(exePATH, PATHMAX);                // 현재 작업 디렉터리의 경로를 받아서 exePATH에 저장
  sprintf(homePATH, "%s", getenv("HOME")); // home/user 경로저장

  strcpy(repoPATH, exePATH); // repo 디렉터리 경로 설정
  strcat(repoPATH, "/.repo");

  strcpy(commitlogPATH, repoPATH); // 커밋 로그 파일 경로 설정
  strcat(commitlogPATH, "/.commit.log");

  strcpy(staginglogPATH, repoPATH); // 스테이징 로그 파일 경로 설정
  strcat(staginglogPATH, "/.staging.log");

  if (access(repoPATH, F_OK)) // repo 디렉터리 생성
    mkdir(repoPATH, 0777);

  int staging_fd;
  int commit_fd;

  // 하위에 커밋 로그 파일 생성
  if ((commit_fd = open(commitlogPATH, O_RDWR | O_CREAT, 0777)) == -1)
  {
    fprintf(stderr, "ERROR: open error for '%s'\n", commitlogPATH);
    return;
  }
  // 스테이징 구역 파일 생성
  if ((staging_fd = open(staginglogPATH, O_RDWR | O_CREAT, 0777)) == -1)
  {
    fprintf(stderr, "ERROR: open error for '%s'\n", staginglogPATH);
    return;
  }

  close(commit_fd);
  close(staging_fd);
}

int main(int argc, char *argv[])
{
  Init(); // 초기화 함수 호출

  if (!strcmp(argv[0], "command"))
  {
    hash = atoi(argv[1]);

    CommandFun(argv + 2);
  }
  // help 명령어 입력
  else if (!strcmp(argv[0], "help"))
  {
    if (argc == 1)
    {
      char *all = "all";
      help(all);
    }
    else if (argc == 2)
    {
      help(argv[1]);
    }
  }
  else
  {
    strcpy(exeNAME, argv[0]);

    hash = HASH_MD5;

    Prompt();
  }

  exit(0);
}