/*
 * Copyright (c) 2013
 *	The President and Fellows of Harvard College.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Process support.
 *
 * There is (intentionally) not much here; you will need to add stuff
 * and maybe change around what's already present.
 *
 * p_lock is intended to be held when manipulating the pointers in the
 * proc structure, not while doing any significant work with the
 * things they point to. Rearrange this (and/or change it to be a
 * regular lock) as needed.
 *
 * Unless you're implementing multithreaded user processes, the only
 * process that will have more than one thread is the kernel process.
 */

#include <types.h>
#include <proc.h>
#include <current.h>
#include <addrspace.h>
#include <vnode.h>
#include <vfs.h>
#include <synch.h>
#include <kern/fcntl.h>  
#if OPT_A2
#include "array.h"
#include "kern/wait.h" 
#include "kern/limits.h"
#endif /* OPT_A2 */

/*
 * The process for the kernel; this holds all the kernel-only threads.
 */
struct proc *kproc;

/*
 * Mechanism for making the kernel menu thread sleep while processes are running
 */
#ifdef UW
/* count of the number of processes, excluding kproc */
static volatile unsigned int proc_count;
/* provides mutual exclusion for proc_count */
/* it would be better to use a lock here, but we use a semaphore because locks are not implemented in the base kernel */ 
static struct semaphore *proc_count_mutex;
/* used to signal the kernel menu thread when there are no processes */
struct semaphore *no_proc_sem;   
#if OPT_A2
static volatile unsigned int proc_pid; // NEED TO CHANGE THIS SO THAT WE REUSE PIDS
struct array *processes; // need to check whether or not to make volatile - takeout
struct array *process_exits; // need to check whether or not to make volatile - takeout
static struct lock *processes_lock;  // take out 
static struct lock *process_exits_lock; // take out
//static volatile int avail_pid[__PID_MAX]; 
//static struct lock *avail_pid_lock[__PID_MAX]; 
//static struct cv *avail_pid_cv[__PID_MAX]; 
//static struct lock *avail_pid_arr_lock; 
static struct lock *proc_pid_lock; 

#endif /* OPT_A2 */

#endif  // UW




/*
 * Create a proc structure.
 */
static
struct proc *
proc_create(const char *name)
{
	struct proc *proc;

	proc = kmalloc(sizeof(*proc));
	if (proc == NULL) {
		return NULL;
	}
	proc->p_name = kstrdup(name);
	if (proc->p_name == NULL) {
		kfree(proc);
		return NULL;
	}

	threadarray_init(&proc->p_threads);
	spinlock_init(&proc->p_lock);

	/* VM fields */
	proc->p_addrspace = NULL;
    
	/* VFS fields */
	proc->p_cwd = NULL;

#ifdef UW
	proc->console = NULL;
#endif // UW

	return proc;
}

/*
 * Destroy a proc structure.
 */
void
proc_destroy(struct proc *proc)
{
	/*
         * note: some parts of the process structure, such as the address space,
         *  are destroyed in sys_exit, before we get here
         *
         * note: depending on where this function is called from, curproc may not
         * be defined because the calling thread may have already detached itself
         * from the process.
	 */
    
    struct thread *cur; // delete
	cur = curthread; // delete
	struct proc *cur_proc; //delete
	cur_proc = curproc; // delete
	
	KASSERT(proc != NULL);
	KASSERT(proc != kproc);

	/*
	 * We don't take p_lock in here because we must have the only
	 * reference to this structure. (Otherwise it would be
	 * incorrect to destroy it.)
	 */

	/* VFS fields */
	if (proc->p_cwd) {
		VOP_DECREF(proc->p_cwd);
		proc->p_cwd = NULL;
	}


#ifndef UW  // in the UW version, space destruction occurs in sys_exit, not here
	if (proc->p_addrspace) {
		/*
		 * In case p is the currently running process (which
		 * it might be in some circumstances, or if this code
		 * gets moved into exit as suggested above), clear
		 * p_addrspace before calling as_destroy. Otherwise if
		 * as_destroy sleeps (which is quite possible) when we
		 * come back we'll be calling as_activate on a
		 * half-destroyed address space. This tends to be
		 * messily fatal.
		 */
		struct addrspace *as;

		as_deactivate();
		as = curproc_setas(NULL);
		as_destroy(as);
	}
#endif // UW

#ifdef UW
	if (proc->console) {
	  vfs_close(proc->console);
	}
#endif // UW

    #if OPT_A2 
    
	lock_acquire(processes_lock); // take out
	int index = find_index_processes(proc->pid); // take out 
	array_remove(processes, index); // take out
	lock_release(processes_lock); // take out
	
    #endif /* OPT_A2 */
    
	threadarray_cleanup(&proc->p_threads); 
	spinlock_cleanup(&proc->p_lock);
	//int curpid = proc->pid; 
	kfree(proc->p_name);
	kfree(proc);
	proc = NULL; 
	
	#if OPT_A2 
	 
	//lock_release(avail_pid_lock[curpid]); // might need to change to cv
	
	#endif /* OPT_A2 */

	
	

#ifdef UW
	/* decrement the process count */
        /* note: kproc is not included in the process count, but proc_destroy
	   is never called on kproc (see KASSERT above), so we're OK to decrement
	   the proc_count unconditionally here */
	P(proc_count_mutex); 
	KASSERT(proc_count > 0);
	proc_count--;
	/* signal the kernel menu thread if the process count has reached zero */
	if (proc_count == 0) {
	  V(no_proc_sem);
	}
	V(proc_count_mutex);
#endif // UW
	

}

/*
 * Create the process structure for the kernel.
 */
void
proc_bootstrap(void)
{
  kproc = proc_create("[kernel]");
  kproc->pid = 0; 
  if (kproc == NULL) {
    panic("proc_create for kproc failed\n");
  }
#ifdef UW
  proc_count = 0;
  #if OPT_A2
  /*for (int i =0; i < __PID_MAX; i++){
    avail_pid[i] = -1; 
    char buf[100]; 
    char buf2[100]; 
    create_lock_name(i, buf); 
    create_cv_name(i, buf2); 
    avail_pid_lock[i] = lock_create(buf); 
    avail_pid_cv[i] = cv_create(buf2); 
  }*/
  proc_pid = __PID_MIN; 
  proc_pid_lock = lock_create("proc_pid_lock"); 
  //avail_pid_arr_lock = lock_create("avail_pid_arr_lock"); 
  processes = array_create();  // take out
  array_add(processes, kproc, NULL); //take out
  process_exits = array_create(); // take out
  processes_lock = lock_create("processes_lock"); //take out 
  process_exits_lock = lock_create("process_exits_lock");//take out 
  #endif /* OPT_A2 */
  proc_count_mutex = sem_create("proc_count_mutex",1);
  if (proc_count_mutex == NULL) {
    panic("could not create proc_count_mutex semaphore\n");
  }
  no_proc_sem = sem_create("no_proc_sem",0);
  if (no_proc_sem == NULL) {
    panic("could not create no_proc_sem semaphore\n");
  }
#endif // UW 
}

/*
 * Create a fresh proc for use by runprogram.
 *
 * It will have no address space and will inherit the current
 * process's (that is, the kernel menu's) current directory.
 */
struct proc *
proc_create_runprogram(const char *name)
{
	struct proc *proc;
	char *console_path;
    
	proc = proc_create(name);
	if (proc == NULL) {
		return NULL;
	}
	

#ifdef UW
	/* open the console - this should always succeed */
	console_path = kstrdup("con:");
	if (console_path == NULL) {
	  panic("unable to copy console path name during process creation\n");
	}
	if (vfs_open(console_path,O_WRONLY,0,&(proc->console))) {
	  panic("unable to open the console during process creation\n");
	}
	kfree(console_path);
#endif // UW
	  
	/* VM fields */

	proc->p_addrspace = NULL;

	/* VFS fields */

#ifdef UW
	/* we do not need to acquire the p_lock here, the running thread should
           have the only reference to this process */
        /* also, acquiring the p_lock is problematic because VOP_INCREF may block */
	if (curproc->p_cwd != NULL) {
		VOP_INCREF(curproc->p_cwd);
		proc->p_cwd = curproc->p_cwd;
	}
#else // UW
	spinlock_acquire(&curproc->p_lock);
	if (curproc->p_cwd != NULL) {
		VOP_INCREF(curproc->p_cwd);
		proc->p_cwd = curproc->p_cwd;
	}
	spinlock_release(&curproc->p_lock);
#endif // UW

#ifdef UW
	/* increment the count of processes */
        /* we are assuming that all procs, including those created by fork(),
           are created using a call to proc_create_runprogram  */
	P(proc_count_mutex); 
	proc_count++;
	#if OPT_A2
	proc->children = array_create();
	lock_acquire(proc_pid_lock); 
	//if (pid_taken(proc_pid)){
	//    proc_pid = find_next_avail_pid();  // ADD THIS REUSABILITY FUNCTIONALITY AFTER
	//}
	proc->pid = proc_pid; 
	proc_pid++; 
	lock_release(proc_pid_lock); 
	proc->parent_pid = curproc->pid; // might have to change back to int 
	#endif /* OPT_A2 */
	V(proc_count_mutex);
	
	#if OPT_A2
	
	//lock_acquire(avail_pid_arr_lock); 
	//avail_pid[proc->pid] = 1; 
	//lock_release(avail_pid_arr_lock); 
	
	char buf[100]; 
    char buf2[100]; 
    char buf3[100]; // maybe takeout
    create_lock_name(proc->pid, buf); 
    create_cv_name(proc->pid, buf2); 
	create_lock_name(proc->pid, buf3); // maybe takeout
	 
	struct exit_struct *proc_exit; //take out
	proc_exit = kmalloc(sizeof(*proc_exit)); //take out 
	KASSERT(proc_exit != NULL); //take out
	proc_exit->pid = proc->pid; //take out
	proc_exit->exit_status = 4; //take out
	proc_exit->exited = false; 
	proc_exit->wait_lock = lock_create(buf); //take out
	proc_exit->wait_cv = cv_create(buf2); 
	
	lock_acquire(process_exits_lock); //take out
	array_add(process_exits, proc_exit, NULL); //take out
	lock_release(process_exits_lock); //take out
	
	lock_acquire(processes_lock); //take out
	array_add(processes, proc, NULL); //take out
	lock_release(processes_lock); //take out 
	
	#endif /* OPT_A2 */
	
#endif // UW

	return proc;
}

/*
 * Add a thread to a process. Either the thread or the process might
 * or might not be current.
 */
int
proc_addthread(struct proc *proc, struct thread *t)
{
	int result;

	KASSERT(t->t_proc == NULL);

	spinlock_acquire(&proc->p_lock);
	result = threadarray_add(&proc->p_threads, t, NULL);
	spinlock_release(&proc->p_lock);
	if (result) {
		return result;
	}
	t->t_proc = proc;
	return 0;
}

/*
 * Remove a thread from its process. Either the thread or the process
 * might or might not be current.
 */
void
proc_remthread(struct thread *t)
{
	struct proc *proc;
	unsigned i, num;

	proc = t->t_proc;
	KASSERT(proc != NULL);

	spinlock_acquire(&proc->p_lock);
	/* ugh: find the thread in the array */
	num = threadarray_num(&proc->p_threads);
	for (i=0; i<num; i++) {
		if (threadarray_get(&proc->p_threads, i) == t) {
			threadarray_remove(&proc->p_threads, i);
			spinlock_release(&proc->p_lock);
			t->t_proc = NULL;
			return;
		}
	}
	/* Did not find it. */
	spinlock_release(&proc->p_lock);
	panic("Thread (%p) has escaped from its process (%p)\n", t, proc);
}

/*
 * Fetch the address space of the current process. Caution: it isn't
 * refcounted. If you implement multithreaded processes, make sure to
 * set up a refcount scheme or some other method to make this safe.
 */
struct addrspace *
curproc_getas(void)
{
	struct addrspace *as;
#ifdef UW
        /* Until user processes are created, threads used in testing 
         * (i.e., kernel threads) have no process or address space.
         */
	if (curproc == NULL) {
		return NULL;
	}
#endif

	spinlock_acquire(&curproc->p_lock);
	as = curproc->p_addrspace;
	spinlock_release(&curproc->p_lock);
	return as;
}

/*
 * Change the address space of the current process, and return the old
 * one.
 */
struct addrspace *
curproc_setas(struct addrspace *newas)
{
	struct addrspace *oldas;
	struct proc *proc = curproc;

	spinlock_acquire(&proc->p_lock);
	oldas = proc->p_addrspace;
	proc->p_addrspace = newas;
	spinlock_release(&proc->p_lock);
	return oldas;
}

#if OPT_A2
// Need to have the proc_pid_lock before calling
/*int find_next_avail_pid(){
    for (unsigned i = proc_pid; i < __PID_MAX; i++){
        if (avail_pid[i] == -1){
            return i;  
        }
    }
    for (unsigned i = __PID_MIN; i < proc_pid; i++){
        if (avail_pid[i] == -1){
            return i; 
        }
    }
    // throw out of processes error
    return -1; 
}
*/
void post_exitcode(pid_t pid, int exitcode){
    lock_acquire(process_exits_lock); 
    struct exit_struct *tmp = find_exit_struct(pid);
    tmp->exit_status = _MKWAIT_EXIT(exitcode);
	tmp->exited = true;
	lock_release(process_exits_lock); 
	lock_acquire(tmp->wait_lock); 
	cv_signal(tmp->wait_cv, tmp->wait_lock); 
	lock_release(tmp->wait_lock); 
}

void create_lock_name(int i, char * buf){
    const char * tmp = "lock_for_"; 
    int index = 0; 
    while (tmp[index] != '\0'){
        buf[index] = tmp[index]; 
        index++; 
    }
    long rev_sum = 0; 
    for (int j = 0; j < 5; j++){
        rev_sum += i % 10; 
        i = i / 10;     
    }
    for (int j = 0; j < 5; j++){
        buf[index + j] = (char) ((rev_sum % 10) + 48); 
        rev_sum = rev_sum / 10; 
    } 
    buf[index + 5] = '\0'; 
}

 
void create_cv_name(int i, char* buf){
    const char * tmp = "cv_for_"; 
    int index = 0; 
    while (tmp[index] != '\0'){
        buf[index] = tmp[index]; 
        index++; 
    }
    long rev_sum = 0; 
    for (int j = 0; j < 5; j++){
        rev_sum += i % 10; 
        i = i / 10;     
    }
    for (int j = 0; j < 5; j++){
        buf[index + j] = (char) ((rev_sum % 10) + 48); 
        rev_sum = rev_sum / 10; 
    } 
    buf[index + 5] = '\0'; 

}

// need to hold the process_exits_lock before calling
int find_index_exits(pid_t pid){
    struct exit_struct * tmp;
    int index = -1;  
    int len = array_num(process_exits); 
    for (int i = 0; i < len; i++){
        tmp = array_get(process_exits, i); 
        if (tmp->pid == pid){
            index = i; 
            break; 
        }
    }
    return index; 
}

// need to hold the processes_lock before calling
int find_index_processes(pid_t pid){
    struct proc * tmp;
    int index = -1; 
    int len = array_num(processes); 
    for (int i = 0; i < len; i++){
        tmp = array_get(processes, i); 
        if (tmp->pid == pid){
            index = i; 
            break; 
        }
    }
    return index; 
}

// need to hold the processes_lock before calling
struct proc * find_proc_struct(pid_t pid){ 
    int index = find_index_processes(pid);
    if (index == -1){
        return NULL; 
    }
    struct proc * tmp = array_get(processes, index); 
    return tmp; 
}

// need to hold the process_exits_lock before calling
struct exit_struct * find_exit_struct(pid_t pid){
    int index = find_index_exits(pid); 
    if (index == -1){
        return NULL; 
    }
    struct exit_struct * tmp = array_get(process_exits, index);
    return tmp; 
}  


int get_wait_lock(pid_t pid){
    lock_acquire(process_exits_lock); 
    struct exit_struct *tmp = find_exit_struct(pid);
    lock_release(process_exits_lock);  
    KASSERT(tmp != NULL); 
    lock_acquire(tmp->wait_lock); 
    return tmp->exit_status; 
}

void release_wait_lock(pid_t pid){
    struct thread * cur; 
    cur = curthread; 
    lock_acquire(process_exits_lock);
    struct exit_struct * tmp = find_exit_struct(pid);
    lock_release(process_exits_lock);  
    KASSERT(tmp != NULL); 
    lock_release(tmp->wait_lock);
}

void destroy_exit_struct(pid_t pid){
    lock_acquire(process_exits_lock);
    int index = find_index_exits(pid); 
    struct exit_struct * tmp = find_exit_struct(pid);
    lock_release(process_exits_lock);
    KASSERT(tmp != NULL); 
    lock_acquire(process_exits_lock);
    lock_destroy(tmp->wait_lock); 
    cv_destroy(tmp->wait_cv); 
    kfree(tmp); // MIGHT NOT NEED THIS 
    array_remove(process_exits, index);
    lock_release(process_exits_lock);
}

bool hasExited(pid_t pid){
    lock_acquire(process_exits_lock); 
    struct exit_struct *tmp = find_exit_struct(pid); 
    lock_release(process_exits_lock); 
    //KASSERT(tmp != NULL); 
    if (tmp == NULL){
        panic("pid of %d doesn't exit", pid);
    }
    return tmp->exited; 
}

void acquire_process_lock(void){
    lock_acquire(processes_lock); 
}

void release_process_lock(void){
    lock_release(processes_lock);
}

void acquire_process_exit_lock(void){
    lock_acquire(process_exits_lock); 
}

void release_process_exit_lock(void){
    lock_release(process_exits_lock); 
}

#endif /* OPT_A2 */
