#include <stdlib.h>
#include <mqueue.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "mqdata.h"


int main(){

mqd_t mq[2];
struct request request;

int res;

mq[0] = mq_open(MQDATA1, O_RDWR | O_CREAT, 0666, NULL);
mq[1] = mq_open(MQDATA2, O_RDWR);
if (mq[0] == -1) {
    perror("Created MQ1 failed\n");
    exit(1);
}
if (mq[1] == -1) {
    perror("Opening MQ2 failed\n");
    exit(1);
}

printf("MQ1 Created, mq id = %d\n", (int) mq[0]);
int i = 0;


struct mq_attr mq_attr;
struct request *reqptr;

int m;

char *bufptr;
int buflen;

mq_getattr(mq[1], &mq_attr);
printf("mq maximum msgsize = %d\n", (int) mq_attr.mq_msgsize);

buflen = mq_attr.mq_msgsize;
bufptr = (char *) malloc(buflen);

while(1)
{
    char str[64];


	printf("\ncount\navg\nmax\ncount <start> <end>\navg <start> <end>\nrange <start> <end> <K>\nquit\nType one of the commands above to make a request:");
	fgets(str, 64, stdin);
	printf("\n");

    printf("Your request is: %s\n", str);


    int length = strlen(str);
    if(length > 0 && str[length - 1] == '\n')
    {
        str[length - 1] = '\0';
    }

    request.id = i;
    strncpy(request.astr, str, 64);

    res = mq_send(mq[0], (char *) &request, sizeof(struct request), 0);

    if( res == -1)
    {
        perror("mq_send failed\n");
        exit(1);
    }



    if(strcmp(str, "quit") == 0)
    {
    	mq_close(mq[1]);
        mq_close(mq[0]);
        free(bufptr);
        exit(0);
    }
    i++;




    printf("--------------------------------------------------------\n");

    while(1){
        m = mq_receive(mq[1], (char *) bufptr, buflen, NULL);
        if( m == -1)
        {
            perror("Receive failed\n");
            exit(1);
        }


        reqptr = (struct request *) bufptr;
        if( strcmp(reqptr->astr, "done") == 0)
        {
            break;
        }

		if( strcmp(reqptr->astr, "invalid") == 0)
        {
            printf("Please make a valid request!\n");
            break;
        }

        printf("%s\n", reqptr->astr);
        printf("\n");
    }
    printf("--------------------------------------------------------\n");
}

free(bufptr);
mq_close(mq[0]);
mq_close(mq[1]);

return 0;
}
