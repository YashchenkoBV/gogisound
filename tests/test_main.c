#include <stdio.h>
#include <stdlib.h>
#include <sndfile.h>
#include <assert.h>

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

void test_cut_wav_segment_normal_case() {
    const char *input_path = "audio/song1.wav";
    const char *output_path = "audio/test.wav";
    double start_time = 2.0;  // start at 2 seconds
    double end_time = 5.0;    // end at 5 seconds

    // Run the cut function
    cut_wav_segment(input_path, output_path, start_time, end_time);

    // Validate that output file exists and has the expected duration
    SF_INFO sf_info;
    SNDFILE *output_file = sf_open(output_path, SFM_READ, &sf_info);
    assert(output_file != NULL); // Ensure file opened successfully

    // Calculate expected duration of the output file
    double output_duration = get_audio_length(output_path);
    double input_duration = get_audio_length(input_path);
    assert(output_duration == input_duration - (end_time - start_time)); // Duration should match the cut range (5.0 - 2.0 = 3.0 seconds)

    printf("----Normal case test passed.\n");
    sf_close(output_file);
}

void test_cut_wav_segment_start_time_before_audio() {
    const char *input_path = "audio/song1.wav";
    const char *output_path = "audio/test.wav";
    double start_time = -2.0; // start before the start of the audio
    double end_time = 3.0;

    // Run the cut function
    cut_wav_segment(input_path, output_path, start_time, end_time);

    // Validate that output file exists and has the expected duration
    SF_INFO sf_info;
    SNDFILE *output_file = sf_open(output_path, SFM_READ, &sf_info);
    assert(output_file != NULL); // Ensure file opened successfully

    // Duration should be from the start of the audio to 3 seconds
    double output_duration = get_audio_length(output_path);
    double input_duration = get_audio_length(input_path);
    assert(output_duration == input_duration - 3.0); // Expected duration 3 seconds

    printf("----Edge case test (start time before audio) passed.\n");

    sf_close(output_file);
}

void test_cut_wav_segment_end_time_after_audio() {
    const char *input_path = "audio/song1.wav";
    const char *output_path = "audio/test.wav";
    double start_time = 2.0;
    double end_time = get_audio_length(input_path) * 100;  // end time far beyond the audio file duration

    // Run the cut function
    cut_wav_segment(input_path, output_path, start_time, end_time);

    // Validate that output file exists and has the expected duration
    SF_INFO sf_info;
    SNDFILE *output_file = sf_open(output_path, SFM_READ, &sf_info);
    assert(output_file != NULL); // Ensure file opened successfully

    // Duration should be from 2.0 to the end of the audio
    double output_duration = (sf_info.frames / sf_info.samplerate);
    assert(output_duration == 2.0); // Should only contain the portion from 2.0 to the end of the file

    printf("----Edge case test (end time after audio) passed.\n");

    sf_close(output_file);
}

void test_cut_wav_segment_file_not_found() {
    const char *input_path = "non_existent_file.wav";
    const char *output_path = "../audio/test.wav";
    double start_time = 2.0;
    double end_time = 5.0;

    // Run the cut function
    cut_wav_segment(input_path, output_path, start_time, end_time);

    // Verify that the output file was not created
    SNDFILE *output_file = sf_open(output_path, SFM_READ, NULL);
    assert(output_file == NULL); // Should be NULL since the input file doesn't exist

    printf("----Error handling test (file not found) passed.\n");
}

void test_add_fade_in() {
    const char *input_path = "audio/song1.wav";
    const char *output_path = "audio/test.wav";
    double fading_time = 10.0;


    // Run the add_fade_in function
    add_fade_in(input_path, output_path, fading_time);

    //Verify that the output was created and has the same length
    SF_INFO sf_info;
    SNDFILE *output_file = sf_open(output_path, SFM_READ, &sf_info);
    assert(output_file != NULL); // Ensure file opened successfully

    double output_duration = get_audio_length(output_path);
    double input_duration = get_audio_length(input_path);
    assert(output_duration == input_duration);

    printf("----Fade-in test passed for sufficient argument.\n");

    sf_close(output_file);

    fading_time = -10.0;

    // Run the add_fade_in function
    add_fade_in(input_path, output_path, fading_time);

    // Verify that the output file was not created
    output_file = sf_open(output_path, SFM_READ, NULL);
    assert(output_file == NULL); // Should be NULL since the input file doesn't exist

    printf("----Fade-in test passed for insufficient argument.\n");
}

void test_add_fade_out() {
    const char *input_path = "audio/song1.wav";
    const char *output_path = "audio/test.wav";
    double fading_time = 20.0;


    // Run the add_fade_in function
    add_fade_in(input_path, output_path, fading_time);

    //Verify that the output was created and has the same length
    SF_INFO sf_info;
    SNDFILE *output_file = sf_open(output_path, SFM_READ, &sf_info);
    assert(output_file != NULL); // Ensure file opened successfully

    double output_duration = get_audio_length(output_path);
    double input_duration = get_audio_length(input_path);
    assert(output_duration == input_duration);

    printf("----Fade-out test passed for sufficient argument.\n");

    sf_close(output_file);

    fading_time = -20.0;

    // Run the add_fade_in function
    add_fade_in(input_path, output_path, fading_time);

    // Verify that the output file was not created
    output_file = sf_open(output_path, SFM_READ, NULL);
    assert(output_file == NULL); // Should be NULL since the input file doesn't exist

    printf("----Fade-out test passed for insufficient argument.\n");
}

void test_merge_wav_files() {
    const char *input1_path = "audio/song1.wav";
    const char *input2_path = "audio/song3.wav";
    const char *output_path = "audio/test.wav";

    merge_wav_files(input1_path, input2_path, output_path);

    SF_INFO sf_info;
    SNDFILE *output_file = sf_open(output_path, SFM_READ, &sf_info);
    assert(output_file != NULL); // Ensure file opened successfully

    int output_duration = get_audio_length(output_path);
    int input_duration = get_audio_length(input1_path) + get_audio_length(input2_path);
    assert(output_duration == input_duration);

    printf("----Merging test passed for compatible files.\n");

    input2_path = "audio/song2.wav";

    merge_wav_files(input1_path, input2_path, output_path);

    // Verify that the output file was not created
    output_file = sf_open(output_path, SFM_READ, NULL);
    assert(output_file == NULL); // Should be NULL since the input file doesn't exist

    printf("----Merging test passed for incompatible files.\n");
}


int main() {
    printf("\n");
    printf("Running tests...\n");
    printf("\n");
    printf("----Testing audio trimming...\n");
    test_cut_wav_segment_normal_case();
    test_cut_wav_segment_start_time_before_audio();
    test_cut_wav_segment_end_time_after_audio();
    test_cut_wav_segment_file_not_found();
    printf("\n");
    printf("----Testing fade-in and fade-out\n");
    test_add_fade_in();
    test_add_fade_out();
    printf("\n");
    printf("----Testing merging...\n");
    test_merge_wav_files();
    printf("\n");
    printf("All tests passed.\n");

    return 0;
}
