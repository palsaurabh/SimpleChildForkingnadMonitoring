#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
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
    char        processExecName[100];
}processData;

int runmain = 1;
void signalHandler(int signo)
{
    if(signo == SIGINT)
        printf("Received sigint\n");

    int runmain = 0;
}

void terminateAllChildProcess(processData *processLaunchedList)
{
    for (int count = 0; count < PROCESS_MAX; count++)
    {
        int ret = kill(processLaunchedList[count].PID, 2);
        if(ret < 0)
        {
            printf("Unable to send signal to child process\n");
        }
        else if(ret == 0)
        {
            printf("Successfully sent signal to child process\n");
        }
        
    }
}

void processMonitor(processData *processLaunchedList, pid_t pid)
{
    pid_t forkReturnedPID;
    for(int count = 0; count < PROCESS_MAX; count++)
    {
        if(processLaunchedList[count].PID == pid)
        {
            printf("Found the crashed child details\nRelauching...\n");
            forkReturnedPID = fork();
            switch (forkReturnedPID)
            {
            case -1:
                //Error in forking child process
                printf("Error in forking child process again\n");
                break;
            case 0:
                //Case of child process
                //Provide some time between crash and relauch
                sleep(1);
                int ret = execv(processLaunchedList[count].processExecName, NULL);
                if(ret < 0)
                {
                    printf("Error in relaunching child process\n");
                }
            break;
            default:
                printf("Relaunched child process\n");
                struct  timeval now;
                gettimeofday(&now, NULL);
                processLaunchedList[count].timeofLaunch = now.tv_sec;
                processLaunchedList[count].PID = forkReturnedPID;
                break;
            }
        }
    }
}

int main()
{
    pid_t           forkReturnedPID;
    pid_t           crashedPID[PROCESS_MAX];
    processData     processLaunchedList[PROCESS_MAX];
    int             processStatus;
    int             relaunchProcess = 0;

    strcpy(processLaunchedList[0].processExecName, "operatorOverloadingPractice.exe");
    strcpy(processLaunchedList[0].processExecName, "operatorOverloadingPractice.exe1");
    strcpy(processLaunchedList[0].processExecName, "operatorOverloadingPractice.exe2");
    strcpy(processLaunchedList[0].processExecName, "operatorOverloadingPractice.exe3");

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
                ret = execv(processLaunchedList[count].processExecName, NULL);
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
                printf("Launched Child process %s with PID : %d\n", processLaunchedList[count].processExecName, processLaunchedList[count].PID);
                break;
            }
        }
    }
    printf("Launched all the processes\n");
    printf("In process with PID : %d\n", getpid());
    while(runmain)
    {
        for (int count = 0; count < PROCESS_MAX; count++)
        {
            crashedPID[count] = waitpid(-1, &processStatus, WNOHANG);
            if(crashedPID[count] < 0)
            {
                printf("Error in waitpid()\n");
            }
            else if (crashedPID[count] == 0)
            {
                printf("All good, no child crasdhed/stopped yet\n");
            }
            else if(crashedPID[count] > 0)
            {
                printf("Child process terminated with PID = %d\n", crashedPID[count]);
                if (WIFEXITED(processStatus))
                {
                    printf("Child Process exited normally, not relaunching\n");
                }
                if(WIFSIGNALED(processStatus))
                {
                    int ret = WTERMSIG(processStatus);
                    printf("Signal number that terminated the child process was : %x", ret);
                    printf("relaunching the child process\n");
                    relaunchProcess = 1;
                }
                if(WIFSTOPPED(processStatus))
                {
                    int ret = WSTOPSIG(processStatus);
                    printf("Signal number that stopped the child process was: %x\n", ret);
                    relaunchProcess = 1;
                }
                if(relaunchProcess)
                {
                    processMonitor(processLaunchedList, crashedPID[count]);
                    relaunchProcess = 0;
                }
            }
        }
        usleep(100 * 1000);
    }

    terminateAllChildProcess(processLaunchedList);
    
    return 0;
}