

#ifndef __SHUTDOWN_MANAGER_H__
#define __SHUTDOWN_MANAGER_H__

class ShockerScriptableControlObject;



void shutdown_manager_init ();
void shutdown_manager_shutdown ();

void shutdown_manager_wait_increment ();
void shutdown_manager_wait_decrement ();
void shutdown_manager_wait ();
void shutdown_manager_queue_shutdown (ShockerScriptableControlObject *shocker);

#endif  // __SHUTDOWN_MANAGER_H__

