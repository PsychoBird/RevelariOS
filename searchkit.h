#include <stdio.h>
#include <mach/mach.h>
#include <unistd.h>
#include <sys/types.h>
#include <mach-o/dyld.h>
#include <string.h>
#include <stdlib.h>
#include <mach/vm_map.h>
#include <stdlib.h>
#include <sys/sysctl.h>
#include <pwd.h>
#include <sys/utsname.h>

#define SEARCH_SUCCESS 0
#define SEARCH_FAILURE 1
#define BYTES_UNEVEN 2

typedef int search_t;
typedef unsigned char byte_t;

kern_return_t get_region_size(mach_port_t task, vm_address_t *baseaddr, vm_address_t *endaddr) {
    vm_address_t addr = 0;
    vm_size_t size = 0;
    vm_region_flavor_t flavor = VM_REGION_BASIC_INFO_64;
    vm_region_basic_info_data_64_t info;
    mach_msg_type_number_t count = VM_REGION_BASIC_INFO_COUNT_64;
    mach_port_t object = 0;
    kern_return_t kret;
    int id = 0;
    
    while (1) {
        addr += size;
        kret = vm_region_64(task, &addr, &size, flavor, (vm_region_info_64_t) &info, &count, &object);
        
        if (kret != KERN_SUCCESS) {
            break; }
        else if (id < 3 && kret != KERN_SUCCESS) {
            return KERN_FAILURE; }
        id++;
        
        if (id == 3) {
            *baseaddr = addr;
        }
    }
    
    *endaddr = addr;
    return KERN_SUCCESS;
}

search_t search_data(mach_port_t task, bool isString, vm_address_t baseaddr, vm_address_t endaddr, vm_address_t *outaddr, char in[100]) {
    int pgsz = getpagesize();
    
    size_t bytes = pgsz;
    byte_t readOut[pgsz];
    kern_return_t kret;
    int accuracy = 0;
    uint8_t cmpbyte[100];
    int scannum;
    
    if (!isString) {
        scannum = (strlen(in)-1)/2;
        if ((strlen(in)-1) % 2 != 0) {
            return BYTES_UNEVEN;
        }
        char tocmpbyte[50][2];
        int numin = 0;
        for (int i=0; i<scannum; i++) {
            tocmpbyte[i][0] = in[numin];
            tocmpbyte[i][1] = in[numin+1];
            numin += 2;
            cmpbyte[i] = (uint8_t) strtol(tocmpbyte[i], NULL, 16);
        }
    }
    else {
        scannum = strlen(in)-1;
        for (int i=0; i<scannum; i++) {
            cmpbyte[i] = (uint8_t) in[i];
        }
    }
    
    for (; baseaddr < endaddr; baseaddr+=pgsz) {
        kret = vm_read_overwrite(task, baseaddr, bytes, (vm_offset_t) &readOut, &bytes);
        int i;
        for (i=0; i < pgsz; i++) {
            if (kret != KERN_SUCCESS) {
                break;
            }
            accuracy = 0;
            if (cmpbyte[0] == readOut[i]) {
                accuracy++;
                for (int j=(i+0x1); j<pgsz; j++) {
                    if (cmpbyte[accuracy] == (uint8_t) readOut[j]) {
                        accuracy++;
                    }
                    else {
                        break;
                    }
                    if (accuracy == scannum) {
                        *outaddr = baseaddr+i;
                        return SEARCH_SUCCESS;
                    }
                }
            }
        }
    }
    
    *outaddr = 0;
    return SEARCH_FAILURE;
}
