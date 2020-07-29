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

#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[00m"

#define GOOD "\033[32m # " WHITE
#define ERROR "\033[31m # " WHITE

#define EXIT printf(ERROR"Exiting RevelariOS...\n"); exit(0);

typedef unsigned char byte_t;

kern_return_t get_region_size(mach_port_t task,
                              vm_address_t *baseaddr,
                              vm_address_t *endaddr);


kern_return_t read_lines(mach_port_t task,
                         vm_address_t addr,
                         int lines,
                         bool printchar);


kern_return_t write_data(mach_port_t task,
                         bool isString,
                         vm_address_t addr,
                         char in[100]);


kern_return_t search_data(mach_port_t task,
                          bool isString,
                          vm_address_t baseaddr,
                          vm_address_t end,
                          vm_address_t *outaddr,
                          char in[100]);
