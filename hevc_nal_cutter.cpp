#include "hevc_nal.h"

#include <stdio.h>
#include <unistd.h>


static const char* nut_name[] = {
    "TRAIL_N",          // 0
    "TRAIL_R",          // 1
    "TSA_N",            // 2
    "TSA_R",            // 3
    "STSA_N",           // 4
    "STSA_R",           // 5
    "RADL_N",           // 6
    "RADL_R",           // 7
    "RASL_N",           // 8
    "RASL_R",           // 9
    "RSV_VCL_N10",      // 10
    "RSV_VCL_N11",      // 11
    "RSV_VCL_N12",      // 12
    "RSV_VCL_N13",      // 13
    "RSV_VCL_N14",      // 14
    "RSV_VCL_N15",      // 15
    "BLA_W_LP",         // 16
    "BLA_W_RADL",       // 17
    "BLA_N_LP",         // 18
    "IDR_W_RADL",       // 19
    "IDR_N_LP",         // 20
    "CRA_NUT",          // 21
    "RSV_IRAP_VCL22",   // 22
    "RSV_IRAP_VCL23",   // 23
    "RSV_VCL24",        // 24
    "RSV_VCL25",        // 25
    "RSV_VCL25",        // 26
    "RSV_VCL25",        // 27
    "RSV_VCL25",        // 28
    "RSV_VCL25",        // 29
    "RSV_VCL25",        // 30
    "RSV_VCL25",        // 31
    "VPS_NUT",          // 32
    "SPS_NUT",          // 33
    "PPS_NUT",          // 34
    "AUD_NUT",          // 35
    "EOS_NUT",          // 36
    "EOB_NUT",          // 37
    "FD_NUT",           // 39
    "PREFIX_SEI_NUT",   // 39
    "SUFFIX_SEI_NUT",   // 40
};

#define VPS_MASK ((1 << 32))
#define SPS_MASK ((1 << 33))
#define PPS_MASK ((1 << 34))
#define AUD_MASK ((1 << 35))

#define IDR_MASK ((1 << 19) | (1 << 20))
#define CRA_MASK ((1 << 21))
#define BLA_MASK ((1 << 16) | (1 << 17) | (1 << 18))
#define VCL_MASK (0xFFFFFFFF) // [31:0]

#define MAX_NUM_VPS 1 // [FIXME]
#define MAX_NUM_SPS 1 // [FIXME]
#define MAX_NUM_PPS 1 // [FIXME]

static int start = 0;
static char* infile = NULL;
static char* outfile = NULL;

static int verbose = 0;

void parse_option(int argc, char** argv);
void print_usage(char* executable);
void print_options();

int main(int argc, char** argv)
{
    int nut;
    int i;

    const unsigned char shoft_startcode[] = { '\0', '\0', '\1' };
    const unsigned char long_startcode[] = { '\0', '\0', '\0', '\1' };

    int frame_idx = -1;
    int rap_found = 0;

    parse_option(argc, argv);

    if (infile == NULL)
    {
        fprintf(stderr, "infile is missing\n");
        print_usage(argv[0]);
        return 1;
    }
    else if (outfile == NULL)
    {
        fprintf(stderr, "outfile is missing\n");
        print_usage(argv[0]);
        return 1;
    }
    else
    {
        print_options();
    }

    HevcNal nal(infile);

    while (frame_idx < 100)
    {
        nal.read_nal();

        nut = nal.get_nal_unit_type();

        if (nut < 0)
            break;

        // VCL
        if (nut < 32 && nal.get_first_slice_flag())
        {
            frame_idx++;
        }

        if (frame_idx >= start && nut >= 16 && nut <= 23)
        {
            if (!rap_found)
            {
                fprintf(stdout, "found random access point at offset 0x%x, frame_idx: #%d\n", nal.get_nal_offset(), frame_idx);
                rap_found = 1;
            }
        }

        if (rap_found)
        {
            //fwrite(long_startcode, 1, 4, outfile);
            //fwrite(nal.get_payload(), 1, nal.get_payload_size(), outfile);
        }
        else if (nut < 36 && nut > 32)
        {
            // VPS/SPS/PPS

            //fwrite(long_startcode, 1, 4, outfile);
            //fwrite(nal.get_payload(), 1, nal.get_payload_size(), outfile);
        }

        if (verbose)
        {
            if (nut < 41)
                fprintf(stdout, "nal found at 0x%x and nut is %s\n", nal.get_nal_offset(), nut_name[nut]);
            if (nut < 32)
                fprintf(stdout, "frame_idx: %d - first slice flag: %d\n", frame_idx, nal.get_first_slice_flag());
        }
    }

    return 0;
}

void parse_option(int argc, char** argv)
{
    int opt;
    int idx = 0;

    while (1)
    {
        while( (opt = getopt(argc, argv, "s:v")) != -1)
        {
            fprintf(stderr, "getopt returns %d\n", opt);
            // -1 means getopt() parse all options
            switch(opt)
            {
                case 's':
                    start = atoi(optarg);
                    break;
                case 'v':
                    verbose = 1;
                    break;
                default:
                    fprintf(stderr, "unknown option: -%c\n", optopt);
                    break;
            }
        }

        if (optind < argc)
        {
            if (idx == 0)
            {
                infile = argv[optind];
                idx++;
            }
            else if (idx == 1)
            {
                outfile = argv[optind];
                idx++;
            }

            optind++;
        }
        else
        {
            break;
        }
    }
}

void print_usage(char* executable)
{
    fprintf(stderr, "Usage: %s [-s start_frame] [-v] infile outfile\n", executable);
}

void print_options()
{
    fprintf(stdout, "infile: %s\n", infile);
    fprintf(stdout, "outfile: %s\n", outfile);
    fprintf(stdout, "start_frame: %d\n", start);
    fprintf(stdout, "\n");
}
