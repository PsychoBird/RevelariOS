#include <stdio.h>
#include <mach/mach.h>
#include <unistd.h>
#include <mach-o/dyld.h>

typedef int search_t;
typedef unsigned char byte_t;
typedef uint8_t result_t;

#define SEARCH_SUCCESS 0
#define SEARCH_FAILURE 1
#define BYTES_UNEVEN 2
#define DATA_TOO_LARGE 3
#define WRITE_SUCCESS 4
#define WRITE_FAILURE 5
#define WRITE_BAD_ADDRESS 6

#define SEARCH_MAX ((result_t) (~0 >> 1)) + 1
#define PAGE_SIZE getpagesize()
#define MAX_INPUT_DATA 100


kern_return_t get_region_size(mach_port_t task, vm_address_t *baseaddr, vm_address_t *endaddr) {
    vm_address_t addr = 0;
    vm_size_t size = 0;
    vm_region_flavor_t flavor = VM_REGION_BASIC_INFO_64;
    vm_region_basic_info_data_64_t info;
    mach_msg_type_number_t count = VM_REGION_BASIC_INFO_COUNT_64;
    mach_port_t object = 0;
    kern_return_t kret;
    int id = 0;
    bool found = false;

    while (1) {
        addr += size;
        kret = vm_region_64(task, &addr, &size, flavor, (vm_region_info_64_t) &info, &count, &object);

        if (kret != KERN_SUCCESS) {
            break; }
        else if (id < 3 && kret != KERN_SUCCESS) {
            return KERN_FAILURE; }
        id++;

        if (addr > 0 && !found) {
            found = true;
            *baseaddr = addr;
        }
    }

    *endaddr = addr;
    return KERN_SUCCESS;
}


kern_return_t search_data(mach_port_t task, bool isString, bool quitOnFirstResult, vm_address_t baseaddr, vm_address_t endaddr, vm_address_t *outaddr[SEARCH_MAX], result_t *resultnum, char in[MAX_INPUT_DATA]) {
    if (strlen(in) > MAX_INPUT_DATA) {
        return DATA_TOO_LARGE;
    }
    int pgsz = getpagesize();

    size_t bytes = pgsz;
    byte_t readOut[pgsz];
    kern_return_t kret;
    int accuracy = 0;
    uint8_t cmpbyte[100];
    int scannum;

    if (!isString) {
        scannum = strlen(in)/2;
        if (strlen(in) % 2 != 0) {
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
        scannum = strlen(in);
        for (int i=0; i<scannum; i++) {
            cmpbyte[i] = (uint8_t) in[i];
        }
    }

    result_t foundtotal = 0;
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
                        if (quitOnFirstResult) {
                            *resultnum = foundtotal;
                            *outaddr = baseaddr+i;
                            return SEARCH_SUCCESS;
                        }
                        else {
                            *(outaddr + foundtotal) = baseaddr+i;
                            foundtotal++;
                            if (foundtotal == SEARCH_MAX-1) {
                                *resultnum = foundtotal;
                                return SEARCH_SUCCESS;
                            }
                        }
                    }
                }
            }
        }
    }

    if (foundtotal == 0) {
        *resultnum = 0;
        *outaddr = 0;
        return SEARCH_FAILURE;
    }
    return SEARCH_SUCCESS;
}

search_t write_data(mach_port_t task, bool isString, vm_address_t addr, char in[100]) {
    if (addr == 0x0) {
        return WRITE_BAD_ADDRESS;
    }
    if (strlen(in) > 100) {
        return DATA_TOO_LARGE;
    }

    size_t bytes = strlen(in);
    kern_return_t kret;
    int scannum;

    if (!isString) {
        byte_t writebyte[strlen(in)/2];
        scannum = strlen(in)/2;
        if (strlen(in) % 2 != 0) {
            return BYTES_UNEVEN;
        }
        char tocmpbyte[50][2];
        int numin = 0;
        for (int i=0; i<scannum; i++) {
            tocmpbyte[i][0] = in[numin];
            tocmpbyte[i][1] = in[numin+1];
            numin += 2;
            writebyte[i] = (uint8_t) strtol(tocmpbyte[i], NULL, 16);
        }
        kret = vm_write(task, addr, &writebyte, sizeof(writebyte));
    }
    else {
        byte_t writebyte[strlen(in)];
        scannum = strlen(in);
        for (int i=0; i<scannum; i++) {
            writebyte[i] = (uint8_t) in[i];
        }
        kret = vm_write(task, addr, &writebyte, sizeof(writebyte));
    }


    if (kret == KERN_SUCCESS) {
        return WRITE_SUCCESS;
    }

    return WRITE_FAILURE;
}
