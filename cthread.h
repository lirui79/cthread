
#ifndef CTHREAD_H_INCLUDED
#define CTHREAD_H_INCLUDED


#include <stddef.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C"{
#endif

struct cmutex_t;
typedef struct  cmutex_t  cmutex;

/*   alloc: alloc thread mutex
 *   
 *   return:  return mutex pointer
 */
cmutex*     cmutex_alloc();

/*   lock: thread mutex lock
 *   mt: thread mutex pointer
 *   return:  return code   0 - success,other - failed code
 */
int         cmutex_lock(cmutex* mt);

/*   trylock: thread mutex  trylock 
 *   mt: thread mutex pointer
 *   return:  return code   0 - success,other - failed code
 */
int         cmutex_trylock(cmutex* mt);

/*   unlock: thread mutex  unlock
 *   mt: thread mutex pointer
 *   return:  return code   0 - success,other - failed code
 */
int         cmutex_unlock(cmutex* mt);

/*   free: free thread mutex
 *   mt: thread mutex pointer
 */
void        cmutex_free(cmutex *mt);


//////////////////////////////////////////////
//////////////////////////////////////////////

struct cwait_t;
typedef struct  cwait_t  cwait;

/*   alloc: alloc thread wait
 *   mt: thread mutex pointer
 *   return:  return thread wait pointer
 */
cwait*      cwait_alloc(cmutex *mt);

/*   wait: thread wait signal or broadcast
 *   wt: thread wait pointer
 *   return:  return code   0 - success,other - failed code
 */
int         cwait_wait(cwait* wt);

/*   timedwait: thread timedwait  signal or broadcast or timeout
 *   wt: thread wait pointer
 *   timeout: time out ms
 *   return:  return code   0 - success,other - failed code
 */
int         cwait_timedwait(cwait* wt, uint64_t timeout);

/*   signal: thread signal wake up one thread
 *   wt: thread wait pointer
 *   return:  return code   0 - success,other - failed code
 */
int         cwait_signal(cwait* wt);

/*   broadcast: thread broadcast wake up all threads
 *   wt: thread wait pointer
 *   return:  return code   0 - success,other - failed code
 */
int         cwait_broadcast(cwait* wt);

/*   free: free thread wait
 *   wt: thread wait pointer
 */
void        cwait_free(cwait* wt);

//////////////////////////////////////////////
//////////////////////////////////////////////

struct cthread_t;
typedef struct  cthread_t  cthread;


/*   alloc: alloc thread
 *   return:  return thread pointer
 */
cthread*     cthread_alloc();

/*   start: set thread exe func and start thread run
 *   td: thread pointer
 *   proc: func pointer  return >=0 thread circle; return < 0 thread once
 *   argv: struct pointer
 *   return:  return code   0 - success,other - failed code
 */
int         cthread_start(cthread* td, int (*proc)(void *argv), void *argv);

/*   stop: stop thread exe func
 *   td: thread pointer
 *   exit: wakeup exit circle func pointer
 *   argv: struct pointer
 *   return:  return code   0 - success,other - failed code
 */
int         cthread_stop(cthread* td, int (*exit)(void *argv), void *argv);

/*   free: free thread
 *   td: thread pointer
 */
void        cthread_free(cthread* td);



#ifdef __cplusplus
}
#endif

#endif 