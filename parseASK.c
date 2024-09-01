#include <stdio.h>
#include <stdint.h>

// This is a total guess
#define FILTER_COEF 0.4
// THRESHOLD level for incoming samples
#define THRESHOLD 64
// This is a guess but it represents the number of samples we are going to take to get in sync
#define HALF_PREAMBLE_BITS 16

void parseASK(int16_t i, int16_t q)
{
    static int bitlength = 0;       // Number of samples a bit should take
    static int bitstream = 0;       // flag for when we should start generating bit data
    static int bitlength_count = 0; // Number of samples that have been measured to work out timing
    static int bucket = 0;          // Bucket used to capture bit samples
    static int16_t lpfsamp = 0;

    // rectify sample - doubles freq
    int16_t rsamp = (i < 0 ? -i : i)*100;

    // filter sample with LPF set to half the transmission freq (443MHz)
    lpfsamp = (rsamp + lpfsamp * 10) / 11;

    // threshold block in gnuradio
    int sqsamp = lpfsamp > THRESHOLD ? 256 : 0;

    // edge trigger
    static int16_t prevsqsamp = 0;

    if (i!=0)
    printf("%d,%d,%d,%d,%d\n",i,q,rsamp,lpfsamp,sqsamp);
}

// Read the file assuming it is a binary dump with 2 bytes per sample
void read_file(const char *file_path)
{
    FILE *file = fopen(file_path, "rb");
    if (file == NULL)
    {
        perror("Error opening file");
        return;
    }

    // complex16s is 2 bytes first is I second is Q
    int8_t value[2];
    while (fread(&value, sizeof(int16_t), 1, file) == 1)
    {
        parseASK(value[0], value[1]);
    }

    fclose(file);
}

int main(int argc, char **argv)
{
    printf("Parsing data file");
    read_file("RTL-SDR-20240822_184832-434MHz-10KSps-10KHz.complex16s");
}