#include "ssu_header.h"
void printFileEvents(const char *logFilePath, const char *targetFilePath); // List 명령어 -> 경로가 일반 파일일 때 출력
int checkPidInFile(char *pid, char *filepath); // pid가 존재하는지 확인하고 (존재한다면) filepath에 경로를 넘겨줌
int ListCommand(command_parameter *parameter); // List 명령어
void removeStringFromFile(const char *monitoringListLogFilePath, const char *string_to_remove); // 삭제하려는 pid 로그 내용만 지우는 함수 (그 문자만 지우고 다른 문자들은 다시 복사)
int emptyDirectory(const char *dirname); // 디렉터리 비우는 함수
int RemoveCommand(command_parameter *parameter); // Remove 명령어
int isDaemonExist(char *path); // // 해당 데몬 프로세스가 이미 존재하는지 확인하는 함수
void backupFile(char *path, char *date, char *basedir, char *logfilePath, int flag); // 백업 파일을 생성하는 함수 (로그파일경로도 함께 넘겨줌)
bool isFileModified(const char *filePath); // 파일이 수정되었는지 여부를 확인하는 함수
dirNode *initDirNode(char *dirPath); // 디렉터리 초기화 함수
void insertFileNode(char *logPath, dirNode *toDir, char *backupDirPath, char *currentFilePath); // 파일 삽입 함수
void insertDirNode(char *logPath, dirNode *toDir, char *backupDirPath, char *currentDirPath); // 디렉터리 삽입 함수
fileNode *findBackupFile(char *originalPath, dirNode *currentDir); // 현재 디렉터리(currentDir) 내에서 원본 파일 경로(originalPath)에 해당하는 백업 파일 노드를 검색하는 함수
void traverseDirectoryNode(char *logPath, dirNode *currentBackupDir, char *backupDirPath, char *currentDirPath, int flag1); // 디렉터리 순회 함수
void validateRemovedFiles(char *logPath, dirNode *currentDir); // 현재 디렉터리(currentDir) 내에서 삭제된 파일을 확인하는 함수
void monitoringFile(char *filePath, int flag1, int period); // 지정된 파일 또는 디렉터리를 변경 사항을 모니터링하고, 백업 수행
int AddCommand(command_parameter *parameter); // Add 명령어
void CommandFun(char **arglist); // 입력된 명령어에 따라 명령 함수 실행
void CommandExec(command_parameter parameter); // 명령어 실행 함수
void HelpExec(char *command); // help 실행 함수
void ParameterInit(command_parameter *parameter); // 파라미터 관련 구조체 초기화 함수
int ParameterProcessing(int argcnt, char **arglist, int command, command_parameter *parameter); // 명령줄로 받은 명령어들을 정리하고 알맞게 저장하는 함수
int Prompt(); // 사용자에게 명령어를 계속 입력받기 위해 호출하는 함수
void Init(); // 초기화 함수


// List 명령어 -> 경로가 일반 파일일 때 출력
void printFileEvents(const char *logFilePath, const char *targetFilePath)
{
    FILE *file = fopen(logFilePath, "r");
    if (file == NULL)
    {
        perror("Error opening file");
        return;
    }

    char line[STRMAX];
    while (fgets(line, sizeof(line), file))
    {
        char event[20], timestamp[20], path[PATHMAX];
        // 일시, 이벤트, 경로를 파싱하여 추출
        if (sscanf(line, "[%[^]]][%[^]]][%[^]]", timestamp, event, path) != 3)
        {
            fprintf(stderr, "Error: Failed to parse timestamp, event, and path\n");
            return;
        }

        if (strcmp(path, targetFilePath) == 0)
        {
            printf("[%s][%s]\n", event, timestamp);
        }
    }

    fclose(file);
}

// pid가 존재하는지 확인하고 (존재한다면) filepath에 경로를 넘겨줌
int checkPidInFile(char *pid, char *filepath)
{
    int found = 0;
    char buffer[STRMAX]; // 파일에서 읽을 줄의 최대 길이
    FILE *file;

    // 파일 열기
    file = fopen(monitorListLogPATH, "r");
    if (file == NULL)
    {
        printf("ERROR: Cannot open file\n");
        return -1;
    }

    // 파일에서 한 줄씩 읽기
    while (fgets(buffer, sizeof(buffer), file) != NULL)
    {
        // 개행 문자 제거
        buffer[strcspn(buffer, "\n")] = '\0';

        // 파일 내에 PID가 존재하는지 확인
        if (strncmp(buffer, pid, strlen(pid)) == 0 && buffer[strlen(pid)] == ' ')
        {
            found = 1;
            // ":" 이후의 부분을 찾음
            char *path = strchr(buffer, ':');
            if (path != NULL)
            {
                // ":" 다음에 오는 공백 문자들을 건너뜀
                while (*path != '\0' && (*path == ' ' || *path == ':'))
                {
                    path++;
                }
                // 추출된 경로 출력
                strcpy(filepath, path);
            }
            break; // PID를 찾았으면 반복문 종료
        }
    }

    // 파일 닫기
    fclose(file);

    return found;
}

// List 명령어
int ListCommand(command_parameter *parameter)
{
    // list에서는 filename에 pid가 넘어감 (문자열로 넘어옴)
    char *targetPid = parameter->filename;
    char path[PATHMAX];       // 해당 pid의 경로가 담김
    char pathParent[PATHMAX]; // 경로의 부모경로
    struct stat statbuf;
    char backupPath[5000];

    if (checkPidInFile(targetPid, path))
    {
        printf("%s\n", path);

        if (lstat(path, &statbuf) < 0)
        {
            fprintf(stderr, "ERROR: lstat error for %s\n", path);
            return -1;
        }

        sprintf(backupPath, "%s/%s.log", backupPATH, targetPid); // 백업파일을 관리하는 로그 파일의 경로를 가지고 있음
        // printf("backupPath : %s\n", backupPath);

        if (S_ISREG(statbuf.st_mode))
        {
            // 일반 파일을 경로로 받은 경우
            printFileEvents(backupPath, path);
        }
        else if (S_ISDIR(statbuf.st_mode))
        {
            // 디렉터리를 경로로 받은 경우
            printf("List : 디렉터리를 경로로 받은 경우 -> 구현 못했습니다 .. \n");
        }
    }
    else
    {
        // 입력받은 pid가 monitor_list에 존재하지 않는 경우 에러 처리
        printf("ERROR: PID %s does not exist\n", targetPid);
        return -1;
    }
}

// 삭제하려는 pid 로그 내용만 지우는 함수 (그 문자만 지우고 다른 문자들은 다시 복사)
void removeStringFromFile(const char *monitoringListLogFilePath, const char *string_to_remove)
{
    char buffer[STRMAX];
    FILE *file, *temp_file;

    // 원본 파일을 읽기 모드로 열기
    file = fopen(monitoringListLogFilePath, "r");
    if (file == NULL)
    {
        printf("ERROR: Cannot open original file\n");
        return;
    }

    // 임시 파일 열기
    temp_file = fopen("temp.txt", "w");
    if (temp_file == NULL)
    {
        printf("ERROR: Cannot create temporary file\n");
        fclose(file);
        return;
    }

    // 파일에서 문자열을 제외한 내용을 임시 파일에 복사
    while (fgets(buffer, sizeof(buffer), file) != NULL)
    {
        if (strstr(buffer, string_to_remove) == NULL)
        {
            fputs(buffer, temp_file);
        }
    }

    // 파일 닫기
    fclose(file);
    fclose(temp_file);

    // 임시 파일을 원래 파일로 교체
    remove(monitoringListLogFilePath);
    rename("temp.txt", monitoringListLogFilePath);
}

// 디렉터리 비우는 함수
int emptyDirectory(const char *dirname)
{
    DIR *dir;
    struct dirent *entry;
    char path[PATHMAX];
    struct stat statbuf;

    // 디렉터리 열기
    dir = opendir(dirname);
    if (dir == NULL)
    {
        fprintf(stderr, "ERROR: opendir error\n");
        return -1;
    }

    // 디렉터리 내의 각 파일 및 서브디렉터리 삭제
    while ((entry = readdir(dir)) != NULL)
    {
        // .과 ..는 건너뜀
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        snprintf(path, sizeof(path), "%s/%s", dirname, entry->d_name);

        // 파일 상태를 확인
        if (stat(path, &statbuf) == 0)
        {
            // 디렉터리인 경우 재귀적으로 삭제
            if (S_ISDIR(statbuf.st_mode))
            {
                if (emptyDirectory(path) != 0)
                {
                    closedir(dir);
                    return -1;
                }

                // 디렉터리 삭제
                if (rmdir(path) != 0)
                {
                    fprintf(stderr, "ERROR: rmdir error\n");
                    closedir(dir);
                    return -1;
                }
            }
            else
            {
                // 파일 삭제
                if (remove(path) != 0)
                {
                    fprintf(stderr, "ERROR: remove error\n");
                    closedir(dir);
                    return -1;
                }
            }
        }
        else
        {
            fprintf(stderr, "ERROR: stat error\n");
            closedir(dir);
            return -1;
        }
    }

    // 디렉터리 닫기
    closedir(dir);

    return 0;
}

// Remove 명령어
int RemoveCommand(command_parameter *parameter)
{
    char filepath[PATHMAX];
    // paramter->filename 으로 삭제하려는 pid를 받음
    // printf("RemoveCommand ) pid : %s\n", parameter->filename);

    if (checkPidInFile(parameter->filename, filepath))
    {
        removeStringFromFile(monitorListLogPATH, filepath);

        int pid = atoi(parameter->filename);
        // printf("PID num : %d exitst\n", pid);

        kill(pid, SIGUSR1); // SIGUSR1 시그널을 보내 데몬 프로세스 종료
        printf("monitoring ended (%s) : %d\n", filepath, pid);

        // 해당 데몬 프로세스의 로그 파일 및 디렉터리도 삭제해야 함
        char logfilePath[PATHMAX];
        strcpy(logfilePath, backupPATH);
        strcat(logfilePath, "/");
        strcat(logfilePath, parameter->filename);
        strcat(logfilePath, ".log");
        // printf("logfilePath : %s\n", logfilePath);

        remove(logfilePath); // 로그 파일 삭제

        char dirPath[PATHMAX];
        strcpy(dirPath, backupPATH);
        strcat(dirPath, "/");
        strcat(dirPath, parameter->filename);

        // printf("removeCommand ) dirPath : %s\n", dirPath);

        emptyDirectory(dirPath);
        rmdir(dirPath);
    }
    else
    {
        // 입력받은 pid가 monitor_list.log에 존재하지 않는 경우 에러처리 후 프롬포트 재출력
        printf("ERROR: PID %s does not exist\n", parameter->filename);
        return -1;
    }

    return 0;
}

// 해당 데몬 프로세스가 이미 존재하는지 확인하는 함수
int isDaemonExist(char *path)
{
    FILE *fp;
    char lineBuffer[10001];
    int i;

    // 파일 open
    if ((fp = fopen(monitorListLogPATH, "r")) == NULL)
    {
        fprintf(stderr, "ERROR : fopen error for %s\n", monitorListLogPATH);
        exit(EXIT_FAILURE);
    }

    // 로그 파일에서 각 줄을 읽어들여서 경로가 일치하는지 확인
    while (fgets(lineBuffer, sizeof(lineBuffer), fp) != NULL)
    {
        for (i = 0; lineBuffer[i]; i++)
        {
            if (lineBuffer[i] == ':')
            {
                break;
            }
        }
        if (!strncmp(lineBuffer + i + 2, path, strlen(path)))
        {
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0; // 경로에 해당하는 데몬 프로세스가 존재하지 않음
}

// 백업 파일을 생성하는 함수 (로그파일경로도 함께 넘겨줌)
// flag 0 : create, flag 1 : modify
void backupFile(char *path, char *date, char *basedir, char *logfilePath, int flag)
{
    int len;
    int fd1, fd2;
    char *buf = (char *)malloc(sizeof(char *) * STRMAX);
    struct stat statbuf, tmpbuf;

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
    // printf("backupFile ) filepath : %s\n", filepath);

    for (idx = strlen(filepath) - 1; filepath[idx] != '/'; idx--)
        ;

    strcpy(filename, filepath + idx + 1);
    filepath[idx] = '\0';
    // 위의 과정을 지나면 filepath에는 파일 이름 전까지의 디렉터리까지만 담김

    // printf("backupFile ) filename : %s\n", filename);

    if (lstat(path, &statbuf) < 0)
    {
        fprintf(stderr, "ERROR: lstat error for %s\n", path);
        exit(EXIT_FAILURE);
    }

    ConvertHash(path, filehash);

    sprintf(tmpdir, "%s", basedir); // 백업 파일을 담을 pid를 이름으로 가지는 디렉터리 경로 저장
    sprintf(newPath, "%s/%s_%s", tmpdir, filename, date);
    // printf("\nbackupFile ) path : %s\n", path);
    // printf("backupFile ) newPath : %s\n", newPath);

    if ((fd1 = open(path, O_RDONLY)) < 0)
    {
        fprintf(stderr, "ERROR: open error for %s\n", path);
        exit(EXIT_FAILURE);
    }

    if ((fd2 = open(newPath, O_CREAT | O_TRUNC | O_WRONLY, 0666)) < 0)
    {
        fprintf(stderr, "ERROR: open error for %s\n", newPath);
        exit(EXIT_FAILURE);
    }

    while ((len = read(fd1, buf, statbuf.st_size)) > 0)
    {
        write(fd2, buf, len);
    }

    // 백업 성공시 로그파일에 출력
    printToLogFile(logfilePath, date, path, flag);
}

// 파일이 수정되었는지 여부를 확인하는 함수
bool isFileModified(const char *filePath)
{
    struct stat statbuf;

    // 파일 정보 조회
    if (stat(filePath, &statbuf) == -1)
    {
        fprintf(stderr, "ERROR: stat error for %s\n", filePath);
        return false;
    }

    // 현재 시간 가져오기
    time_t currentTime;
    time(&currentTime);

    // 파일의 최근 수정 시간과 현재 시간을 비교하여 수정 여부 확인
    if (difftime(currentTime, statbuf.st_mtime) > 0)
    {
        // 파일이 수정된 경우
        return true;
    }
    else
    {
        // 파일이 수정되지 않은 경우
        return false;
    }
}

// 디렉터리 초기화 함수
dirNode *initDirNode(char *dirPath)
{
    dirNode *newDirNode = (dirNode *)malloc(sizeof(dirNode));
    if (newDirNode == NULL)
    {
        return NULL; // 메모리 할당 실패 시 NULL 반환
    }

    strncpy(newDirNode->dirPath, dirPath, PATHMAX);
    newDirNode->headFile = NULL;
    newDirNode->tailFile = NULL;
    newDirNode->subDirs = NULL;
    newDirNode->prevDir = NULL;
    newDirNode->nextDir = NULL;

    return newDirNode;
}

// 파일 삽입 함수
void insertFileNode(char *logPath, dirNode *toDir, char *backupDirPath, char *currentFilePath)
{
    fileNode *newFileNode = (fileNode *)malloc(sizeof(fileNode));
    if (newFileNode == NULL)
    {
        return; // 메모리 할당 실패 시 함수 종료
    }

    strncpy(newFileNode->originPath, currentFilePath, PATHMAX);
    strncpy(newFileNode->backupPath, backupDirPath, PATHMAX);
    newFileNode->fileName = strdup(GetFileName(newFileNode->originPath));
    newFileNode->prevFile = NULL;
    newFileNode->nextFile = NULL;

    struct stat currentFileStat;
    if (stat(newFileNode->originPath, &currentFileStat) < 0)
    {
        fprintf(stderr, "stat(%s) error\n", newFileNode->originPath);
        exit(0);
    }
    newFileNode->modTime = currentFileStat.st_mtime;

    if (toDir->headFile == NULL)
    {
        toDir->headFile = newFileNode;
        toDir->tailFile = newFileNode;
    }
    else
    {
        toDir->tailFile->nextFile = newFileNode;
        newFileNode->prevFile = toDir->tailFile;
        toDir->tailFile = newFileNode;
    }

    backupFile(newFileNode->originPath, getDate(), backupDirPath, logPath, 0);
}

// 디렉터리 삽입 함수
void insertDirNode(char *logPath, dirNode *toDir, char *backupDirPath, char *currentDirPath)
{
    dirNode *newDirNode = initDirNode(currentDirPath);
    if (newDirNode == NULL)
    {
        return; // 메모리 할당 실패 시 함수 종료
    }

    if (toDir->subDirs == NULL)
    {
        toDir->subDirs = (dirList *)malloc(sizeof(dirList));
        if (toDir->subDirs == NULL)
        {
            return; // 메모리 할당 실패 시 함수 종료
        }
        toDir->subDirs->head = newDirNode;
        toDir->subDirs->tail = newDirNode;
    }
    else
    {
        toDir->subDirs->tail->nextDir = newDirNode;
        newDirNode->prevDir = toDir->subDirs->tail;
        toDir->subDirs->tail = newDirNode;
    }

    char buf[PATHMAX];
    snprintf(buf, sizeof(buf), "%s/%s", backupDirPath, GetFileName(currentDirPath));
    if (access(buf, F_OK) != 0)
    {
        mkdir(buf, 0777);
    }
}

// 현재 디렉터리(currentDir) 내에서 원본 파일 경로(originalPath)에 해당하는 백업 파일 노드를 검색하는 함수
fileNode *findBackupFile(char *originalPath, dirNode *currentDir)
{
    fileNode *tmpNode;
    dirNode *tmpDir;

    // printf("originalPath : %s\n", originalPath);

    tmpNode = currentDir->headFile;

    while (tmpNode != NULL)
    {
        if (strcmp(tmpNode->originPath, originalPath) == 0)
        {
            return tmpNode;
        }

        tmpNode = tmpNode->nextFile;
    }

    if (currentDir->subDirs == NULL)
    {
        return NULL;
    }

    tmpDir = currentDir->subDirs->head;

    while (tmpDir != NULL)
    {
        tmpNode = findBackupFile(originalPath, tmpDir);
        if (tmpNode != NULL)
        {
            return tmpNode;
        }

        tmpDir = tmpDir->nextDir;
    }

    return NULL;
}

// 디렉터리 순회 함수 (현재 백업 디렉터리를 탐색하며 파일 노드를 업데이트하거나 새로 추가)
// flag1 : 2(-r 옵션), 1(-d 옵션)
void traverseDirectoryNode(char *logPath, dirNode *currentBackupDir, char *backupDirPath, char *currentDirPath, int flag1)
{
    DIR *openedDir = opendir(currentDirPath);

    if (openedDir == NULL)
    {
        fprintf(stderr, "ERROR: opendir error\n");
        exit(EXIT_FAILURE);
    }

    struct dirent *currentFile;
    char currentFilePath[PATHMAX];
    char backupFilePath[PATHMAX];
    struct stat currentFileStat;

    while ((currentFile = readdir(openedDir)) != NULL)
    {
        if (strcmp(currentFile->d_name, ".") == 0 || strcmp(currentFile->d_name, "..") == 0 || strstr(currentFile->d_name, ".swp") != NULL)
        // .swp 파일 무시하는 코드 추가
        {
            continue;
        }

        snprintf(currentFilePath, sizeof(currentFilePath), "%s/%s", currentDirPath, currentFile->d_name);
        if (stat(currentFilePath, &currentFileStat) < 0)
        {
            fprintf(stderr, "ERROR: stat error for %s\n", currentFilePath);
            exit(EXIT_FAILURE);
        }

        if (S_ISDIR(currentFileStat.st_mode))
        {
            if (flag1 == 2)
            { // -r 옵션
                snprintf(backupFilePath, sizeof(backupFilePath), "%s/%s", backupDirPath, currentFile->d_name);
                if (access(backupFilePath, F_OK) == 0)
                {
                    continue;
                }
                insertDirNode(logPath, currentBackupDir, backupDirPath, currentFilePath);
            }
            else
            { // -d 옵션
                continue;
            }
        }
        else
        {
            fileNode *foundBackupFile = findBackupFile(currentFilePath, currentBackupDir);
            if (foundBackupFile == NULL)
            {
                insertFileNode(logPath, currentBackupDir, backupDirPath, currentFilePath);
            }
            else
            {
                if (foundBackupFile->modTime != currentFileStat.st_mtime)
                {
                    backupFile(foundBackupFile->originPath, getDate(), backupDirPath, logPath, 1);
                    foundBackupFile->modTime = currentFileStat.st_mtime; // 최신 수정 시간으로 업데이트
                }
            }
        }
    }
    closedir(openedDir);

    if (flag1 == 2 && currentBackupDir->subDirs != NULL)
    {
        dirNode *currentDirNode = currentBackupDir->subDirs->head;
        while (currentDirNode != NULL)
        {
            snprintf(backupFilePath, sizeof(backupFilePath), "%s/%s", backupDirPath, GetFileName(currentDirNode->dirPath));
            traverseDirectoryNode(logPath, currentDirNode, backupFilePath, currentDirNode->dirPath, flag1);
            currentDirNode = currentDirNode->nextDir;
        }
    }
}

// 현재 디렉터리(currentDir) 내에서 삭제된 파일을 확인하는 함수
void validateRemovedFiles(char *logPath, dirNode *currentDir)
{
    fileNode *tempFileNode = currentDir->headFile;   // 현재 디렉터리의 파일 노드를 가리키는 임시 포인터
    fileNode *nextFileNode;                          // 다음 파일 노드를 가리키는 임시 포인터
    time_t currentTime = time(NULL);                 // 현재 시간을 저장하는 변수
    struct tm *backupTime = localtime(&currentTime); // 현재 시간을 로컬 시간으로 변환하여 저장하는 구조체 포인터

    // 현재 디렉터리의 모든 파일 노드들을 반복하여 검사
    while (tempFileNode != NULL)
    {
        nextFileNode = tempFileNode->nextFile;          // 다음 파일 노드를 임시 포인터에 저장
        if (access(tempFileNode->originPath, F_OK) < 0) // 삭제된 파일인지 확인
        {
            if (currentDir->headFile == tempFileNode) // 삭제된 파일인 경우
            {
                currentDir->headFile = tempFileNode->nextFile; // 첫번째 파일 노드가 삭제된 경우 헤드 포인터 갱신
            }
            else if (currentDir->tailFile == tempFileNode)
            {
                currentDir->tailFile = tempFileNode->prevFile; // 마지막 파일 노드가 삭제된 경우 테일 포인터 갱신
                if (currentDir->headFile != tempFileNode)
                {
                    tempFileNode->prevFile->nextFile = NULL; // 삭제된 파일 노드의 이전 파일 노드의 next 갱신
                }
            }
            else
            {
                // 중간에 위치한 파일노드를 삭제할 경우 이전 파일 노드와 다음 파일 노드의 연결 수정
                tempFileNode->nextFile->prevFile = tempFileNode->prevFile;
                tempFileNode->prevFile->nextFile = tempFileNode->nextFile;
            }
            // 로그 파일에 삭제된 파일 정보 기록
            printToLogFile(logPath, getDate(), tempFileNode->originPath, 2);
            free(tempFileNode);
        }
        tempFileNode = nextFileNode;
    }

    if (currentDir->subDirs == NULL)
    {
        return;
    }

    dirNode *tempDirNode = currentDir->subDirs->head; // 현재 디렉터리의 첫 번째 하위 디렉터리를 가리키는 임시 포인터
    // 현재 디렉터리의 모든 하위 디렉터리들에 대해 validateRemovedFiles 함수 재귀 호출
    while (tempDirNode != NULL)
    {
        validateRemovedFiles(logPath, tempDirNode);
        tempDirNode = tempDirNode->nextDir; // 다음 하위 디렉터리로 이동
    }
}

// 지정된 파일 또는 디렉터리를 변경 사항을 모니터링하고, 백업 수행
void monitoringFile(char *filePath, int flag1, int period)
{
    struct stat statbuf, tmpbuf; // 파일 정보를 저장하는 구조체
    char *backupDirPath = (char *)malloc(sizeof(char *) * PATHMAX); // pid를 디렉터리 이름으로 가지는 백업 디렉터리 경로
    char *logfilePath = (char *)malloc(sizeof(char *) * PATHMAX);   // 파일의 상태를 출력할 로그파일 경로

    time_t lastModifiedTime; // 파일의 마지막 수정시간
    // time_t nowTime; // 현재 시간
    dirNode *directoryNode; // 디렉터리 노드를 가리키는 포인터

    // printf("filePath : %s\n", filePath);

    // 파일 정보 조회
    if (stat(filePath, &statbuf) == -1)
    {
        perror("stat error");
        return;
    }

    // 지정된 파일 경로에 대해 데몬 프로세스가 이미 존재하는 지 확인
    if (isDaemonExist(filePath))
    {
        fprintf(stderr, "Daemon Process about %s is already exist\n", filePath);
        return;
    }

    int pid;
    int child;
    FILE *fp;

    // 파일을 모니터링할 자식 프로세스 생성
    if ((pid = fork()) < 0)
    {
        fprintf(stderr, "ERROR: fork error\n");
        exit(EXIT_FAILURE);
    }
    if (pid > 0) // 부모 프로세스인 경우
    {
        if (wait(&child) >= 0) // 자식 프로세스가 종료될 때까지 기다림
        {
            return;
        }
        // 부모 프로세스는 자식 프로세스의 종료를 기다리면서 데몬 프로세스가 실행되도록 함
    }

    // 위에서 자식프로세스를 생성했기 때문에
    // 이후에 실행되는 코드는 자식 프로세스와 부모 프로세스 양쪽에서 실행
    // 여기서 한번 더 자식 프로세스를 생성 (즉, 두 개의 자식 프로세스 생성)
    // 데몬 프로세스의 역할을 수행
    if ((pid = fork()) < 0)
    {
        fprintf(stderr, "ERROR: fork error\n");
        exit(EXIT_FAILURE);
    }
    else if (pid > 0)
    {
        // 로그파일에 작성
        int fd = open(monitorListLogPATH, O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (fd != -1)
        {
            dprintf(fd, "%d : %s\n", pid, filePath);
            close(fd);
        }
        else
        {
            fprintf(stderr, "ERROR: open error for %s\n", monitorListLogPATH);
        }
        printf("monitoring started (%s) : %d\n", filePath, pid);
        exit(EXIT_SUCCESS);
    }

    // session 작성
    setsid();

    // SIGTTIN, SIGTTOU, SIGTSTP 시그널 무시
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);

    // masking 안함
    umask(0);

    // 작업 디렉터리를 루트로 설정
    chdir("/");

    // 표준입력, 표준출력, 표준에러를 /dev/null로 리다이렉션
    open("/dev/null", O_RDWR);
    dup(0);
    dup(0);

    // 백업 디렉터리 생성
    sprintf(backupDirPath, "%s/%d", backupPATH, getpid());
    if (access(backupDirPath, F_OK) != 0)
        mkdir(backupDirPath, 0777);

    // 로그파일 생성
    sprintf(logfilePath, "%s/%d.log", backupPATH, getpid());

    // 경로가 파일인 경우
    if (S_ISREG(statbuf.st_mode))
    {
        // 위에서 생성한 pid를 이름으로 가지는 디렉터리 아래에 백업 파일을 생성하는 함수 호출
        backupFile(filePath, getDate(), backupDirPath, logfilePath, 0);
        lastModifiedTime = statbuf.st_mtime;
        while (true)
        {
            if (stat(filePath, &tmpbuf) == -1) // 파일이 존재하지 않는 경우 삭제되었다고 간주함
            {
                printToLogFile(logfilePath, getDate(), filePath, 2);
                break;
            }
            // 파일이 변경된 경우 반복문 밖에서 저장한 mtime과 현재 mtime이 다름
            if (lastModifiedTime != tmpbuf.st_mtime)
            {
                backupFile(filePath, getDate(), backupDirPath, logfilePath, 1);
                lastModifiedTime = tmpbuf.st_mtime;
            }
            sleep(period);
        }
    }
    // 경로가 디렉터리인 경우
    else if (S_ISDIR(statbuf.st_mode))
    {
        directoryNode = initDirNode(filePath); // 디렉터리 초기화 함수 호출

        while (1)
        {
            traverseDirectoryNode(logfilePath, directoryNode, backupDirPath, filePath, flag1);
            validateRemovedFiles(logfilePath, directoryNode);
            sleep(period);
        }
    }

    exit(EXIT_SUCCESS);
}

// Add 명령어
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
    int period = 1; // 기본 주기는 1초로 설정

    int flag1 = 0; // 옵션 d = 1, 옵션 r = 2
    int flag2 = 0; // 옵션 t 적용하면 1로 변경

    strcpy(originPath, parameter->filename); // originPath에 백업하려는 파일의 경로를 저장

    // printf("originPath : %s\n", originPath);

    if (lstat(originPath, &statbuf) < 0) // 존재하지 않는 경로를 입력한 경우
    {
        fprintf(stderr, "ERROR: lstat error for %s\n", originPath);
        return 1;
    }

    if (!S_ISREG(statbuf.st_mode) && !S_ISDIR(statbuf.st_mode)) // 일반 파일이나 디렉터리가 아닌 경우
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
        fprintf(stderr, "\"%s\" is directory\n", originPath);
        return -1;
    }

    if (parameter->commandopt & OPT_T)
    {
        flag2 = 1;
        period = atoi(parameter->tmpname);
        // 인자로 받은 문자열을 정수형으로 변환해야 함
    }

    monitoringFile(originPath, flag1, period);
    // 일단 파일이건 디렉터리건 한꺼번에 넘기는 방식
    // flag1 -> -r(2)인지 -d(1)인지 (파일일 경우 0)
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
        commandFun = ListCommand;
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

// 파라미터 관련 구조체 초기화 함수
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
    optind = 0;
    opterr = 0;
    int lastind;
    int option;
    int optcnt = 0;

    switch (command)
    {
    case CMD_ADD:
    {
        if (argcnt < 2) // 경로입력이 없을 경우
        {
            fprintf(stderr, "Usage : %s <FILENAME> [OPTION]\n", arglist[0]);
            return -1;
        }

        if (ConvertPath(arglist[1], parameter->filename) != 0) // 올바르지 않은 경로를 입력한 경우
        {
            fprintf(stderr, "ERROR: %s is invalid filepath\n", parameter->filename);
            return -1;
        }

        // 경로가 사용자 디렉토리를 벗어난 경우 예외처리, 경로가 백업 디렉터리인 경우, 백업 디렉터리를 포함하는 경우 예외처리
        if (strncmp(parameter->filename, homePATH, strlen(homePATH)) ||
            !strcmp(parameter->filename, backupPATH) ||
            !strncmp(parameter->filename, backupPATH, strlen(backupPATH)))
        {
            fprintf(stderr, "ERROR: %s can't be backuped\n", parameter->filename);
            return -1;
        }

        if (lstat(parameter->filename, &buf) < 0) // 파일이 존재하지 않아서 lstat이 안되는 경우
        {
            fprintf(stderr, "ERROR: lstat error for %s\n", parameter->filename);
            return -1;
        }

        if (!S_ISREG(buf.st_mode) && !S_ISDIR(buf.st_mode)) // 일반 파일이나 디렉터리가 아닌 경우
        {
            fprintf(stderr, "ERROR: %s is not regular file\n", parameter->filename);
            return -1;
        }

        lastind = 2;

        while ((option = getopt(argcnt, arglist, "drt:")) != -1) // 옵션 d, r, l, t(뒤에 PERIOD 받아야함)
        {
            if (option != 'd' && option != 'r' && option != 't')
            { // 인자로 받은 옵션이 올바르지 않을 경우
                help("add");
                return -1;
            }

            if (optind == lastind)
            {
                fprintf(stderr, "ERROR: wrong option input\n");
                return -1;
            }

            // 옵션 중 -d나 -r을 사용하였는데 인자로 받은 경로가 파일인 경우
            if ((option == 'd' || option == 'r') && !S_ISDIR(buf.st_mode))
            {
                fprintf(stderr, "ERROR: %s is not directory\n", parameter->filename);
                return -1;
            }

            // 옵션 d를 입력한 경우
            if (option == 'd')
            {
                if (parameter->commandopt & OPT_D)
                {
                    fprintf(stderr, "ERROR: duplicate option -%c\n", option);
                    return -1;
                }
                parameter->commandopt |= OPT_D;
            }

            // 옵션 r을 입력한 경우
            if (option == 'r')
            {
                if (parameter->commandopt & OPT_R)
                {
                    fprintf(stderr, "ERROR: duplicate option -%c\n", option);
                    return -1;
                }
                parameter->commandopt |= OPT_R;
            }

            // 옵션 t를 입력한 경우
            if (option == 't')
            {
                if (parameter->commandopt & OPT_T)
                {
                    fprintf(stderr, "ERROR: duplicate option -%c\n", option);
                    return -1;
                }

                if (optarg == NULL)
                {
                    // 옵션 t는 뒤에 <PERIOD>를 받아야 함
                    fprintf(stderr, "ERROR: <PERIOD> is null\n");
                    return -1;
                }

                parameter->tmpname = optarg;
                // 인자로 받은 값을 tmpname에 넘김
                parameter->commandopt |= OPT_T;
            }

            optcnt++;
            lastind = optind;
        }

        if ((argcnt - optcnt != 2) && !(parameter->commandopt & OPT_T))
        {
            fprintf(stderr, "ERROR: argument error\n");
            return -1;
        }
        break;
    }
    case CMD_REM:
    {
        if (argcnt < 2)
        {
            fprintf(stderr, "Usage : %s <DAEMON_PID> : delete daemon process with <DAEMON_PID>\n", arglist[0]);
            return -1;
        }
        parameter->filename = arglist[1]; // remove에서는 filename에 pid가 넘어감
        // printf("pid : %s\n", parameter->filename);
        break;
    }
    case CMD_LIST:
    {
        if (argcnt < 2)
        { // 인자를 입력하지 않은 경우
            // 현재 실행중인 데몬 프로세스들의 목록을 출력 -> monitor_list.log 파일 전체 출력
            FILE *file = fopen(monitorListLogPATH, "r");
            if (file == NULL)
            {
                fprintf(stderr, "ERROR: fopen error for %s\n", monitorListLogPATH);
                return -1;
            }

            int ch;
            while ((ch = fgetc(file)) != EOF)
            {
                putchar(ch);
            }

            fclose(file);
            return -1;
        }
        parameter->filename = arglist[1]; // list에서는 filename에 pid가 넘어감

        break;
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
            command = CMD_LIST;
        }
        else if (!strcmp(arglist[0], commanddata[3]))
        {
            command = CMD_HELP;
        }
        else if (!strcmp(arglist[0], commanddata[4]))
        {
            command = CMD_EXIT;
            exit(0);
        }
        else
        {
            command = NOT_CMD;
        }

        if (command & (CMD_ADD | CMD_REM | CMD_LIST))
        {
            ParameterInit(&parameter);
            parameter.command = arglist[0];
            if (ParameterProcessing(argcnt, arglist, command, &parameter) == -1)
            {
                continue;
            }

            CommandExec(parameter);
        }
        else if (command & CMD_HELP || command == NOT_CMD)
        {
            HelpExec(arglist[1]);
        }
    }
}

// 초기화 함수
void Init()
{
    getcwd(exePATH, PATHMAX);
    sprintf(homePATH, "%s", getenv("HOME"));
    sprintf(backupPATH, "%s/backup", getenv("HOME")); // 백업 경로

    sprintf(monitorListLogPATH, "%s", backupPATH);
    strcat(monitorListLogPATH, "/monitor_list.log"); // 로그파일 경로

    if (access(backupPATH, F_OK)) // 백업파일 존재하지 않는다면 생성
        mkdir(backupPATH, 0777);

    if (access(monitorListLogPATH, F_OK) != 0)
    { // 로그파일 존재하지 않는다면 생성
        int fd = open(monitorListLogPATH, O_CREAT | O_WRONLY, 0666);
        if (fd == -1)
        {
            fprintf(stderr, "ERROR: file open error for %s\n", monitorListLogPATH);
            exit(EXIT_FAILURE);
        }
        close(fd);
    }
}

int main(int argc, char *argv[])
{
    Init();

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