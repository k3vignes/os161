#include <types.h>
#include <lib.h>
#include <synchprobs.h>
#include <synch.h>
#include <opt-A1.h>

/* 
 * This simple default synchronization mechanism allows only vehicle at a time
 * into the intersection.   The intersectionSem is used as a a lock.
 * We use a semaphore rather than a lock so that this code will work even
 * before locks are implemented.
 */

/* 
 * Replace this default synchronization mechanism with your own (better) mechanism
 * needed for your solution.   Your mechanism may use any of the available synchronzation
 * primitives, e.g., semaphores, locks, condition variables.   You are also free to 
 * declare other global variables if your solution requires them.
 */

/*
 * replace this with declarations of any synchronization and other variables you need here
 */
// static struct semaphore *intersectionSem; -- Default
static struct lock *intersect_lock = 0; 
static struct lock *NE_lock = 0;
static struct lock *NS_lock = 0;
static struct lock *NW_lock = 0;

static struct lock *ES_lock = 0;
static struct lock *EW_lock = 0;
static struct lock *EN_lock = 0;

static struct lock *SW_lock = 0;
static struct lock *SN_lock = 0;
static struct lock *SE_lock = 0;

static struct lock *WN_lock = 0;
static struct lock *WE_lock = 0;
static struct lock *WS_lock = 0;

/* 
 * The simulation driver will call this function once before starting
 * the simulation
 *
 * You can use it to initialize synchronization and other variables.
 * 
 */
void
intersection_sync_init(void)
{
  /* replace this default implementation with your own implementation 

  intersectionSem = sem_create("intersectionSem",1);
  if (intersectionSem == NULL) {
    panic("could not create intersection semaphore");
  }
  return;
  
  */
    if (intersect_lock == NULL){
        intersect_lock = lock_create("intersect_lock"); 
        if (intersect_lock == NULL) {
			panic("traffic_synch: lock_create failed\n");
		}
    }
    if (NE_lock == NULL){
        NE_lock = lock_create("NE_lock"); 
        if (NE_lock == NULL) {
			panic("traffic_synch: lock_create failed\n");
		}
    }
    if (NS_lock == NULL){
        NS_lock = lock_create("NS_lock"); 
        if (NE_lock == NULL) {
			panic("traffic_synch: lock_create failed\n");
		}
    }
    if (NW_lock == NULL){
        NW_lock = lock_create("NW_lock"); 
        if (NW_lock == NULL) {
			panic("traffic_synch: lock_create failed\n");
		}
    }
    if (ES_lock == NULL){
        ES_lock = lock_create("ES_lock"); 
        if (ES_lock == NULL) {
			panic("traffic_synch: lock_create failed\n");
		}
    }
    if (EW_lock == NULL){
        EW_lock = lock_create("EW_lock"); 
        if (EW_lock == NULL) {
			panic("traffic_synch: lock_create failed\n");
		}
    }
    if (EN_lock == NULL){
        EN_lock = lock_create("EN_lock"); 
        if (EN_lock == NULL) {
			panic("traffic_synch: lock_create failed\n");
		}
    }
    if (SW_lock == NULL){
        SW_lock = lock_create("SW_lock"); 
        if (SW_lock == NULL) {
			panic("traffic_synch: lock_create failed\n");
		}
    }
    if (SN_lock == NULL){
        SN_lock = lock_create("SN_lock"); 
        if (SN_lock == NULL) {
			panic("traffic_synch: lock_create failed\n");
		}
    }
    if (SE_lock == NULL){
        SE_lock = lock_create("SW_lock"); 
        if (SW_lock == NULL) {
			panic("traffic_synch: lock_create failed\n");
		}
    }
    if (WN_lock == NULL){
        WN_lock = lock_create("WN_lock"); 
        if (WN_lock == NULL) {
			panic("traffic_synch: lock_create failed\n");
		}
    }
    if (WE_lock == NULL){
        WE_lock = lock_create("WE_lock"); 
        if (WE_lock == NULL) {
			panic("traffic_synch: lock_create failed\n");
		}
    }
    if (WS_lock == NULL){
        WS_lock = lock_create("WS_lock"); 
        if (WS_lock == NULL) {
			panic("traffic_synch: lock_create failed\n");
		}
    }
}

/* 
 * The simulation driver will call this function once after
 * the simulation has finished
 *
 * You can use it to clean up any synchronization and other variables.
 *
 */
void
intersection_sync_cleanup(void)
{
  /* replace this default implementation with your own implementation 
  KASSERT(intersectionSem != NULL);
  sem_destroy(intersectionSem);
  */
  lock_destroy(NE_lock);
  lock_destroy(NS_lock);
  lock_destroy(NW_lock);
  
  lock_destroy(ES_lock);
  lock_destroy(EW_lock);
  lock_destroy(EN_lock);
  
  lock_destroy(SW_lock);
  lock_destroy(SN_lock);
  lock_destroy(SE_lock);
  
  lock_destroy(WN_lock);
  lock_destroy(WE_lock);
  lock_destroy(WS_lock);
}


/*
 * The simulation driver will call this function each time a vehicle
 * tries to enter the intersection, before it enters.
 * This function should cause the calling simulation thread 
 * to block until it is OK for the vehicle to enter the intersection.
 *
 * parameters:
 *    * origin: the Direction from which the vehicle is arriving
 *    * destination: the Direction in which the vehicle is trying to go
 *
 * return value: none
 */

void
intersection_before_entry(Direction origin, Direction destination) 
{
  /* replace this default implementation with your own implementation */
  //(void)origin;  /* avoid compiler complaint about unused parameter */
  //(void)destination; /* avoid compiler complaint about unused parameter */
  //KASSERT(intersectionSem != NULL);
  //P(intersectionSem);
  
  KASSERT(NE_lock != NULL); 
  KASSERT(NS_lock != NULL);
  KASSERT(NW_lock != NULL);
  
  KASSERT(ES_lock != NULL);
  KASSERT(EW_lock != NULL);
  KASSERT(EN_lock != NULL);
  
  KASSERT(SW_lock != NULL);
  KASSERT(SN_lock != NULL);
  KASSERT(SE_lock != NULL);
  
  KASSERT(WN_lock != NULL);
  KASSERT(WE_lock != NULL);
  KASSERT(WS_lock != NULL);
  
  if (origin == north && destination == east){
    // kprintf("LOCKING DIRECTION: NE\n"); 
    lock_acquire(intersect_lock); 
    // kprintf("LOCKED - intersect_lock"); 
    lock_acquire(ES_lock); 
    // kprintf("LOCKED - ES\n");
    lock_acquire(EW_lock);
    // kprintf("LOCKED - EW\n"); 
    lock_acquire(SN_lock);
    // kprintf("LOCKED - SN\n"); 
    lock_acquire(SE_lock); 
    // kprintf("LOCKED - SE\n");
    lock_acquire(SW_lock);
    // kprintf("LOCKED - SW\n"); 
    lock_acquire(WN_lock);
    // kprintf("LOCKED - WN\n"); 
    lock_acquire(WE_lock); 
    // kprintf("LOCKED - WE\n");  
    lock_release(intersect_lock); 
    // kprintf("UNLOCKED - intersect_lock");
  }
  else if (origin == north && destination == south){
    // kprintf("LOCKING DIRECTION: NS\n"); 
    lock_acquire(intersect_lock); 
    // kprintf("LOCKED - intersect_lock"); 
    lock_acquire(ES_lock);
    // kprintf("LOCKED - ES\n"); 
    lock_acquire(EW_lock);
    // kprintf("LOCKED - EW\n"); 
    lock_acquire(SW_lock);
    // kprintf("LOCKED - SW\n"); 
    lock_acquire(WN_lock);
    // kprintf("LOCKED - WN\n");
    lock_acquire(WE_lock);
    // kprintf("LOCKED - WE\n"); 
    lock_acquire(WS_lock);  
    // kprintf("LOCKED - WS\n");     
    lock_release(intersect_lock); 
    // kprintf("UNLOCKED - intersect_lock");
  }
  else if (origin == north && destination == west){
    // kprintf("LOCKING DIRECTION: NW\n"); 
    lock_acquire(intersect_lock); 
    // kprintf("LOCKED - intersect_lock"); 
    lock_acquire(SW_lock);
    // kprintf("LOCKED - SW\n"); 
    lock_acquire(EW_lock);
    // kprintf("LOCKED - EW\n"); 
    lock_release(intersect_lock); 
    // kprintf("UNLOCKED - intersect_lock"); 
  }
  else if (origin == east && destination == south){
    // kprintf("LOCKING DIRECTION: ES\n");
    lock_acquire(intersect_lock); 
    // kprintf("LOCKED - intersect_lock");  
    lock_acquire(NE_lock);
    // kprintf("LOCKED - NE\n"); 
    lock_acquire(NS_lock); 
    // kprintf("LOCKED - NS\n");
    lock_acquire(SN_lock);
    // kprintf("LOCKED - SN\n"); 
    lock_acquire(SW_lock);
    // kprintf("LOCKED - SW\n"); 
    lock_acquire(WS_lock);
    // kprintf("LOCKED - WS\n"); 
    lock_acquire(WN_lock);
    // kprintf("LOCKED - WN\n"); 
    lock_acquire(WE_lock);
    // kprintf("LOCKED - WE\n");  
    lock_release(intersect_lock); 
    // kprintf("UNLOCKED - intersect_lock");
  }
  else if (origin == east && destination == west){
    // kprintf("LOCKING DIRECTION: EW\n");
    lock_acquire(intersect_lock); 
    // kprintf("LOCKED - intersect_lock"); 
    lock_acquire(NE_lock); 
    // kprintf("LOCKED - NE\n");
    lock_acquire(NS_lock);
    // kprintf("LOCKED - NS\n"); 
    lock_acquire(NW_lock);
    // kprintf("LOCKED - NW\n"); 
    lock_acquire(SN_lock);
    // kprintf("LOCKED - SN\n");
    lock_acquire(SW_lock);
    // kprintf("LOCKED - SW\n"); 
    lock_acquire(WN_lock);
    // kprintf("LOCKED - WN\n");   
    lock_release(intersect_lock); 
    // kprintf("UNLOCKED - intersect_lock");
  }
  else if (origin == east && destination == north){
    // kprintf("LOCKING DIRECTION: EN\n");
    lock_acquire(intersect_lock); 
    // kprintf("LOCKED - intersect_lock");
    lock_acquire(SN_lock);
    // kprintf("LOCKED - SN\n"); 
    lock_acquire(WN_lock); 
    // kprintf("LOCKED - WN\n"); 
    lock_release(intersect_lock); 
    // kprintf("UNLOCKED - intersect_lock");
  }
  else if (origin == south && destination == west){
    // kprintf("LOCKING DIRECTION: SW\n");
    lock_acquire(intersect_lock); 
    // kprintf("LOCKED - intersect_lock"); 
    lock_acquire(NE_lock);
    // kprintf("LOCKED - NE\n"); 
    lock_acquire(NS_lock);
    // kprintf("LOCKED - NS\n"); 
    lock_acquire(NW_lock);
    // kprintf("LOCKED - NW\n"); 
    lock_acquire(ES_lock);
    // kprintf("LOCKED - ES\n"); 
    lock_acquire(EW_lock);
    // kprintf("LOCKED - EW\n"); 
    lock_acquire(WN_lock);
    // kprintf("LOCKED - WN\n"); 
    lock_acquire(WE_lock);
    // kprintf("LOCKED - WE\n");
    lock_release(intersect_lock); 
    // kprintf("UNLOCKED - intersect_lock");
  }
  else if (origin == south && destination == north){
    // kprintf("LOCKING DIRECTION: SN\n");
    lock_acquire(intersect_lock); 
    // kprintf("LOCKED - intersect_lock"); 
    lock_acquire(NE_lock); 
    // kprintf("LOCKED - NE\n");
    lock_acquire(EN_lock);
    // kprintf("LOCKED - EN\n"); 
    lock_acquire(ES_lock);
    // kprintf("LOCKED - ES\n"); 
    lock_acquire(EW_lock);
    // kprintf("LOCKED - EW\n");
    lock_acquire(WN_lock);
    // kprintf("LOCKED - WN\n"); 
    lock_acquire(WE_lock);  
    // kprintf("LOCKED - WE\n"); 
    lock_release(intersect_lock); 
    // kprintf("UNLOCKED - intersect_lock");
  }
  else if (origin == south && destination == east){
    // kprintf("LOCKING DIRECTION: SE\n");
    lock_acquire(intersect_lock); 
    // kprintf("LOCKED - intersect_lock"); 
    lock_acquire(NE_lock);
    // kprintf("LOCKED - NE\n"); 
    lock_acquire(WE_lock); 
    // kprintf("LOCKED - WE\n");
    lock_release(intersect_lock); 
    // kprintf("UNLOCKED - intersect_lock"); 
  }
  else if (origin == west && destination == north){
    // kprintf("LOCKING DIRECTION: WN\n");
    lock_acquire(intersect_lock); 
    // kprintf("LOCKED - intersect_lock"); 
    lock_acquire(NE_lock); 
    // kprintf("LOCKED - NE\n");
    lock_acquire(NS_lock);
    // kprintf("LOCKED - NS\n"); 
    lock_acquire(EN_lock);
    // kprintf("LOCKED - EN\n"); 
    lock_acquire(ES_lock);
    // kprintf("LOCKED - ES\n"); 
    lock_acquire(EW_lock);
    // kprintf("LOCKED - EW\n"); 
    lock_acquire(SN_lock); 
    // kprintf("LOCKED - SN\n");
    lock_acquire(SW_lock); 
    // kprintf("LOCKED - SW\n"); 
    lock_release(intersect_lock); 
    // kprintf("UNLOCKED - intersect_lock"); 
  }
  else if (origin == west && destination == east){
    // kprintf("LOCKING DIRECTION: WE\n");
    lock_acquire(intersect_lock); 
    // kprintf("LOCKED - intersect_lock"); 
    lock_acquire(NE_lock);
    // kprintf("LOCKED - NE\n"); 
    lock_acquire(NS_lock);
    // kprintf("LOCKED - NS\n"); 
    lock_acquire(ES_lock);
    // kprintf("LOCKED - ES\n"); 
    lock_acquire(SN_lock);
    // kprintf("LOCKED - SN\n");
    lock_acquire(SE_lock);
    // kprintf("LOCKED - SE\n"); 
    lock_acquire(SW_lock);
    // kprintf("LOCKED - SW\n");  
    lock_release(intersect_lock); 
    // kprintf("UNLOCKED - intersect_lock");  
  }
  else if (origin == west && destination == south){
    // kprintf("LOCKING DIRECTION: WS\n");
    lock_acquire(intersect_lock); 
    // kprintf("LOCKED - intersect_lock"); 
    lock_acquire(NS_lock);
    // kprintf("LOCKED - NS\n"); 
    lock_acquire(ES_lock);
    // kprintf("LOCKED - ES\n");  
    lock_release(intersect_lock); 
    // kprintf("UNLOCKED - intersect_lock"); 
  }
}


/*
 * The simulation driver will call this function each time a vehicle
 * leaves the intersection.
 *
 * parameters:
 *    * origin: the Direction from which the vehicle arrived
 *    * destination: the Direction in which the vehicle is going
 *
 * return value: none
 */

void
intersection_after_exit(Direction origin, Direction destination) 
{
  /* replace this default implementation with your own implementation */
  (void)origin;  /* avoid compiler complaint about unused parameter */
  (void)destination; /* avoid compiler complaint about unused parameter */
  //KASSERT(intersectionSem != NULL);
  //V(intersectionSem);
  
  KASSERT(NE_lock != NULL); 
  KASSERT(NS_lock != NULL);
  KASSERT(NW_lock != NULL);
  
  KASSERT(ES_lock != NULL);
  KASSERT(EW_lock != NULL);
  KASSERT(EN_lock != NULL);
  
  KASSERT(SW_lock != NULL);
  KASSERT(SN_lock != NULL);
  KASSERT(SE_lock != NULL);
  
  KASSERT(WN_lock != NULL);
  KASSERT(WE_lock != NULL);
  KASSERT(WS_lock != NULL);
  
  if (origin == north && destination == east){
    // kprintf("UNLOCKING DIRECTION: NE\n");
    lock_release(ES_lock);
    // kprintf("UNLOCKED - ES\n"); 
    lock_release(EW_lock);
    // kprintf("UNLOCKED - EW\n"); 
    lock_release(SN_lock);
    // kprintf("UNLOCKED - SN\n"); 
    lock_release(SE_lock);
    // kprintf("UNLOCKED - WE\n"); 
    lock_release(SW_lock);
    // kprintf("UNLOCKED - SW\n"); 
    lock_release(WN_lock); 
    // kprintf("UNLOCKED - WN\n");
    lock_release(WE_lock); 
    // kprintf("UNLOCKED - WE\n"); 
  }
  else if (origin == north && destination == south){
    // kprintf("UNLOCKING DIRECTION: NS\n");
    lock_release(ES_lock);
    // kprintf("UNLOCKED - ES\n"); 
    lock_release(EW_lock);
    // kprintf("UNLOCKED - EW\n"); 
    lock_release(SW_lock);
    // kprintf("UNLOCKED - SW\n"); 
    lock_release(WN_lock);
    // kprintf("UNLOCKED - WN\n");
    lock_release(WE_lock);
    // kprintf("UNLOCKED - WE\n"); 
    lock_release(WS_lock);  
    // kprintf("UNLOCKED - WS\n"); 
  }
  else if (origin == north && destination == west){
    // kprintf("UNLOCKING DIRECTION: NW\n");
    lock_release(SW_lock);
    // kprintf("UNLOCKED - SW\n"); 
    lock_release(EW_lock);
    // kprintf("UNLOCKED - EW\n");  
  }
  else if (origin == east && destination == south){
    // kprintf("UNLOCKING DIRECTION: ES\n");
    lock_release(NE_lock);
    // kprintf("UNLOCKED - NE\n"); 
    lock_release(NS_lock); 
    // kprintf("UNLOCKED - NS\n");
    lock_release(SN_lock);
    // kprintf("UNLOCKED - SN\n"); 
    lock_release(SW_lock);
    // kprintf("UNLOCKED - SW\n"); 
    lock_release(WS_lock);
    // kprintf("UNLOCKED - WS\n"); 
    lock_release(WN_lock);
    // kprintf("UNLOCKED - WN\n"); 
    lock_release(WE_lock);
    // kprintf("UNLOCKED - WE\n");  
  }
  else if (origin == east && destination == west){
    // kprintf("UNLOCKING DIRECTION: EW\n");
    lock_release(NE_lock);
    // kprintf("UNLOCKED - NE\n"); 
    lock_release(NS_lock);
    // kprintf("UNLOCKED - NS\n"); 
    lock_release(NW_lock);
    // kprintf("UNLOCKED - NW\n"); 
    lock_release(SN_lock);
    
    // kprintf("UNLOCKED - SN\n");
    lock_release(SW_lock);
    // kprintf("UNLOCKED - SW\n"); 
    lock_release(WN_lock);
    // kprintf("UNLOCKED - WN\n");   
  }
  else if (origin == east && destination == north){
    // kprintf("UNLOCKING DIRECTION: EN\n");
    lock_release(SN_lock);
    // kprintf("UNLOCKED - SN\n"); 
    lock_release(WN_lock);
    // kprintf("UNLOCKED - WN\n");  
  }
  else if (origin == south && destination == west){
    // kprintf("UNLOCKING DIRECTION: SW\n");
    lock_release(NE_lock);
    // kprintf("UNLOCKED - NE\n"); 
    lock_release(NS_lock);
    // kprintf("UNLOCKED - NS\n"); 
    lock_release(NW_lock);
    // kprintf("UNLOCKED - NW\n"); 
    lock_release(ES_lock);
    // kprintf("UNLOCKED - ES\n"); 
    lock_release(EW_lock);
    // kprintf("UNLOCKED - EW\n"); 
    lock_release(WN_lock);
    // kprintf("UNLOCKED - WN\n"); 
    lock_release(WE_lock);
    // kprintf("UNLOCKED - WE\n");  
  }
  else if (origin == south && destination == north){
    // kprintf("UNLOCKING DIRECTION: SN\n");
    lock_release(NE_lock); 
    // kprintf("UNLOCKED - NE\n");
    lock_release(EN_lock);
    // kprintf("UNLOCKED - EN\n"); 
    lock_release(ES_lock);
    // kprintf("UNLOCKED - ES\n"); 
    lock_release(EW_lock);
    // kprintf("UNLOCKED - EW\n");
    lock_release(WN_lock);
    // kprintf("UNLOCKED - WN\n"); 
    lock_release(WE_lock);
    // kprintf("UNLOCKED - WE\n");  
  }
  else if (origin == south && destination == east){
    // kprintf("UNLOCKING DIRECTION: SE\n");
    lock_release(NE_lock); 
    lock_release(WE_lock);
    // kprintf("UNLOCKED - SE\n");  
  }
  else if (origin == west && destination == north){
    // kprintf("UNLOCKING DIRECTION: WN\n");
    lock_release(NE_lock); 
    // kprintf("UNLOCKED - NE\n");
    lock_release(NS_lock);
    // kprintf("UNLOCKED - NS\n"); 
    lock_release(EN_lock);
    // kprintf("UNLOCKED - EN\n"); 
    lock_release(ES_lock);
    // kprintf("UNLOCKED - ES\n"); 
    lock_release(EW_lock);
    // kprintf("UNLOCKED - EW\n"); 
    lock_release(SN_lock);
    // kprintf("UNLOCKED - SN\n"); 
    lock_release(SW_lock);
    // kprintf("UNLOCKED - SW\n");  
  }
  else if (origin == west && destination == east){
    // kprintf("UNLOCKING DIRECTION: WE\n");
    lock_release(NE_lock); 
    // kprintf("UNLOCKED - NE\n");
    lock_release(NS_lock);
    // kprintf("UNLOCKED - NS\n"); 
    lock_release(ES_lock);
    // kprintf("UNLOCKED - ES\n"); 
    lock_release(SN_lock);
    // kprintf("UNLOCKED - SN\n");
    lock_release(SE_lock);
    // kprintf("UNLOCKED - SE\n"); 
    lock_release(SW_lock);
    // kprintf("UNLOCKED - SW\n");   
  }
  else if (origin == west && destination == south){
    // kprintf("UNLOCKING DIRECTION: WS\n");
    lock_release(NS_lock);
    // kprintf("UNLOCKED - NS\n"); 
    lock_release(ES_lock); 
    // kprintf("UNLOCKED - ES\n"); 
  }  
}
