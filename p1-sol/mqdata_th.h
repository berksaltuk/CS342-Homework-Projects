struct request {
	int id;
	char astr[64];
};

struct node{

    char astr[64];
    struct node *next;
};

#define MQDATA1 "/MQDATA1"
#define MQDATA2 "/MQDATA2"
