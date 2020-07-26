#include "mem.h"

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

kern_return_t read_lines(mach_port_t task, vm_address_t addr, int lines, bool printchar) {
    if (addr == 0) {
        printf(ERROR"Read address is 0x0. Did you do a scan yet?\n");
        return KERN_FAILURE;
    }
    
    int sub = addr % 16;
    vm_address_t zeroaddr = addr - sub;
    kern_return_t kret;
    
    printf(GOOD"Reading %d lines of memory from address: 0x%lx\n", lines, addr);
    printf(GREEN "0x%lx " WHITE "| ", addr);
    if (!printchar) {
        printf(YELLOW "00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F "WHITE"|\n");
    }
    else {
        printf(YELLOW "0 1 2 3 4 5 6 7 8 9 A B C D E F "WHITE"|\n");
    }
    
    
    byte_t readOut[16];
    vm_size_t bytes = 16;
    
    for (int j = 0; j < lines; j++) {
        printf(CYAN "0x%lx " WHITE "| ", zeroaddr);
        kret = vm_read_overwrite(task, zeroaddr, bytes, (vm_offset_t) &readOut, &bytes);
        if (kret == KERN_SUCCESS) {
            for (int i=0; i<=0xf; i++) {
                if (!printchar) {
                    if (readOut[i] <= 0xf) {
                        printf("0%x ", readOut[i]);
                    } else {
                        printf("%lx ", readOut[i]);
                    }
                }
                else {
                    if ((int) readOut[i] >= 41 && (int) readOut[i] <= 126) {
                        printf("%c ", readOut[i]);
                    } else {
                        printf(RED"? "WHITE);
                    }
                }
            }
            zeroaddr += 0x10;
        }
        else {
            printf(ERROR"\nvm_read_overwrite failed: %s\n", mach_error_string(kret));
            return KERN_FAILURE;
        }
        printf(WHITE "|\n");
    }
    return KERN_SUCCESS;
}


kern_return_t search_data(mach_port_t task, bool isString, vm_address_t baseaddr, vm_address_t end, vm_address_t *outaddr, char in[100]) {
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
            printf(ERROR"Enter an even number of bytes!\n");
        }
        char tocmpbyte[50][2];
        int numin = 0;
        for (int i=0; i<scannum; i++) {
            tocmpbyte[i][0] = in[numin];
            tocmpbyte[i][1] = in[numin+1];
            numin += 2;
            cmpbyte[i] = (uint8_t) strtol(tocmpbyte[i], NULL, 16);
            //printf(GOOD"0x%c%c | %d\n", tocmpbyte[i][0], tocmpbyte[i][1], cmpbyte[i]);
        }
        //printf("\n");
    }
    else {
        scannum = strlen(in)-1;
        for (int i=0; i<scannum; i++) {
            cmpbyte[i] = (uint8_t) in[i];
        }
    }
    
    for (; baseaddr < end; baseaddr+=pgsz) {
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
                        printf(GOOD"Scan found matching string at 0x%lx\n", baseaddr+i);
                        *outaddr = baseaddr+i;
                        return KERN_SUCCESS;
                    }
                }
            }
        }
    }
    
    printf(ERROR"No matching string found!\n");
    *outaddr = 0;
    return KERN_SUCCESS;
}
