#include "DTC.h"

UDS_DTC_t dtcDB[2]= {
    //Format: dtcCode(3bytes), snapshotData[] (DTC Status byte in this case), offset_index
    {0x00A3D800, {0x00}, 0},
    {0x00A3D900, {0x00}, 1},
};