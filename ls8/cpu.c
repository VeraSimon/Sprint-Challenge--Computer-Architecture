#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpu.h"

/**
 * RAM read & write
 */
unsigned char *cpu_ram_read(struct cpu *cpu, int address)
{
    if (address < MAX_RAM)
    {
        return &cpu->ram[address];
    }
    else
    {
        return NULL;
    }
}

void cpu_ram_write(struct cpu *cpu, int address, unsigned char value)
{
    if (address < MAX_RAM)
    {
        cpu->ram[address] = value;
    }
}

void cpu_stack_push(struct cpu *cpu, int reg)
{
    // TODO: Check for a stack overflow
    cpu->registers[SP]--;
    int stack_p = cpu->registers[SP];
    cpu_ram_write(cpu, stack_p, reg);
}

unsigned char *cpu_stack_pop(struct cpu *cpu)
{
    if (cpu->registers[SP] <= IVT - 1)
    {
        int stack_p = cpu->registers[SP];
        unsigned char *stack_val = cpu_ram_read(cpu, stack_p);
        cpu->registers[SP]++;
        return stack_val;
    }
    else
    {
        fprintf(stderr, "Stack underflow!");
        exit(-1);
    }
}

/**
 * Load the binary bytes from an .ls8 source file into a RAM array
 */
void cpu_load(struct cpu *cpu, char *ls8_file)
{
    // Arbitrary large number. Setting shorter than a line's length in file
    // causes issues with zeroed indexes due to the strtoul conversion. Don't
    // set this to an instruction's length (8) + 1.
    int instr_buf = MAX_RAM;

    FILE *fp;
    char line[instr_buf];
    fp = fopen(ls8_file, "r");

    if (fp == NULL)
    {
        printf("Error opening file\n");
        exit(2);
    }

    int address = 0;

    while (fgets(line, instr_buf, fp) != NULL)
    {
        char *endptr;
        unsigned char val = strtoul(line, &endptr, 2);

        // Skip those pesky single-line comments
        if (line == endptr)
        {
            continue;
        }

        // Find out if address++ increments before or after evaluation here?
        // "A post-increment operator is used to increment the value of variable
        // after executing expression completely in which post increment is used."
        cpu_ram_write(cpu, address++, val);
    }

    fclose(fp);
}

/**
 * ALU
 */
void alu(struct cpu *cpu, enum alu_op op, unsigned char regA, unsigned char regB)
{
    switch (op)
    {
    case ALU_ADD:
        cpu->registers[regA] += cpu->registers[regB];
        break;

    case ALU_AND:
        cpu->registers[regA] &= cpu->registers[regB];
        break;

    case ALU_CMP:
        // clear the 3 CMP bits
        cpu->FL &= ~0b111;
        // set the CMP bits
        if (cpu->registers[regA] < cpu->registers[regB])
        {
            cpu->FL |= 1 << 2;
        }
        else if (cpu->registers[regA] > cpu->registers[regB])
        {
            cpu->FL |= 1 << 1;
        }
        else
        {
            cpu->FL |= 1 << 0;
        }

        if (debug)
        {
            printf("Flags: 0x%X\n", cpu->FL);
        }
        break;

    case ALU_DEC:
        cpu->registers[regA]--;
        break;

    case ALU_DIV:
        if (regB == 0)
        {
            fprintf(stderr, "ERROR: Can't divide by zero!\n");
            exit(-1);
        }
        else
        {
            cpu->registers[regA] /= cpu->registers[regB];
        }
        break;

    case ALU_INC:
        cpu->registers[regA]++;
        break;

    case ALU_MOD:
        if (regB == 0)
        {
            fprintf(stderr, "ERROR: Can't divide by zero!\n");
            exit(-1);
        }
        else
        {
            cpu->registers[regA] %= cpu->registers[regB];
        }
        break;

    case ALU_MUL:
        cpu->registers[regA] *= cpu->registers[regB];
        break;

    case ALU_NOT:
        cpu->registers[regA] = ~cpu->registers[regA];
        break;

    case ALU_OR:
        cpu->registers[regA] |= cpu->registers[regB];
        break;

    case ALU_SHL:
        cpu->registers[regA] = cpu->registers[regA] << cpu->registers[regB];
        break;

    case ALU_SHR:
        cpu->registers[regA] = cpu->registers[regA] >> cpu->registers[regB];
        break;

    case ALU_SUB:
        cpu->registers[regA] -= cpu->registers[regB];
        break;

    case ALU_XOR:
        cpu->registers[regA] ^= cpu->registers[regB];
        break;

    default:
        fprintf(stderr, "ERROR: Invalid instruction %u!\n", cpu->IR);
        exit(-1);
    }
}

/**
 * Run the CPU
 */
void cpu_run(struct cpu *cpu)
{
    // Operand storage per ls8/README.md step 4
    unsigned int operandA;
    unsigned int operandB;

    int running = 1; // True until we get a HLT instruction
    while (running)
    {
        if (debug)
        {
            printf("Registers:\n[ ");
            for (int i = 0; i < MAX_REGISTERS; i++)
            {
                printf("%u ", cpu->registers[i]);
            }
            printf("]\n");
            printf("RAM:\n[ ");
            for (int i = 0; i < MAX_RAM; i++)
            {
                if (cpu->ram[i] != 0)
                {
                    printf("%i:%u ", i, cpu->ram[i]);
                }
            }
            printf("]\n");
            printf("Stack pointer: %u\n", cpu->registers[SP]);
            printf("Program counter: %u\n", cpu->PC);
        }

        // 1. Get the value of the current instruction (in address PC).
        cpu->IR = *cpu_ram_read(cpu, cpu->PC);

        // 2. Figure out how many operands this next instruction requires
        int operands = cpu->IR >> 6;

        if (debug)
        {
            printf("Instruction register: %i\n", cpu->IR);
            printf("Operand count: %i\n", operands);
        }

        // 3. Get the appropriate value(s) of the operands following this instruction
        switch (operands)
        {
        case 1: // 0b0001
            operandA = *cpu_ram_read(cpu, cpu->PC + 1);
            if (debug)
            {
                printf("Value of operandA: %u\n", operandA);
            }
            break;
        case 2: // 0b0010
            operandA = *cpu_ram_read(cpu, cpu->PC + 1);
            operandB = *cpu_ram_read(cpu, cpu->PC + 2);
            if (debug)
            {
                printf("Value of operandA: %u\n", operandA);
                printf("Value of operandB: %u\n", operandB);
            }
            break;
        case 3: // 0b0011
            // do something maybe? this is dependent on the last 2 digits, so 0, 1, 2, & 3 are all possibilities.
            break;
        default: // 0b0000 or unexpected
            break;
        }

        // 4. switch() over it to decide on a course of action.
        // 5. Do whatever the instruction should do according to the spec.
        switch (cpu->IR)
        {
        case ADD:
            alu(cpu, ADD, operandA, operandB);
            break;

        case AND:
            alu(cpu, AND, operandA, operandB);
            break;

        case CALL:
            cpu_stack_push(cpu, cpu->PC + operands + 1);
            cpu->PC = cpu->registers[operandA];
            break;

        case CMP:
            alu(cpu, CMP, operandA, operandB);
            break;

        case DEC:
            alu(cpu, DEC, operandA, operandB);
            break;

        case DIV:
            alu(cpu, DIV, operandA, operandB);
            break;

        case HLT:
            running = 0;
            break;

        case INC:
            alu(cpu, INC, operandA, operandB);
            break;

        case JEQ:
            if ((cpu->FL & 0b00000001) == 1)
            {
                cpu->PC = cpu->registers[operandA];
            }
            else
            {
                cpu->PC += operands + 1;
            }
            break;

        case JMP:
            cpu->PC = cpu->registers[operandA];
            break;

        case JNE:
            if ((cpu->FL & 0b00000001) == 0)
            {
                cpu->PC = cpu->registers[operandA];
            }
            else
            {
                cpu->PC += operands + 1;
            }
            break;

        case LDI:
            cpu->registers[operandA] = operandB;
            break;

        case MOD:
            alu(cpu, MOD, operandA, operandB);
            break;

        case MUL:
            alu(cpu, MUL, operandA, operandB);
            break;

        case NOT:
            alu(cpu, NOT, operandA, operandB);
            break;

        case OR:
            alu(cpu, OR, operandA, operandB);
            break;

        case POP:
            cpu->registers[operandA] = *cpu_stack_pop(cpu);
            break;

        case PRN:
            if (debug)
            {
                printf("Result: ");
            }
            fprintf(stdout, "%i\n", cpu->registers[operandA]);
            break;

        case PUSH:
            cpu_stack_push(cpu, cpu->registers[operandA]);
            break;

        case RET:
            cpu->PC = *cpu_stack_pop(cpu);
            break;

        case SHL:
            alu(cpu, SHL, operandA, operandB);
            break;

        case SHR:
            alu(cpu, SHR, operandA, operandB);
            break;

        case SUB:
            alu(cpu, SUB, operandA, operandB);
            break;

        case XOR:
            alu(cpu, XOR, operandA, operandB);
            break;

        default:
            fprintf(stderr, "ERROR: Invalid instruction %u!\n", cpu->IR);
            exit(-1);
        }

        // 6. Move the PC to the next instruction.
        switch (cpu->IR)
        {
        // Don't increment cpu->PC for instructions that do it themselves
        case CALL:
        case JEQ:
        case JMP:
        case JNE:
        case RET:
            break;

        default:
            cpu->PC += operands + 1;
        }

        if (debug)
        {
            printf("\n");
        }
    }
}

/**
 * Initialize a CPU struct
 */
void cpu_init(struct cpu *cpu)
{
    memset(cpu->registers, 0, sizeof(char) * MAX_REGISTERS);
    cpu->registers[SP] = IVT;
    cpu->PC = 0;
    cpu->FL = 0;
    memset(cpu->ram, 0, sizeof(char) * MAX_RAM);
}
