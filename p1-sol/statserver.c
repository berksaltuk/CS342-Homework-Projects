#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <mqueue.h>
#include <limits.h>

#include "mqdata.h"
#include <time.h>

int processFileAvg(char *fileName, int start, int end, int hasRange);
int processFileCount(char *fileName, int start, int end, int hasRange);
int processFileMax(char *fileName);
int * processFileRange(char *fileName, int start, int end, int *K, int * arr);





int main(int argc, char **argv){


int N = atoi(argv[1]);

if( N > 10)
{
    printf("Too many input files.\n");
	exit(1);
}
if( N < 1)
{
    printf("Not enough input file specifier!");
    exit(1);
}

pid_t pid;


mqd_t mq[2];
struct mq_attr mq_attr;
struct request *reqptr;
int n;
int m;
char *bufptr;
int buflen;

mq[0] = mq_open(MQDATA1, O_RDWR);
mq[1] = mq_open(MQDATA2, O_RDWR| O_CREAT, 0666, NULL);
if (mq[0] == -1) {
    perror("MQ1 open failed!\n");
    exit(1);
}
if (mq[0] == -1)
{
    perror("MQ2 creation failed!\n");
}


printf("mq1 opened, mq id = %d\n", (int) mq[0]);

mq_getattr(mq[0], &mq_attr);
printf("mq1 maximum msgsize = %d\n", (int) mq_attr.mq_msgsize);

printf("mq2 created, mq id = %d\n", (int) mq[1]);

int a = 0;

buflen = mq_attr.mq_msgsize;
bufptr = (char *) malloc(buflen);


while (1) {
	clock_t begin = clock();
    int req_Avg_nr = 0;
    int req_Avg_r = 0;
    int req_Count_nr = 0;
    int req_Count_r = 0;
    int req_Max = 0;
    int req_Range = 0;
    int K = 0;
    n = mq_receive(mq[0], (char *) bufptr, buflen, NULL);
    if (n == -1) {
        perror("mq_receive failed\n");
        exit(1);
    }

    printf("mq_receive success, message size = %d\n", n);

    reqptr = (struct request *) bufptr;

    printf("received request->id = %d\n", reqptr->id);
    printf("received request->astr = %s\n", reqptr->astr);
    printf("\n");

    char *command[3];
    int keep = 0;

    for( char * a = strtok(reqptr->astr, " "); a != NULL; a = strtok(NULL, " "))
    {

        command[keep++] = a;

    }


    int start = 0;
    int end = 0;

	int notOneOfThem = 0;
    if(keep == 1)
    {
        if(strcmp(command[0], "avg") == 0)
        {
            req_Avg_nr = 1;
        }
        else if( strcmp(command[0], "count") == 0)
        {
            req_Count_nr = 1;
        }
        else if( strcmp(command[0], "max") == 0)
        {
            req_Max = 1;
        }
        else if(strcmp(command[0], "quit") == 0)
        {
            printf("Server has been terminated by user.\n");
            free(bufptr);
            mq_close(mq[0]);
            mq_close(mq[1]);
            exit(0);
        }

        else{notOneOfThem = 1;}
    }
    else if(keep == 3)
    {
        if(strcmp(command[0], "avg") == 0)
        {
            req_Avg_r = 1;
            start = atoi(command[1]);
            end = atoi(command[2]);
        }
        else if( strcmp(command[0], "count") == 0)
        {
            req_Count_r = 1;
            start = atoi(command[1]);
            end = atoi(command[2]);
        }
        else{notOneOfThem = 1;}
    }
    else if(keep == 4)
    {
        if(strcmp(command[0], "range") == 0)
        {
            req_Range = 1;
            start = atoi(command[1]);
            end = atoi(command[2]);
            K = atoi(command[3]);

        }
        else{notOneOfThem = 1;}
    }
	else{notOneOfThem = 1;}

	if(notOneOfThem == 1)
    {
        struct request send;
        send.id = a;

        strncpy( send.astr, "invalid", 64);

        m = mq_send(mq[1], (char * ) &send, sizeof(struct request), 0);

        if ( m == -1)
        {
            perror("Send failed...\n");
            exit(1);
        }

        a++;
        continue;
    }

    int fd[N][2];

    for (int i = 0; i < N; i++)
    {


        if(pipe(fd[i]) < 0)
        {
        printf("Pipe creation failed!\n");
        exit(1);
        }

        pid = fork();
        if(pid < 0)
        {
            printf("Fork failed!\n");
            exit(1);
        }

        char *fileName = argv[i+2];

        if(pid == 0){
            int result = 0;
            int *resArr = NULL;

            close(fd[i][0]); //Child will write the result


            if( req_Avg_nr )
            {
                result = processFileAvg(fileName, start, end, 0);
            }
            else if( req_Avg_r)
            {
                result = processFileAvg(fileName, start, end, 1);
            }
            else if( req_Count_nr)
            {
                result = processFileCount(fileName, start, end, 0);
            }
            else if( req_Count_r)
            {
                result = processFileCount(fileName, start, end, 1);
            }
            else if( req_Max)
            {
                result = processFileMax(fileName);
            }
            else if( req_Range)
            {
                if( K > 0 && K < 1001)
                {
                    int arr[K];
                    resArr = processFileRange(fileName, start, end, &K, arr);

                    for(int j = 0; j < K; j++)
                    {
                        write (fd[i][1], &resArr[j], sizeof(resArr[j]));
                    }
                }

            }
            if(!req_Range)
            {
                write (fd[i][1], &result, sizeof(result));

            }

            result = -1;
            write (fd[i][1], &result, sizeof(result));
            close(fd[i][1]);
            exit(0);
        }


    }


    for ( int i = 0; i < N; i++)
    {
        wait(NULL);
    }

    for ( int i = 0; i < N; i++)
    {
        close (fd[i][1]);

        char *fileName = argv[i+2];
        struct request send;
        char msg[64] = "Result of the request for file ";
        strcat(msg, fileName);


        send.id = a;

        strncpy( send.astr, msg, 64);

        m = mq_send(mq[1], (char * ) &send, sizeof(struct request), 0);

        a++;

        if ( m == -1)
        {
            perror("Send failed...\n");
            exit(1);
        }
        int recv_int;

        while (read(fd[i][0], &recv_int, sizeof(recv_int)) > 0)
        {
            if(recv_int == -1)
            {

                close(fd[i][0]);

            }
            else
            {

                char result[64];
                sprintf(result, "%d ", recv_int);

                send.id = a;

                strncpy( send.astr, result, 64);

                m = mq_send(mq[1], (char * ) &send, sizeof(struct request), 0);

                a++;

            }
        }

            printf ("\n");
            fflush (stdout);


    }


struct request send;

send.id = a;

strncpy( send.astr, "done", 64);

m = mq_send(mq[1], (char * ) &send, sizeof(struct request), 0);

a++;

clock_t finish = clock();

double elapsed = (finish - begin)*1000000/(double)CLOCKS_PER_SEC;

printf("It took %.2f microseconds to send the result from all childs.\n", elapsed);

}



free(bufptr);
mq_close(mq[0]);
mq_close(mq[1]);

return 0;
}

int processFileAvg(char *fileName, int start, int end, int hasRange){

int count = 0;
int sum = 0;

FILE *inputFile = fopen(fileName, "r");
if( !inputFile)
{
    printf("Something went wrong, could not open the file!");
    exit(1);
}

char *curLine = NULL;
size_t length = 0;
ssize_t rd;

if(!hasRange){

    while( (rd = getline(&curLine, &length, inputFile)) != -1)
    {
        sum += atoi(curLine);
        count++;
    }

}
else{

    while( (rd = getline(&curLine, &length, inputFile)) != -1)
    {
        if( start <= atoi(curLine) && atoi(curLine) <= end ){
            sum += atoi(curLine);
            count++;
        }
    }
}

fclose(inputFile);
if( curLine)
{
    free(curLine);
}
return sum/count;
}

int processFileCount(char *fileName, int start, int end, int hasRange){

int count = 0;

FILE *inputFile = fopen(fileName, "r");
if( !inputFile)
{
    printf("Something went wrong, could not open the file!");
    exit(1);
}

char *curLine = NULL;
size_t length = 0;
ssize_t rd;

if(!hasRange){

    while( (rd = getline(&curLine, &length, inputFile)) != -1)
    {
        count++;
    }

}
else{

    while( (rd = getline(&curLine, &length, inputFile)) != -1)
    {
        if( start <= atoi(curLine) && atoi(curLine) <= end ){
            count++;
        }
    }
}

fclose(inputFile);
if( curLine)
{
    free(curLine);
}
return count;
}


int processFileMax(char* fileName){

int max = INT_MIN;

FILE *inputFile = fopen(fileName, "r");
if( !inputFile)
{
    printf("Something went wrong, could not open the file!");
    exit(1);
}

char *curLine = NULL;
size_t length = 0;
ssize_t rd;

while( (rd = getline(&curLine, &length, inputFile)) != -1)
{
    if( atoi(curLine) >= max)
    {
        max = atoi(curLine);
    }
}


fclose(inputFile);
if( curLine)
{
    free(curLine);
}
return max;
}

int * processFileRange(char *fileName, int start, int end, int *K, int * arr){

int result = processFileCount(fileName, start, end, 1);

int count = processFileCount(fileName, 0,0,0) + 1;
int indices[count];
memset(indices, 0, count*(sizeof(int)));

if( result < *K)
{
     *K = result;
}

for( int i = 0; i < *K; i++){

    FILE *inputFile = fopen(fileName, "r");
    if( !inputFile)
    {
        printf("Something went wrong, could not open the file!");
        exit(1);
    }

    char *curLine = NULL;
    size_t length = 0;
    ssize_t rd;

    int c = 0;
    int max = INT_MIN;
    int maxi = 0;
    while( (rd = getline(&curLine, &length, inputFile)) != -1)
    {
    	c++;
        if( start <= atoi(curLine) && atoi(curLine) <= end &&  atoi(curLine) >= max){

            if(indices[c] == 0)
            {
                max = atoi(curLine);
                maxi = c;
            }
        }
    }
    if( curLine)
    {
        free(curLine);
    }
    fclose(inputFile);

	indices[maxi] = 1;
    arr[*K-i-1] = max;
}




    return arr;
}
