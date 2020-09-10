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
                    if ((int) readOut[i] >= 33 && (int) readOut[i] <= 126) {
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

search_t write_data(mach_port_t task, bool isString, vm_address_t addr, char in[MAX_INPUT_DATA]) {
    if (addr == 0x0) {
        return WRITE_BAD_ADDRESS;
    }
    if (strlen(in) > MAX_INPUT_DATA) {
        printf(ERROR"Data in is too large! (> 100)\n");
        return DATA_TOO_LARGE;
    }

    in[strlen(in)-1] = '\0';
    size_t bytes = strlen(in);
    kern_return_t kret;
    int scannum;

    if (!isString) {
        byte_t writebyte[strlen(in)/2];
        scannum = strlen(in)/2;
        if (strlen(in) % 2 != 0) {
            printf(ERROR"Enter an even number of bytes!\n");
            return BYTES_UNEVEN;
        }
        char tocmpbyte[MAX_INPUT_DATA / 2][2];
        int numin = 0;
        for (int i=0; i<scannum; i++) {
            tocmpbyte[i][0] = in[numin];
            tocmpbyte[i][1] = in[numin+1];
            numin += 2;
            writebyte[i] = (uint8_t) strtol(tocmpbyte[i], NULL, 16);
        }
        printf(GOOD"Writing %s to 0x%lx\n", in, addr);
        kret = vm_write(task, addr, &writebyte, sizeof(writebyte));
    }
    else {
        byte_t writebyte[strlen(in)];
        scannum = strlen(in);
        for (int i=0; i<scannum; i++) {
            writebyte[i] = (uint8_t) in[i];
        }
        printf(GOOD"Writing %s to 0x%lx\n", in, addr);
        kret = vm_write(task, addr, &writebyte, sizeof(writebyte));
    }


    if (kret == KERN_SUCCESS) {
        printf(GOOD"Successfully wrote data!\n");
        return WRITE_SUCCESS;
    }
    return WRITE_FAILURE;
}

search_t search_data(mach_port_t task, bool isString, bool quitOnFirstResult, vm_address_t baseaddr, vm_address_t endaddr, vm_address_t *outaddr[SEARCH_MAX], result_t *resultnum, char in[MAX_INPUT_DATA]) {
    if (strlen(in) > MAX_INPUT_DATA) {
        printf(ERROR"Data in is too large! (> %d)\n", MAX_INPUT_DATA);
        return DATA_TOO_LARGE;
    }

    size_t bytes = READ_PAGE_SIZE;
    byte_t readOut[READ_PAGE_SIZE];
    kern_return_t kret;
    int accuracy = 0;
    uint8_t cmpbyte[MAX_INPUT_DATA];
    int scannum;

    if (!isString) {
        scannum = (strlen(in)-1)/2;
        if ((strlen(in)-1) % 2 != 0) {
            printf(ERROR"Enter an even number of bytes!\n");
            return BYTES_UNEVEN;
        }
        char tocmpbyte[MAX_INPUT_DATA / 2][2];
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

    result_t foundtotal = 0;
    for (; baseaddr < endaddr; baseaddr+=READ_PAGE_SIZE) {
        kret = vm_read_overwrite(task, baseaddr, bytes, (vm_offset_t) &readOut, &bytes);
        int i;
        for (i=0; i < READ_PAGE_SIZE; i++) {
            if (kret != KERN_SUCCESS) {
                break;
            }
            accuracy = 0;
            if (cmpbyte[0] == readOut[i]) {
                accuracy++;
                for (int j=(i+0x1); j<READ_PAGE_SIZE; j++) {
                    if (cmpbyte[accuracy] == (uint8_t) readOut[j]) {
                        accuracy++;
                    }
                    else {
                        break;
                    }
                    if (accuracy == scannum) {
                        if (quitOnFirstResult) {
                            *resultnum = 1;
                            *outaddr = baseaddr+i;
                            printf(GOOD"Found result at 0x%lx!\n", baseaddr+i);
                            return SEARCH_SUCCESS;
                        }
                        else {
                            *(outaddr + foundtotal) = baseaddr+i;
                            foundtotal++;
                            *resultnum = foundtotal;
                            if (foundtotal == SEARCH_MAX - 1) {
                                printf(ERROR"Found max number of supported results!\n");
                                *resultnum = foundtotal;
                                return SEARCH_SUCCESS;
                            }
                        }
                    }
                }
            }
        }
    }


    if (quitOnFirstResult) {
        printf(ERROR"Could not find data!\n");
        *resultnum = 0;
        *outaddr = 0;
        return SEARCH_FAILURE;
    }
    if (foundtotal > 0) {
        printf(GOOD"Search complete! RevelariOS found %d results\n", foundtotal);
    }
    else {
        printf(ERROR"Could not find data!\n");
        *resultnum = 0;
        *outaddr = 0;
        return SEARCH_FAILURE;
    }
    return SEARCH_SUCCESS;
}
