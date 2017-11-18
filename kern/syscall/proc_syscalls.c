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
#include <vfs.h>
#include <kern/fcntl.h>
  /* this implementation of sys__exit does not do anything with the exit code */
  /* this needs to be fixed to get exit() and waitpid() working properly */

#if OPT_A2

int counter; 



void sys__exit(int exitcode) {
  //kprintf("sys_exit\n"); 
  struct addrspace *as;
  struct proc *p = curproc;
  pid_t pid; 
  pid = p->pid; 
  //lock_acquire(p->creation_lock); 
  spinlock_acquire(&curproc->p_lock); 
  spinlock_release(&curproc->p_lock); 
  pid_t parent_pid = curproc->parent_pid; 
  //kfree(curproc->parent_pid); 
  //curproc->parent_pid = NULL; 
  acquire_process_lock(); 
  int curpid = p->pid;
  remove_process_from_table(curpid); 
  //int len = array_num(p->children);
  release_process_lock(); 
  
  while (array_num(p->children) > 0){
    pid_t * child_pid = array_get(p->children, 0);
    array_remove(p->children, 0); 
    acquire_process_exit_lock();
    if (find_index_exits(*child_pid) != -1){
        release_process_exit_lock(); 
        destroy_exit_struct(*child_pid); 
    }
    //kprintf("freeing child: %d\n", *child_pid); 
    kfree(child_pid);
    child_pid = NULL; 
  }
  /*for (unsigned i = 0; i < array_num(p->children) ; i++){
    pid_t * child_pid = array_get(p->children, i); 
    
    //if (hasExited(*child_pid)){  // do something with the exit code if you want
    destroy_exit_struct(*child_pid); 
    kfree(child_pid); 
    child_pid = NULL; 
  }*/
  
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
  // if parent is kernel then delete your exit struct
  acquire_process_lock(); 
  struct proc *parent = find_proc_struct(parent_pid); 
  release_process_lock(); 
  if (parent != NULL){
    post_exitcode(curpid, exitcode);
  }
  //else{
  //  destroy_exit_struct(curpid); //add this rule back in  
  //}
  //kprintf("9\n"); 
  //release_process_lock(); 
  //lock_release(p->creation_lock); 
  proc_destroy(p);
  thread_exit();
  /* thread_exit() does not return, so we should never get here */
  panic("return from thread_exit in sys_exit\n");
}


int sys_getpid(pid_t * retval){
    *retval = curproc->pid; 
    return 0; 
}
void procCreated(struct proc * child_proc);
void procCreated(struct proc * child_proc){
    struct proc * tmp; 
    tmp = child_proc; 
}

pid_t sys__fork(struct trapframe *tf, int * err){
    //kprintf("sys_fork\n"); 
    struct proc *p = curproc; 
    struct proc *child_proc = proc_create_runprogram(p->p_name); 
    spinlock_acquire(&child_proc->p_lock); 
    procCreated(child_proc); // tmp fn for debugging
    if (child_proc == NULL){
        *err = ENPROC; 
        return -1; 
    }
    KASSERT(child_proc != NULL); 
    
    //lock_acquire(child_proc->creation_lock);
    struct addrspace *child_as = as_create(); 
    if (child_as == NULL){
        KASSERT(child_as != NULL); // tmp
        *err = ENOMEM; 
        return -1; 
    }
    //KASSERT(child_as != NULL);  
    as_copy(p->p_addrspace, &child_as); 
    
	child_proc->p_addrspace = child_as;
	
	pid_t * child_pid = kmalloc(sizeof(pid_t)); 
	if (child_pid == NULL){
	    KASSERT(child_pid != NULL); //tmp
        *err = ENPROC; 
        return -1; 	
	}
	*child_pid = child_proc->pid; 
	int ret = array_add(p->children, child_pid, NULL); 
	KASSERT(ret == 0);  
	struct trapframe *newtrap = kmalloc(sizeof(struct trapframe)); 
	KASSERT(newtrap != NULL); 
    *newtrap = *tf;  
    //lock_release(child_proc->creation_lock); 
	spinlock_release(&child_proc->p_lock);
	thread_fork(p->p_name, child_proc, enter_forked_process, newtrap, 2); 
	*err = 0; 
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
    //kprintf("sys_fork\n"); 
    //acquire_process_lock(); 
    //struct proc* child_proc = find_proc_struct(pid); 
    //KASSERT(child_proc != NULL); 
    //pid_t parent_pid; // debuggin purposes
    //parent_pid = child_proc->parent_pid; 
    //release_process_lock();  
    //if(child_proc != NULL){
        //spinlock_acquire(&child_proc->p_lock); 
        //spinlock_release(&child_proc->p_lock); 
        //spinlock_acquire(
        //lock_acquire(child_proc->creation_lock); 
        //lock_release(child_proc->creation_lock);  // add back in after figurin out deadlock
    //}
    #if OPT_A2
    
    //KASSERT(isChild(pid)); // temporary untill i fig out how to do it
    if(!isChild(pid)){
        panic("curproc: %d, trying to wait for child_pid: %d \n", curproc->pid, pid);
        // RETURN ERROR   // not sure what error to throw or if not to throw error and just return
    }
    //spinlock_release(&child_proc->p_lock); 
    // get pointer to child proc
    //acquire_process_lock(); 
    //struct proc* child_proc = find_proc_struct(pid); 
    //release_process_lock();
     
    //if (child_proc != NULL){
    //    spinlock_acquire(&child_proc->p_lock);
    //    spinlock_release(&child_proc->p_lock);
    //}
    
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
    *retval = pid;
    return(0); 
}

int sys__execv(struct trapframe *tf){
    char * prog_name; 
    struct addrspace *as;
	struct vnode *v;
	vaddr_t entrypoint, stackptr;
	int result; 
	prog_name = kstrdup((char *) tf->tf_a0); 
	
    //kprintf("prog: %s\n", prog_name);
    
    KASSERT(curproc->p_addrspace != NULL); // not sure if need to check this
  
  
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
    

	/* Open the file. */
	result = vfs_open(prog_name, O_RDONLY, 0, &v);
	if (result) {
		return result;
	}

	/* Create a new address space. */
	as = as_create();
	if (as ==NULL) {
		vfs_close(v);
		return ENOMEM;
	}

	/* Switch to it and activate it. */
	curproc_setas(as);
	as_activate();

	/* Load the executable. */
	result = load_elf(v, &entrypoint);
	if (result) {
		/* p_addrspace will go away when curproc is destroyed */
		vfs_close(v);
		return result;
	}

	/* Done with the file now. */
	vfs_close(v);

	/* Define the user stack in the address space */
	result = as_define_stack(as, &stackptr);
	if (result) {
		/* p_addrspace will go away when curproc is destroyed */
		return result;
	}

	/* Warp to user mode. */
	enter_new_process(0 /*argc*/, NULL /*userspace addr of argv*/,
			  stackptr, entrypoint);
	
	/* enter_new_process does not return. */
	panic("enter_new_process returned\n");
	return EINVAL;


}

#else 
/*void sys__exit(int exitcode) {
  
  struct addrspace *as;
  struct proc *p = curproc;
   //for now, just include this to keep the compiler from complaining about
     an unused variable */
  //(void)exitcode;

  //DEBUG(DB_SYSCALL,"Syscall: _exit(%d)\n",exitcode);

  //KASSERT(curproc->p_addrspace != NULL);
  //as_deactivate();
  /*
   * clear p_addrspace before calling as_destroy. Otherwise if
   * as_destroy sleeps (which is quite possible) when we
   * come back we'll be calling as_activate on a
   * half-destroyed address space. This tends to be
   * messily fatal.
   */
  //as = curproc_setas);
  //as_destroy(as);

  /* detach this thread from its process */
  /* note: curproc cannot be used after this call */
  //proc_remthread(curthread);

  /* if this is the last user process in the system, proc_destroy()
     will wake up the kernel menu thread */
  //proc_destroy(p);
  
  //thread_exit();
  /* thread_exit() does not return, so we should never get here */
  //panic("return from thread_exit in sys_exit\n");
//}
*/
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




