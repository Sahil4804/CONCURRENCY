#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/time.h>
#include <string.h>

// inits
const int N = 1e5 + 5;
const int INF = 1e9 + 5;
int n, k, f, t;
int l[10000], r[10000];
int quantity[10000] = {0};
int globaltimer = -1;
pthread_t tid[1000];
pthread_t tid2[1000];
int globalcounter = 0;
sem_t lock1;
sem_t lock2;
sem_t sema[10000];
sem_t sema2[10000];
int gotcustomer[10000] = {0};
int lackofspace[10000] = {0};

int currentwaiters = 0;
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
// int pauser = 0;
// void pausetimer()
// {
//     pauser = passedtime();
// }

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
} icecreamstruct;
typedef struct
{
    char *type;
    int index;
} toppingsstruct;
typedef struct
{
    int custnumber;
    char *icecreamtype;
    int timetoprepare;
    int *toppings;
} orderstruct;
typedef struct
{
    int custnumber;
    int numoforders;
    int arrivaltime;
    orderstruct *orders;
} customerstruct;

toppingsstruct **toppings;
icecreamstruct **icecream;
customerstruct **customer;
orderstruct *allorder;
int completedarr[10000] = {0};
int isfinished[10000] = {0};
// int orderindex;
int mymachine[10000] = {-1};
int isprev[10000] = {0};

int workingmachines[10000] = {0};
int start = 0;
int endflag = 0;
void *machine_handler(void *args)
{
    // printf("%d %dFROM MACHINE with sleep %d\n", *(int *)args, globalcounter, l[*(int *)args]);
    if (l[*(int *)args] > 0)
        sleep(l[*(int *)args]);

    printf("\e[38;2;255;85;0mMachine %d has started working at %d(s)\033[0m\n", *(int *)args, passedtime());
    int ff = 0;
    while (1)
    {

        if (passedtime() >= r[*(int *)args])
        {
            break;
        }
        sem_wait(&lock1);

        // printf("got the lock %d\n", *(int *)args);
        int index = -1;
        for (int i = 0; i < globalcounter; i++)
        {
            if (completedarr[i] == 1)
                continue;
            // if(passedtime()<customer[allorder[i].custnumber]->arrivaltime) continue;
            // printf("the value of completed arr %d \n", completedarr[i]);
            // printf("%d %d %d\n", r[*(int *)args], passedtime(), allorder[i].timetoprepare);
            // set accordingly
            if (r[*(int *)args] - passedtime() > allorder[i].timetoprepare)
            {
                int flagforflag = 0;
                for (int j = 1; j < *(int *)args; j++)
                {
                    if (isprev[j] == 0 && passedtime() >= l[j] && ((r[j] - passedtime()) > allorder[i].timetoprepare))
                    {
                        flagforflag = 1;
                        break;
                    }
                }
                // printf("flag for flag %d\n", flagforflag);
                // flagforflag = 0;
                if (flagforflag)
                {
                    index = INF;
                    sem_post(&lock1);
                    break;
                }

                int f = 0;
                for (int j = 1; j <= t; j++)
                {
                    if (allorder[i].toppings[j] == 1)
                    {
                        if (quantity[j] == 0)
                        {
                            // printf("Machine %d: Not enough toppings for making %s for customer %d\n", *(int *)args, allorder[i].icecreamtype, allorder[i].custnumber);
                            gotcustomer[allorder[i].custnumber] = 1;
                            f = 1;
                            break;
                        }
                    }
                }
                if (f == 1)
                {
                    for (int j = 0; j < globalcounter; j++)
                    {
                        if (allorder[j].custnumber == allorder[i].custnumber)
                        {
                            completedarr[j] = 1;
                        }
                    }
                    continue;
                }

                index = i;
                break;
            }
            // printf("%d \n", index);
        }
        if (index == INF)
        {

            sem_post(&lock1);
            continue;
        }
        if (index == -1)
        {
            // printf("machine exited %d\n", *(int *)args);
            sem_post(&lock1);
            break;
        }

        // printf("Machine %d: Started preparing %s for customer %d\n", *(int *)args, allorder[index].icecreamtype, allorder[index].custnumber);
        completedarr[index] = 1;
        mymachine[index] = *(int *)args;
        sem_post(&sema[index]);
        isprev[*(int *)args] = 1;
        // printf("released the lock %d\n", *(int *)args);
        sem_wait(&sema2[*(int *)args]);
        isprev[*(int *)args] = 0;
        sleep(0.01);
        // usleep(1000);
    }
    // printf("completed\n");
    // if (passedtime() >= r[*(int *)args] && ff==0)
    // {
    //     printf("Machine %d: No more orders. Closing for the day at %d.\n", *(int *)args, r[*(int *)args]);
    // }

    return NULL;
}

void *order_handler(void *args)
{
    sleep(customer[allorder[*(int *)args].custnumber]->arrivaltime + 1);
    if (lackofspace[allorder[*(int *)args].custnumber] == 1)
    {

        // printf("%d FROM ORDERS\n", *(int *)args);
        return NULL;
    }
    // sleep(1);
    // sleep(1);
    sem_wait(&sema[*(int *)args]);
    if (endflag == 1)
    {
        sem_post(&lock1);
        return NULL;
    }
    for (int j = 1; j <= t; j++)
    {
        if (allorder[*(int *)args].toppings[j] == 1)
        {
            quantity[j]--;
        }
    }
    sem_post(&lock1);

    // sleep(1);
    int counter2 = 0;
    for (int i = 0; i < globalcounter; i++)
    {
        if (allorder[i].custnumber == allorder[*(int *)args].custnumber)
        {
            counter2++;
        }
        if (i == (*(int *)args))
            break;
    }
    printf("\033[0;36mMachine %d starts preparing icecream %d for customer %d at %d(s)\033[0m\n", mymachine[*(int *)args], counter2, allorder[*(int *)args].custnumber, passedtime());
    sleep(allorder[*(int *)args].timetoprepare);
    // printf("order completed at %d\n", passedtime());
    printf("\033[0;34mMachine %d finished  preparing ice cream %d for customer %d at %d(s)\033[0m\n", mymachine[*(int *)args], counter2, allorder[*(int *)args].custnumber, passedtime());
    isfinished[*(int *)args] = 1;
    sem_post(&sema2[mymachine[*(int *)args]]);
    // added for exit printer
    sleep(1);
    return NULL;
}
int alreadyarrived[10000] = {0};
void *timerthreadhandler(void *args)
{
    // printf("go on %d\n",*(int *)args);
    int current = 0;
    while (1)
    {
        for (int i = 1; i < *(int *)args; i++)
        {
            // if(passedtime()<customer[i]->arrivaltime)
            // continue;
            if (completedarr[i] != 0)
                continue;
            int totaltopping[t + 1];
            for (int j = 0; j <= t; j++)
            {
                totaltopping[j] = 0;
            }
            for (int j = 0; j < customer[i]->numoforders; j++)
            {
                for (int k = 1; k <= t; k++)
                {
                    totaltopping[k] += customer[i]->orders[j].toppings[k];
                }
            }
            int f = 1;
            for (int j = 0; j < t; j++)
            {
                // printf("%d ", totaltopping[j + 1]);
                if (quantity[j + 1] >= totaltopping[j + 1])
                {
                }
                else
                {
                    f = 0;
                }
            }
            if (f == 0)
            {
                // printf("Customer %d cannot be served due to fsdfadlack of ingredients\n", i);
                // completedarr[i] = 2;
                gotcustomer[i] = 1;
                for (int j = 0; j < globalcounter; j++)
                {
                    if (allorder[j].custnumber == i)
                    {
                        completedarr[j] = 1;
                    }
                }
            }
            else
            {
                // just to reserve just remove the indgredients add the loop here
                for (int j = 0; j < globalcounter; j++)
                {
                    if (allorder[j].custnumber == i)
                    {
                        completedarr[j] = 0;
                    }
                }
            }
        }
        start = 1;
        return NULL;
    }
}

int isarrivalprinted[10000] = {0};
int isleftprinted[10000] = {0};
int customerhasher[10000] = {0};
void *printerthreadhandler(void *args)
{
    // printf("%d just checking\n", *(int *)args);
    for (int i = 0; i < *(int *)args; i++)
    {
        customerhasher[i] = 0;
    }
    for (int i = 0; i < globalcounter; i++)
    {
        isfinished[i] = 0;
    }

    while (1)
    {

        for (int i = 1; i < *(int *)args; i++)
        {

            if (passedtime() == customer[i]->arrivaltime && isarrivalprinted[i] == 0)
            {
                printf("Customer %d enters at %d(s)\n", i, passedtime());

                currentwaiters++;
                if (currentwaiters > k)
                {
                    currentwaiters--;
                    printf("Customer %d left due to lack of space\n", i);
                    lackofspace[i] = 1;
                    for (int j = 0; j < globalcounter; j++)
                    {
                        if (allorder[j].custnumber == i)
                        {
                            completedarr[j] = 1;
                        }
                    }
                }
                int counter1 = 1;
                for (int j = 0; j < globalcounter; j++)
                {
                    if (allorder[j].custnumber == i)
                    {
                        printf("\033[0;33mIce cream %d : \033[0m", counter1);
                        counter1++;
                        printf("\033[0;33m%s \033[0m", allorder[j].icecreamtype);
                        for (int k = 1; k <= t; k++)
                        {
                            if (allorder[j].toppings[k])
                            {
                                printf("\033[0;33m%s \033[0m", toppings[k]->type);
                            }
                        }
                        printf("\n");
                    }
                }

                isarrivalprinted[i] = 1;
            }
            if (gotcustomer[i] == 1 && passedtime() >= customer[i]->arrivaltime && customerhasher[i] == 0 && isarrivalprinted[i] == 1)
            {
                currentwaiters--;
                // printf("\n");
                printf("\033[1;31mCustomer %d can not be served due to lack of ingredients\033[0m\n", i);
                printf("Customer %d leaves at %d(s)\n", i, passedtime());
                customerhasher[i] = 1;
                gotcustomer[i] = 2;
                for (int j = 0; j < globalcounter; j++)
                {
                    if (allorder[j].custnumber == i)
                    {
                        completedarr[j] = 1;
                    }
                }
            }

            int all = 1;
            for (int j = 0; j < globalcounter; j++)
            {
                if (allorder[j].custnumber == i)
                {
                    if (isfinished[j] != 1)
                    {
                        all = 0;
                        // break;
                    }
                }
            }
            // if(passedtime()==7)
            // printf("%d -> %d", i, all);
            if (all == 1 && customerhasher[i] == 0)
            {
                currentwaiters--;
                printf("\033[0;32mCustomer %d has collected their order(s) and left at %d second(s)\033[0m\n", i, passedtime());
                customerhasher[i] = 1;
            }
        }
        for (int j = 1; j <= n; j++)
        {
            if (passedtime() == r[j] && isleftprinted[j] == 0)
            {
                printf("\e[38;2;255;85;0mMachine %d has stopped working at %d(s)\033[0m\n", j, passedtime());
                workingmachines[j] = 1;
                isleftprinted[j] = 1;
            }
        }

        // for (int j = 0; j < globalcounter; j++)
        // {
        //     if (passedtime() >= customer[allorder[j].custnumber]->arrivaltime && isarrivalprinted[allorder[j].custnumber] == 1)
        //     {
        //         if (completedarr[j] != 1 && customerhasher[allorder[j].custnumber] == 0)
        //         {
        //             if (gotcustomer[allorder[j].custnumber] == 1)
        //             {
        //                 customerhasher[allorder[j].custnumber] = 1;
        //                 printf("Customer %d can not be served due to lack of ingredients22\n", allorder[j].custnumber);
        //                 printf("Customer %d leaves at %d(s)\n", allorder[j].custnumber, passedtime());
        //             }
        //         }
        //     }
        // }

        int sum = 0;
        for (int j = 1; j <= n; j++)
        {
            sum += workingmachines[j];
        }
        if (sum == n)
            break;
    }
    sleep(1);
    // usleep(10000);
    for (int i = 1; i < *(int *)args; i++)
    {

        if (passedtime() == customer[i]->arrivaltime && isarrivalprinted[i] == 0)
        {
            printf("Customer %d enters at %d(s)\n", i, passedtime());

            currentwaiters++;
            if (currentwaiters > k)
            {
                currentwaiters--;
                printf("Customer %d left due to lack of space\n", i);
                lackofspace[i] = 1;
                for (int j = 0; j < globalcounter; j++)
                {
                    if (allorder[j].custnumber == i)
                    {
                        completedarr[j] = 1;
                    }
                }
            }
            int counter1 = 1;
            for (int j = 0; j < globalcounter; j++)
            {
                if (allorder[j].custnumber == i)
                {
                    printf("\033[0;33mIce cream %d : \033[0m", counter1);
                    counter1++;
                    printf("\033[0;33m%s \033[0m", allorder[j].icecreamtype);
                    for (int k = 1; k <= t; k++)
                    {
                        if (allorder[j].toppings[k])
                        {
                            printf("\033[0;33m%s \033[0m", toppings[k]->type);
                        }
                    }
                    printf("\n");
                }
            }

            isarrivalprinted[i] = 1;
        }
        if (gotcustomer[i] == 1 && passedtime() >= customer[i]->arrivaltime && customerhasher[i] == 0 && isarrivalprinted[i] == 1)
        {
            currentwaiters--;

            printf("\033[1;31mCustomer %d can not be served due to lack of ingredients\033[0m\n", i);
            printf("Customer %d leaves at %d(s)\n", i, passedtime());
            customerhasher[i] = 1;
            gotcustomer[i] = 2;
            for (int j = 0; j < globalcounter; j++)
            {
                if (allorder[j].custnumber == i)
                {
                    completedarr[j] = 1;
                }
            }
        }

        int all = 1;
        for (int j = 0; j < globalcounter; j++)
        {
            if (allorder[j].custnumber == i)
            {
                if (isfinished[j] != 1)
                {
                    all = 0;
                    // break;
                }
            }
        }
        // if(passedtime()==7)
        // printf("%d -> %d", i, all);
        if (all == 1 && customerhasher[i] == 0)
        {
            currentwaiters--;
            printf("\033[0;32mCustomer %d has collected their order(s) and left at %d second(s)\033[0m\n", i, passedtime() - 1);
            customerhasher[i] = 1;
        }
    }
    // for (int i = 1; i < *(int *)args; i++)
    // {
    //     printf("%d ", customerhasher[i]);
    //     for (int j = 0; j < globalcounter; j++)
    //     {
    //         if(allorder[j].custnumber==i){
    //             printf("%d ->", isfinished[j]);
    //         }
    //     }
    //     printf("\n");
    //     }
    // printf("\n");

    for (int i = 0; i < globalcounter; i++)
    {
        if (completedarr[i] == 0 && customerhasher[allorder[i].custnumber] == 0)
        {

            customerhasher[allorder[i].custnumber] = 1;
            printf("\033[1;31mCustomer %d can not be served due to machine unavailibility\033[0m\n", allorder[i].custnumber);
            // printf("Customer left at ")
        }
    }
    endflag = 1;
    for (int i = 0; i < globalcounter; i++)
    {
        sem_post(&sema[i]);
    }
    // for (int i = 1; i <=n; i++)
    // {
    //     sema_post(&sema2[i]);
    // }

    return NULL;
}
// n machines
// k total capacity
// f types of icecream
// t number of toppings
int main()
{
    sem_init(&lock1, 0, 1);
    sem_init(&lock2, 0, 1);
    allorder = (orderstruct *)malloc(sizeof(orderstruct) * (1000));
    for (int i = 0; i < 1000; i++)
    {
        allorder[i].custnumber = 0;
        allorder[i].icecreamtype = (char *)malloc(sizeof(char) * 100);
        allorder[i].toppings = (int *)malloc(sizeof(int) * 100);
    }

    scanf("%d %d %d %d", &n, &k, &f, &t);
    for (int i = 1; i <= n; i++)
    {
        scanf("%d %d", &l[i], &r[i]);
    }
    icecream = (icecreamstruct **)malloc(sizeof(icecreamstruct *) * (f + 33));
    for (int i = 1; i <= f; i++)
    {
        icecream[i] = (icecreamstruct *)malloc(sizeof(icecreamstruct));
        icecream[i]->type = (char *)malloc(sizeof(char) * 100);
        scanf("%s %d", icecream[i]->type, &icecream[i]->timetoprepare);
    }
    // printf("done");
    toppings = (toppingsstruct **)malloc(sizeof(toppingsstruct *) * (t + 33));
    for (int i = 1; i <= t; i++)
    {
        toppings[i] = (toppingsstruct *)malloc(sizeof(toppingsstruct));
        toppings[i]->type = (char *)malloc(sizeof(char) * 100);
        toppings[i]->index = i;
        scanf("%s %d\n", toppings[i]->type, &quantity[i]);
        if (quantity[i] == -1)
            quantity[i] = INF;
    }

    customer = (customerstruct **)malloc(sizeof(customerstruct *) * (1000 + 1));
    int c = 1;
    while (1)
    {

        customer[c] = (customerstruct *)malloc(sizeof(customerstruct));
        customer[c]->orders = (orderstruct *)malloc(sizeof(orderstruct));
        int numoforders = 0;
        for (int i = 0; i <= 100; i++)
        {
            customer[c]->orders[i].custnumber = c;
            customer[c]->orders[i].icecreamtype = (char *)malloc(sizeof(char) * 1024);
            customer[c]->orders[i].toppings = (int *)malloc(sizeof(int) * 100);
        }
        int cc;
        // scanf("%d %d %d\n", &cc, &customer[c] ->arrivaltime, &numoforders);
        char buffer[1024];
        fgets(buffer, 1024, stdin);
        char *token = strtok(buffer, " \t\n");
        int counter = 0;
        while (token != NULL)
        {
            if (counter == 0)
            {
                cc = atoi(token);
                counter++;
            }
            else if (counter == 1)
            {
                customer[c]->arrivaltime = atoi(token);
                counter++;
            }
            else
            {
                numoforders = atoi(token);
                customer[c]->numoforders = numoforders;
            }
            token = strtok(NULL, " \t\n");
        }

        // printf("%d %d %d\n", cc, customer[c]->arrivaltime, numoforders);
        if (numoforders == 0)
            break;
        for (int i = 0; i < numoforders; i++)
        {
            char input[100]; // Assuming a maximum input length of 100 characters
            // printf("Enter space-separated values (terminate with Enter):\n");
            int count = 0;
            if (fgets(input, sizeof(input), stdin) != NULL)
            {
                char *token = strtok(input, " \t\n");
                // strcpy(customer[c]->orders[i].icecreamtype, token);
                for (int j = 1; j <= t; j++)
                {
                    customer[c]->orders[i].toppings[j] = 0;
                }
                while (token != NULL)
                {
                    if (count == 0)
                    {
                        char *temp = (char *)malloc(sizeof(char) * 100);
                        strcpy(temp, token);
                        customer[c]->orders[i].icecreamtype = (char *)malloc(sizeof(char) * 100);
                        strcpy(customer[c]->orders[i].icecreamtype, temp);
                        count = 1;
                    }
                    else
                    {
                        for (int j = 1; j <= t; j++)
                        {
                            if (strcmp(token, toppings[j]->type) == 0)
                            {
                                // printf("%s %s\n", token, toppings[j]->type);
                                customer[c]->orders[i].toppings[j] = 1;
                            }
                        }
                    }
                    // printf("Read: %s\n", token);
                    token = strtok(NULL, " \t\n");
                }
            }
            else
            {
                printf("Error reading input.\n");
            }
        }
        // printf("done\n");
        for (int i = 0; i < numoforders; i++)
        {
            // printf("%d %s\n", customer[c]->orders[i].custnumber, customer[c]->orders[i].icecreamtype);
            completedarr[globalcounter] = 0;
            sem_init(&sema[globalcounter], 0, 0);
            allorder[globalcounter].custnumber = c;
            allorder[globalcounter].icecreamtype = (char *)malloc(sizeof(char) * 100);
            strcpy(allorder[globalcounter].icecreamtype, customer[c]->orders[i].icecreamtype);
            allorder[globalcounter].toppings = (int *)malloc(sizeof(int) * 100);
            for (int j = 1; j <= t; j++)
            {
                allorder[globalcounter].toppings[j] = customer[c]->orders[i].toppings[j];
            }
            for (int j = 1; j <= f; j++)
            {
                if (strcmp(allorder[globalcounter].icecreamtype, icecream[j]->type) == 0)
                {
                    allorder[globalcounter].timetoprepare = icecream[j]->timetoprepare;
                }
            }

            // for (int j = 1; j <= t; j++)
            // {
            //     printf("%d ", customer[c]->orders[i].toppings[j]);
            // }
            // printf("\n");
            globalcounter++;
        }
        c++;
    }
    for (int i = 1; i <= n; i++)
    {
        sem_init(&sema2[i], 0, 0);
    }
    passedtime();
    // printf("the time is %d\n", globaltimer);
    printf("----------------------------------------------\n");
    int temptemp[n + 10];
    int temptemp2[globalcounter + 10];
    pthread_t timerthread;
    pthread_create(&timerthread, NULL, timerthreadhandler, (void *)&c);
    pthread_t printerthread;
    pthread_create(&printerthread, NULL, printerthreadhandler, (void *)&c);

    for (int i = 1; i <= n; i++)
    {
        // printf("steat\n ");
        temptemp[i] = i;
        pthread_create(&tid[i], NULL, machine_handler, (void *)(temptemp + i));
    }
    for (int i = 0; i < globalcounter; i++)
    {
        temptemp2[i] = i;
        pthread_create(&tid2[i], NULL, order_handler, (void *)(temptemp2 + i));
    }
    for (int i = 1; i <= n; i++)
    {
        pthread_join(tid[i], NULL);
    }
    for (int i = 1; i <= globalcounter; i++)
    {
        pthread_join(tid2[i], NULL);
    }
    pthread_join(timerthread, NULL);
    pthread_join(printerthread, NULL);
    printf("Parlour closed\n");
}

// for (int i = 1; i <= t; i++)
// {
//     printf("%s %d\n", toppings[i]->type, quantity[i]);
// }

// printf("Enter customer d's details:\n");
// scanf("%s", customer[c]->orders[i].icecreamtype);

// while (1)
// {
//     char *temp = (char *)malloc(sizeof(char) * 1024);
//     fgets(temp, 1024, stdin);
//     if (temp[0] == '\n')
//         break;

//     for (int j = 1; j <= t; j++)
//     {
//         if (strcmp(temp, toppings[j]->type) == 0)
//         {
//             customer[c]->orders[i].toppings[j] = 1;
//         }
//     }
//     fgets(temp, 1024, stdin);
//     // printf("%s\n", temp);
//     if (temp[0] == '\n')
//         break;
// }
// // printf("next\n");

// 2 2 2 3
// 0 7
// 4 8
// vanilla 3
// chocolate 4
// caramel 2
// brownie 4
// strawberry 2
// 1 1 2
// vanilla caramel
// chocolate caramel strawberry
// 2 1 2
// vanilla strawberry caramel
// vanilla strawberry caramel

// int f = 0;
//                 for (int j = 1; j <= t; j++)
//                 {
//                     if (allorder[i].toppings[j] == 1)
//                     {
//                         if (quantity[j] == 0)
//                         {
//                             printf("Machine %d: Not enough toppings for making %s for customer %d\n", *(int *)args, allorder[i].icecreamtype, allorder[i].custnumber);
//                             f = 1;
//                             break;
//                         }
//                     }
//                 }
//                 if (f == 1)
//                 {
//                     continue;
//                 }
//                 for (int j = 1; j <= t; j++)
//                 {
//                     if (allorder[i].toppings[j] == 1)
//                     {
//                         quantity[j]--;
//                     }
//                 }
//                 index = i;
//                 break;
//

// 4 4 2 3
// 0 10
// 0 10
// 0 10
// 0 10
// a 5
// b 5
// c 5
// d 5
// e 5
// 1 1 1
// a c d e
// 2 1 1
// b c d e
// 3 1 1
// a c d e
// 4 1 1
// b c d e
// 5 1 1 1
// a c d e

// 2 3 2 3
// 0 7
// 4 10
// vanilla 3
// chocolate 4
// caramel -1
// brownie 4
// strawberry 4
// 1 1 2
// vanilla caramel
// chocolate brownie strawberry
// 2 8 1
// vanilla strawberry caramel

// 2 2 2 3
// 0 4
// 0 7
// vanilla 5
// chocolate 4
// caramel -1
// brownie 4
// strawberry 4
// 1 1 1
// vanilla caramel
