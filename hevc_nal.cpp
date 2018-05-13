#include "hevc_nal.h"


int HevcNal::get_nal_unit_type()
{
    if (nal_offset < 0 || nal_payload_size < 2)
        return -1;

    return (nal_payload[0] >> 1) & 0x3F;
}

int HevcNal::get_first_slice_flag()
{
    if (nal_payload_size < 3)
        return 0;

    return (nal_payload[2] >> 7) & 0x1;
}
