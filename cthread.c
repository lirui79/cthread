#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include "cthread.h"

struct cmutex_t {
    pthread_mutex_t mutex;
};

/*   alloc: alloc thread mutex
 *   
 *   return:  return mutex pointer
 */
cmutex*     cmutex_alloc() {
    cmutex* mt = (cmutex *)malloc(sizeof(cmutex));
    int code = pthread_mutex_init(&(mt->mutex), NULL);
    if (code == 0) {
        return mt;
    }
    free(mt);
    printf("error: pthread_mutex_init return code %d\n", code);
    return NULL;
}

/*   lock: thread mutex
 *   mt: thread mutex pointer
 *   return:  return code   0 - success,other - failed code
 */
int         cmutex_lock(cmutex* mt) {
    if (mt == NULL) {
        printf("error: lock mt == NULL\n");
        return -1;
    }
    return pthread_mutex_lock(&(mt->mutex));
}

/*   trylock: thread mutex
 *   mt: thread mutex pointer
 *   return:  return code   0 - success,other - failed code
 */
int         cmutex_trylock(cmutex* mt) {
    if (mt == NULL) {
        printf("error: trylock mt == NULL\n");
        return -1;
    }
    return pthread_mutex_trylock(&(mt->mutex));
}


/*   unlock: thread mutex
 *   mt: thread mutex pointer
 *   return:  return code   0 - success,other - failed code
 */
int         cmutex_unlock(cmutex* mt) {
    if (mt == NULL) {
        printf("error: unlock mt == NULL\n");
        return -1;
    }
    return pthread_mutex_unlock(&(mt->mutex));
}

/*   free: free thread mutex
 *   mt: thread mutex pointer
 */
void        cmutex_free(cmutex *mt) {
    int code = 0;
    if (mt == NULL) {
        return; 
    }
    code = pthread_mutex_destroy(&(mt->mutex));
    if (code == 0) {
        free(mt);
        return;
    }
    printf("error: pthread_mutex_destroy return code %d\n", code);
}

//////////////////////////////////////////////
//////////////////////////////////////////////
struct cwait_t {
    pthread_mutex_t *mutex;
    pthread_cond_t   cond;
    int              signal;
};

/*   alloc: alloc thread wait
 *   mt: thread mutex pointer
 *   return:  return thread wait pointer
 */
cwait*     cwait_alloc(cmutex *mt) {
    cwait *wt = NULL;
    int    code = -1;
    if (mt == NULL) {
        return NULL;
    }

    wt = (cwait *) malloc(sizeof(cwait));
    code = pthread_cond_init(&(wt->cond), NULL);
    wt->signal = 0;
    if (code == 0) {
        wt->mutex = &(mt->mutex);
        return wt;
    }
    free(wt);
    printf("error: pthread_cond_init return code %d\n", code);
    return NULL;
}

/*   wait: thread wait signal or broadcast //int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);  
 *   wt: thread wait pointer
 *   return:  return code   0 - success,other - failed code
 */
int         cwait_wait(cwait* wt) {
    int code = -1;
    if (wt == NULL) {
        printf("error: wait wt == NULL\n");
        return -1;
    }
    if (wt->signal == 0) {
        code = pthread_cond_wait(&(wt->cond), wt->mutex);
    } else {
        code = 0;
    }
    wt->signal = 0;
    return code;
}

static   void cwait_timeout(uint64_t ms, struct timespec* now) {
    while(clock_gettime(CLOCK_REALTIME, now) != 0) 
        ; // -lrt 库

    ms += (uint64_t)(now->tv_nsec / 1000 /1000);
    now->tv_sec += ( ms / 1000) ;
    ms = (ms % 1000) ;
    now->tv_nsec = ((uint64_t)ms) * 1000 * 1000;
}

/*   timedwait: thread timedwait  signal or broadcast or timeout //int pthread_cond_timedwait(pthread_cond_t *cond,pthread_mutex_t *mutex, const timespec *abstime);  
 *   wt: thread wait pointer
 *   timeout: time out ms
 *   return:  return code   0 - success,other - failed code
 */
int         cwait_timedwait(cwait* wt, uint64_t timeout) {
    struct timespec now;
    int code = -1;
    if (wt == NULL) {
        printf("error: timedwait wt == NULL\n");
        return -1;
    }
    if (wt->signal == 0) {
        cwait_timeout(timeout, &now);
        code = pthread_cond_timedwait(&(wt->cond), wt->mutex, &now);
    } else {
        code = 0;
    }
    wt->signal = 0;
    return code;
}

/*   signal: thread signal wake up one thread
 *   wt: thread wait pointer
 *   return:  return code   0 - success,other - failed code
 */
int         cwait_signal(cwait* wt) {
    int code = -1;
    if (wt == NULL) {
        printf("error: signal wt == NULL\n");
        return -1;
    }
    code = pthread_cond_signal(&(wt->cond));
    wt->signal = 1;
    return code;
}

/*   broadcast: thread broadcast wake up all threads
 *   wt: thread wait pointer
 *   return:  return code   0 - success,other - failed code
 */
int         cwait_broadcast(cwait* wt) {
    int code = -1;
    if (wt == NULL) {
        printf("error: broadcast wt == NULL\n");
        return -1;
    }
    code = pthread_cond_broadcast(&(wt->cond));
    wt->signal = 1;
    return code;
}

/*   free: free thread wait
 *   wt: thread wait pointer
 */
void        cwait_free(cwait* wt) {
    int code = 0;
    if (wt == NULL) {
        return; 
    }
    code = pthread_cond_destroy(&(wt->cond));
    if (code == 0) {
        wt->mutex  = NULL;
        wt->signal = 0;
        free(wt);
        return;
    }
    printf("error: pthread_cond_destroy return code %d\n", code);
}

//////////////////////////////////////////////
//////////////////////////////////////////////
struct cthread_t {
    pthread_t        pid;
    int            (*proc)(void *argv);
    void            *argv;
    _Atomic int      state;
};

static void* threadProc(void *argv) {
    cthread *thread = (cthread *)argv;
    int state = 0;
    atomic_store(&(thread->state), 2);
    while(1) {
        state= atomic_load(&(thread->state));
        if (state != 2) {
            break;
        }

        if (thread->proc(thread->argv) < 0) {
            break;
        }
    }

    atomic_store(&(thread->state), 0);
    return NULL;
}

/*   alloc: alloc thread
 *   return:  return thread pointer
 */
cthread*     cthread_alloc() {
    cthread *td = (cthread *) malloc(sizeof(cthread));
    td->pid   = 0;
    td->proc  = NULL;
    td->argv  = NULL;
    atomic_init(&(td->state), 0);
    return td;
}

/*   start: set thread exe func and start thread run
 *   td: thread pointer
 *   proc: func pointer  return >=0 thread circle; return < 0 thread once
 *   return:  return code   0 - success,other - failed code
 */
int         cthread_start(cthread* td, int (*proc)(void *argv), void *argv) {
    int state = -1, code = -1;
    if (td == NULL) {
        printf("error: start td == NULL\n");
        return -1;
    }
    state= atomic_load(&(td->state));
    if (state != 0) {
        return 0;
    }
    atomic_store(&(td->state), state + 1);
    td->proc  = proc;
    td->argv  = argv;
    code = pthread_create(&(td->pid), NULL, &threadProc, td);
    if (code != 0) {
        atomic_store(&(td->state), 0);
        return -2;
    }
    pthread_detach(td->pid);
    return 1;
}

/*   stop: stop thread exe func
 *   td: thread pointer
 *   return:  return code   0 - success,other - failed code
 */
int         cthread_stop(cthread* td) {
    int state = -1, stop = 0;
    if (td == NULL) {
        printf("error: stop td == NULL\n");
        return -1;
    }
    state= atomic_load(&(td->state));
    if (state != 2) {
        return 0;
    }
    
    while((stop++) < 1000) {
        atomic_store(&(td->state), 1);
        usleep(1000);
        state= atomic_load(&(td->state));
        if (state == 0) {
            break;
        }
    }
    return 1;
}

/*   free: free thread
 *   td: thread pointer
 */
void         cthread_free(cthread* td) {
    int state = -1;
    if (td == NULL) {
        printf("error: stop td == NULL\n");
        return;
    }
    state= atomic_load(&(td->state));
    if (state != 0) {
        return;
    }
    free(td);
}