#include "searchkit.h"

int main() {
    mach_port_t task;
    pid_t pid;
    //check if root
    if (geteuid() && getuid()) {
        printf("Run RevelariOS as root.\n");
        return -1;
    }

    //get pid to attach
    printf("PID to attach: ");
    scanf("%d", &pid);
    getchar();

    //task_for_pid
    kern_return_t kret = task_for_pid(mach_task_self(), pid, &task);
    if (kret != KERN_SUCCESS) {
        printf("Couldn't obtain task_for_pid(%d)\n", pid);
        printf("Do you have proper entitlements?\n");
        return -1;
    }
    else {
        printf("Obtained task_for_pid(%d)\n", pid); }
    
    vm_address_t base;
    vm_address_t end;
    vm_address_t out;
    
    get_region_size(task, &base, &end);
    printf("addr range: 0x%lx - 0x%lx\n", base, end);
    search_t sret;
    sret = search_data(task, false, base, end, &out, "4142434445");
    if (sret != SEARCH_SUCCESS) {
        printf("search failed!\n");
        return -1;
    }
    sret = write_data(task, true, out, "ABCDEFG");
    if (sret != WRITE_SUCCESS) {
        printf("write failed!\n");
    }
    
    return 0;
    
}
