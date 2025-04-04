#include <stdio.h>
#include <sndfile.h>
#include <stdlib.h>
#include "audio_processing.h"
#include <stdint.h> // For SIZE_MAX

// Function to get the length of an audio file in seconds
double get_audio_length(const char *filepath) {
    SF_INFO info;
    info.format = 0;

    // Open the audio file
    SNDFILE *file = sf_open(filepath, SFM_READ, &info);

    if (!file) {
        return -1;
    }

    // Calculate the length in seconds
    double length = (double)info.frames / (double)info.samplerate;

    // Close the file
    sf_close(file);

    return length;
}

// Function to trim audio file
void cut_wav_segment(const char *input_path, const char *output_path, double start_time, double end_time) {
    SF_INFO sf_info;

    // Open the input file
    SNDFILE *input_file = sf_open(input_path, SFM_READ, &sf_info);
    if (!input_file) {
        fprintf(stderr, "Error: Could not open input file %s\n", input_path);
        return;
    }

    // Calculate total number of samples and positions for cut
    const int total_samples = (int) sf_info.frames * sf_info.channels;
    int start_sample = (int)(start_time * sf_info.samplerate * sf_info.channels + 0.5);
    int end_sample = (end_time > 0) ? (int)(end_time * sf_info.samplerate * sf_info.channels + 0.5) : total_samples;
    // Clamp values to file bounds
    if (start_sample < 0) start_sample = 0;
    if (end_sample > total_samples) end_sample = total_samples;

    // Check for integer overflow before allocating memory
    if ((unsigned long long)total_samples > SIZE_MAX / sizeof(int)) {
        fprintf(stderr, "Memory allocation error: size too large.\n");
        sf_close(input_file);
        return;
    }

    // Allocate a buffer for the samples
    int *buffer = malloc(total_samples * sizeof(int));
    if (!buffer) {
        fprintf(stderr, "Memory allocation error.\n");
        sf_close(input_file);
        return;
    }

    // Read all samples from the input file
    const int samples_read = (int) sf_read_int(input_file, buffer, total_samples);
    if (samples_read < total_samples) {
        fprintf(stderr, "Error reading samples.\n");
        free(buffer);
        sf_close(input_file);
        return;
    }

    // Open the output file
    SNDFILE *output_file = sf_open(output_path, SFM_WRITE, &sf_info);
    if (!output_file) {
        fprintf(stderr, "Error: Could not open output file %s\n", output_path);
        free(buffer);
        sf_close(input_file);
        return;
    }

    // Write the samples before the cut segment
    if (start_sample > 0) {
        sf_write_int(output_file, buffer, start_sample);
    }

    // Write the samples after the cut segment
    if (end_sample < total_samples) {
        sf_write_int(output_file, buffer + end_sample, total_samples - end_sample);
    }

    // Clean up
    free(buffer);
    sf_close(input_file);
    sf_close(output_file);

    printf("Segment cut from %s and saved to %s\n", input_path, output_path);
}

// Function to add fade-in
void add_fade_in(const char *input_path, const char *output_path, double fading_time) {
    SF_INFO sfinfo;

    SNDFILE *input_file = sf_open(input_path, SFM_READ, &sfinfo);

    if (fading_time < 0) {
        fprintf(stderr, "Error: insufficient time argument %s\n", input_path);
        return;
    }

    if (!input_file) {
        fprintf(stderr, "Error: Could not open input file %s\n", input_path);
        return;
    }

    // Allocate a buffer to hold the entire audio file
    const sf_count_t total_samples = sfinfo.frames * sfinfo.channels;
    float *buffer = (float *)malloc(total_samples * sizeof(float));
    if (!buffer) {
        fprintf(stderr, "Error: Could not allocate memory for audio data.\n");
        sf_close(input_file);
        return;
    }

    // Read all samples from the input file
    const sf_count_t read_count = sf_read_float(input_file, buffer, total_samples);
    if (read_count != total_samples) {
        fprintf(stderr, "Error: Could not read all samples from the input file.\n");
        free(buffer);
        sf_close(input_file);
        return;
    }
    sf_close(input_file);

    double file_duration = (double)sfinfo.frames / sfinfo.samplerate;
    if (fading_time > file_duration) {
        fprintf(stderr, "Warning: Fade-in time exceeds file duration. Adjusting fade-in time to file duration (%.1f seconds).\n", file_duration);
        fading_time = file_duration;
    }

    // Calculate the number of samples affected by the fade-in
    const sf_count_t fade_samples = (sf_count_t)(fading_time * sfinfo.samplerate);

    // Apply the fade-in effect
    for (sf_count_t i = 0; i < fade_samples; ++i) {
        const float fade_factor = (float)i / (float)fade_samples;
        for (int ch = 0; ch < sfinfo.channels; ++ch) {
            buffer[i * sfinfo.channels + ch] *= fade_factor;
        }
    }

    // Open the output audio file
    SNDFILE *output_file = sf_open(output_path, SFM_WRITE, &sfinfo);
    if (!output_file) {
        fprintf(stderr, "Error: Could not open output file %s\n", output_path);
        free(buffer);
        return;
    }

    // Write the modified audio data to the output file
    sf_count_t write_count = sf_write_float(output_file, buffer, total_samples);
    if (write_count != total_samples) {
        fprintf(stderr, "Error: Could not write all samples to the output file.\n");
    }

    // Clean up
    sf_close(output_file);
    free(buffer);

    printf("Fade-in added to first %d seconds of %s and saved to %s\n", (int) fading_time, input_path, output_path);
}

// Function to add fade-out
void add_fade_out(const char *input_path, const char *output_path, double fading_time) {
    SF_INFO sfinfo;

    SNDFILE *input_file = sf_open(input_path, SFM_READ, &sfinfo);

    if (fading_time < 0) {
        fprintf(stderr, "Error: insufficient time argument %s\n", input_path);
        return;
    }

    if (!input_file) {
        fprintf(stderr, "Error: Could not open input file %s\n", input_path);
        return;
    }

    // Allocate a buffer to hold the entire audio file
    sf_count_t total_samples = sfinfo.frames * sfinfo.channels;
    float *buffer = (float *)malloc(total_samples * sizeof(float));
    if (!buffer) {
        fprintf(stderr, "Error: Could not allocate memory for audio data.\n");
        sf_close(input_file);
        return;
    }

    // Read all samples from the input file
    sf_count_t read_count = sf_read_float(input_file, buffer, total_samples);
    if (read_count != total_samples) {
        fprintf(stderr, "Error: Could not read all samples from the input file.\n");
        free(buffer);
        sf_close(input_file);
        return;
    }
    sf_close(input_file);

    // Check if fade-out duration exceeds the audio file duration
    double file_duration = (double)sfinfo.frames / sfinfo.samplerate;
    if (fading_time > file_duration) {
        fprintf(stderr, "Warning: Fade-out time exceeds file duration. Adjusting fade-out time to file duration (%.2f seconds).\n", file_duration);
        fading_time = file_duration;
    }

    // Calculate the number of samples affected by the fade-out
    sf_count_t fade_samples = (sf_count_t)(fading_time * sfinfo.samplerate);

    // Apply the fade-out effect
    sf_count_t start_fade_index = sfinfo.frames - fade_samples;
    if (start_fade_index < 0) start_fade_index = 0; // Edge case: small files

    for (sf_count_t i = start_fade_index; i < sfinfo.frames; ++i) {
        float fade_factor = (float)(sfinfo.frames - i) / (float)fade_samples;
        if (fade_factor < 0) fade_factor = 0; // Just in case
        for (int ch = 0; ch < sfinfo.channels; ++ch) {
            buffer[i * sfinfo.channels + ch] *= fade_factor;
        }
    }

    // Open the output audio file
    SNDFILE *output_file = sf_open(output_path, SFM_WRITE, &sfinfo);
    if (!output_file) {
        fprintf(stderr, "Error: Could not open output file %s\n", output_path);
        free(buffer);
        return;
    }

    // Write the modified audio data to the output file
    sf_count_t write_count = sf_write_float(output_file, buffer, total_samples);
    if (write_count != total_samples) {
        fprintf(stderr, "Error: Could not write all samples to the output file.\n");
    }

    // Clean up
    sf_close(output_file);
    free(buffer);

    printf("Fade-out added to last %d seconds of %s and saved to %s\n", (int) fading_time, input_path, output_path);
}

// Function to merge audio file
void merge_wav_files(const char *input1_path, const char *input2_path, const char *output_path) {
    // File handles and info structures
    SNDFILE *input_file = NULL, *output_file = NULL;
    SF_INFO input_info = {0}, output_info = {0};

    // Buffers
    float *buffer = NULL;

    // Open the first input file
    input_file = sf_open(input1_path, SFM_READ, &input_info);
    if (!input_file) {
        fprintf(stderr, "Error: Could not open first input file %s\n", input1_path);
        return;
    }

    // Set up the output file's info (same as the first input file)
    output_info = input_info;

    // Open the output file
    output_file = sf_open(output_path, SFM_WRITE, &output_info);
    if (!output_file) {
        fprintf(stderr, "Error: Could not open output file: %s\n", output_path);
        sf_close(input_file);
        return;
    }

    // Allocate a buffer for reading chunks of data
    sf_count_t buffer_size = 1024 * input_info.channels; // Arbitrary chunk size
    buffer = (float *)malloc(buffer_size * sizeof(float));
    if (!buffer) {
        fprintf(stderr, "Error: Could not allocate memory for buffer.\n");
        sf_close(input_file);
        sf_close(output_file);
        return;
    }

    // Read and write the first file in chunks
    sf_count_t read_count;
    while ((read_count = sf_read_float(input_file, buffer, buffer_size)) > 0) {
        sf_write_float(output_file, buffer, read_count);
    }

    sf_close(input_file); // Close the first input file

    // Open the second input file
    input_file = sf_open(input2_path, SFM_READ, &input_info);
    if (!input_file) {
        fprintf(stderr, "Error: Could not open second input file: %s\n", input2_path);
        free(buffer);
        sf_close(output_file);
        return;
    }

    // Ensure the second file matches the format of the first
    // Ensure the second file matches the format of the first
    if (input_info.format != output_info.format ||
        input_info.samplerate != output_info.samplerate ||
        input_info.channels != output_info.channels) {
        fprintf(stderr, "Error: Input files are not compatible for merging due to:\n");
        if (input_info.format != output_info.format) {
            fprintf(stderr, "- Different audio formats: %d vs %d\n", output_info.format, input_info.format);
        }
        if (input_info.samplerate != output_info.samplerate) {
            fprintf(stderr, "- Different sample rates: %d Hz vs %d Hz\n", output_info.samplerate, input_info.samplerate);
        }
        if (input_info.channels != output_info.channels) {
            fprintf(stderr, "- Different channel counts: %d vs %d\n", output_info.channels, input_info.channels);
        }

        sf_close(input_file);
        free(buffer);
        sf_close(output_file);
        if (unlink(output_path) != 0) {
            fprintf(stderr, "Error: Failed to delete output file %s", output_path);
        }
        return;
        }

    // Read and write the second file in chunks
    while ((read_count = sf_read_float(input_file, buffer, buffer_size)) > 0) {
        sf_write_float(output_file, buffer, read_count);
    }

    // Clean up
    free(buffer);
    sf_close(input_file);
    sf_close(output_file);

    printf("Successfully merged %s and %s into %s.\n", input1_path, input2_path, output_path);
}


// Function to print help instructions
void print_help() {
    printf("\n");
    printf("GoGiSound -- Sound Editor Program\n");
    printf("Description:\n");
    printf("    This is a simple sound editor for trimming and merging audio files and adding fade-in/fade-out.\n");
    printf("    effects to them! Use --cut to cut out an unnecessary segment by its time borders, --fade-in to\n");
    printf("    to add fading effect to first several seconds of audio file, --fade-out to add fading effect to\n");
    printf("    last several seconds and --merge to connect two audio files, one after the other. You can just\n");
    printf("    write the name of audio file after calling the editor to learn its length\n");
    printf("\n");
    printf("    Important: all commands have attribute --name, after which you can write a name of the output file.\n");
    printf("    If you don't use this attributed, processed audio file will be save to ../audio/gogi.wav\n");
    printf("    The default directory for editor's output files is ../audio/ (relative to the directory of the\n");
    printf("    editor itself). If you want to save files to other places, please use relative paths (which start\n");
    printf("    at ../audio/).\n");
    printf("\n");
    printf("Usage syntax:\n");
    printf("    Show this help message:\n");
    printf("        ./ggsound --help\n");
    printf("    Cut out unnecessary segment (Use time borders of the segment you WANT TO CUT OUT):\n");
    printf("        ./ggsound --cut <input name> [start:end] (--name <output name>)\n");
    printf("    Add fade-in:\n");
    printf("        ./ggsound --fade-in <input name> fading-time (--name <output name>)\n");
    printf("    Add fade-out:\n");
    printf("        ./ggsound --fade-out <input name> fading-time (--name <output name>)\n");
    printf("    Merge 2 files:\n");
    printf("        ./ggsound --merge <first file> <second file> (--name <output name>)\n");
    printf("    Learn file's duration:\n");
    printf("        ./ggsound <filename.wav>\n");
    printf("\n");
    printf("    Note 1: parameters in the round brackets are optional.\n");
    printf("    Note 2: all time values are written in seconds and can be entered in both int and float formats.\n");
    printf("\n");
    printf("Have fun!\n");
}
