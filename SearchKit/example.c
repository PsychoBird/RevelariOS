#include "SearchKit.h"

int main() {
    mach_port_t task;
    pid_t pid;
    printf("PAGE_SIZE = %d\n", PAGE_SIZE);
    printf("SEARCH_MAX = %d\n", SEARCH_MAX);
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
    vm_address_t out[SEARCH_MAX]; //256 by default

    result_t results;

    get_region_size(task, //task obtained by task_for_pid
                    &base, //base addr found by get_region_size (out)
                    &end); //end addr found by get_region_size (out)

    printf("addr range: 0x%lx - 0x%lx\n", base, end);

    //search bytes
    search_t sret;
    printf("Searching for MH_MAGIC_64 (feedfacf) in memory. Remember to put bytes in Little Endian for searching! The input string now becomes MH_CIGAM_64 (cffaedfe)\n");
    sret = search_data(task, // task obtained by task_for_pid
                       false, // isString = false - we're not searching for a string (feedfacf)
                       false, // quitOnFirstResult = false - look for SEARCH_MAX (256 default) results
                       base, // base address found by get_region_size
                       end, // end address found by get_region_size
                       &out, // out array found by search_data (256)
                       &results, // out result_t of found number of results (256 max)
                       "cffaedfe"); // bytes to find - feedfacf (MH_MAGIC_64) in little endian (MH_CIGAM_64)

    if (sret != SEARCH_SUCCESS) {
        printf("Search failed with error - %d\n", sret);
        return -1;
    }

    /*
    EXAMPLE FOR SEARCHING STRING

    sret = search_data(task, // task obtained by task_for_pid
                       true, // isString = true - we're looking for a string (Hello)
                       true, // quitOnFirstResult = true - look for 1 result
                       base, // base address found by get_region_size
                       end, // end address found by get_region_size
                       &out, // out array found by search_data (256)
                       &results, // out result_t of found number of results (256 max)
                       "Hello"); // String to find - Hello

    if (sret != SEARCH_SUCCESS) {
        printf("search failed!\n");
        return -1;
    }
    */

    //print results
    printf("PRINTING MH_MAGIC_64 FOUND IN MEMORY\n");
    for (int i=0; i<results; i++) {
        printf("Result #%d - 0x%lx\n", i, out[i]);
    }

    /*
    Example for writing data
    sret = write_data(task,  // task obtained by task_for_pid
                      true, // isString = true - we're writing a string (ABCDEFG) to out[5]
                      out[5], // out[5] - Address at out[5] - if 0, it will fail
                      "ABCDEFG"); // string to be overwritten at address out[5]

    if (sret != WRITE_SUCCESS) {
        printf("write failed!\n");
    }
    */

    printf("Complete! Read the docs and example.c for extra usage.\n");

    return 0;

}
