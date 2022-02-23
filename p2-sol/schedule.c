#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#define MINQUANTUM      10
#define MAXQUANTUM      300

#define MAXBURSTS       1000

#define MINBURSTLENGTH  5
#define MAXBURSTLENGTH  400

int main(int argc, char **argv){ // The program will be invoked from CommandPrompt

if( argc != 3)
{
    printf("Wrong number of arguments, please try to invoke in the format of './schedule infile.txt quantum'");
    exit(1);
}
char *filename = argv[1];
int quantum = atoi(argv[2]);

if(quantum < 10 || quantum > 300)
{
    printf("Quantum must be between 10 and 300 ms, your quantum value: %d\n", quantum);
    fprintf(stderr, "%s", "Out of range!");
    exit(1);

}

printf("filename is %s and quantum is %d\n", filename, quantum);

// I will also reach each process with process' number - 1 in arrays
int arrivalTimes[MAXBURSTS] = {0};
int burstLengths[MAXBURSTS] = {0};

double waitingTimes[MAXBURSTS] = {0};
double taTimes[MAXBURSTS] = {0};
double realWaiting[MAXBURSTS] = {0};

double avgTurnArounds[4] = {0};

FILE *file;
file = fopen(filename, "r");

int index = 0;
int arrival = 0;
int burstlen = 0;

int processCount = 0;
while( fscanf(file, "%d %d %d", &index, &arrival, &burstlen) > 0)
{
    if( burstlen < 5 || burstlen > 400)
    {
        printf("Burst length must be between 10 and 300 ms, your burst length for Process %d value: %d\n", index, burstlen);
        fprintf(stderr, "%s", "Out of range!");
        exit(1);
    }
    arrivalTimes[index - 1] = arrival;
    burstLengths[index - 1] = burstlen;
    processCount++;
}
fclose(file);
if( processCount > 1000)
{
    printf("The maximum allowed number for processes is 1000. You tried to invoke program with %d processes!\n", processCount);
    exit(1);
}
printf("Process count: %d\n", processCount);


// Calculation for FCFS
waitingTimes[0] = arrivalTimes[0];
for( int i = 1; i < processCount; i++) // calculation for waiting times
{
    waitingTimes[i] = (double)(waitingTimes[i - 1] + burstLengths[i - 1]);
    realWaiting[i] = (double)(waitingTimes[i] - arrivalTimes[i]);
}

for( int i = 0; i < processCount; i++)
{
    taTimes[i] = (double)(burstLengths[i] + realWaiting[i]);
}

double FCFSsum = 0;
for( int i = 0; i < processCount; i++)
{
    FCFSsum += taTimes[i];
}

avgTurnArounds[0] = (double)(FCFSsum / processCount);


// Calculation for SJF

int curTime = arrivalTimes[0];

int isReady[MAXBURSTS] = {0};

int processLeft = processCount;

int finished[MAXBURSTS] = {0};

while( processLeft != 0){

    for( int i = 0; i < processCount; i++)
    {

        if( curTime >= arrivalTimes[i] && finished[i] == 0)
        {
            isReady[i] = 1;
        }
    }

    int min = INT_MAX;
    int needed = 0;
    for( int i = 0; i < processCount; i++)
    {
        if( isReady[i] == 1 && burstLengths[i] < min)
        {
            min = burstLengths[i];
            needed = i;
        }
    }

    processLeft--;
    finished[needed] = 1;
    isReady[needed] = 0;
    curTime += burstLengths[needed];
    taTimes[needed] = (double)curTime - arrivalTimes[needed];
}

double SJFsum = 0;
for( int i = 0; i < processCount; i++)
{
    SJFsum += taTimes[i];
}

avgTurnArounds[1] = (double)(SJFsum / processCount);

// Stuff is getting real now, preemptive algorithms!

// Calculation for SRJF

curTime = arrivalTimes[0];

processLeft = processCount;

for(int i = 0; i < processCount; i++)
{
    finished[i] = 0;
}

int rem_burst[MAXBURSTS] = {0};

for(int i = 0; i < processCount; i++)
{
    rem_burst[i] = burstLengths[i];
}

while( processLeft != 0){

    for( int i = 0; i < processCount; i++)
    {

        if( curTime == arrivalTimes[i] && finished[i] == 0)
        {
            isReady[i] = 1;
        }
    }

    int min = INT_MAX;
    int needed = 0;
    for( int i = 0; i < processCount; i++)
    {
        if( isReady[i] == 1 && rem_burst[i] < min)
        {
            min = rem_burst[i];
            needed = i;
        }
    }


    rem_burst[needed] = rem_burst[needed] - 1;
    curTime++;

    if( rem_burst[needed] == 0)
    {
        taTimes[needed] = (double)curTime - arrivalTimes[needed];
        finished[needed] = 1;
        isReady[needed] = 0;
        processLeft--;
    }

}

double SRJFsum = 0;
for( int i = 0; i < processCount; i++)
{
    SRJFsum += taTimes[i];
}

avgTurnArounds[2] = (double)(SRJFsum / processCount);

// Calculation for RR

/*I will use an array-based priority queue*/

int priorityQueue[MAXBURSTLENGTH] = {-1};
int tail = -1;

for(int i = 0; i < processCount; i++)
{
    rem_burst[i] = burstLengths[i];
}

for(int i = 0; i < processCount; i++)
{
    finished[i] = 0;
}


curTime = arrivalTimes[0] + 1;

processLeft = processCount;

int needed = 0;

int curq = quantum;


while(processLeft != 0)
{
    if( needed == -1)
    {
        break;
    }
    for( int i = 0; i < processCount; i++)
    {
        if(curTime == arrivalTimes[i])
        {
            tail++;
            priorityQueue[tail] = i;
        }
    }

    rem_burst[needed] = rem_burst[needed] - 1;
    curq--;

    if( rem_burst[needed] == 0 )
    {
        processLeft--;

        taTimes[needed] = (double)(curTime - arrivalTimes[needed]);
        rem_burst[needed] = 0;

        needed = priorityQueue[0];
        for( int i = 0; i < tail; i++)
        {
            int temp = priorityQueue[i];
            priorityQueue[i] = priorityQueue[i + 1];
            priorityQueue[i + 1] = temp;
        }
        priorityQueue[tail] = -1;
        tail--;
        curq = quantum;
    }
    else if( curq == 0)
    {
        tail++;
        priorityQueue[tail] = needed;

        curq = quantum;
        needed = priorityQueue[0];
        for( int i = 0; i < tail; i++)
        {
            int temp = priorityQueue[i];
            priorityQueue[i] = priorityQueue[i + 1];
            priorityQueue[i + 1] = temp;
        }
        priorityQueue[tail] = -1;
        tail--;
    }
    curTime++;

}

double RRsum = 0;
for( int i = 0; i < processCount; i++)
{
    RRsum += taTimes[i];
}

avgTurnArounds[3] = (double)(RRsum / processCount);

// Printing results
printf("\n");
printf("FCFS %d\n", (int)avgTurnArounds[0]);
printf("SJF  %d\n", (int)avgTurnArounds[1]);
printf("SRJF %d\n", (int)avgTurnArounds[2]);
printf("RR   %d\n", (int)avgTurnArounds[3]);

return 0;
}
