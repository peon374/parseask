#include <stdio.h>
#include <stdint.h>
/**
 * @brief Explanation of the changes:
We've added a new function synchronizeBitStream that takes sqsamp as input.
Inside parseASK, we now call synchronizeBitStream(sqsamp) after calculating sqsamp.
The synchronizeBitStream function does the following:
Detects edges in the input signal.
During synchronization, it counts edges until it reaches HALF_PREAMBLE_BITS.
Once synchronized, it calculates the bitlength based on the synchronization period.
After synchronization, it accumulates samples in a bucket for each bit period.
When a full bit period is reached, it decides the bit value (0 or 1) and prints it.
It continuously adjusts to the incoming signal by resetting counters on each edge.
This implementation assumes that the preamble consists of alternating high and low signals. Adjust the HALF_PREAMBLE_BITS constant if needed to match your specific protocol.
Remember to include appropriate error handling and potentially add a timeout mechanism for losing synchronization if no edges are detected for an extended period.
 * 
 */
// This is a total guess
#define FILTER_COEF 10
// THRESHOLD level for incoming samples
#define THRESHOLD 64
// This is a guess but it represents the number of samples we are going to take to get in sync
#define HALF_PREAMBLE_BITS 16

void parseASK(int16_t i, int16_t q)
{
     static int16_t lpfsamp = 0;

    // rectify sample - doubles freq
    int16_t rsamp = (i < 0 ? -i : i)*100;

    // filter sample with LPF set to half the transmission freq (443MHz)
    lpfsamp = (rsamp + (lpfsamp * FILTER_COEF)) / (FILTER_COEF+1);

    // threshold block 
    int sqsamp = lpfsamp > THRESHOLD ? 256 : 0;

    // Add this function call
    synchronizeBitStream(sqsamp);

}

// Add this new function
void synchronizeBitStream(int sqsamp)
{
    static int bitlength = 0;
    static int bitstream = 0;
    static int bitlength_count = 0;
    static int bucket = 0;
    static int prev_sqsamp = 0;
    static int sync_count = 0;
    static int bit_value = 0;

    // Edge detection
    if (sqsamp != prev_sqsamp)
    {
        if (bitstream == 0)
        {
            // Still synchronizing
            sync_count++;
            if (sync_count >= HALF_PREAMBLE_BITS)
            {
                // Synchronized, start decoding
                bitstream = 1;
                bitlength = bitlength_count / HALF_PREAMBLE_BITS;
                bitlength_count = 0;
                bucket = 0;
            }
        }
        else
        {
            // Already synchronized, reset counters
            bitlength_count = 0;
            bucket = 0;
        }
    }

    if (bitstream == 1)
    {
        // Accumulate samples
        bucket += sqsamp;
        bitlength_count++;

        if (bitlength_count >= bitlength)
        {
            // Decide bit value
            bit_value = (bucket > (bitlength * 128)) ? 1 : 0;
            printf("Bit: %d\n", bit_value);

            // Reset for next bit
            bitlength_count = 0;
            bucket = 0;
        }
    }
    else
    {
        // Still synchronizing, count samples between edges
        bitlength_count++;
    }

    prev_sqsamp = sqsamp;
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