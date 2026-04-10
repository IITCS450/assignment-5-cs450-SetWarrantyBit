#include "types.h"
#include "stat.h"
#include "user.h"
#include "uthread.h"

#define MAXTHREAD  16
#define STACKSIZE  4096

// TODO: Implement cooperative user-level threads.

void thread_init(void){}
tid_t thread_create(void (*fn)(void*), void *arg){ (void)fn; (void)arg; return -1; }
void thread_yield(void){}
int thread_join(tid_t tid){ (void)tid; return -1; }

enum tstate {FREE, RUNNABLE, RUNNING, ZOMBIE};

struct context{
    uint edi;
    uint esi;
    uint ebx;
    uint ebp;
    uint eip;
};

struct thread{
    tid_t tid;
    enum tstate state;
    struct context *ctx;
    char *stack;
    void (*fn)(void*);
    void *arg;
};

static struct thread threads[MAXTHREAD];
static struct thread *current;
static int next_tid = 1;


static void thread_stub(void){
    current->fn(current->arg);
    current->state = ZOMBIE;
    thread_yield();
}

void thread_init(void){
    int i;
    for(i = 0; i < MAXTHREAD; i++) threads[i].state = FREE;

    threads[0].tid = next_tid++;
    threads[0].state = RUNNING;
    threads[0].stack = 0;
    current = &threads[0];
}

tid_t thread_create(void (*fn)(void*), void *arg){
    int i;
    struct thread *t = 0;

    for(i = 0; i < MAXTHREAD; i++){
        if(threads[i].state == FREE){
            t = &threads[i];
            break; 
        }
    }
    if(!t) return -1;

    t->stack = malloc(STACKSIZE);
    if(!t->stack) return -1;

    t->tid = next_tid++;
    t->fn = fn;
    t->arg = arg;
    t->state = RUNNABLE;

    char *sp = t->stack + STACKSIZE;
    sp -= sizeof(struct context);
    struct context *c = (struct context*)sp;
    c->eip = (uint)thread_stub;
    t->ctx = c;

    return t->tid;
}

static struct thread* pick_next(void){
    int start = (int)(current - threads);
    int i;
    for(i = 1; i <= MAXTHREAD; i++){
        struct thread *t = &threads[(start + i) % MAXTHREAD];
        if(t->state == RUNNABLE) return t;
    }
    return 0;
}

void thread_yield(void){
    struct thread *next = pick_next();
    if(!next) return;

    struct thread *prev = current;
    if(prev->state == RUNNING) prev->state = RUNNABLE;
    next->state = RUNNING;
    current = next;
    uswtch(&prev->ctx, next->ctx);
}

int thread_join(tid_t tid){
    int i;
    for(;;){
        int found = 0;
        for(i = 0; i < MAXTHREAD; i++){
        if(threads[i].tid == tid && threads[i].state != FREE){
            found = 1;
            if(threads[i].state == ZOMBIE){
            free(threads[i].stack);
            threads[i].state = FREE;
            threads[i].stack = 0;
            return 0;
            }
            break;
        }
        }
        if(!found) return -1;
        thread_yield();
    }
}