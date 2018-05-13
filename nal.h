#ifndef __NAL_H__
#define __NAL_H__

#include <fstream>

class Nal {
    public:
        Nal(char* filename);
        ~Nal();

        int read_nal();

        unsigned char* get_nal_payload();
        int get_nal_offset();
        int get_nal_size();

        unsigned char* nal_payload;
        int nal_payload_size;

        int nal_offset;
        int start_code_found;

    private:
        int read_byte();
        int get_offset();

        int seek_start_code();

        std::ifstream file;

        int next_nal_offset;

        int trailing_zeroes;
};

#endif
