#ifndef JAVA_OPCODES_H
#define JAVA_OPCODES_H

#include <cstdint>

namespace opcodes
{
    const uint8_t
        ICONST_M1     = 0x02,
        ICONST_0      = 0x03,
        ICONST_1      = 0x04,
        ICONST_2      = 0x05,
        ICONST_3      = 0x06,
        ICONST_4      = 0x07,
        ICONST_5      = 0x08,
        BIPUSH        = 0x10,
        SIPUSH        = 0x11,
        LDC           = 0x12,
        LDC_W         = 0x13,
        LDC2_W        = 0x14,
        ILOAD         = 0x15,
        ALOAD         = 0x19,
        ILOAD_0       = 0x1A,
        ILOAD_1       = 0x1B,
        ILOAD_2       = 0x1C,
        ILOAD_3       = 0x1D,
        ALOAD_0       = 0x2A,
        ALOAD_1       = 0x2B,
        ALOAD_2       = 0x2C,
        ALOAD_3       = 0x2D,
        ISTORE        = 0x36,
        ASTORE        = 0x3A,
        ISTORE_0      = 0x3B,
        ISTORE_1      = 0x3C,
        ISTORE_2      = 0x3D,
        ISTORE_3      = 0x3E,
        ASTORE_0      = 0x4B,
        ASTORE_1      = 0x4C,
        ASTORE_2      = 0x4D,
        ASTORE_3      = 0x4E,
        POP           = 0x57,
        DUP           = 0x59,
        IINC          = 0x84,
        IF_ICMPGE     = 0xA2,
        GOTO          = 0xA7,
        RETURN        = 0xB1,
        GETSTATIC     = 0xB2,
        PUTSTATIC     = 0xB3,
        GETFIELD      = 0xB4,
        PUTFIELD      = 0xB5,
        INVOKEVIRTUAL = 0xB6,
        INVOKESPECIAL = 0xB7,
        INVOKESTATIC  = 0xB8,
        NEW           = 0xBB,
        IADD          = 0x60,
        IMUL          = 0x68;
}

#endif /* JAVA_OPCODES_H */

