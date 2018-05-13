#ifndef __HEVC_NAL_H__
#define __HEVC_NAL_H__

#include "nal.h"

class HevcNal: public Nal {
    public:
        HevcNal(char* filename) : Nal(filename) {}
        
        int get_nal_unit_type();
        int get_first_slice_flag();

    private:

};

#endif
