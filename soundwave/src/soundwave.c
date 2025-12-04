#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#define APPLY_EFFECT(i, dj, start, end) (!(dj) || ((i) >= (start) && (i) < (end)))//dj mode allows us to apply effect only in a segment

//Let's become global
unsigned char riff[4], wave[4], fmt[4], data[4];
uint32_t file_size, fmt_chunk_size, sample_rate, bytes_per_sec, data_chunk_size;
uint16_t format_type, channels, block_align, bits_per_samp;
uint32_t bytes_read_after_size=0;
//TRUST ME, we are gonna need these helpers
//helper to read 4 bytes
void read4(unsigned char *buf) {
    for (int i = 0; i < 4; i++) {
        int c = getchar();
        if (c == EOF) {
            fprintf(stderr, "Error: unexpected EOF reading 4 bytes\n");
            exit(1);
        }
        buf[i] = (unsigned char)c;
    }
}
//helper to read 2 bytes
void read2(unsigned char *buf) {
    for (int i = 0; i < 2; i++) {
        int c = getchar();
        if (c == EOF) {
            fprintf(stderr, "Error: unexpected EOF reading 2 bytes\n");
            exit(1);
        }
        buf[i] = (unsigned char)c;
    }

}
//helper to write bytes from an array in stdin
void write_bytes(const unsigned char *buf, int n) {
    for (int i = 0; i < n; i++) {
        putchar(buf[i]);
    }
}
//Little-endian helpers
//helper to read 4 bytes in little-endian
uint32_t read4_le() {
    uint32_t val = 0;
    for (int i = 0; i < 4; i++) {
        int c = getchar();
        if (c == EOF) {
            fprintf(stderr, "Error: unexpected EOF reading 4 bytes little-endian\n");
            exit(1);
        }
        val |= ((uint32_t)c) << (i * 8);
    }
    return val;
}
//helper to read 2 bytes in little-endian
uint16_t read2_le() {
    uint16_t val = 0;
    for (int i = 0; i < 2; i++) {
        int c = getchar();
        if (c == EOF) {
            fprintf(stderr, "Error: unexpected EOF reading 2 bytes little-endian\n");
            exit(1);
        }
        val |= ((uint16_t)c) << (i * 8);
    }
    return val;
}
// Helper to write a 4-byte integer in little-endian
void put4bytes(uint32_t val) {
    for (int i = 0; i < 4; i++) {
        putchar((val >> (8 * i)) & 0xFF);
    }
}
// Helper to write a 2-byte integer in little-endian
void put2bytes(uint16_t val) {
    for (int i = 0; i < 2; i++) {
        putchar((val >> (8 * i)) & 0xFF);
    }
}

//BORING ANALYSIS FUNCTION 
void info(void) {
    unsigned char buf[4];
 
    // --- RIFF ---
     read4(buf);
     if (memcmp(buf, "RIFF", 4) != 0) {
        fprintf(stderr,"Error! \"RIFF\" not found\n");
        exit(1);
     }
    // Read file size (little endian)
     file_size = read4_le();   

     printf("size of file: %u\n", file_size );
    
    // --- WAVE ---
     read4(buf);
     bytes_read_after_size += 4;
     if (memcmp(buf, "WAVE", 4) != 0) {
        fprintf(stderr,"Error! \"WAVE\" not found\n");
        exit(1);
     }  
    // --- fmt ---
     read4(buf);
     bytes_read_after_size += 4;
     if (memcmp(buf, "fmt ", 4) != 0) {
        fprintf(stderr,"Error! \"fmt \" not found\n");
        exit(1);
     }
    // Read fmt chunk size -must be 16 
     fmt_chunk_size = read4_le();
     bytes_read_after_size += 4;

     printf("size of format chunk: %u\n", fmt_chunk_size);

     if (fmt_chunk_size != 16) {
        fprintf(stderr, "Error! size of format chunk should be 16\n");
        exit(1);
     }

    //read the rest of fmt chunk
    //wave format- must be 1
     format_type = read2_le();
     bytes_read_after_size += 2;
     
     printf("WAVE type format: %u\n", format_type);

     if (format_type != 1) {
        fprintf(stderr, "Error! WAVE type format should be 1\n");
        exit(1);
     }
    //stereo/mono 1=mono 2=stereo
     channels = read2_le();
     bytes_read_after_size += 2;

     printf("mono/stereo: %u\n", channels);

     if (channels != 1 && channels != 2) {
        fprintf(stderr, "Error! mono/stereo should be 1 or 2\n");
        exit(1);
     }
    //sample rate
     sample_rate = read4_le();
     bytes_read_after_size += 4;

     printf("sample rate: %u\n", sample_rate);

    //byte rate
     bytes_per_sec = read4_le();
     bytes_read_after_size += 4;  

     printf("bytes/sec: %u\n", bytes_per_sec);

    //block align
     block_align = read2_le();
     bytes_read_after_size += 2;

     printf("block alignment: %u\n", block_align);

     uint32_t expected_bytes_per_sec = sample_rate * block_align;

       if (bytes_per_sec != expected_bytes_per_sec) {
            fprintf(stderr, "Error! bytes/second should be sample rate x block alignment\n");
            exit(1);
     }
    //bits per sample      
     bits_per_samp = read2_le();
     bytes_read_after_size += 2; 

     printf("bits/sample: %u\n", bits_per_samp);

    //validate bits per sample
     if (bits_per_samp != 16 && bits_per_samp != 8) {
        fprintf(stderr, "Error! bits/sample should be 8 or 16\n");
        exit(1);
     }
    //validate block_align
     uint16_t expected_block_align = bits_per_samp/8 * channels;
     if (block_align != expected_block_align) {
        fprintf(stderr, "Error! block alignment should be bits per sample / 8 x mono/stereo\n");
        exit(1);
     }
    // --- data chunk ---
     read4(buf);
     bytes_read_after_size += 4;
     if(memcmp(buf,"data",4)!=0){
        fprintf(stderr,"Error! \"data\" not found\n");
        exit(1);
     }
     data_chunk_size = read4_le();
     bytes_read_after_size += 4;

     printf("size of data chunk: %u\n", data_chunk_size);

    // --- count the data chunck ---
     int d;
     for (uint32_t i = 0; i < data_chunk_size; i++) {
        d = getchar();
        if (d == EOF) {
            fprintf(stderr, "Error! insufficient data\n");
            exit(1);
        }
    }
    //count extra bytes
    uint32_t extra_bytes = 0;
    int c;
    while ((c = getchar()) != EOF) {
        extra_bytes++;
    }
    // --- Final size check ---
    uint32_t total_counted = bytes_read_after_size + data_chunk_size + extra_bytes;
    // --- Compare bytes read with header file_size ---
     if ( total_counted < file_size) {
        fprintf(stderr,"Error! bad file size (file ended before expected end of file)\n");
        exit(1);
     }
     if (total_counted > file_size) {
        fprintf(stderr,"Error! bad file size (found data past the expected end of file)\n");
        exit(1);
     }
}  

//Helper to read header and put it to vectors
void read_header(void) {
    unsigned char buf[4];
    // --- RIFF ---
     read4(buf);
     if (memcmp(buf, "RIFF", 4) != 0) {
        fprintf(stderr,"Error! \"RIFF\" not found\n");
        exit(1);
     }   
     memcpy(riff, buf, 4); 
    // --- file size (little endian) ---
     file_size = read4_le();
    // --- WAVE ---
     read4(buf);
     bytes_read_after_size += 4;
     if (memcmp(buf, "WAVE", 4) != 0) {
        fprintf(stderr,"Error! \"WAVE\" not found\n");
        exit(1);
     }    
     memcpy(wave, buf, 4);
    // --- fmt ---
        read4(buf);
        bytes_read_after_size += 4;
        if (memcmp(buf, "fmt ", 4) != 0) {
            fprintf(stderr,"Error! \"fmt \" not found\n");
            exit(1);
        }    
        memcpy(fmt, buf, 4);
    // --- fmt chunk size ---
        fmt_chunk_size = read4_le();   
        bytes_read_after_size += 4;
        if (fmt_chunk_size != 16) {
            fprintf(stderr, "Error! size of format chunk should be 16\n");
            exit(1);
        }
    // --- rest of fmt chunk ---
    // --- WAVE type format ahould be 1
        format_type = read2_le();
          bytes_read_after_size += 2;
     if (format_type != 1) {
        fprintf(stderr, "Error! WAVE type format should be 1\n");
        exit(1);
     }
        channels = read2_le(); //channels should be 1 or 2
          bytes_read_after_size += 2;
          if (channels != 1 && channels != 2) {
        fprintf(stderr, "Error! mono/stereo should be 1 or 2\n");
        exit(1);
     }
        sample_rate = read4_le(); //sample rate
          bytes_read_after_size += 4;
        bytes_per_sec = read4_le(); //bytes per second
          bytes_read_after_size += 4;
        block_align = read2_le(); //block alignment
          bytes_read_after_size += 2;
        bits_per_samp = read2_le(); //bits per sample
          bytes_read_after_size += 2;
     //validate bytes per sec
       uint32_t expected_bytes_per_sec = sample_rate * block_align;
       if (bytes_per_sec != expected_bytes_per_sec) {
            fprintf(stderr, "Error! bytes/second should be sample rate x block alignment\n");
            exit(1);
     }
    //validate bits per sample
     if (bits_per_samp != 16 && bits_per_samp != 8) {
        fprintf(stderr, "Error! Unsupported bits per sample: %u (only 16 and 8 bits per sample supported)\n", bits_per_samp);
        exit(1);
     }     
    //validate block align
     uint16_t expected_block_align = bits_per_samp/8 * channels;
     if (block_align != expected_block_align) {
        fprintf(stderr, "Error! block alignment should be bits per sample / 8 x mono/stereo\n");
        exit(1);
     }

    // --- data ---
        read4(buf);
        bytes_read_after_size += 4;
        if(memcmp(buf,"data",4)!=0){
            fprintf(stderr,"Error! \"data\" not found\n");
            exit(1);
        }    
        memcpy(data, buf, 4);
    // --- data chunk size ---
      data_chunk_size = read4_le();
        bytes_read_after_size += 4; 


}

//helper to write header with possible modified sample_rate and bytes_per_sec
void write_header(uint32_t new_sample_rate, uint32_t new_bytes_per_sec, uint16_t new_channels, uint16_t new_block_align, uint32_t new_data_chunk_size, uint32_t new_file_size) {
    // --- RIFF ---
     write_bytes(riff, 4);
    // --- file size ---
     put4bytes(new_file_size);
    // --- WAVE ---
     write_bytes(wave, 4);
    // --- fmt ---
     write_bytes(fmt, 4);
    // --- fmt chunk size ---
     put4bytes(fmt_chunk_size);
    // --- rest of fmt chunk ---
     put2bytes(format_type);
     put2bytes(new_channels);
     put4bytes(new_sample_rate);
     put4bytes(new_bytes_per_sec);
     put2bytes(new_block_align);
     put2bytes(bits_per_samp);
    // --- data ---
     write_bytes(data, 4);
    // --- data chunk size ---
     put4bytes(new_data_chunk_size);
}

// Copies all bytes remaining after the data chunk and returns how many there are
uint32_t copy_extra_bytes(void){
    uint32_t extra_bytes = 0;
    int c;

    while ((c = getchar()) != EOF) {
        putchar(c);
        extra_bytes++;
    }

    return extra_bytes;
}

// Checks that total bytes read match file_size
void check_final_size(uint32_t bytes_read_after_size, uint32_t data_chunk_size, uint32_t extra_bytes, uint32_t file_size){
    uint32_t total_counted = bytes_read_after_size + data_chunk_size + extra_bytes;

    if (total_counted < file_size) {
        fprintf(stderr,
            "Error! bad file size (file ended before expected end of file)\n");
        exit(1);
    }

    if (total_counted > file_size) {
        fprintf(stderr,
            "Error! bad file size (found data past the expected end of file)\n");
        exit(1);
    }
}

//Let's speed up (or slow down)
void rate(double fp_rate) {
   
    // --- read the full header ---
     read_header();
    // --- compute new sample_rate and bytes_per_sec and channels ---
     uint32_t new_sample_rate = (uint32_t)(sample_rate * fp_rate);
     uint32_t new_bytes_per_sec = (uint32_t)(bytes_per_sec * fp_rate);
     uint16_t new_channels = channels;
        uint16_t new_block_align = block_align;
        uint32_t new_data_chunk_size = data_chunk_size;
        uint32_t new_file_size = file_size;
    // --- write the header with updated values ---
        write_header(new_sample_rate, new_bytes_per_sec, new_channels, new_block_align, new_data_chunk_size, new_file_size  );
        
    // --- copy audio unchanged ---
     for (uint32_t i = 0; i < data_chunk_size; i++) {
        int c = getchar();
        if (c == EOF) {
            fprintf(stderr, "Error! insufficient data\n");
            exit(1);
        }

        putchar(c);
    }

    // extra bytes
    uint32_t extra_bytes = copy_extra_bytes();
    
   // Final size check
   check_final_size(bytes_read_after_size, data_chunk_size, extra_bytes, file_size);
  
    
}

//Let's extract a channel
void channel(char mode) {
    if (mode != 'L' && mode != 'R') {
        fprintf(stderr, "Error! Invalid mode. Use 'L' or 'R'\n");
        exit(1);
    }

    // Read full WAV header
    read_header();

    uint16_t bytes_per_sample = bits_per_samp / 8;
    uint16_t new_channels = (channels == 2) ? 1 : channels;
    uint16_t new_block_align = bytes_per_sample * new_channels;
    uint32_t new_bytes_per_sec = sample_rate * new_block_align;
    uint32_t new_data_chunk_size = (channels == 2) ? data_chunk_size / 2 : data_chunk_size;
    uint32_t new_file_size = bytes_read_after_size + new_data_chunk_size;

    // Write updated header
    write_header(sample_rate, new_bytes_per_sec, new_channels,
                 new_block_align, new_data_chunk_size, new_file_size);

    if (channels == 1) {
        // Mono → just copy all samples
        for (uint32_t i = 0; i < data_chunk_size; i++) {
            int c = getchar();
            if (c == EOF) {
            fprintf(stderr, "Error! insufficient data\n");
            exit(1);
        }
            putchar(c);
        }
    } else {
        // Stereo → extract requested channel
        uint32_t stereo_frames = data_chunk_size / (bytes_per_sample * 2);//This computes how many (L,R) sample pairs exist in the file.
        for (uint32_t i = 0; i < stereo_frames; i++) {//repeat for every frame
            int left = 0, right = 0;

            if (bytes_per_sample == 1) {//For 8-bit WAV unsigned samples it is one left and one right sample
                left = getchar();
                right = getchar();
            } else { // 16-bit 2 left folloed by 2 right samples stored in little-endian
                int loL = getchar();
                int hiL = getchar();
                int loR = getchar();
                int hiR = getchar();
                if (loL == EOF || hiL == EOF || loR == EOF || hiR == EOF) {
            fprintf(stderr, "Error! insufficient data\n");
            exit(1);
        }        //reconstructing
                left = loL | (hiL << 8); 
                right = loR | (hiR << 8);
            }

            if (mode == 'L') { //extracting only one channel
                if (bytes_per_sample == 1) putchar(left);
                else { putchar(left & 0xFF); putchar((left >> 8) & 0xFF); }
            } else { // 'R'
                if (bytes_per_sample == 1) putchar(right);
                else { putchar(right & 0xFF); putchar((right >> 8) & 0xFF); }
            }
        }
    }

    // Copy any extra chunks after audio
    uint32_t extra_bytes = copy_extra_bytes();
    // Final size check
    check_final_size(bytes_read_after_size, data_chunk_size, extra_bytes, file_size);
}

//Semi-cool soundwave generator
void mysound(int dur, int sr, double fm, double fc, double mi, double amp) {
    int num_samples = dur * sr;

    uint16_t num_channels = 1;       // Mono
    uint16_t bits_per_sample = 16;   // 2 bytes
    uint32_t byte_rate = sr * num_channels * bits_per_sample / 8;
    uint16_t block_align = num_channels * bits_per_sample / 8;
    uint32_t data_chunk_size = num_samples * num_channels * bits_per_sample / 8;
    uint32_t chunk_size = 36 + data_chunk_size;

    // --- WAV header ---
    putchar('R'); putchar('I'); putchar('F'); putchar('F');
    put4bytes(chunk_size);           // ChunkSize
    putchar('W'); putchar('A'); putchar('V'); putchar('E');

    // fmt subchunk
    putchar('f'); putchar('m'); putchar('t'); putchar(' ');
    put4bytes(16);                   // fmt chunk Size
    put2bytes(1);                     // AudioFormat = PCM
    put2bytes(num_channels);          // NumChannels
    put4bytes(sr);                    // SampleRate
    put4bytes(byte_rate);             // ByteRate
    put2bytes(block_align);           // BlockAlign
    put2bytes(bits_per_sample);       // BitsPerSample

    // data chunk
    putchar('d'); putchar('a'); putchar('t'); putchar('a');
    put4bytes(data_chunk_size);        // Subchunk2Size

    // --- Generate samples ---
    for (int n = 0; n < num_samples; n++) {
        double t = (double)n / sr;
        // formula: f(t) = trunc(amp * sin(2πfc t − mi * sin(2πfm t)))
        double val = trunc(amp * sin(2 * M_PI * fc * t - mi * sin(2 * M_PI * fm * t)));
        int16_t sample = (int16_t)val;

        // Write sample in little-endian
        put2bytes(sample);
    }
}

//Entering mode dj where we become time lords 
// let's cranck the volume up or down 
void volume(double fp_volume, int soundwave_dj_mode, double start_time, double end_time) {
    
    read_header();  // read WAV header
    
    uint16_t bytes_per_sample = bits_per_samp / 8;
    uint32_t total_samples = data_chunk_size / bytes_per_sample;

    // calculate total frames
    uint32_t total_frames = total_samples / channels;

    // Convert time → frame indices. The effect applies only between start frame and end frame
    uint32_t start_frame = (uint32_t)(start_time * sample_rate);
    uint32_t end_frame   = (uint32_t)(end_time   * sample_rate);
    if (end_frame > total_frames) end_frame = total_frames;
    
    // Write header
    write_header(sample_rate, bytes_per_sec, channels, block_align, data_chunk_size, file_size);
    
    // Process frame-by-frame
    for (uint32_t frame = 0; frame < total_frames; frame++) {//reading all frames
        for (int ch = 0; ch < channels; ch++) {// loop through te channels
            if (bytes_per_sample == 1) {//case 1 for 8 bit samples
                int sample = getchar();
                if (sample == EOF) {
                    fprintf(stderr, "Error! insufficient data\n");
                    exit(1);
                }

                if (APPLY_EFFECT(frame, soundwave_dj_mode, start_frame, end_frame)) {//effect is applied for the frames specified
                    int new_sample = (int)trunc(sample * fp_volume);//processing samples
                    if (new_sample > 255) new_sample = 255;//clampling
                    if (new_sample < 0)   new_sample = 0;//...
                    putchar(new_sample);
                } else {
                    putchar(sample);
                }

            } else { // 16-bit ,just as we did before
                int lo = getchar();
                int hi = getchar();
                if (lo == EOF || hi == EOF) {
                    fprintf(stderr, "Error! insufficient data\n");
                    exit(1);
                }

                int16_t sample = lo | (hi << 8);

                if (APPLY_EFFECT(frame, soundwave_dj_mode, start_frame, end_frame)) {//aplying...
                    int new_sample = (int)trunc(sample * fp_volume);//processing...
                    if (new_sample > 32767) new_sample = 32767;//clamping...
                    if (new_sample < -32768) new_sample = -32768;
                    putchar(new_sample & 0xFF);
                    putchar((new_sample >> 8) & 0xFF);
                } else {
                    putchar(lo);
                    putchar(hi);
                }
            }
        }
    }       

    // Copy leftover chunks (extra bytes)
    uint32_t extra_bytes = copy_extra_bytes();
    check_final_size(bytes_read_after_size, data_chunk_size, extra_bytes, file_size);
}

//let's add some fuzz
void fuzz(double fp_fuzz, int soundwave_dj_mode, double start_time, double end_time) {

    read_header(); // read WAV header

    uint16_t bytes_per_sample = bits_per_samp / 8;
    uint32_t total_samples = data_chunk_size / bytes_per_sample;

    // Adjust for channels: calculate total frames
    uint32_t total_frames = total_samples / channels;

    // Convert time → frame indices
    uint32_t start_frame = (uint32_t)(start_time * sample_rate);
    uint32_t end_frame   = (uint32_t)(end_time   * sample_rate);
    if (end_frame > total_frames) end_frame = total_frames;

    write_header(sample_rate, bytes_per_sec, channels, block_align, data_chunk_size, file_size);

    // Process frame-by-frame
    for (uint32_t frame = 0; frame < total_frames; frame++) {//reading all frames
        for (int ch = 0; ch < channels; ch++) {//reading through all channels
            if (bytes_per_sample == 1) {//8bit samples
                int sample = getchar();
                if (sample == EOF) {
                    fprintf(stderr, "Error! insufficient data\n");
                    exit(1);
                }

                if (APPLY_EFFECT(frame, soundwave_dj_mode, start_frame, end_frame)) {//applying..
                    double x = (sample - 128) / 128.0;//processing...
                    x *= fp_fuzz;
                    if (x > 1.0) x = 1.0;
                    if (x < -1.0) x = -1.0;
                    x = x * x * x;  // cubic shaping
                    int new_sample = (int)(x * 127.0 + 128.0);
                    if (new_sample > 255) new_sample = 255;
                    if (new_sample < 0) new_sample = 0;
                    putchar(new_sample);
                } else {
                    putchar(sample);
                }

            } else { // 16-bit samples
                int lo = getchar();
                int hi = getchar();
                if (lo == EOF || hi == EOF) {
                    fprintf(stderr, "Error! insufficient data\n");
                    exit(1);
                }

                int16_t sample = lo | (hi << 8);

                if (APPLY_EFFECT(frame, soundwave_dj_mode, start_frame, end_frame)) {//applying...
                    double x = sample / 32768.0;//processing...
                    x *= fp_fuzz;
                    if (x > 1.0) x = 1.0;
                    if (x < -1.0) x = -1.0;
                    x = x * x * x; // cubic shaping
                    int new_sample = (int)(x * 32767.0);
                    if (new_sample > 32767) new_sample = 32767;
                    if (new_sample < -32768) new_sample = -32768;
                    putchar(new_sample & 0xFF);
                    putchar((new_sample >> 8) & 0xFF);
                } else {
                    putchar(lo);
                    putchar(hi);
                }
            }
        }
    }

    // Copy leftover chunks
    uint32_t extra_bytes = copy_extra_bytes();
    check_final_size(bytes_read_after_size, data_chunk_size, extra_bytes, file_size);
}

//This function gives you the drive you need
void overdrive(double drive, int soundwave_dj_mode, double start_time, double end_time) {

    read_header(); // read WAV header

    uint16_t bytes_per_sample = bits_per_samp / 8;
    uint32_t total_samples = data_chunk_size / bytes_per_sample;

    // Adjust for channels: calculate total frames
    uint32_t total_frames = total_samples / channels;

    // Convert time → frame indices
    uint32_t start_frame = (uint32_t)(start_time * sample_rate);
    uint32_t end_frame   = (uint32_t)(end_time   * sample_rate);
    if (end_frame > total_frames) end_frame = total_frames;

    write_header(sample_rate, bytes_per_sec, channels, block_align, data_chunk_size, file_size);
    //processing frame-by-frame
    for (uint32_t frame = 0; frame < total_frames; frame++) {//reading all frames
        for (int ch = 0; ch < channels; ch++) {//and all channels
            if (bytes_per_sample == 1) {//8 bit
                int sample = getchar();
                if (sample == EOF) {
                    fprintf(stderr, "Error! insufficient data\n"); 
                    exit(1);
                }

                if (APPLY_EFFECT(frame, soundwave_dj_mode, start_frame, end_frame)) {//aplying effect...
                    double x = (sample - 128) / 128.0; // normalize [-1,1]
                    x *= drive;                        // apply drive
                    x = tanh(x);                       // soft clipping
                    int new_sample = (int)(x * 127.0 + 128.0);
                    if (new_sample > 255) new_sample = 255;
                    if (new_sample < 0) new_sample = 0;
                    putchar(new_sample);
                } else {
                    putchar(sample);
                }

            } else { // 16-bit
                int lo = getchar();
                int hi = getchar();
                if (lo == EOF || hi == EOF) {
                    fprintf(stderr, "Error! insufficient data\n");
                    exit(1);
                }

                int16_t sample = lo | (hi << 8);

                if (APPLY_EFFECT(frame, soundwave_dj_mode, start_frame, end_frame)) {//applying
                    double x = sample / 32768.0;
                    x *= drive;
                    x = tanh(x);
                    int new_sample = (int)(x * 32767.0);
                    if (new_sample > 32767) new_sample = 32767;
                    if (new_sample < -32768) new_sample = -32768;
                    putchar(new_sample & 0xFF);
                    putchar((new_sample >> 8) & 0xFF);
                } else {
                    putchar(lo);
                    putchar(hi);
                }
            }
        }
    }

    // Copy leftover bytes
    uint32_t extra_bytes = copy_extra_bytes();
    check_final_size(bytes_read_after_size, data_chunk_size, extra_bytes, file_size);
}

//I sense a tremolo coming
void tremolo(double rate, double depth, int soundwave_dj_mode, double start_time, double end_time) {

    read_header();  // read WAV header

    uint16_t bytes_per_sample = bits_per_samp / 8;
    uint32_t total_samples = data_chunk_size / bytes_per_sample;
    uint32_t total_frames  = total_samples / channels;  // frames instead of samples

    // Convert time → frame indices
    uint32_t start_frame = (uint32_t)(start_time * sample_rate);
    uint32_t end_frame   = (uint32_t)(end_time   * sample_rate);
    if (end_frame > total_frames) end_frame = total_frames;
    
    write_header(sample_rate, bytes_per_sec, channels, block_align, data_chunk_size, file_size);
     //some math here for later
    double phase = 0.0;
    double phase_inc = 2.0 * M_PI * rate / sample_rate;
    
    for (uint32_t frame = 0; frame < total_frames; frame++) {//frames frames ...
        for (int ch = 0; ch < channels; ch++) {//channels channels...
            double sample = 0.0;
                
            // --- read and normalize ---
            if (bytes_per_sample == 1) {//8 bit
                int c = getchar();
                if (c == EOF) {
                    fprintf(stderr, "Error! insufficient data\n");
                    exit(1);
                }
                sample = ((double)c - 128.0) / 128.0; // 8-bit unsigned -> [-1,1]
            } else {//16 bit
                int lo = getchar();
                int hi = getchar();
                if (lo == EOF || hi == EOF) {
                    fprintf(stderr, "Error! insufficient data\n");
                    exit(1);
                }
                int16_t val = lo | (hi << 8);
                sample = val / 32768.0; // 16-bit signed -> [-1,1]
            }
            
            // --- apply tremolo if in DJ-mode range ---
            if (APPLY_EFFECT(frame, soundwave_dj_mode, start_frame, end_frame)) {//apply at every freame specified
                double lfo = (1.0 - depth) + depth * 0.5 * (1.0 + sin(phase));//math with phase
                sample *= lfo;
            }

            // --- clamp to [-1,1] ---
            if (sample > 1.0) sample = 1.0;
            if (sample < -1.0) sample = -1.0;

            // --- denormalize and write ---
            if (bytes_per_sample == 1) {
                int out = (int)(sample * 127.0 + 128.0);
                if (out > 255) out = 255;
                if (out < 0) out = 0;
                putchar(out);
            } else {
                int out = (int)(sample * 32767.0);
                if (out > 32767) out = 32767;
                if (out < -32768) out = -32768;
                putchar(out & 0xFF);
                putchar((out >> 8) & 0xFF);
            }
        }

        // --- advance phase per frame ---
        phase += phase_inc;
        if (phase > 2.0 * M_PI) phase -= 2.0 * M_PI;
    }

    // --- copy any leftover bytes ---
    uint32_t extra_bytes = copy_extra_bytes();
    check_final_size(bytes_read_after_size, data_chunk_size, extra_bytes, file_size);
}

//let's crush some bits
void bitcrush(int bits, int soundwave_dj_mode, double start_time, double end_time) {

    read_header(); // read WAV header

    uint16_t bytes_per_sample = bits_per_samp / 8;
    uint32_t total_samples = data_chunk_size / bytes_per_sample;
    uint32_t total_frames  = total_samples / channels;  // frames instead of samples

    // Convert time → frame indices
    uint32_t start_frame = (uint32_t)(start_time * sample_rate);
    uint32_t end_frame   = (uint32_t)(end_time * sample_rate);
    if (end_frame > total_frames) end_frame = total_frames;

    // Write header unchanged
    write_header(sample_rate, bytes_per_sec, channels, block_align, data_chunk_size, file_size);

    int32_t max_val = (1 << (bits - 1)) - 1;
    int32_t min_val = -(1 << (bits - 1));

    for (uint32_t frame = 0; frame < total_frames; frame++) {
        for (int ch = 0; ch < channels; ch++) {
            double sample = 0.0;

            // --- read and normalize ---
            if (bytes_per_sample == 1) {//8 bit
                int c = getchar();
                if (c == EOF) {
                    fprintf(stderr, "Error! insufficient data\n");
                    exit(1);
                }
                sample = ((double)c - 128.0) / 128.0; // 8-bit unsigned -> [-1,1]
            } else {//16-bit
                int lo = getchar();
                int hi = getchar();
                if (lo == EOF || hi == EOF) {
                    fprintf(stderr, "Error! insufficient data\n");
                    exit(1);
                }
                int16_t val = lo | (hi << 8);
                sample = val / 32768.0; // 16-bit signed -> [-1,1]
            }

            // --- apply bitcrush only if DJ mode allows ---
            if (APPLY_EFFECT(frame, soundwave_dj_mode, start_frame, end_frame)) {//apply effect
                double scaled = sample * max_val;
                scaled = round(scaled);
                if (scaled > max_val) scaled = max_val;
                if (scaled < min_val) scaled = min_val;
                sample = scaled / max_val;
            }

            // --- denormalize and write ---
            if (bytes_per_sample == 1) {
                int out = (int)(sample * 127.0 + 128.0);
                if (out > 255) out = 255;
                if (out < 0) out = 0;
                putchar(out);
            } else {
                int out = (int)(sample * 32767.0);
                if (out > 32767) out = 32767;
                if (out < -32768) out = -32768;
                putchar(out & 0xFF);
                putchar((out >> 8) & 0xFF);
            }
        }
    }

    // Copy leftover bytes
    uint32_t extra_bytes = copy_extra_bytes();
    check_final_size(bytes_read_after_size, data_chunk_size, extra_bytes, file_size);
}

//ECHO...Echo...echo
void echo(double delay_sec, double decay, int dj_mode, double start_time, double end_time) {
    if (delay_sec <= 0 || decay < 0 || decay > 1) {
        fprintf(stderr,"Error! Invalid delay or decay\n");
        exit(1);
    }

    read_header();  // read WAV header

    uint16_t bytes_per_sample = bits_per_samp / 8;
    uint32_t total_samples = data_chunk_size / bytes_per_sample;
    uint32_t total_frames  = total_samples / channels;  // frames for multi-channel
    uint32_t start_frame   = (uint32_t)(start_time * sample_rate);
    uint32_t end_frame     = (uint32_t)(end_time   * sample_rate);
    if (end_frame > total_frames) end_frame = total_frames;

    write_header(sample_rate, bytes_per_sec, channels, block_align, data_chunk_size, file_size);

    uint32_t delay_samples = (uint32_t)(delay_sec * sample_rate);//compute delay buffer size
    //alocate the delay buffer per cahnnel + error handling in case of not enough memory
    //Every index stores a delayed floating-point sample.
    double **delay_buf = calloc(channels, sizeof(double*));
    if (!delay_buf) { fprintf(stderr,"Error! cannot allocate delay buffer\n"); exit(1); }
    for (int ch = 0; ch < channels; ch++) {
        delay_buf[ch] = calloc(delay_samples, sizeof(double));
        if (!delay_buf[ch]) { fprintf(stderr,"Error! cannot allocate delay buffer\n"); exit(1); }
    }

    uint32_t pos = 0;//This is where the next sample will be read from and written into.
    
    for (uint32_t frame = 0; frame < total_frames; frame++) {//Each frame contains channels samples.
        double samples[channels];

        // --- read and normalize all channels ---
        for (int ch = 0; ch < channels; ch++) {
            if (bytes_per_sample == 1) {
                int c = getchar();
                if (c == EOF) { fprintf(stderr, "Error! insufficient data\n"); exit(1); }
                samples[ch] = ((double)c - 128.0) / 128.0;
            } else {
                int lo = getchar();
                int hi = getchar();
                if (lo == EOF || hi == EOF) { fprintf(stderr, "Error! insufficient data\n"); exit(1); }
                int16_t val = lo | (hi << 8);
                samples[ch] = val / 32768.0;
            }
        }

        // --- apply echo only in DJ range ---
        for (int ch = 0; ch < channels; ch++) {
            if (APPLY_EFFECT(frame, dj_mode, start_frame, end_frame)){
                samples[ch] += decay * delay_buf[ch][pos];
            }
        }

        // --- store in delay buffer ---
        for (int ch = 0; ch < channels; ch++) {
            delay_buf[ch][pos] = samples[ch];
        }

        // --- denormalize and write ---
        for (int ch = 0; ch < channels; ch++) {
            double out = samples[ch];
            if (out > 1.0) out = 1.0;
            if (out < -1.0) out = -1.0;

            if (bytes_per_sample == 1) {
                int out8 = (int)(out * 127.0 + 128.0);
                if (out8 > 255) out8 = 255;
                if (out8 < 0) out8 = 0;
                putchar(out8);
            } else {
                int out16 = (int)(out * 32767.0);
                if (out16 > 32767) out16 = 32767;
                if (out16 < -32768) out16 = -32768;
                putchar(out16 & 0xFF);
                putchar((out16 >> 8) & 0xFF);
            }
        }

        //(advance delay buffer position) This ensures the delay is exactly delay_samples long.
        pos = (pos + 1) % delay_samples;
    }

    // --- free delay buffer ---
    for (int ch = 0; ch < channels; ch++) free(delay_buf[ch]);
    free(delay_buf);

    uint32_t extra_bytes = copy_extra_bytes();
    check_final_size(bytes_read_after_size, data_chunk_size, extra_bytes, file_size);
}

//reverberating
void reverb(double room,  int dj_mode, double start_time, double end_time) {
    
    read_header(); // read WAV header
    
    //new values...
    uint16_t bytes_per_sample = bits_per_samp / 8;
    uint32_t total_samples = data_chunk_size / bytes_per_sample;
    uint32_t total_frames  = total_samples / channels;

    // Convert time → frame indices
    uint32_t start_frame = (uint32_t)(start_time * sample_rate);
    uint32_t end_frame   = (uint32_t)(end_time   * sample_rate);
    if(end_frame > total_frames) end_frame = total_frames;
    
     // Write header unchanged
    write_header(sample_rate, bytes_per_sec, channels, block_align, data_chunk_size, file_size);
    
 // --- Reverb parameters (computes how many comb and all pass filters are used) ---
 int num_combs = 4 + (int)(4 * room); // 4..8 combs 
 int num_aps   = 2 + (int)(2 * room); // 2..4 all-pass

 // Delay lengths (ensure >=1)
 //Builds arrays of delay lengths (in samples) for each comb and each all-pass.
 //Formula scales delays with room and adds slight per-filter variation (0.8 + 0.05*i).
 //Ensures each delay is at least 1 sample to avoid division/modulo-by-zero.
 int comb_delays[num_combs];
 int ap_delays[num_aps];
 for (int i = 0; i < num_combs; i++) {
    comb_delays[i] = (int)((0.05 + 0.45 * room) * sample_rate * (0.8 + 0.05 * i));
    if (comb_delays[i] < 1) comb_delays[i] = 1;
 }
 for (int i = 0; i < num_aps; i++) {
    ap_delays[i] = (int)((0.02 + 0.03 * room) * sample_rate * (0.9 + 0.05 * i));
    if (ap_delays[i] < 1) ap_delays[i] = 1;
 }

 // Feedback & mix (unchanged)
 double comb_fb = 0.55 + room * 0.30;//feedback coefficient for combs (0.55..0.85). Higher → slower decay.
 double ap_fb   = 0.45 + room * 0.25;//all-pass feedback coefficient (0.45..0.70). Controls diffusion
 double wet     = 0.2  + room * 0.30; //wet grows with room 
 double dry     = 1.0 - wet;//dry is complementary

 // --- Allocate on heap: comb_buffers[ch][comb][delay_index] ---
 // comb_buffers: double*** with size [channels][num_combs][comb_delays[i]]
 //Allocates a top-level array comb_buffers of length channels, each entry will be a double** (array of pointers to per-comb buffers)
 //comb_idx is an array of channels pointers, each later allocated to hold num_combs ints (the circular buffer indices).
 //Checks for allocation failure (Out Of Memory = OOM).
 double ***comb_buffers = calloc(channels, sizeof(double**));
 int **comb_idx = calloc(channels, sizeof(int*)); // comb_idx[ch][i]
 if (!comb_buffers || !comb_idx) { fprintf(stderr,"OOM\n"); exit(1); } 
 
 //channells...channels...
 //Allocate comb_buffers[ch] (array of num_combs pointers).
 //Allocate comb_idx[ch] (array of ints to hold indices).
 //On OOM, clean up previously allocated memory and exit. This prevents leaks.
 //For each comb i, allocate the delay buffer comb_buffers[ch][i] with comb_delays[i] doubles and set comb_idx[ch][i] = 0.
 for (int ch = 0; ch < channels; ch++) {
    comb_buffers[ch] = calloc(num_combs, sizeof(double*));
    comb_idx[ch] = calloc(num_combs, sizeof(int));
    if (!comb_buffers[ch] || !comb_idx[ch]) {
        fprintf(stderr,"OOM\n");
        // free previously allocated channel blocks
        for (int k = 0; k < ch; k++) {
            for (int j = 0; j < num_combs; j++) free(comb_buffers[k][j]);
            free(comb_buffers[k]);
            free(comb_idx[k]);
        }
        free(comb_buffers);
        free(comb_idx);
        exit(1);
    }
    for (int i = 0; i < num_combs; i++) {
        comb_buffers[ch][i] = calloc((size_t)comb_delays[i], sizeof(double));
        comb_idx[ch][i] = 0;
        if (!comb_buffers[ch][i]) {
            fprintf(stderr,"OOM\n");
            // cleanup partially allocated buffers for this channel
            for (int j = 0; j < i; j++) free(comb_buffers[ch][j]);
            free(comb_buffers[ch]);
            free(comb_idx[ch]);
            // cleanup prior channels
            for (int k = 0; k < ch; k++) {
                for (int j = 0; j < num_combs; j++) free(comb_buffers[k][j]);
                free(comb_buffers[k]);
                free(comb_idx[k]);
            }
            free(comb_buffers);
            free(comb_idx);
            exit(1);
        }
    }
 }

 // --- Allocate AP buffers similarly ---
 //Allocates the all-pass buffers and indices the same way as combs
 double ***ap_buffers = calloc(channels, sizeof(double**));
 int **ap_idx = calloc(channels, sizeof(int*));
 if (!ap_buffers || !ap_idx) { fprintf(stderr,"OOM\n"); exit(1); }

 for (int ch = 0; ch < channels; ch++) {
    ap_buffers[ch] = calloc(num_aps, sizeof(double*));
    ap_idx[ch] = calloc(num_aps, sizeof(int));
    if (!ap_buffers[ch] || !ap_idx[ch]) {
        fprintf(stderr,"OOM\n");
        // minimal cleanup then exit (left as exercise)
        exit(1);
    }
    for (int i = 0; i < num_aps; i++) {
        ap_buffers[ch][i] = calloc((size_t)ap_delays[i], sizeof(double));
        ap_idx[ch][i] = 0;
        if (!ap_buffers[ch][i]) {
            fprintf(stderr,"OOM\n");
            // cleanup as above (omitted for brevity)
            exit(1);
        }
    }
 }

 // --- Frame processing (the same way we have been doing it) ---
 for (uint32_t frame = 0; frame < total_frames; ++frame) {
    double in_samples_local[channels];

    // Read & normalize
    for (int ch = 0; ch < channels; ch++) {
        if (bytes_per_sample == 1) {
            int c = getchar();
            if (c == EOF) { fprintf(stderr,"Error! insufficient data\n"); exit(1); }
            in_samples_local[ch] = ((double)c - 128.0) / 128.0;
        } else {
            int lo = getchar();
            int hi = getchar();
            if (lo == EOF || hi == EOF) { fprintf(stderr,"Error! insufficient data\n"); exit(1); }
            int16_t val = lo | (hi << 8);
            in_samples_local[ch] = val / 32768.0;
        }
    }

    double processed_local[channels];

    for (int ch = 0; ch < channels; ch++) {
        // Comb sum
        double comb_sum = 0.0;
        for (int i = 0; i < num_combs; i++) {
            int idx = comb_idx[ch][i];
            double yc = comb_buffers[ch][i][idx];
            double c_out = in_samples_local[ch] + comb_fb * yc;
            comb_buffers[ch][i][idx] = c_out;
            comb_idx[ch][i] = (idx + 1) % comb_delays[i];
            comb_sum += c_out;
        }
        comb_sum /= num_combs;

        double ap = comb_sum;
        for (int i = 0; i < num_aps; i++) {
            int idx = ap_idx[ch][i];
            double z = ap_buffers[ch][i][idx];
            double a_out = -ap_fb * ap + z;
            ap_buffers[ch][i][idx] = ap + ap_fb * a_out;
            ap_idx[ch][i] = (idx + 1) % ap_delays[i];
            ap = a_out;
        }
        processed_local[ch] = ap;
    }

    // Mix & write (same as your code)
    for (int ch = 0; ch < channels; ch++) {
        double out_sample = in_samples_local[ch];
        if (!dj_mode || APPLY_EFFECT(frame, dj_mode, start_frame, end_frame))
            out_sample = dry * in_samples_local[ch] + wet * processed_local[ch];

        if (out_sample > 1.0) out_sample = 1.0;
        if (out_sample < -1.0) out_sample = -1.0;

        if (bytes_per_sample == 1) {
            int out8 = (int)(out_sample * 127.0 + 128.0);
            if (out8 > 255) out8 = 255;
            if (out8 < 0) out8 = 0;
            putchar(out8);
        } else {
            int out16 = (int)(out_sample * 32767.0);
            if (out16 > 32767) out16 = 32767;
            if (out16 < -32768) out16 = -32768;
            putchar(out16 & 0xFF);
            putchar((out16 >> 8) & 0xFF);
        }
    }
 }

 // --- Cleanup (free in reverse order of allocation) ---
 for (int ch = 0; ch < channels; ch++) {
    for (int i = 0; i < num_combs; i++) free(comb_buffers[ch][i]);
    free(comb_buffers[ch]);
    free(comb_idx[ch]);

    for (int i = 0; i < num_aps; i++) free(ap_buffers[ch][i]);
    free(ap_buffers[ch]);
    free(ap_idx[ch]);
 }
 free(comb_buffers);
 free(comb_idx);
 free(ap_buffers);
 free(ap_idx);


    // --- Leftover & validate ---
    uint32_t extra_bytes = copy_extra_bytes();
    check_final_size(bytes_read_after_size, data_chunk_size, extra_bytes, file_size);

}

//COME HERE TO FIND YOUR WAY
void help_info() {
    printf("Invocation: soundwave info \n");
    printf("Prints metadata about an audio file, revealing its sample rate,\n");
    printf("duration, channels, and other secrets it has been keeping.\n");
    printf("     Useful for confirming that the file is, in fact, alive.\n");
}

void help_rate() {
    printf("Invocation: soundwave rate <fp_rate>\n");
    printf("Adjusts playback speed by resampling, effectively persuading\n");
    printf("time itself to hurry up or slow down, depending on your mood.\n");
    printf("  <fp_rate>: — speed multiplier\n");
    printf("     Higher values create chipmunk-esque urgency.\n");
    printf("     Lower values create dramatic, pondering slowness.\n");
}

void help_volume() {
    printf("Invocation: soundwave volume <fp_volume>\n");
    printf("Scales the amplitude of the audio, much like turning a knob\n");
    printf("that insists it is far more important than it really is.\n");
    printf("  <fp_volume>: — multiplier applied to every sample\n");
    printf("     Values above 1.0 result in enthusiastic loudness.\n");
    printf("     Values below 1.0 result in 'perhaps we keep it down?'.\n");
}

void help_generate() {
    printf("Invocation: soundwave generate [options]\n");
    printf("Constructs a tone entirely from numbers, enthusiasm, and whatever\n");
    printf("your CPU considers ‘music’. All knobs are optional; defaults are\n");
    printf("politely provided below.\n\n");

    printf("Options:\n");
    printf("  --dur <seconds>\n");
    printf("      How long the sound should exist before wandering off.\n");
    printf("      (default: 3)\n\n");

    printf("  --sr <rate>\n");
    printf("      Sample rate in Hz. Higher values mean smoother audio, or at\n");
    printf("      least a greater sense of technical superiority. (default: 44100)\n\n");

    printf("  --fm <freq>\n");
    printf("      Modulator frequency — how quickly the sound quivers with\n");
    printf("      self-doubt. (default: 2.0)\n\n");

    printf("  --fc <freq>\n");
    printf("      Carrier frequency — the main pitch, or the bit you hum.\n");
    printf("      (default: 1500.0)\n\n");

    printf("  --mi <index>\n");
    printf("      Modulation index — how dramatic the wobble is. Higher values\n");
    printf("      make it behave like it’s had too much caffeine. (default: 100.0)\n\n");

    printf("  --amp <value>\n");
    printf("      Loudness. Turn it up too far and the neighbours may file a note\n");
    printf("      of complaint with local authorities. (default: 30000.0)\n\n");

    printf("Example:\n");
    printf("  soundwave generate --dur 5 --fc 800 --mi 50\n");
    printf("Produces a polite 5-second tone of medium wobbliness.\n");
}

void help_fuzz() {
    printf("Invocation: soundwave fuzz <fp_fuzz>\n");
    printf("Introduces harmonic annihilation and waveform indignities.\n");
    printf("A clean signal enters; a mangled, overexcited waveform leaves.\n");
    printf("  <fp_fuzz>: 0.0 to who knows \n");
    printf("     Higher gain(fp_fuzz) forces the waveform to choose violence.\n");
    
}

void help_overdrive() {
    printf("Invocation: soundwave overdrive <amount>\n");
    printf("Applies soft-clipping nonlinearity, gently encouraging the\n");
    printf("signal to exceed its boundaries and reconsider its life choices.\n");
    printf("  <amount>: 0.0 to who knows\n");
    printf("     Higher values produce the classic 'mathematician with\n");
    printf("     too much caffeine' oscillation.\n");
}

void help_bitcrush() {
    printf("Invocation: soundwave bitcrush <bits>\n");
    printf("Reduces bit depth, converting your precise high-fidelity audio\n");
    printf("into something more akin to a binary creature scribbling on paper.\n");
    printf("  <bits>: 1 – 16\n");
    printf("     Lower values yield increasing quantization despair.\n");
}

void help_tremolo() {
    printf("Invocation: soundwave tremolo <rate> <depth>\n");
    printf("Periodically changes the volume up and down using a sine wave.\n");
    printf("Think of it as gently turning the volume knob back and forth.\n");
    printf("\n");
    printf("  <rate>: frequency in Hz — how fast the volume wobbles\n");
    printf("\n");
    printf("  <depth>: 0.0 – 1.0 — how strong the wobble is\n");
    printf("           0.0 = almost no change\n");
     printf("     Depth near 1.0 results in full 'now you hear me, now you don't'.\n");
}

void help_echo() {
    printf("Invocation: soundwave echo <delay_sec> <decay>\n");
    printf("Implements a simple delay line with scalar feedback, creating\n");
    printf("reverberant repetitions like a corridor that refuses to forget.\n");
    printf("\n");
    printf("  <delay_sec>: how long the sound wanders down the corridor\n");
    printf("                before it decides to return (in seconds)\n");
    printf("\n");
    printf("  <decay>: 0.0 – 1.0 — how much quieter each returning echo is\n");
    printf("           Values above 0.7 make the corridor unusually chatty\n");
    printf("           and reluctant to stop repeating itself.\n");
}

void help_reverb() {
    printf("Invocation: soundwave reverb <room>\n");
    printf("Engages a primitive reverberation model employing comb and\n");
    printf("all-pass filters because real acoustics are far too exhausting.\n");
    printf("  <room>: 0.0 – 1.0\n");
    printf("     Higher values emulate larger and more judgmental rooms.\n");
}

void help_dj() {
    printf("Invocation: soundwave dj <start_time> <end_time> [effect] [args]\n");
    printf("Enables DJ mode, allowing effects to be applied only within\n");
    printf("a specified time range. Perfect for those moments when you\n");
    printf("want to spice up just a segment of your audio.\n");
    printf("\n");
    printf("  <start_time>: when the effect should begin (in seconds)\n");
    printf("  <end_time>:   when the effect should stop (in seconds)\n");
    printf("\n");
    printf("P.S. -- dj mode doesn't apply to rate, channel, generate.");
    printf("\n");
}

void print_help(const char *prog_name) {
            printf("SoundWave Audio Processor\n"); 
            printf("A small collection of sonic contraptions for bending audio\n"); 
            printf("in ways that may alarm nearby sound engineers.\n\n"); 
            printf("Available incantations:\n"); 
            printf(" info — peeks into an audio file’s secrets: sample rate, duration, channels… basically its diary\n"); 
            printf(" rate — persuades time to hurry or slow down\n"); 
            printf(" volume — turns the amplitude knob of reality\n"); 
            printf(" generate — conjures tones from pure math and CPU whimsy; all knobs optional, defaults are generous\n"); 
            printf(" fuzz — turns signals into scorched rubble\n"); 
            printf(" overdrive — persuades audio to misbehave politely\n"); 
            printf(" bitcrush — removes bits it deems unnecessary (all of them)\n"); 
            printf(" tremolo — makes the volume wobble like a jelly in an earthquake\n"); 
            printf(" echo — convinces your sound to come back for another round\n"); 
            printf(" reverb — places audio into rooms of questionable architectural merit\n\n"); 
            printf("For enlightenment on any effect:\n"); 
            printf(" soundwave <effect> --help\n"); 
            printf("…and it shall reveal its secrets.\n\n"); 
            printf("For those who dare to experiment, try dj --help for a special mode of chaos.\n"); 
            fprintf(stderr, "Usage: %s <command> [args]\n", prog_name);
}

//This fanction converts the number given by the user in double.
//Makes sure nothing is out line.
//1...a, 1Q, .... won't work
int parse_double(const char *s, double *out) {
    char *end;
    double val = strtod(s, &end);// store the last character tha is not the number to the pointer n
    if (*end != '\0') 
    return 0;  // in case of invalid trailing chars
    *out = val;//put the converted double to out
    return 1;
}

//The same thing here.
//This time for integers instead of doubles.
int parse_int(const char *s, int *out) {
    char *end;
    long val = strtol(s, &end, 10);//.... Also base 10(we store them as decimal numbers-integers)

    if (end == s) 
    return 0;
    if (*end != '\0') 
    return 0;// in case of invalid trailing chars

    *out = (int)val;//put the converted integer to out
    return 1;
}

int handle_effect(int argc, char **argv) {
    if (strcmp(argv[1], "info") == 0) {
        info();
        return 0;
    }
    else if (strcmp(argv[1], "rate") == 0) {
        if (argc != 3) {
            fprintf(stderr, "Usage: %s rate <fp_rate>, try --help for more.\n", argv[0]);
            return 1;
        }
        double fp_rate ;  // convert string to double
        
        if (!parse_double(argv[2], &fp_rate)) {
            fprintf(stderr, "Error: invalid fp_rate\n");
            return 1;
        }
        
        if (fp_rate <= 0) {
            fprintf(stderr, "Error: fp_rate must be positive\n");
            return 1;
        }
        rate(fp_rate);
        return 0;
    }
    else if (strcmp(argv[1], "channel") == 0) {
     // Usage: program channel <left|right>
     if (argc != 3) {
        fprintf(stderr, "Usage: %s channel <left|right>, try --help for more\n", argv[0]);
        return 1;
     }

     char *mode_str = argv[2];
     char mode;

     if (strcmp(mode_str, "left") == 0) {
        mode = 'L';
     } else if (strcmp(mode_str, "right") == 0) {
        mode = 'R';
     } else {
        fprintf(stderr, "Error: mode must be 'left' or 'right'\n");
        return 1;
     }

     channel(mode);
     return 0;  // call your channels function
    } 
    else if (strcmp(argv[1], "volume") == 0) {
        if (argc != 3) { 
            fprintf(stderr,"Usage: %s volume <fp_volume>\n",argv[0]); 
            return 1; }
        double fp_volume;

        if (!parse_double(argv[2], &fp_volume)) {
            fprintf(stderr,"Error: fp_volume must be a valid number\n");
            return 1;
        }
        if (fp_volume <= 0) {
            fprintf(stderr,"Error: fp_volume must be positive\n");
            return 1;
        }
        volume(fp_volume, 0, 0, 0);
        return 0;
    }
    else if (strcmp(argv[1], "generate") == 0) {
     // Defaults
     int dur = 3;
     int sr = 44100;
     double fm = 2.0;
     double fc = 1500.0;
     double mi = 100.0;
     double amp = 30000.0;

     // Optional arguments parsing
     for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--dur") == 0 && i+1 < argc) {
         if (!parse_int(argv[++i], &dur)) {
                    fprintf(stderr, "Error: invalid --dur\n");
                    return 1;
                }
        }        
        else if (strcmp(argv[i], "--sr") == 0 && i+1 < argc) {
                if (!parse_int(argv[++i], &sr)) {
                    fprintf(stderr, "Error: invalid --sr\n");
                    return 1;
                }
        }
        else if (strcmp(argv[i], "--fm") == 0 && i+1 < argc) {
                if (!parse_double(argv[++i], &fm)) {
                    fprintf(stderr, "Error: invalid --fm\n");
                    return 1;
                }
        }
        else if (strcmp(argv[i], "--fc") == 0 && i+1 < argc) {
                if (!parse_double(argv[++i], &fc)) {
                    fprintf(stderr, "Error: invalid --fc\n");
                    return 1;
                }
        }
        else if (strcmp(argv[i], "--mi") == 0 && i+1 < argc) {
                if (!parse_double(argv[++i], &mi)) {
                    fprintf(stderr, "Error: invalid --mi\n");
                    return 1;
                }
        }
            else if (strcmp(argv[i], "--amp") == 0 && i+1 < argc) {
                if (!parse_double(argv[++i], &amp)) {
                    fprintf(stderr, "Error: invalid --amp\n");
                    return 1;
                }
        }
        else {
                fprintf(stderr,"Unknown option: %s\n", argv[i]);
                return 1;
        }
     }

     // Call your mysound() function
     mysound(dur, sr, fm, fc, mi, amp);
     return 0;
    }
    else if (strcmp(argv[1], "fuzz") == 0) {
        if (argc != 3) { 
            fprintf(stderr,"Usage: %s fuzz <fp_fuzz>, try --help for more.\n",argv[0]); 
            return 1; }

        double fp_fuzz;
        if (!parse_double(argv[2], &fp_fuzz)) {
            fprintf(stderr,"Error: invalid fp_fuzz\n");
            return 1;
        }
        if (fp_fuzz <= 0) { 
            fprintf(stderr,"Error: fp_fuzz must be positive\n"); 
            return 1; }

        fuzz(fp_fuzz, 0, 0, 0);
        return 0;
    }
    else if (strcmp(argv[1], "overdrive") == 0) {
        if (argc != 3) { 
            fprintf(stderr,"Usage: %s overdrive <amount>, try --help for more\n",argv[0]); 
            return 1; }
        double amount;
        if (!parse_double(argv[2], &amount)) {
            fprintf(stderr,"Error: invalid amount\n");
            return 1;
        }
        if (amount <= 0) { 
            fprintf(stderr,"Error: overdrive must be positive\n"); 
            return 1; }
        overdrive(amount, 0, 0, 0);
        return 0;
    }
    else if (strcmp(argv[1], "reverb") == 0) {
        if (argc != 3) { 
            fprintf(stderr,"Usage: %s reverb <room>, try --help for more.\n",argv[0]); 
            return 1; }
        double room;
        
        if (!parse_double(argv[2], &room)) {
            fprintf(stderr,"Error: invalid room value\n");
            return 1;
        }

        if (room <= 0 || room >= 1) { 
            fprintf(stderr,"Error: room must be 0-1\n"); 
            return 1; }
        reverb(room, 0, 0, 0);
        return 0;
    }
    else if (strcmp(argv[1], "echo") == 0) {
        if (argc != 4) { fprintf(stderr,"Usage: %s echo <delay_sec> <decay>, try --help for more.\n",argv[0]); return 1; }
        double delay, decay;
          
        if (!parse_double(argv[2], &delay) || !parse_double(argv[3], &decay)) {
            fprintf(stderr,"Error: invalid echo params\n");
            return 1;
        }
        echo(delay, decay, 0, 0, 0);
        return 0;
    }
    else if (strcmp(argv[1], "tremolo") == 0) {
        if (argc != 4) { fprintf(stderr,"Usage: %s tremolo <rate> <depth>\n",argv[0]); return 1; }
          double rate, depth;
        if (!parse_double(argv[2], &rate) || !parse_double(argv[3], &depth)) {
            fprintf(stderr,"Error: invalid tremolo params\n");
            return 1;
        }
        tremolo(rate, depth, 0, 0, 0);
        return 0;
    }
    else if (strcmp(argv[1], "bitcrush") == 0) {
        if (argc != 3) { fprintf(stderr,"Usage: %s bitcrush <bits>, try --help for more.\n",argv[0]); return 1; }
        int bits;
        
        if (!parse_int(argv[2], &bits)) {
            fprintf(stderr,"Error: invalid bits\n");
            return 1;
        }

        bitcrush(bits, 0, 0, 0);
        return 0;
    }

    else {
        fprintf(stderr, "Unknown effect: %s\n", argv[1]);
        return 1;
    }
    
}

int handle_dj_effect(int argc, char **argv) {
     if (argc < 5) {
        fprintf(stderr,
            "Usage: %s dj <start_sec> <finish_sec> <effect> [effect_args]\n"
            "Try '%s dj --help'\n",
            argv[0], argv[0]);
        return 1;
    }

    double start_sec, finish_sec;
    if (!parse_double(argv[2], &start_sec) ||
        !parse_double(argv[3], &finish_sec)) {
        fprintf(stderr,"Error: start/finish must be numbers\n");
        return 1;
    }
    
    if (start_sec < 0 || finish_sec <= start_sec) {
        fprintf(stderr, "Error: invalid start/finish\n");
        return 1;
    }

    char *effect = argv[4];
    int effect_argc = argc - 4;
    char **effect_argv = &argv[4];

    // ----- VOLUME -----
    if (strcmp(effect, "volume") == 0) {

        if (effect_argc < 2) {
            fprintf(stderr,
                "Usage: %s dj <start> <finish> volume <fp_volume>\n"
                "Try '%s dj --help'\n",
                argv[0], argv[0]);
            return 1;
        }

        double fp_volume;
        if (!parse_double(effect_argv[1], &fp_volume)) {
            fprintf(stderr,"Error: invalid fp_volume\n");
            return 1;
        }
        volume(fp_volume, 1, start_sec, finish_sec);
        return 0;
    }

    // ----- FUZZ -----
    else if (strcmp(effect, "fuzz") == 0) {

        if (effect_argc < 2) {
            fprintf(stderr,
                "Usage: %s dj <start> <finish> fuzz <fp_fuzz>\n"
                "Try '%s dj --help'\n",
                argv[0], argv[0]);
            return 1;
        }

        double fp_fuzz;
        if (!parse_double(effect_argv[1], &fp_fuzz)) {
            fprintf(stderr,"Error: invalid fuzz amount\n");
            return 1;
        }
        fuzz(fp_fuzz, 1, start_sec, finish_sec);
        return 0;
    }

    // ----- OVERDRIVE -----
    else if (strcmp(effect, "overdrive") == 0) {

        if (effect_argc < 2) {
            fprintf(stderr,
                "Usage: %s dj <start> <finish> overdrive <amount>\n"
                "Try '%s dj --help'\n",
                argv[0], argv[0]);
            return 1;
        }

        double amount;
        if (!parse_double(effect_argv[1], &amount)) {
            fprintf(stderr,"Error: invalid overdrive amount\n");
            return 1;
        }
        overdrive(amount, 1, start_sec, finish_sec);
        return 0;
    }

    // ----- ECHO -----
    else if (strcmp(effect, "echo") == 0) {

        if (effect_argc < 3) {
            fprintf(stderr,
                "Usage: %s dj <start> <finish> echo <delay> <decay>\n"
                "Try '%s dj --help'\n",
                argv[0], argv[0]);
            return 1;
        }

      
        double delay, decay;
        if (!parse_double(effect_argv[1], &delay) || !parse_double(effect_argv[2], &decay)) {
            fprintf(stderr,"Error: invalid echo params\n");
            return 1;
        }
        echo(delay, decay, 1, start_sec, finish_sec);
        return 0;
    }

    // ----- TREMOLO -----
    else if (strcmp(effect, "tremolo") == 0) {

        if (effect_argc < 3) {
            fprintf(stderr,
                "Usage: %s dj <start> <finish> tremolo <rate> <depth>\n"
                "Try '%s dj --help'\n",
                argv[0], argv[0]);
            return 1;
        }

       double rate, depth;
        if (!parse_double(effect_argv[1], &rate) || !parse_double(effect_argv[2], &depth)) {
            fprintf(stderr,"Error: invalid tremolo params\n");
            return 1;
        }
        tremolo(rate, depth, 1, start_sec, finish_sec);
        return 0;
    }

    // ----- BITCRUSH -----
    else if (strcmp(effect, "bitcrush") == 0) {

        if (effect_argc < 2) {
            fprintf(stderr,
                "Usage: %s dj <start> <finish> bitcrush <bits>\n"
                "Try '%s dj --help'\n",
                argv[0], argv[0]);
            return 1;
        }

        int bits;
        if (!parse_int(effect_argv[1], &bits)) {
            fprintf(stderr,"Error: invalid bits\n");
            return 1;
        }

        bitcrush(bits, 1, start_sec, finish_sec);
        return 0;
    }
    // ----- REVERB -----
    else if (strcmp(effect, "reverb") == 0) {

        if (effect_argc < 2) {
         fprintf(stderr,
            "Usage: %s dj <start> <finish> reverb <room>\n"
            "Try '%s dj --help'\n",
            argv[0], argv[0]);
          return 1;
        }

        double room;
        if (!parse_double(effect_argv[1], &room)) {
            fprintf(stderr,"Error: invalid room\n");
            return 1;
        }

         if (room <= 0 || room >= 1) {
         fprintf(stderr, "Error: room must be 0-1\n");
         return 1;
        }

        reverb(room, 1, start_sec, finish_sec);
        return 0;
    }
    // ----- UNKNOWN EFFECT -----
    else {
        fprintf(stderr,
            "Unknown DJ effect: %s\n"
            "Try '%s dj --help'\n",
            effect, argv[0]);
        return 1;
    }
}

//Here come the main
int main(int argc, char **argv) { 
    if (argc < 2) { 
        fprintf(stderr, "SoundWave awaits orders. For guidance, use 'soundwave --help'.\n"); 
        return 1; } 
    if (argc == 2 && strcmp(argv[1], "--help") == 0) { 
          print_help(argv[0]);
            return 0; 
    } // Help system 
    if (argc == 3 && strcmp(argv[2], "--help") == 0) { 
            if (strcmp(argv[1], "info") == 0) { 
                help_info(); 
                return 0; } 
            if (strcmp(argv[1], "rate") == 0) { 
                help_rate(); 
                return 0; } 
            if (strcmp(argv[1], "volume") == 0) { 
                help_volume(); 
                return 0; } 
            if (strcmp(argv[1], "generate") == 0) { 
                help_generate(); 
                return 0; } 
            if (strcmp(argv[1], "fuzz") == 0) { 
                help_fuzz(); 
                return 0; } 
            if (strcmp(argv[1], "overdrive") == 0) { 
                help_overdrive(); 
                return 0; } 
            if (strcmp(argv[1], "bitcrush") == 0) { 
                help_bitcrush(); 
                return 0; } 
            if (strcmp(argv[1], "tremolo") == 0) { 
                help_tremolo(); 
                return 0; } 
            if (strcmp(argv[1], "echo") == 0) { 
                help_echo(); 
                return 0; } 
            if (strcmp(argv[1], "reverb") == 0) { 
                help_reverb(); 
                return 0; } 
            if (strcmp(argv[1], "dj") == 0) { 
                help_dj(); 
                return 0; } 
             printf("Unknown effect: %s\n", argv[1]); 
             return 1; 
    }
    if (strcmp(argv[1], "dj") == 0)
        return handle_dj_effect(argc, argv);
    else
        return handle_effect(argc, argv);
}
