#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/wait.h>

typedef enum
{
    PROCESS_1,
    PROCESS_2,
    PROCESS_3,
    PROCESS_4,
    PROCESS_MAX
}procName;

typedef struct 
{
    procName    processName;
    pid_t       PID;
    long int    timeofLaunch;
}processData;

int main()
{
    pid_t           forkReturnedPID;
    pid_t           crashedPID[PROCESS_MAX];
    processData     processLaunchedList[PROCESS_MAX];

    const char execList[PROCESS_MAX][100] = {".\\operatorOverloadingPractice.exe",
                                    ".\\operatorOverloadingPractice1.exe",
                                    ".\\operatorOverloadingPractice2.exe",
                                    ".\\operatorOverloadingPractice3.exe"};

    printf("Parent process PID = %d\n", getpid());
    for (int count = 0; count < (int)(PROCESS_MAX); count++)
    {
        forkReturnedPID = fork();
        switch (forkReturnedPID)
        {
            case -1:
                // This is error in fork;
                printf("Error in forking the child\n");
                exit(1);
            break;
            case 0:
            {
                //this is the child process where execve or other equivalent calls are to be made.
                int ret;
                ret = execv(execList[count], NULL);
                if(ret < 0)
                {
                    perror("Error:");
                    printf("Failed in exec\n");
                    exit(1);
                }
            }
            break;
            default:
            {
                //This is the paret process.
                //Capture the details of just lauched child process
                struct timeval now;
                gettimeofday(&now, NULL);
                processLaunchedList[count].timeofLaunch = now.tv_sec;
                processLaunchedList[count].processName = (procName)(count);
                processLaunchedList[count].PID = forkReturnedPID;
                printf("Launched Child process %s with PID : %d\n", execList[count], processLaunchedList[count].PID);
                break;
            }
        }
    }
    printf("Launched all the processes\n");
    printf("In process with PID : %d\n", getpid());
    while(1)
    {
        sleep(1);
    }
    return 0;
}