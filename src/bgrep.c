#include "bgrep.h"
#include <setjmp.h>

static sigjmp_buf sigbus_jmp;
static char *current_file = NULL;

void sigbus_handler(int sig) {
    siglongjmp(sigbus_jmp, 1);
}

int main(int argc, char* argv[]) {
    //this is the case where there is no flags, only the default

    bool pattern_flag = false;
    bool context_flag = false;
    char *pattern = NULL;
    int context_bytes = 0;

    char **file_arr = malloc(BUFSIZ * sizeof(char*)); // can store up to BUFSIZ filenames

    int opt;
    while ((opt = getopt(argc, argv, "c:p:")) != -1) {
        switch (opt) {
            case 'c':
                context_flag = true;
                if((context_bytes = atoi(optarg)) == 0) {
                    fprintf(stderr, "Could not convert optarg %s to integer. %s\n", optarg, strerror(errno));                    
                    return -1;
                }
                break;
            case 'p':
                pattern_flag = true;
                pattern = optarg;
                break;
            case '?':
                return -1;
        }
    }

    if (!pattern_flag) {
        pattern = argv[optind++];
    }
    
    int file_count = 0;
    while ((argc - optind) > 0) {
        file_arr[file_count++] = argv[optind++]; 
    }

    int bgrep_return_value = bgrep(pattern_flag, context_flag, pattern, file_arr, file_count, context_bytes);

    free(file_arr);

    if(bgrep_return_value != 0) {
        // error occured in bgrep function call
        return bgrep_return_value;
    }
    return 0;

}


int bgrep(bool pattern_flag, bool context_flag, char *pattern, char **file_arr, int file_count, int context_bytes) {

    bool match_flag = false;
    bool sigbus_flag = false; // set if sigbus is detected which means file might have changed size while still being mmapped

    int pattern_length = 0;
    
    struct sigaction sa;
    sa.sa_handler = sigbus_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    
    if (sigaction(SIGBUS, &sa, NULL) == -1) {
        fprintf(stderr, "Failed sigaction for SIGBUS handling. %s\n", strerror(errno));
        return -1;
    }
    
    if(pattern_flag == true) {
        int pattern_fd = open(pattern, O_RDONLY);
        
        if(pattern_fd == -1) {
            fprintf(stderr, "Could not open pattern file: %s. %s\n", pattern, strerror(errno));
            return -1;
        }

        pattern_length = lseek(pattern_fd, 0, SEEK_END);
        
        pattern = mmap(NULL, pattern_length, PROT_READ, MAP_PRIVATE, pattern_fd, 0);

        if(close(pattern_fd) == -1) {
            fprintf(stderr, "Could not close pattern file. %s\n", strerror(errno));
            return -1;
        }

        if(pattern == MAP_FAILED) {
            fprintf(stderr, "Failed to map pattern files. %s\n", strerror(errno));
            return -1;
        }
    }

    else {
        pattern_length = strlen(pattern);
    }



    for(int i = 0; i < file_count; i++) {

        int file_fd = open(file_arr[i], O_RDONLY);

        if(file_fd == -1) {
            fprintf(stderr, "Could not open file: %s. %s\n", file_arr[i], strerror(errno));
            return -1;
        }

        int mapped_file_length = lseek(file_fd,0, SEEK_END);

        char *mapped_file = mmap(NULL, mapped_file_length, PROT_READ, MAP_PRIVATE, file_fd, 0);

        if(close(file_fd) == -1) {
            fprintf(stderr, "Could not close file: %s. %s\n", file_arr[i], strerror(errno));
            return -1;
        }

        if(mapped_file == MAP_FAILED) {
            fprintf(stderr, "Failed to map file: %s. %s\n", file_arr[i], strerror(errno));
            return -1;
        }
        
        //from this point on now i have everything ready

        current_file = file_arr[i];
        
        if (sigsetjmp(sigbus_jmp, 1) != 0) {
            // if sigbus detected, go to the next file
            fprintf(stderr, "Got a SIGBUS while mmapping file: %s\n", current_file);
            munmap(mapped_file, mapped_file_length);
            sigbus_flag = true;
            continue;
        }
        
        for(int j = 0; j < mapped_file_length; j++) {

            int bit_count = 0;

            if(pattern[0] == mapped_file[j]) {
                for(int k = 0; k < pattern_length; k++) {
                    if(pattern[k] == mapped_file[k+j]) {
                        bit_count++;
                    }
                }
            }

            if(bit_count == pattern_length) {
            
                fprintf(stdout, "%s:%d", file_arr[i], j);

                if(context_flag == true) {
                    for(int k = j-context_bytes; k < j+pattern_length+context_bytes-1; k++) {
                        if(k < 0 || k >= mapped_file_length) {
                            continue;
                        }

                        fprintf(stdout, " %c", mapped_file[k]);
                    }

                    for(int k = j-context_bytes; k < j+pattern_length+context_bytes-1; k++) {
                        if(k < 0 || k >= mapped_file_length) {
                            continue;
                        }

                        fprintf(stdout, " %X", mapped_file[k]);
                    }
                }

                fprintf(stdout, "\n");

                match_flag = true;
            }

        
        }

        if(munmap(mapped_file, mapped_file_length) == -1) {
            fprintf(stderr, "Failed to munmap a mapped file. %s\n", strerror(errno));
            return -1;
        }

    }

    if(pattern_flag == true) { //this part not really necessary because the process will just unmap automatically upon exiting
        if(munmap(pattern, pattern_length) == -1) {
            fprintf(stderr, "Failed to munmap pattern file. %s\n", strerror(errno));
            return -1;
        }
    }

    if (sigbus_flag) {
        return -1;
    }

    if(match_flag == true) {
        return 0;
    }

    fprintf(stdout, "no errors or matches found\n");

    return 1;

}