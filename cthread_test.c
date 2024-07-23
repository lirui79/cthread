#include  <stddef.h>
#include  <stdio.h>
#include  <stdint.h>
#include  <stdlib.h>
#include  <string.h>

#include  "cthread.h"


struct test_item {
    int id;
    char name[256];
    cmutex *mutex;
    cwait  *wait;
};

static int thread1(void *argv) {
    struct test_item *item = (struct test_item *) argv;
    int id = 0;
    cmutex_lock(item->mutex);
    id = item->id;
    item->id += 1;
    cmutex_unlock(item->mutex);
    printf("111 %d %d\n", getpid(), id);
    if (id == 10000) {
        return -1;
    }
    if (id % 256 == 0) {
        return 1;
    }
    usleep(20);
    return 0;
}


static int thread2(void *argv) {
    struct test_item *item = (struct test_item *) argv;
    int id = 0;
    cmutex_lock(item->mutex);
    id = item->id;
    item->id += 1;
    cmutex_unlock(item->mutex);
    printf("222 %d %d\n", getpid(), id);
    if (id == 100) {
        return -1;
    }
    if (id % 333 == 0) {
        return 1;
    }
    usleep(20);
    return 0;
}

void test_thread1() {
    struct test_item item = { 0, "world!", 0, 0};
    int id = 0;
    item.mutex = cmutex_alloc();
    item.wait  = cwait_alloc(item.mutex);
    cthread *td1 = cthread_alloc();
    cthread *td2 = cthread_alloc();
    cthread_start(td1, &thread1, &item);
    cthread_start(td2, &thread2, &item);
    cmutex_lock(item.mutex);
    id = item.id;
    item.id += 3;
    cmutex_unlock(item.mutex);
    printf("333 %d %d\n", getpid(), id);
    sleep(5);
    cthread_stop(td1);
    cthread_stop(td2);
//    usleep(500000);
    cthread_free(td1);
    cthread_free(td2);
    cwait_free(item.wait);
    cmutex_free(item.mutex);    
}


void test_thread2() {
    
}



int main(int argc, const char *argv[]) {
	test_thread1();
	test_thread2();
	return 0;
}
