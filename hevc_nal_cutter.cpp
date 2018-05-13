#include "hevc_nal.h"

#include <stdio.h>
#include <unistd.h>

using namespace std;

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

enum
{
    NUT_VCL_START   =  0,
    NUT_VCL_END     = 31,
    NUT_RAP_START   = 16,
    NUT_RAP_END     = 23,
    NUT_VPS         = 32,
    NUT_SPS         = 33,
    NUT_PPS         = 34,
    NUT_AUD         = 35,
    NUT_EOS         = 36,
    NUT_EOB         = 37,
    NUT_FD          = 38,
    NUT_PREFIX_SEI  = 39,
    NUT_SUFFIX_SEI  = 40,
    NUT_NUM         = 41
};

#define MAX_NUM_VPS 1 // [FIXME]
#define MAX_NUM_SPS 1 // [FIXME]
#define MAX_NUM_PPS 1 // [FIXME]

static int start = 0;
static char* infile = NULL;
static char* outfile = NULL;

static int verbose = 0;

static FILE* out = NULL;

static void parse_option(int argc, char** argv);
static void print_usage(char* executable);
static void print_options();

static int outfile_open();
static int outfile_write(unsigned char* payload, int size);
static void outfile_close();

int main(int argc, char** argv)
{
    int nut;
    int i;

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

    // open outfile
    if (!outfile_open())
    {
        fprintf(stderr, "outfile(%s) open failed\n", outfile);
        return 1;
    }

    while (1)
    {
        nal.read_nal();

        nut = nal.get_nal_unit_type();

        if (nut < 0)
            break;

        // VCL
        if (nut <= NUT_VCL_END && nal.get_first_slice_flag())
        {
            frame_idx++;
        }

        if (frame_idx >= start && nut >= NUT_RAP_START && nut <= NUT_RAP_END)
        {
            if (!rap_found)
            {
                fprintf(stdout, "found random access point at offset 0x%x, frame_idx: #%d\n", nal.get_nal_offset(), frame_idx);
                rap_found = 1;
            }
        }

        if (rap_found)
        {
            outfile_write(nal.get_nal_payload(), nal.get_nal_size());
        }
        else if (nut >= NUT_VPS && nut <= NUT_PPS)
        {
            // VPS/SPS/PPS
            outfile_write(nal.get_nal_payload(), nal.get_nal_size());
        }

        if (verbose)
        {
            if (nut < NUT_NUM)
                fprintf(stdout, "nal found at 0x%x and nut is %s\n", nal.get_nal_offset(), nut_name[nut]);
            if (nut < NUT_VCL_END)
                fprintf(stdout, "frame_idx: %d - first slice flag: %d\n", frame_idx, nal.get_first_slice_flag());
        }
    }

    if (!rap_found)
        fprintf(stdout, "random access point not found\n");

    // close outfile
    outfile_close();

    return 0;
}

static void parse_option(int argc, char** argv)
{
    int opt;
    int idx = 0;

    while (1)
    {
        while( (opt = getopt(argc, argv, "s:v")) != -1)
        {
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

static void print_usage(char* executable)
{
    fprintf(stderr, "Usage: %s [-s start_frame] [-v] infile outfile\n", executable);
}

static void print_options()
{
    fprintf(stdout, "infile: %s\n", infile);
    fprintf(stdout, "outfile: %s\n", outfile);
    fprintf(stdout, "start_frame: %d\n", start);
    fprintf(stdout, "\n");
}

static int outfile_open()
{
    if ((out = fopen(outfile, "wb")) == NULL)
        return 0;

    return 1;
}

static int outfile_write(unsigned char* payload, int size)
{
    const unsigned char startcode[] = { '\0', '\0', '\1' };

    fwrite(startcode, 1, 3, out);
    fwrite(payload, 1, size, out);

    return 1;
}

static void outfile_close()
{
    if (out)
        fclose(out);
}
