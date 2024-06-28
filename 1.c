#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/time.h>
#include <string.h>

// inits
int totalwaittime = 0;
const int N = 1e5 + 5;
const int INF = 1e9 + 5;
int arrival[100] = {INF};
int thread[100] = {INF};
int b, k, n;
sem_t sema[100];
sem_t sema2[100];
sem_t translation[100];
pthread_t tid[100];
sem_t lock1;
int globaltimer;
int wastedcoffee = 0;

int passedtime()
{
    struct timeval timerstruct;
    gettimeofday(&timerstruct, NULL);
    if (globaltimer == -1)
    {
        globaltimer = timerstruct.tv_sec;
        return 0;
    }
    return timerstruct.tv_sec - globaltimer;
}

int min(int a, int b)
{
    if (a < b)
        return a;
    return b;
}

// structs
typedef struct
{
    char *type;
    int timetoprepare;
} coffee;

typedef struct
{

    int custnumber;
    coffee *order;
    int tolerance;
    int arrivaltime;

} customer_q;
int isworking[100] = {0};
void *barista_handler(void *args)
{
    // printf("trying to find the min %d\n", *(int *)args);
    while (1)
    {
        // printf("kaun %d\n", *(int *)args);
        sem_wait(&lock1);

        int index = 0;
        int mn = INF;
        for (int i = 0; i < n; i++)
        {
            mn = min(mn, arrival[i]);
        }
        for (int i = 0; i < n; i++)
        {
            if (mn == arrival[i])
            {
                index = i;
                break;
            }
        }

        // printf("Found min %d at thread %d\n", mn, *(int *)args);
        if (mn >= INF)
        {
            sem_post(&lock1);
            break;
        }
        int copper = 0;
        for (int i = 0; i < *(int *)args; i++)
        {
            if (isworking[i] == 0)
            {
                copper = 1;
                break;
            }
        }
        // printf("%d on thread %d\n",co)
        if (copper)
        {
            sem_post(&lock1);
            continue;
        }

        if (mn > passedtime() && mn < INF)
        {
            sem_post(&lock1);
            continue;
        }

        arrival[index] = INF;
        thread[index + 1] = *(int *)args;
        sem_post(&sema[index + 1]);
        isworking[*(int *)args] = 1;
        sem_post(&lock1);
        // sleep()
        sem_wait(&sema2[*(int *)args]);
        sleep(0.01);
        usleep(100000);
    }

    return NULL;
}
int printer[100] = {0};
int waithasher[10000] = {0};

void *arrivalprinter(customer_q **args)
{
    int c = 0;
    customer_q *temp = *args;
    while (c < n)
    {
        int cor = passedtime();
        c = 0;
        for (int i = 0; i < n; i++)
        {
            if ((cor == arrival[i] + temp[i].tolerance + 1))
            {

                printf("\033[1;31mCustomer %d leaves without their order at %d second(s)\033[0m\n", temp[i].custnumber, passedtime());
                if(waithasher[temp[i].custnumber]==0){
                    totalwaittime += temp[i].tolerance;
                    waithasher[temp[i].custnumber] = 1;
                }
                arrival[i] = 2 * INF;
            }
        }
        for (int i = 0; i < n; i++)
        {
            if (arrival[i] == 2 * INF)
                c++;
        }
    }
    for (int i = 0; i < n; i++)
    {

        // thread[i + 1] = -1;
        sem_post(&sema[i + 1]);
    }
    for (int i = 0; i <= b; i++)
    {
        sem_post(&sema2[i]);
    }
    return NULL;
}
void *customer_handler(void *args)
{
    sleep(((customer_q *)args)->arrivaltime);
    printf("Customer %d arrives at %d(s)\n", ((customer_q *)args)->custnumber, ((customer_q *)args)->arrivaltime);
    printf("\033[1;33mCustomer %d orders %s\033[0m\n", ((customer_q *)args)->custnumber, ((customer_q *)args)->order->type);
    sem_wait(&sema[((customer_q *)args)->custnumber]);
    // printf("due to customer %d wait is %d\n", ((customer_q *)args)->custnumber, (passedtime() - ((customer_q *)args)->arrivaltime));
    if (arrival[((customer_q *)args)->custnumber - 1] == 2 * INF)
    {
        // printf("who\n");
        // wastedcoffee++;
        // if (waithasher[((customer_q *)args)->custnumber] == 0)
        // {
        //     totalwaittime += ((customer_q *)args)->tolerance;
        //     waithasher[((customer_q *)args)->custnumber] = 1;
        // }
        isworking[thread[((customer_q *)args)->custnumber]] = 0;
        return NULL;
    }
    if (waithasher[((customer_q *)args)->custnumber] == 0)
    {
        totalwaittime += (passedtime() - ((customer_q *)args)->arrivaltime+1);
        waithasher[((customer_q *)args)->custnumber] = 1;
    }
    arrival[((customer_q *)args)->custnumber - 1] = 2 * INF;
    sleep(1);
    printf("\033[1;36mBarista %d begins preparing the order of customer %d at %d second(s)\033[0m\n", thread[((customer_q *)args)->custnumber], ((customer_q *)args)->custnumber, passedtime());
    // printf("Customer %d started at the machine at %d by thread %d\n", ((customer_q *)args)->custnumber, passedtime(), thread[((customer_q *)args)->custnumber]);
    if (((customer_q *)args)->tolerance - passedtime() + ((customer_q *)args)->arrivaltime < ((customer_q *)args)->order->timetoprepare)
    {
        int store = ((customer_q *)args)->tolerance - passedtime() + ((customer_q *)args)->arrivaltime + 1;
        if (store > 0)
            sleep(((customer_q *)args)->tolerance - passedtime() + ((customer_q *)args)->arrivaltime + 1);
        if (store > 0)
            printf("\033[1;31mCustomer %d leaves without their order at %d second(s)\033[0m\n   ", ((customer_q *)args)->custnumber, passedtime());
        if (((customer_q *)args)->order->timetoprepare - store > 0)
            sleep(((customer_q *)args)->order->timetoprepare - (store - min(0, store)));
        printf("\033[1;34mBarista %d successfully completes the order of customer %d at %d(s)\033[0m\n", thread[((customer_q *)args)->custnumber], ((customer_q *)args)->custnumber, passedtime());
        wastedcoffee++;
    }
    else
    {
        sleep(((customer_q *)args)->order->timetoprepare);
        // printf("finished preparing for customer %d at time %d \n", ((customer_q *)args)->custnumber, passedtime());
        printf("\033[1;32mCustomer %d leaves with their order at %d second(s)\033[0m\n", ((customer_q *)args)->custnumber, passedtime());
        printf("\033[1;34mBarista %d successfully completes the order of customer %d at %d(s)\033[0m\n", thread[((customer_q *)args)->custnumber], ((customer_q *)args)->custnumber, passedtime());
    }
    isworking[thread[((customer_q *)args)->custnumber]] = 0;
    sem_post(&sema2[thread[((customer_q *)args)->custnumber]]);
    return NULL;
}

int main()
{
    scanf("%d %d %d", &b, &k, &n);
    for (int i = 0; i <= b; i++)
    {
        sem_init(&translation[i], 0, 1);
    }
    coffee *coffeetype = (coffee *)malloc(sizeof(coffee) * k);
    for (int i = 0; i < k; i++)
    {
        coffeetype[i].type = (char *)malloc(sizeof(char) * 100);
        scanf("%s %d", coffeetype[i].type, &coffeetype[i].timetoprepare);
    }
    customer_q *queries = (customer_q *)malloc(sizeof(customer_q) * n);
    pthread_t tid2[b];
    for (int i = 0; i < n; i++)
    {
        queries[i].order = (coffee *)malloc(sizeof(coffee));
        queries[i].order->type = (char *)malloc(sizeof(char) * 100);
        scanf("%d", &queries[i].custnumber);
        scanf("%s", queries[i].order->type);
        scanf("%d %d", &queries[i].arrivaltime, &queries[i].tolerance);
        arrival[i] = queries[i].arrivaltime;
        for (int j = 0; j < k; j++)
        {
            if (strcmp(coffeetype[j].type, queries[i].order->type) == 0)
            {
                queries[i].order->timetoprepare = coffeetype[j].timetoprepare;
                break;
            }
        }

        sem_init(&sema[i], 0, 0);
        sem_init(&sema2[i], 0, 0);
    }
    globaltimer = passedtime();

    // printf("%d\n", passedtime());
    sem_init(&lock1, 0, 1);
    int temptemp[b];
    pthread_t timerthread;

    pthread_create(&timerthread, NULL, arrivalprinter, (void *)&queries);
    for (int i = 0; i < b; i++)
    {
        temptemp[i] = i;
        pthread_create(&tid2[i], NULL, barista_handler, (void *)(temptemp + i));
    }
    for (int i = 0; i < n; i++)
    {
        customer_q *p = &queries[i];
        pthread_create(&tid[i], NULL, customer_handler, (void *)p);
    }

    for (int i = 0; i < n; i++)
    {
        pthread_join(tid[i], NULL);
    }
    for (int i = 0; i < b; i++)
    {
        pthread_join(tid2[i], NULL);
    }
    pthread_join(timerthread, NULL);
    // for (int i = 0; i <= n; i++)
    // {
    //     sem_post(&sema2[i]);
    // }
    printf("%d coffee wasted\n", wastedcoffee);
    float timertimer = totalwaittime;
    // printf("%d\n", totalwaittime);
    timertimer /= n;
    printf("%f\n", timertimer);
    return 0;
}

// 4 2 8
// Espresso 3
// Cappuccino 10
// 1 Espresso 3 15
// 2 Espresso 3 6
// 3 Espresso 3 100
// 4 Espresso 3 100
// 5 Cappuccino 3 100
// 6 Cappuccino 4 100
// 7 Cappuccino 4 100
// 8 Cappuccino 4 100

// 3 2 8
// Espresso 3
// Cappuccino 10
// 1 Cappuccino 4 100
// 2 Espresso 3 6
// 3 Espresso 3 100
// 4 Espresso 3 100
// 5 Cappuccino 4 100
// 6 Cappuccino 4 100
// 7 Espresso 3 100
// 8 Cappuccino 4 100

// 2 2 3
// Espresso 3
// Cappuccino 5
// 1 Espresso 2 6
// 2 Espresso 2 5
// 3 Cappuccino 4 15

// 1 2 3
// Espresso 20
// Cappuccino 5
// 1 Espresso 2 15
// 2 Cappuccino 2 6
// 3 Cappuccino 3 5