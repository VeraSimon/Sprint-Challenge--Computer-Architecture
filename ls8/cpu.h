#ifndef _CPU_H_
#define _CPU_H_

// ## CPU macro constants
// ram and register counts
#define MAX_REGISTERS 8
#define MAX_RAM 256
// stack pointer address in the registers
#define SP 7
// interrupt vector table start address
#define IVT 0xF4

// debugging flag
int debug;

// Holds all information about the CPU
struct cpu
{
    // program counter
    unsigned int PC;

    // instruction register
    unsigned char IR;

    // registers
    // * R0-R4 are free for general use
    // * R5 is reserved as the interrupt mask (IM)
    // * R6 is reserved as the interrupt status (IS)
    // * R7 is reserved as the stack pointer (SP)
    unsigned char registers[MAX_REGISTERS];

    // ram
    unsigned char ram[MAX_RAM];

    // flags
    // FL bits: 00000LGE
    unsigned char FL;
};

// ### Instructions ### //

// These use binary literals. If these aren't available with your compiler, hex
// literals should be used.

//# ALU #//
#define ADD 0b10100000
#define CMP 0b10100111
#define DIV 0b10100011
#define MOD 0b10100100
#define MUL 0b10100010
#define SUB 0b10100001

//# PC mutators #//
#define CALL 0b01010000
#define JEQ 0b01010101
#define JMP 0b01010100
#define JNE 0b01010110
#define RET 0b00010001

//# Other #//
#define HLT 0b00000001
#define LDI 0b10000010
#define POP 0b01000110
#define PRN 0b01000111
#define PUSH 0b01000101

// ALU operations
enum alu_op
{
    ALU_ADD = ADD,
    ALU_CMP = CMP,
    ALU_DIV = DIV,
    ALU_MOD = MOD,
    ALU_MUL = MUL,
    ALU_SUB = SUB
};

// ### Function declarations ### //

extern void cpu_load(struct cpu *cpu, char *ls8_file);
extern void cpu_init(struct cpu *cpu);
extern void cpu_run(struct cpu *cpu);

#endif
