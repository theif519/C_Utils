#ifndef MU_SCOPED_LOCK_H
#define MU_SCOPED_LOCK_H

#include <pthread.h>
#include <stdbool.h>
#include "../io/logger.h"

struct c_utils_scoped_lock_log_info {
   const char *line;
   const char *file;
   const char *function;
};

struct c_utils_scoped_lock
{
   // The instance of lock.
   void *lock;
   // Logger used to log debug information and errors.
   struct c_utils_logger *logger;
   // Log information for when something goes wrong.
   struct c_utils_scoped_lock_log_info log_info;
   // For normal locks, I.E Mutex and Spinlock
   void *(*acquire0)(struct c_utils_scoped_lock *, struct c_utils_scoped_lock_log_info);
   // For locks with types of locking mechanisms, I.E RWLocks
   void *(*acquire1)(struct c_utils_scoped_lock *, struct c_utils_scoped_lock_log_info);
   // For when we exit the scope
   void (*release)(struct c_utils_scoped_lock *);
   // For when the user frees this, the lock gets freed too.
   void (*dispose)(struct c_utils_scoped_lock *);
};

#ifdef NO_C_UTILS_PREFIX
/*
   Typedefs
*/
typedef struct c_utils_scoped_lock scoped_lock_t;

/*
   Macros
*/
#define SCOPED_LOCK_FROM(...) C_UTILS_SCOPED_LOCK_FROM(__VA_ARGS__)
#define SCOPED_LOCK(...) C_UTILS_SCOPED_LOCK(__VA_ARGS__)
#define SCOPED_LOCK_RDLOCK(...) C_UTILS_SCOPED_RDLOCK(__VA_ARGS__)
#define SCOPED_LOCK_WRLOCK(...) C_UTILS_SCOPED_WRLOCK(__VA_ARGS__)
#define UNACCESSIBLE C_UTILS_UNACCESSIBLE

/*
   Functions
*/
#define scoped_lock_mutex(...) c_utils_scoped_lock_mutex(__VA_ARGS__)
#define scoped_lock_mutex_from(...) c_utils_scoped_lock_mutex_from(__VA_ARGS__)
#define scoped_lock_spinlock(...) c_utils_scoped_lock_spinlock(__VA_ARGS__)
#define scoped_lock_spinlock_from(...) c_utils_scoped_lock_spinlock_from(__VA_ARGS__)
#define scoped_lock_rwlock(...) c_utils_scoped_lock_rwlock(__VA_ARGS__)
#define scoped_lock_rwlock_from(...) c_utils_scoped_lock_rwlock_from(__VA_ARGS__)
#define scoped_lock_no_op(...) c_utils_scoped_lock_no_op(__VA_ARGS__)
#endif


struct c_utils_scoped_lock *c_utils_scoped_lock_mutex_from(pthread_mutex_t *lock, struct c_utils_logger *logger);

struct c_utils_scoped_lock *c_utils_scoped_lock_mutex(pthread_mutexattr_t *attr, struct c_utils_logger *logger);

struct c_utils_scoped_lock *c_utils_scoped_lock_spinlock_from(pthread_spinlock_t *lock, struct c_utils_logger *logger);

struct c_utils_scoped_lock *c_utils_scoped_lock_spinlock(int flags, struct c_utils_logger *logger);

struct c_utils_scoped_lock *c_utils_scoped_lock_rwlock_from(pthread_rwlock_t *lock, struct c_utils_logger *logger);

struct c_utils_scoped_lock *c_utils_scoped_lock_rwlock(pthread_rwlockattr_t *attr, struct c_utils_logger *logger);

struct c_utils_scoped_lock *c_utils_scoped_lock_no_op();

/**
* Called to automatically unlock the passed c_utils_scoped_lock instance
* once it leaves the scope. This function gets called by the GCC or
* Clang compiler attribute.
*/
void c_utils_auto_unlock(struct c_utils_scoped_lock **s_lock);


// TODO: Implement!
void c_utils_scoped_lock_destroy(struct c_utils_scoped_lock *lock);

#define C_UTILS_SCOPED_LOCK_FROM(lock, logger) _Generic((lock), \
      pthread_mutex_t *: c_utils_scoped_lock_mutex_from, \
      pthread_spinlock_t *: c_utils_scoped_lock_spinlock_from, \
      pthread_rwlock_t *: c_utils_scoped_lock_rwlock_from)(lock, logger)

#define C_UTILS_SCOPE_AUTO_UNLOCK __attribute__ ((__cleanup__(c_utils_auto_unlock)))

/*
   Note how we create a temporary variable to point to the
   passed lock. This is because it needs to be defined
   inside of the scope in order to have the cleanup function be
   called.
*/ 
#define _C_UTILS_SCOPED_LOCK(s_lock, n) \
   for (struct c_utils_scoped_lock *tmp_lock C_UTILS_SCOPE_AUTO_UNLOCK = s_lock, *_test = tmp_lock->acquire ##n (tmp_lock, \
         (struct c_utils_scoped_lock_log_info) { .line = C_UTILS_STRINGIFY(__LINE__), .file = __FILE__, .function = __FUNCTION__ }); \
         _test; _test = NULL)

#define C_UTILS_SCOPED_LOCK0(s_lock) _C_UTILS_SCOPED_LOCK(s_lock, 0)

#define C_UTILS_SCOPED_LOCK1(s_lock) _C_UTILS_SCOPED_LOCK(s_lock, 1)

#define C_UTILS_SCOPED_LOCK(s_lock) C_UTILS_SCOPED_LOCK0(s_lock)

#define C_UTILS_SCOPED_RDLOCK(s_lock) C_UTILS_SCOPED_LOCK1(s_lock)

#define C_UTILS_SCOPED_WRLOCK(s_lock) C_UTILS_SCOPED_LOCK0(s_lock)

#define C_UTILS_UNACCESSIBLE do { __builtin_unreachable(); assert(0 && "Reach unreachable block!"); } while(0)

#endif /* MU_SCOPED_LOCK_H */