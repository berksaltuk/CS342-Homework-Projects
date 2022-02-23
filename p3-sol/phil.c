#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include <time.h>


enum {THINKING, HUNGRY, EATING} state[5];

pthread_t tid[5]; // Dining philosophers' problem solution w/ 5 philosophers
int t_index[5]; // 0 to 4

pthread_mutex_t mutex;

pthread_cond_t cv[5];
#define MAXEAT    5
#define MAXTHINK 10

void test( int index)
{
    if( (state[(index + 4) % 5] != EATING) &&
        (state[(index + 1) % 5] != EATING) &&
        (state[index] == HUNGRY))
    {
        state[index] = EATING;
        pthread_cond_signal(&cv[index]);
    }
}

void pickup( int index)
{
    pthread_mutex_lock(&mutex);

    state[index] = HUNGRY;
    test(index);
    while( state[index] != EATING)
    {
        pthread_cond_wait(&cv[index], &mutex);
    }

    pthread_mutex_unlock(&mutex);
}

void putdown( int index)
{
    pthread_mutex_lock(&mutex);

    state[index] = THINKING;
    test( (index + 4) % 5 );
    test( (index + 1) % 1 );

    pthread_mutex_unlock(&mutex);
}

static void *dining_philosophers( void *arg_ptr)
{
    int t_index = *((int *)arg_ptr);

    int t_think;
    int t_eat;

    // Solution meets the monitor solution THINK & EAT in every execution of loop
    while(1)
    {
        /* Philosopher will think here with sleep */
        t_think = (rand() % MAXTHINK) + 1;
        sleep(t_think);

        pickup(t_index);

        printf("philosopher %d started eating now.\n", t_index);

        t_eat = (rand() % MAXEAT) + 1;
        sleep(t_eat);

        printf("philosopher %d finished eating now.\n", t_index);

        putdown(t_index);

    }

    pthread_exit(NULL);
}

int main(){

printf("This is a solution program for dining philosophers' problem (To Exit: CRTL+C).\n");

srand(time(NULL));

pthread_mutex_init(&mutex, NULL);

// Initially all the philosophers will be in thinking state
for(int i = 0; i < 5; i++)
{
    state[i] = THINKING;
    t_index[i] = i;

    // Also cv array will be initialized here.
    pthread_cond_init(&cv[i], NULL);
}

// Initializing the threads and show starts!
for(int i = 0; i < 5; i++)
{
    if(pthread_create(&tid[i], NULL, dining_philosophers, (void *)(&t_index[i])) != 0)
    {
       printf("thread create failed \n");
    }
}

for(int i = 0; i < 5; i++)
{
    pthread_join(tid[i], NULL);
}

for(int i = 0; i < 5; i++)
{
    pthread_cond_destroy(&cv[i]);
}

pthread_mutex_destroy(&mutex);
return 0;
}
