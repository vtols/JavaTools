#ifndef JAVA_OPCODES_H
#define JAVA_OPCODES_H

#include <cstdint>

namespace opcodes
{
    const uint8_t 
        LDC           = 0x12,
        LDC_W         = 0x13,
        LDC2_W        = 0x14,
        GOTO          = 0xA7,
        RETURN        = 0xB1,
        GETSTATIC     = 0xB2,
        INVOKEVIRTUAL = 0xB6;
}

#endif /* JAVA_OPCODES_H */

