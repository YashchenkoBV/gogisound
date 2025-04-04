#ifndef AUDIO_PROCESSING_H
#define AUDIO_PROCESSING_H

// Default directory for audio files
#define AUDIO_DIR "../audio/"

// Function to get the length of an audio file in seconds
double get_audio_length(const char *filepath);

// Function to trim audio file
void cut_wav_segment(const char *input_path, const char *output_path, double start_time, double end_time);

// Function to add fade-in
void add_fade_in(const char *input_path, const char *output_path, double fading_time);

// Function to add fade-out
void add_fade_out(const char *input_path, const char *output_path, double fading_time);

// Function to merge audio file
void merge_wav_files(const char *input1_path, const char *input2_path, const char *output_path);

// Function to print help instructions
void print_help();

#endif // AUDIO_PROCESSING_H