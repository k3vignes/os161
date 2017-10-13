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

// forward declarations
bool validCrossing(Direction origin, Direction destination); 
struct cv * cur_cv(Direction origin, Direction destination);
struct lock * cur_lock(Direction origin, Direction destination);
void alter_counter(Direction origin, Direction destination, bool increment);
void wake_up_threads(Direction origin, Direction Destination);

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

static struct cv *NE_cv = 0;
static struct cv *NS_cv = 0;
static struct cv *NW_cv = 0;

static struct cv *ES_cv = 0;
static struct cv *EW_cv = 0;
static struct cv *EN_cv = 0;

static struct cv *SW_cv = 0;
static struct cv *SN_cv = 0;
static struct cv *SE_cv = 0;

static struct cv *WN_cv = 0;
static struct cv *WE_cv = 0;
static struct cv *WS_cv = 0;


int volatile NE_counter = 0;
int volatile NS_counter = 0;
int volatile NW_counter = 0;

int volatile ES_counter = 0;
int volatile EW_counter = 0;
int volatile EN_counter = 0;

int volatile SW_counter = 0;
int volatile SN_counter = 0;
int volatile SE_counter = 0;

int volatile WN_counter = 0;
int volatile WE_counter = 0;
int volatile WS_counter = 0;

bool validCrossing(Direction origin, Direction destination){
  if(origin == north && destination == east){
        return !(ES_counter || EW_counter ||
        SN_counter || SE_counter || SW_counter ||
        WN_counter || WE_counter); 
  }
  else if (origin == north && destination == south){
    return !(ES_counter || EW_counter || SW_counter ||
    WN_counter || WE_counter || WS_counter); 
  }
  else if (origin == north && destination == west){
    return !(SW_counter || EW_counter); 
  }
  else if (origin == east && destination == south){
    return !(NE_counter || NS_counter || SN_counter ||
    SW_counter || WS_counter || WN_counter || WE_counter); 
  }
  else if (origin == east && destination == west){
    return !(NE_counter || NS_counter || NW_counter ||
    SN_counter || SW_counter || WN_counter); 
  }
  else if (origin == east && destination == north){
    return !(SN_counter || WN_counter); 
  }
  else if (origin == south && destination == west){
    return !(NE_counter || NS_counter || NW_counter ||
    ES_counter || EW_counter || WN_counter || WE_counter); 
    
  }
  else if (origin == south && destination == north){
    return !(NE_counter || EN_counter || ES_counter ||
    EW_counter || WN_counter || WE_counter); 
  }
  else if (origin == south && destination == east){
    return !(NE_counter || WE_counter); 
  }
  else if (origin == west && destination == north){
    return !(NE_counter || NS_counter || EN_counter ||
    ES_counter || EW_counter || SN_counter || SW_counter); 
  }
  else if (origin == west && destination == east){
    return !(NE_counter || NS_counter || ES_counter ||
    SN_counter || SE_counter || SW_counter); 
  }
  else if (origin == west && destination == south){
    return !(NS_counter || ES_counter);
  }
  else{
    panic("traffic_simulation: thread_fork failed: %s\n", strerror(1));
    return false; 
  }
}

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
    KASSERT(NE_lock == NULL); 
    KASSERT(NS_lock == NULL); 
    KASSERT(NW_lock == NULL); 
    
    KASSERT(ES_lock == NULL); 
    KASSERT(EW_lock == NULL); 
    KASSERT(EN_lock == NULL); 
    
    KASSERT(SW_lock == NULL); 
    KASSERT(SN_lock == NULL); 
    KASSERT(SE_lock == NULL); 
    
    KASSERT(WN_lock == NULL); 
    KASSERT(WE_lock == NULL); 
    KASSERT(WS_lock == NULL); 
    
    KASSERT(NE_cv == NULL); 
    KASSERT(NS_cv == NULL); 
    KASSERT(NW_cv == NULL); 
    
    KASSERT(ES_cv == NULL); 
    KASSERT(EW_cv == NULL); 
    KASSERT(EN_cv == NULL); 
    
    KASSERT(SW_cv == NULL); 
    KASSERT(SN_cv == NULL); 
    KASSERT(SE_cv == NULL); 
    
    KASSERT(WN_cv == NULL); 
    KASSERT(WE_cv == NULL); 
    KASSERT(WS_cv == NULL); 
    
    // lock create
    intersect_lock = lock_create("intersect_lock");
    NE_lock = lock_create("NE_lock");
    NS_lock = lock_create("NS_lock");
    NW_lock = lock_create("NW_lock");

    ES_lock = lock_create("ES_lock");
    EW_lock = lock_create("EW_lock");
    EN_lock = lock_create("EN_lock");

    SW_lock = lock_create("SW_lock");
    SN_lock = lock_create("SN_lock");
    SE_lock = lock_create("SE_lock");

    WN_lock = lock_create("WN_lock");
    WE_lock = lock_create("WE_lock");
    WS_lock = lock_create("WS_lock");
    
    //cv create
    NE_cv = cv_create("NE_cv");
    NS_cv = cv_create("NS_cv");
    NW_cv = cv_create("NW_cv");

    ES_cv = cv_create("ES_cv");
    EW_cv = cv_create("EW_cv");
    EN_cv = cv_create("EN_cv");

    SW_cv = cv_create("SW_cv");
    SN_cv = cv_create("SN_cv");
    SE_cv = cv_create("SE_cv");

    WN_cv = cv_create("WN_cv");
    WE_cv = cv_create("WE_cv");
    WS_cv = cv_create("WS_cv");
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
  // lock destroy
  lock_destroy(intersect_lock); 
  
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
  
  
  // cv destroy
  cv_destroy(NE_cv);
  cv_destroy(NS_cv);
  cv_destroy(NW_cv);
  
  cv_destroy(ES_cv);
  cv_destroy(EW_cv);
  cv_destroy(EN_cv);
  
  cv_destroy(SW_cv);
  cv_destroy(SN_cv);
  cv_destroy(SE_cv);
  
  cv_destroy(WN_cv);
  cv_destroy(WE_cv);
  cv_destroy(WS_cv);
}

struct cv * cur_cv(Direction origin, Direction destination){
  if (origin == north && destination == east){
    return NE_cv;  
  }
  else if (origin == north && destination == south){
    return NS_cv; 
  }
  else if (origin == north && destination == west){
    return NW_cv;  
  }
  else if (origin == east && destination == south){
    return ES_cv; 
  }
  else if (origin == east && destination == west){
    return EW_cv; 
  }
  else if (origin == east && destination == north){
    return EN_cv; 
  }
  else if (origin == south && destination == west){
    return SW_cv;
  }
  else if (origin == south && destination == north){
    return SN_cv;
  }
  else if (origin == south && destination == east){
    return SE_cv; 
  }
  else if (origin == west && destination == north){
    return WN_cv;
  }
  else if (origin == west && destination == east){
    return WE_cv;    
  }
  else if (origin == west && destination == south){
    return WS_cv; 
  }
  else{
    panic("traffic_simulation: thread_fork failed: %s\n", strerror(1));
    return NULL; 
  }
}

struct lock * cur_lock(Direction origin, Direction destination){
  if (origin == north && destination == east){
    return NE_lock;  
  }
  else if (origin == north && destination == south){
    return NS_lock; 
  }
  else if (origin == north && destination == west){
    return NW_lock;  
  }
  else if (origin == east && destination == south){
    return ES_lock; 
  }
  else if (origin == east && destination == west){
    return EW_lock; 
  }
  else if (origin == east && destination == north){
    return EN_lock; 
  }
  else if (origin == south && destination == west){
    return SW_lock;
  }
  else if (origin == south && destination == north){
    return SN_lock;
  }
  else if (origin == south && destination == east){
    return SE_lock; 
  }
  else if (origin == west && destination == north){
    return WN_lock;
  }
  else if (origin == west && destination == east){
    return WE_lock;    
  }
  else if (origin == west && destination == south){
    return WS_lock; 
  }
  else{
    panic("traffic_simulation: thread_fork failed: %s\n", strerror(1));
    return NULL; 
  }
}


void alter_counter(Direction origin, Direction destination, bool increment){
  int tmp; 
  if (increment){
    tmp = 1; 
  }
  else {
    tmp = -1; 
  }
  
  if (origin == north && destination == east){
    NE_counter = NE_counter + tmp; 
  }
  else if (origin == north && destination == south){
    NS_counter = NS_counter + tmp; 
  }
  else if (origin == north && destination == west){
    NW_counter = NW_counter + tmp;  
  }
  else if (origin == east && destination == south){
    ES_counter = ES_counter + tmp; 
  }
  else if (origin == east && destination == west){
    EW_counter = EW_counter + tmp;  
  }
  else if (origin == east && destination == north){
    EN_counter = EN_counter + tmp;  
  }
  else if (origin == south && destination == west){
    SW_counter = SW_counter + tmp; 
  }
  else if (origin == south && destination == north){
    SN_counter = SN_counter + tmp; 
  }
  else if (origin == south && destination == east){
    SE_counter = SE_counter + tmp; 
  }
  else if (origin == west && destination == north){
    WN_counter = WN_counter + tmp; 
  }
  else if (origin == west && destination == east){
    WE_counter = WE_counter + tmp;     
  }
  else if (origin == west && destination == south){
    WS_counter = WS_counter + tmp;  
  }
  else{
    panic("traffic_simulation: thread_fork failed: %s\n", strerror(1));
  }
}


void wake_up_threads(Direction origin, Direction destination){
  if(origin == north && destination == east){
    cv_broadcast(ES_cv, ES_lock); 
    cv_broadcast(EW_cv, EW_lock); 
    cv_broadcast(SN_cv, SN_lock); 
    cv_broadcast(SE_cv, SE_lock); 
    cv_broadcast(SW_cv, SW_lock); 
    cv_broadcast(WN_cv, WN_lock); 
    cv_broadcast(WE_cv, WE_lock); 
  }
  else if (origin == north && destination == south){
    cv_broadcast(ES_cv, ES_lock); 
    cv_broadcast(EW_cv, EW_lock); 
    cv_broadcast(SW_cv, SW_lock); 
    cv_broadcast(WN_cv, WN_lock); 
    cv_broadcast(WE_cv, WE_lock); 
    cv_broadcast(WS_cv, WS_lock); 
  }
  else if (origin == north && destination == west){
    cv_broadcast(EW_cv, EW_lock); 
    cv_broadcast(SW_cv, SW_lock); 
  }
  else if (origin == east && destination == south){
    cv_broadcast(SN_cv, SN_lock); 
    cv_broadcast(SW_cv, SW_lock); 
    cv_broadcast(WS_cv, WS_lock); 
    cv_broadcast(WN_cv, WN_lock); 
    cv_broadcast(WE_cv, WE_lock); 
    cv_broadcast(NE_cv, NE_lock); 
    cv_broadcast(NS_cv, NS_lock);
  }
  else if (origin == east && destination == west){
    cv_broadcast(SN_cv, SN_lock); 
    cv_broadcast(SW_cv, SW_lock); 
    cv_broadcast(WN_cv, WN_lock); 
    cv_broadcast(NE_cv, NE_lock); 
    cv_broadcast(NS_cv, NS_lock); 
    cv_broadcast(NW_cv, NW_lock); 
  }
  else if (origin == east && destination == north){
    cv_broadcast(SN_cv, SN_lock); 
    cv_broadcast(WN_cv, WN_lock); 
  }
  else if (origin == south && destination == west){
    cv_broadcast(WN_cv, WN_lock); 
    cv_broadcast(WE_cv, WE_lock); 
    cv_broadcast(NE_cv, NE_lock); 
    cv_broadcast(NS_cv, NS_lock); 
    cv_broadcast(NW_cv, NW_lock); 
    cv_broadcast(ES_cv, ES_lock); 
    cv_broadcast(EW_cv, EW_lock); 
    
  }
  else if (origin == south && destination == north){
    cv_broadcast(WN_cv, WN_lock); 
    cv_broadcast(WE_cv, WE_lock); 
    cv_broadcast(NE_cv, NE_lock); 
    cv_broadcast(EN_cv, EN_lock); 
    cv_broadcast(ES_cv, ES_lock); 
    cv_broadcast(EW_cv, EW_lock); 
  }
  else if (origin == south && destination == east){
    cv_broadcast(WE_cv, WE_lock);
    cv_broadcast(NE_cv, NE_lock); 
     
  }
  else if (origin == west && destination == north){
    cv_broadcast(NE_cv, NE_lock); 
    cv_broadcast(NS_cv, NS_lock); 
    cv_broadcast(EN_cv, EN_lock); 
    cv_broadcast(ES_cv, ES_lock); 
    cv_broadcast(EW_cv, EW_lock); 
    cv_broadcast(SN_cv, SN_lock); 
    cv_broadcast(SW_cv, SW_lock); 
  }
  else if (origin == west && destination == east){
    cv_broadcast(NE_cv, NE_lock); 
    cv_broadcast(NS_cv, NS_lock); 
    cv_broadcast(ES_cv, ES_lock); 
    cv_broadcast(SN_cv, SN_lock); 
    cv_broadcast(SE_cv, SE_lock); 
    cv_broadcast(SW_cv, SW_lock); 
  }
  else if (origin == west && destination == south){
    cv_broadcast(NS_cv, NS_lock); 
    cv_broadcast(ES_cv, ES_lock); 
  }
  else{
    panic("traffic_simulation: thread_fork failed: %s\n", strerror(1));
  }
  
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
  struct lock* tmp_lock = cur_lock(origin, destination); 
  struct cv* tmp_cv = cur_cv(origin, destination); 
  lock_acquire(tmp_lock); 
  lock_acquire(intersect_lock);
  while (!validCrossing(origin, destination)){
      lock_release(intersect_lock); 
      cv_wait(tmp_cv, tmp_lock); 
      lock_acquire(intersect_lock); 
  }
  alter_counter(origin, destination, true); 
  lock_release(intersect_lock); 
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
  struct lock* tmp_lock = cur_lock(origin, destination);
  lock_acquire(intersect_lock); 
  alter_counter(origin, destination, false); 
  lock_release(intersect_lock); 
  wake_up_threads(origin, destination);  
  lock_release(tmp_lock); 
}
