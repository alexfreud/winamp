#ifndef __OPCODES_H
#define __OPCODES_H


#define OPCODE_NOP          0x00

#define OPCODE_PUSH         0x01
#define OPCODE_POPI         0x02
#define OPCODE_POP          0x03

#define OPCODE_CMPEQ        0x08
#define OPCODE_CMPNE        0x09
#define OPCODE_CMPA         0x0A
#define OPCODE_CMPAE        0x0B
#define OPCODE_CMPB         0x0C
#define OPCODE_CMPBE        0x0D

#define OPCODE_JIZ          0x10
#define OPCODE_JNZ          0x11
#define OPCODE_JMP          0x12

#define OPCODE_CALLM        0x18
#define OPCODE_CALLC        0x19

#define OPCODE_RET          0x20
#define OPCODE_RETF         0x21

#define OPCODE_CMPLT        0x28

#define OPCODE_SET          0x30

#define OPCODE_INCS         0x38
#define OPCODE_DECS         0x39
#define OPCODE_INCP         0x3A
#define OPCODE_DECP         0x3B

#define OPCODE_ADD          0x40
#define OPCODE_SUB          0x41
#define OPCODE_MUL          0x42
#define OPCODE_DIV          0x43
#define OPCODE_MOD          0x44

#define OPCODE_AND          0x48
#define OPCODE_OR           0x49
#define OPCODE_NOT          0x4A
#define OPCODE_BNOT         0x4B
#define OPCODE_NEG          0x4C
#define OPCODE_XOR          0x4D

#define OPCODE_LAND         0x50
#define OPCODE_LOR          0x51

#define OPCODE_SHL          0x58
#define OPCODE_SHR          0x59

#define OPCODE_NEW          0x60
#define OPCODE_DELETE       0x61

#define OPCODE_UMV          0x68
#define OPCODE_UMC          0x69

#define OPCODE_CALLM2       0x70

#endif