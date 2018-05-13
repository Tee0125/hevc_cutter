#include "nal.h"


#define MIN(A,B) ((A)>(B)?(B):(A))
#define MAX_PAYLOAD_SIZE (1024*1024)

Nal::Nal(char* filename)
{
    file.open(filename, std::ios::binary);

    nal_payload = new unsigned char[MAX_PAYLOAD_SIZE];
    nal_payload_size = 0;

    nal_offset = -1;
    next_nal_offset = -1;

    start_code_found = 0;
    trailing_zeroes = 0;
}

Nal::~Nal()
{
    if (file.is_open())
        file.close();

    delete nal_payload;
}

int Nal::read_byte()
{
    return file.get();
}

int Nal::get_offset()
{
    return file.tellg();
}

int Nal::seek_start_code()
{
    int code;

    next_nal_offset = -1;

    // find start code pattern
    while ((code = read_byte()) != EOF)
    {
        if (code == 1 && trailing_zeroes >= 2)
        {
            // save nal start position
            next_nal_offset = get_offset() - trailing_zeroes - 1;
            break;
        }
        else if (code == 3 && trailing_zeroes >= 2)
        {
            // Emulation prevent byte found, consume next byte
            if (read_byte() == EOF)
                break;

            trailing_zeroes = 0;
            continue;
        }

        // update trailing bytes
        if (code == 0)
            trailing_zeroes = MIN(trailing_zeroes+1, 3);
        else
            trailing_zeroes = 0;
    }

    if (next_nal_offset >= 0)
        start_code_found = 1;
    else
        start_code_found = 0;

    return next_nal_offset;
}

int Nal::read_nal()
{
    int code;

    nal_offset = -1;

    if (!start_code_found)
        seek_start_code();

    if (!start_code_found)
        return -1;

    nal_offset = next_nal_offset;
    nal_payload_size = 0;

    next_nal_offset = -1;

    // find start code pattern
    while ((code = read_byte()) != EOF)
    {
        nal_payload[nal_payload_size++] = code;

        if (code == 1 && trailing_zeroes >= 2)
        {
            // save nal start position
            next_nal_offset = get_offset() - trailing_zeroes - 1;
            break;
        }
        else if (code == 3 && trailing_zeroes >= 2)
        {
            // Emulation prevent byte found, consume next byte
            if ((code = read_byte()) == EOF)
                break;

            nal_payload[nal_payload_size++] = code;

            trailing_zeroes = 0;
            continue;
        }
        else if (code == 0 && trailing_zeroes >= 2)
        {
            // stop pattern
            trailing_zeroes = 3;
            break;
        }
        else if (code == 0)
        {
            // update trailing bytes
            trailing_zeroes = trailing_zeroes+1;
        }
        else
        {
            trailing_zeroes = 0;
        }
    }

    if (next_nal_offset >= 0)
    {
        start_code_found = 1;
        nal_payload_size -= trailing_zeroes + 1;
    }
    else
    {
        start_code_found = 0;
    }

    return nal_payload_size;
}

unsigned char* Nal::get_nal_payload()
{
    if (nal_offset < 0)
        return NULL;
    else
        return nal_payload;
}

int Nal::get_nal_offset()
{
    return nal_offset;
}

int Nal::get_nal_size()
{
    return nal_payload_size;
}
