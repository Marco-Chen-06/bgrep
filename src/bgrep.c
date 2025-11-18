#include "bgrep.h"

// disclaimer: argument parsing doesn't handle weird syntax cases. 
int main(int argc, char *argv[]) {
    bool pattern_flag = false;
    bool context_flag = false;
    char *pattern = NULL;

    char **file_arr = malloc(BUFSIZ * sizeof(char*)); // can store up to BUFSIZ filenames

    int opt;
    while ((opt = getopt(argc, argv, "cp:")) != -1) {
        switch (opt) {
            case 'c':
                context_flag = true;
                break;
            case 'p':
                pattern_flag = true;
                pattern = optarg;
                break;
            case '?':
                return 1;
        }
    }

    // store the first non-option argument in "pattern" if -p not found
    if (!pattern_flag) {
        pattern = argv[optind++];
    }
    
    // throw the rest of the non-option arguments into file_arr to be used later
    int file_count;
    while ((argc - optind) > 0) {
        file_arr[file_count++] = argv[optind++]; 
    }

    int bgrep_return_value;
    if (bgrep_return_value = bgrep(pattern_flag, context_flag, pattern, file_arr, file_count) != 0) {
        // error occured in bgrep function call
        return bgrep_return_value;
    }
    return 0;
}

int bgrep(bool pattern_flag, bool context_flag, char *pattern, char **file_arr, int file_count) {
    int pattern_fd; // only used if pattern flag is set
    int pattern_len; // stores length of pattern (if -p is set, this should be the file size of the pattern file)


    // if -p is set, open pattern file with mmap and point *pattern to it. Otherwise, pattern was already instantiated from getopt
    if (pattern_flag) {
        if ((pattern_fd = open(pattern, O_RDONLY)) < 0) {
            fprintf(stderr, "Failed to open pattern file %s: %s.", pattern, strerror(errno));
            return -1;
        }

        pattern_len = lseek(pattern_fd, 0, SEEK_END);
        pattern = mmap(NULL, pattern_len, PROT_READ, MAP_PRIVATE, pattern_fd, 0);
    } else {
        // if pattern flag is not set, then pattern_len is just the length of pattern
        pattern_len = strlen(pattern);
    }

    // entire grep and pattern matching starts here





    // entire grep and pattern matching ends here


    // debug printing, this isn't necessary for the program to work
    printf("Pattern flag: %d\n", pattern_flag);
    printf("Context flag: %d\n", context_flag);
    printf("Pattern: %s\n", pattern);
    for (int i = 0; i < file_count; i++) {
        printf("File %d: %s\n", i, file_arr[i]);
    }
    printf("File count: %d\n", file_count);
    printf("Pattern length: %d\n", pattern_len);

    

    // unmap the pattern if pattern flag is set
    if (pattern_flag) {
        if (munmap(pattern, pattern_len+1) == -1) {
                fprintf(stderr, "Failed to unmap pattern file to a string: %s.", strerror(errno));
                return -1;
        }
    }
    return 0;
}