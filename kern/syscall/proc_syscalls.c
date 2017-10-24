#include <types.h>
#include <kern/errno.h>
#include <kern/unistd.h>
#include <kern/wait.h>
#include <lib.h>
#include <syscall.h>
#include <current.h>
#include <proc.h>
#include <thread.h>
#include <addrspace.h>
#include <copyinout.h>
#include "opt-A2.h"
#include "array.h"
#include "limits.h"
#include "synch.h"
  /* this implementation of sys__exit does not do anything with the exit code */
  /* this needs to be fixed to get exit() and waitpid() working properly */

#if OPT_A2

int counter; 

void sys__exit(int exitcode) {
  
  struct addrspace *as;
  struct proc *p = curproc;
  pid_t parent_pid = curproc->parent_pid; 
  acquire_process_lock(); 
  struct proc *parent = find_proc_struct(parent_pid); 
  release_process_lock(); 
  int curpid = p->pid;
  int len = array_num(p->children);
  
  for (int i = 0; i < len ; i++){
    pid_t * child_pid = array_get(p->children, i); 
    if (hasExited(*child_pid)){  // do something with the exit code if you want
        destroy_exit_struct(*child_pid); 
        kfree(child_pid); 
        child_pid = NULL; 
    }
  }
  
  KASSERT(curproc->p_addrspace != NULL);
  
  as_deactivate();
  /*
   * clear p_addrspace before calling as_destroy. Otherwise if
   * as_destroy sleeps (which is quite possible) when we
   * come back we'll be calling as_activate on a
   * half-destroyed address space. This tends to be
   * messily fatal.
   */
  as = curproc_setas(NULL); 
  as_destroy(as);

  /* detach this thread from its process */
  /* note: curproc cannot be used after this call */
  proc_remthread(curthread);

  /* if this is the last user process in the system, proc_destroy()
     will wake up the kernel menu thread */
  if (parent != NULL){
    post_exitcode(curpid, exitcode);
  }
  else{
    destroy_exit_struct(curpid); 
  }
  proc_destroy(p);
  
  thread_exit();
  /* thread_exit() does not return, so we should never get here */
  panic("return from thread_exit in sys_exit\n");
}


int sys_getpid(pid_t * retval){
    *retval = curproc->pid; 
    return 0; 
}

pid_t sys__fork(struct trapframe *tf){
    struct proc *p = curproc; 
    struct proc *child_proc = proc_create_runprogram(p->p_name); 
    KASSERT(child_proc != NULL); // figure how to handle this exception later
    
    spinlock_acquire(&child_proc->p_lock);// tmp spot
    struct addrspace *child_as = as_create(); 
    KASSERT(child_as != NULL);  // figure out how to handle later
    as_copy(p->p_addrspace, &child_as); 
    
    //spinlock_acquire(&child_proc->p_lock);
	child_proc->p_addrspace = child_as;
	//spinlock_release(&child_proc->p_lock);
	
	pid_t * child_pid = kmalloc(sizeof(pid_t)); 
	*child_pid = child_proc->pid; 
	int ret = array_add(p->children, child_pid, NULL); 
	KASSERT(ret == 0);  // figure out how to handle this error
	//char * tmp = kmalloc((strlen(p->p_name) + 1) * sizeof(char) + 1);
	//*tmp = '\0'; 
	//strcat(strcat(tmp, p->p_name), "asdf"); 
	//unsigned long c_as = (unsigned long) child_as;
	struct trapframe *newtrap = kmalloc(sizeof(struct trapframe)); 
    *newtrap = *tf;  
    spinlock_release(&child_proc->p_lock); // tmp spot
	thread_fork(p->p_name, child_proc, enter_forked_process, newtrap, 2); 
	return *child_pid;     
}

bool isChild(pid_t pid){
    int len = array_num(curproc->children);
    for (int i = 0; i < len; i++){
        pid_t * tmp = array_get(curproc->children, i);
        if (*tmp == pid){
            return true; 
        }    
    }
    return false; 
}

int sys_waitpid(pid_t pid, userptr_t status, int options, pid_t *retval){
    #if OPT_A2
    KASSERT(isChild(pid)); // temporary untill i fig out how to do it
    if(!isChild(pid)){
        
        // RETURN ERROR   // not sure what error to throw or if not to throw error and just return
    }
    
    // get pointer to child proc
    acquire_process_lock(); 
    struct proc* child_proc = find_proc_struct(pid); 
    release_process_lock(); 
    if (child_proc != NULL){
        spinlock_acquire(&child_proc->p_lock);
        spinlock_release(&child_proc->p_lock);
    }
    
    // get pointer to child exit struct
    acquire_process_exit_lock(); 
    struct exit_struct* child_exit = find_exit_struct(pid); 
    release_process_exit_lock(); 
    
    
    
    // wait for child to exit
    lock_acquire(child_exit->wait_lock);
    while(!hasExited(pid)){
        cv_wait(child_exit->wait_cv, child_exit->wait_lock); 
    }
    lock_release(child_exit->wait_lock); 
    int exitstatus = child_exit->exit_status;
    #endif /* OPT_A2 */  
    
    int result = options;  // just to keep options distracted; 
    result = copyout((void *)&exitstatus,status,sizeof(int));
    if (result) {
      return(result);
    }
    *retval = pid; // not sure what this is doing
    return(0); // not sure what to return 
}

#else 
void sys__exit(int exitcode) {
  
  struct addrspace *as;
  struct proc *p = curproc;
  /* for now, just include this to keep the compiler from complaining about
     an unused variable */
  (void)exitcode;

  DEBUG(DB_SYSCALL,"Syscall: _exit(%d)\n",exitcode);

  KASSERT(curproc->p_addrspace != NULL);
  as_deactivate();
  /*
   * clear p_addrspace before calling as_destroy. Otherwise if
   * as_destroy sleeps (which is quite possible) when we
   * come back we'll be calling as_activate on a
   * half-destroyed address space. This tends to be
   * messily fatal.
   */
  as = curproc_setas);
  as_destroy(as);

  /* detach this thread from its process */
  /* note: curproc cannot be used after this call */
  proc_remthread(curthread);

  /* if this is the last user process in the system, proc_destroy()
     will wake up the kernel menu thread */
  proc_destroy(p);
  
  thread_exit();
  /* thread_exit() does not return, so we should never get here */
  panic("return from thread_exit in sys_exit\n");
}

int sys_getpid(pid_t * retval){
    *retval = 1; 
    return(*retval); 
}

/* stub handler for waitpid() system call                */

int
sys_waitpid(pid_t pid,
	    userptr_t status,
	    int options,
	    pid_t *retval)
{
  int exitstatus;
  int result;

  /* this is just a stub implementation that always reports an
     exit status of 0, regardless of the actual exit status of
     the specified process.   
     In fact, this will return 0 even if the specified process
     is still running, and even if it never existed in the first place.

     Fix this!
  */

  if (options != 0) {
    return(EINVAL);
  }
  /* for now, just pretend the exitstatus is 0 */
  exitstatus = 0;
  result = copyout((void *)&exitstatus,status,sizeof(int));
  if (result) {
    return(result);
  }
  *retval = pid;
  return(0);
}

#endif /* OPT_A2 */




