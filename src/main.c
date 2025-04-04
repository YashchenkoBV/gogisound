#include <stdio.h>
#include <string.h>
#include "audio_processing.h"
#include <stdlib.h>

#ifndef TEST_BUILD
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("No arguments given\n");
        printf("Try \"./ggsound --help\"\n");
        return 1;
    }

    if (strcmp(argv[1], "--help") == 0) {
        print_help();
        return 0;
    }

    if (strcmp(argv[1], "--cut") == 0) {
        if (argc != 6 && argc != 4) {
            fprintf(stderr, "Usage: ./ggsound --cut <input name> [start:end] (--name <output name>)\n");
            return 1;
        }

        char input_path[256];
        char output_path[256];
        double start_time = 0.0;
        double end_time = 0.0;

        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "--cut") == 0) {
                snprintf(input_path, sizeof(input_path), "%s%s", AUDIO_DIR, argv[++i]);
            } else if (i == 3) {
                if (argv[i][0] == '[' && argv[i][strlen(argv[i]) - 1] == ']') {
                    char *time_str = argv[i] + 1;
                    time_str[strlen(time_str) - 1] = '\0';

                    char *endptr;
                    if (strstr(time_str, ":") == time_str) {
                        end_time = strtod(time_str + 1, &endptr);
                        if (*endptr != '\0') {
                            fprintf(stderr, "Invalid number format for end_time\n");
                            return 1;
                        }
                        start_time = 0.0;
                    } else if (time_str[strlen(time_str) - 1] == ':') {
                        start_time = strtod(time_str, &endptr);
                        if (*endptr != ':') {
                            fprintf(stderr, "Invalid number format for start_time\n");
                            return 1;
                        }
                        end_time = -1.0;
                    } else {
                        start_time = strtod(time_str, &endptr);
                        if (*endptr != ':') {
                            printf("Incorrect arguments\n");
                            fprintf(stderr, "Usage: ./ggsound --cut <input name> [start:end] (--name <output name>)\n");
                            return 1;
                        }
                        end_time = strtod(endptr + 1, &endptr);
                        if (*endptr != '\0') {
                            fprintf(stderr, "Invalid number format for end_time\n");
                            return 1;
                        }
                    }
                } else {
                    printf("Incorrect arguments\n");
                    fprintf(stderr, "Usage: ./ggsound --cut <input name> [start:end] (--name <output name>)\n");
                    return 1;
                }
            } else if (i == 4) {
                if (strcmp(argv[i], "--name") == 0 && i + 1 < argc) {
                    snprintf(output_path, sizeof(output_path), "%s%s", AUDIO_DIR, argv[++i]);
                } else {
                    printf("Incorrect arguments\n");
                    fprintf(stderr, "Usage: ./ggsound --cut <input name> [start:end] (--name <output name>)\n");
                    return 1;
                }
            }
        }

        if (start_time < 0) {
            fprintf(stderr, "Invalid start time\n");
            return 1;
        }

        if (end_time < 0 && end_time != -1.0) {
            fprintf(stderr, "Invalid end time\n");
            return 1;
        }

        if (argc == 4) {
            snprintf(output_path, sizeof(output_path), "%s%s", AUDIO_DIR, "gogi.wav");
        }

        cut_wav_segment(input_path, output_path, start_time, end_time);
        return 0;
    }

    if (strcmp(argv[1], "--fade-in") == 0) {
        if (argc != 6 && argc != 4) {
            fprintf(stderr, "Usage: ./ggsound --fade-in <input name> fading-time (--name <output name>)\n");
            return 1;
        }

        char input_path[256];
        char output_path[256];
        double fading_time = 0.0;

        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "--fade-in") == 0 && i + 1 < argc) {
                snprintf(input_path, sizeof(input_path), "%s%s", AUDIO_DIR, argv[++i]);
            } else if (strcmp(argv[i], "--name") == 0 && i + 1 < argc) {
                snprintf(output_path, sizeof(output_path), "%s%s", AUDIO_DIR, argv[++i]);
            } else {
                char *endptr;
                fading_time = strtod(argv[i], &endptr);
                if (*endptr != '\0') {
                    fprintf(stderr, "Invalid number format for fading_time\n");
                    return 1;
                }
            }
        }

        if (fading_time < 0) {
            fprintf(stderr, "Invalid fading time\n");
            return 1;
        }

        if (argc == 4) {
            snprintf(output_path, sizeof(output_path), "%s%s", AUDIO_DIR, "gogi.wav");
        }

        add_fade_in(input_path, output_path, fading_time);
        return 0;
    }

    if (strcmp(argv[1], "--fade-out") == 0) {
        if (argc != 6 && argc != 4) {
            fprintf(stderr, "Usage: ./ggsound --fade-out <input name> fading-time (--name <output name>)\n");
            return 1;
        }

        char input_path[256];
        char output_path[256];
        double fading_time = 0.0;

        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "--fade-out") == 0 && i + 1 < argc) {
                snprintf(input_path, sizeof(input_path), "%s%s", AUDIO_DIR, argv[++i]);
            } else if (strcmp(argv[i], "--name") == 0 && i + 1 < argc) {
                snprintf(output_path, sizeof(output_path), "%s%s", AUDIO_DIR, argv[++i]);
            } else {
                char *endptr;
                fading_time = strtod(argv[i], &endptr);
                if  (*endptr != '\0') {
                    fprintf(stderr, "Invalid number format for fading_time\n");
                    return 1;
                }
            }
        }

        if (fading_time < 0) {
            fprintf(stderr, "Invalid fading time\n");
            return 1;
        }

        if (argc == 4) {
            snprintf(output_path, sizeof(output_path), "%s%s", AUDIO_DIR, "gogi.wav");
        }

        add_fade_out(input_path, output_path, fading_time);
        return 0;
    }

    if (strcmp(argv[1], "--merge") == 0) {
        if (argc != 6 && argc != 4) {
            fprintf(stderr, "Usage: ./ggsound --merge <first file> <second file> (--name <output name>)\n");
            return 1;
        }

        char input1_path[256];
        char input2_path[256];
        char output_path[256];

        snprintf(input1_path, sizeof(input1_path), "%s%s", AUDIO_DIR, argv[2]);
        snprintf(input2_path, sizeof(input2_path), "%s%s", AUDIO_DIR, argv[3]);

        if (argc == 6) {
            if (strcmp(argv[4], "--name") == 0) {
                snprintf(output_path, sizeof(output_path), "%s%s", AUDIO_DIR, argv[5]);
            }
            else {
                printf("Incorrect arguments\n");
                fprintf(stderr, "Usage: ./ggsound --merge <first file> <second file> (--name <output name>)\n");
                return 1;
            }
        } else {
            snprintf(output_path, sizeof(output_path), "%s%s", AUDIO_DIR, "gogi.wav");
        }

        merge_wav_files(input1_path, input2_path, output_path);
        return 0;
    }

    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s%s", AUDIO_DIR, argv[1]);

    double length = get_audio_length(filepath);
    if (length >= 0) {
        printf("\"%s\" length: %.1f seconds\n", argv[1], length);
    } else {
        fprintf(stderr, "Incorrect name or path. No such file in the directory.\n");
        return 1;
    }

    return 0;
}
#endif
