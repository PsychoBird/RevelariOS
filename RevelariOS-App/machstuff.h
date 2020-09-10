#ifndef machstuff_h
#define machstuff_h

#include "SearchKit.h"

mach_port_t get_tfp(int pid) {
    mach_port_t task;
    kern_return_t kret;
    kret = task_for_pid(mach_task_self(), pid, &task);
    if (kret != KERN_SUCCESS) {
        return MACH_PORT_NULL;
    }
    return task;
}

#endif /* machstuff_h */
