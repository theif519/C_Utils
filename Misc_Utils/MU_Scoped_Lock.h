#ifndef MU_SCOPED_LOCK_H
#define MU_SCOPED_LOCK_H

typedef struct scoped_lock
{
   // The instance of lock.
   void *instance;
   // For when we first enter the scope.
   void (*unlock)(void *);
   // For when we exit the scope
   void (*lock)(void *);
   // For when the user frees this, the lock gets freed too.
   void (*free)(void *);
} MU_Scoped_Lock_t;

/**
* Called to automatically unlock the passed MU_Scoped_Lock_t instance
* once it leaves the scope. This function gets called by the GCC or
* Clang compiler attribute.
*/
void auto_unlock(MU_Scoped_Lock_t **lock);

/*
   Note how we create a temporary variable to point to the
   passed lock. This is because it needs to be defined
   inside of the scope in order to have the cleanup function be
   called.
*/
#define SCOPED_LOCK(s_lock) { \
   scoped_lock *tmp_lock __attribute__ ((__cleanup__(scoped_lock))) = s_lock; \
   tmp_lock->lock(tmp_lock->instance); \

#define END_SCOPE }

#endif /* MU_SCOPED_LOCK_H */