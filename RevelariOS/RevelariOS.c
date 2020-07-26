#include "mem.h"

void interact(pid_t pid, mach_port_t task) {
    char input[128];
    char args[10][30];
    
    vm_address_t base;
    vm_address_t end;
    vm_address_t out = 0;
    base = end = 0;
    char in[100];
    
    printf(GOOD"To list RevelariOS commands, type 'help'\n");
    
    while (1) {
    
        memset(args, 0, sizeof(args[0][0])*10*30);
    
        printf(CYAN ">> " WHITE);
        fgets(input, 128, stdin);
    
        int charIndex = 0;
        int argIndex = 0;
        int argCharIndex = 0;
    
        input[strlen(input)] = ' ';
    
        for (int i = 0; i < strlen(input); i++) {
            if (input[i] == ' ') {
                argCharIndex = 0;
                for (; charIndex < i; charIndex++) {
                    args[argIndex][argCharIndex] = input[charIndex];
                    argCharIndex++;
                }
                //printf("Argument #%d = %s\n", argIndex, args[argIndex]); //DEBUG
                argIndex++;
                charIndex++;
            }
        }
        
        //HELP
        if (strcmp(args[0], "help\n") == 0) {
            printf(GOOD"List of commands:\n"
                   MAGENTA "search (s) " GREEN "[bytes] " WHITE "- search " GREEN"[bytes] " WHITE "(ex: s 434231)\n"
                   MAGENTA "searchstr (ss) " GREEN "[string] " WHITE "- search " GREEN"[string] " WHITE "(ex: ss CBA)\n"
                   MAGENTA "readmem (rm) " GREEN "[lines] " WHITE "- read " GREEN "[lines] " WHITE "of last search's address\n"
                   MAGENTA "readmemstr (rms) " GREEN "[lines] " WHITE "- read " GREEN "[lines] " WHITE "of last search's address as a string\n"
                   MAGENTA "pid " GREEN "[pid] " WHITE "- changes pid to new [pid]\n"
                   MAGENTA "pause (p) " WHITE "- pauses task\n"
                   MAGENTA "resume (re) " WHITE "- resumes task\n"
                   MAGENTA "quit (q) " WHITE "- exits RevelariOS\n"); }
    
        
        //EXIT
        else if (strcmp(args[0], "exit\n") == 0 || strcmp(args[0], "e\n") == 0) {
            EXIT }
        else if (strcmp(args[0], "quit\n") == 0 || strcmp(args[0], "q\n") == 0) {
            EXIT }
        
        else if (strcmp(args[0], "pid\n") == 0) {
            printf(GREEN "%d" WHITE "\n", pid); }
        else if (strcmp(args[0], "pid") == 0 && args[1][0] != '\0') {
            pid = (int) strtol(args[1], NULL, 0);
            kern_return_t changetfp = task_for_pid(mach_task_self(), pid, &task);
            if (changetfp == KERN_SUCCESS) {
                printf(GOOD"Changed pid to %d\n", pid);
            }
            else {
                printf(ERROR"Could not change PID\n");
            }
        }
        
        //read_lines (2 args)
        else if ((strcmp(args[0], "readmem") == 0 || strcmp(args[0], "rm") == 0) && args[1][0] != '\0') {
            read_lines(task, out, (int) strtol(args[1], NULL, 0), false); }
        else if (strcmp(args[0], "readmem\n") == 0 || strcmp(args[0], "rm\n") == 0) {
            printf(ERROR"Not enough arguments for readmem!\n"); }
        
        //read_lines (2 args)
        else if ((strcmp(args[0], "readmemstr") == 0 || strcmp(args[0], "rms") == 0) && args[1][0] != '\0') {
            read_lines(task, out, (int) strtol(args[1], NULL, 0), true); }
        else if (strcmp(args[0], "readmemstr\n") == 0 || strcmp(args[0], "rms\n") == 0) {
            printf(ERROR"Not enough arguments for readmemstr!\n"); }
        
        //search_read (2 args)
        else if ((strcmp(args[0], "search") == 0 || strcmp(args[0], "s") == 0) && args[1][0] != '\0') {
            get_region_size(task, &base, &end);
            printf(GOOD"Memory Range = 0x%lx - 0x%lx\n", base, end);
            printf(GOOD"Finding bytes - %s\n", args[1]);
            search_data(task, false, base, end, &out, args[1]);
        }
        else if (strcmp(args[0], "search\n") == 0 || strcmp(args[0], "s\n") == 0) {
            printf(ERROR"Not enough arguments for search!\n"); }
        
        
        //search_read (2 args)
        else if ((strcmp(args[0], "searchstr") == 0 || strcmp(args[0], "ss") == 0) && args[1][0] != '\0') {
            get_region_size(task, &base, &end);
            printf(GOOD"Memory Range = 0x%lx - 0x%lx\n", base, end);
            printf(GOOD"Finding string - %s\n", args[1]);
            search_data(task, true, base, end, &out, args[1]);
        }
        else if (strcmp(args[0], "searchstr\n") == 0 || strcmp(args[0], "ss\n") == 0) {
            printf(ERROR"Not enough arguments for searchstr!\n"); }
        
        else if (strcmp(args[0], "pause\n") == 0 || strcmp(args[0], "p\n") == 0) {
            printf(GOOD"Suspending task...\n");
            task_suspend(task);
        }
        else if (strcmp(args[0], "resume\n") == 0 || strcmp(args[0], "r\n") == 0) {
            printf(GOOD"Resuming task...\n");
            task_resume(task);
        }
        else if (strcmp(args[0], "\n") == 0) {
            ;
        }
    

        //UNKNOWN COMMAND
        else { printf(GOOD"Invalid Command\n"); }
    }
}


int main() {
    pid_t pid;
    mach_port_t task;
    
    printf(YELLOW" # Welcome to RevelariOS!\n");

    //check if root
    if (geteuid() && getuid()) {
        printf(ERROR"Run RevelariOS as root.\n");
        EXIT
    }

    //get pid to attach
    printf(GOOD"PID to attach: ");
    scanf("%d", &pid);
    getchar();

    //task_for_pid
    kern_return_t kret = task_for_pid(mach_task_self(), pid, &task);
    if (kret != KERN_SUCCESS) {
        printf(ERROR"Couldn't obtain task_for_pid(%d)\n", pid);
        printf(ERROR"Do you have proper entitlements?\n");
        EXIT
    }
    else {
        printf(GOOD"Obtained task_for_pid(%d)\n", pid); }
    
    interact(pid, task);
    return 0;
}
