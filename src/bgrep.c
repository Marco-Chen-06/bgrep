#include "bgrep.h"

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
                return 255;
        }
    }

    if (!pattern_flag) {
        pattern = argv[optind++];
    }
    
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
    printf("pattern flag: %d\n", pattern_flag);
    printf("context flag: %d\n", context_flag);
    printf("pattern: %s\n", pattern); 

    printf("%d", file_count);

    for (int i = 0; i < file_count; i++) {
        printf("%s ", file_arr[i]);
    }

    return 0;
}