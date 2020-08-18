#include <stdio.h>
#include <mach/mach.h>
#include <unistd.h>
#include <mach-o/dyld.h>

typedef int search_t;
typedef unsigned char byte_t;
typedef uint8_t result_t;

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

#define SEARCH_SUCCESS 0
#define SEARCH_FAILURE 1
#define BYTES_UNEVEN 2
#define DATA_TOO_LARGE 3
#define WRITE_SUCCESS 4
#define WRITE_FAILURE 5
#define WRITE_BAD_ADDRESS 6

#define SEARCH_MAX ((result_t) (~0 >> 1)) + 1
#define READ_PAGE_SIZE getpagesize()
#define MAX_INPUT_DATA 100


kern_return_t get_region_size(mach_port_t task,
                              vm_address_t *baseaddr,
                              vm_address_t *endaddr);


kern_return_t read_lines(mach_port_t task,
                         vm_address_t addr,
                         int lines,
                         bool printchar);


search_t write_data(mach_port_t task,
                         bool isString,
                         vm_address_t addr,
                         char in[MAX_INPUT_DATA]);


search_t search_data(mach_port_t task,
                          bool isString,
                          bool quitOnFirstResult,
                          vm_address_t baseaddr,
                          vm_address_t endaddr,
                          vm_address_t *outaddr[SEARCH_MAX],
                          result_t *resultnum,
                          char in[MAX_INPUT_DATA]);
