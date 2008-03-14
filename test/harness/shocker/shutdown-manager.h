

#ifndef __SHUTDOWN_MANAGER_H__
#define __SHUTDOWN_MANAGER_H__

void shutdown_manager_init ();
void shutdown_manager_shutdown ();

void shutdown_manager_wait_threads_increment ();
void shutdown_manager_wait_threads_decrement ();
void shutdown_manager_wait_for_threads ();
void shutdown_manager_queue_shutdown ();

#endif  // __SHUTDOWN_MANAGER_H__

