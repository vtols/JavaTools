#ifndef JAVA_OPCODES_H
#define JAVA_OPCODES_H

#include <cstdint>

namespace opcodes
{
    const uint8_t
        BIPUSH        = 0x10,
        LDC           = 0x12,
        LDC_W         = 0x13,
        LDC2_W        = 0x14,
        ISTORE        = 0x36,
        ASTORE        = 0x3A,
        GOTO          = 0xA7,
        RETURN        = 0xB1,
        GETSTATIC     = 0xB2,
        INVOKEVIRTUAL = 0xB6,
        IADD          = 0x60,
        IMUL          = 0x68;
}

#endif /* JAVA_OPCODES_H */

