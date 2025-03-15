#include <stdio.h>
#include <stdlib.h>

#include "gt.h"

void count(void)
{
    int N = (int)gt_getarg(); // GCC will complain, but this is OK
    for (int i = 0; i < N; i++) {
        printf("Thread %d, i = %d\n", gt_getid(), i);
        gt_yield();
    }
    printf("THREAD %d IS DONE!\n", gt_getid());
    gt_exit();
}

void main1(void)
{
    // start threads
    gt_create(&count, (void*)10);
    gt_create(&count, (void*)5);
    gt_create(&count, (void*)3);

    while (gt_alive() > 1) {
        gt_yield();
    }
    gt_exit();
}

int main(void)
{
    gt_init(&main1, NULL);

    return 0;
}
