#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

#define BM_EXECUTION_LIMIT 69
#define BM_STACK_CAPACITY 1024
#define BM_PROGRAM_CAPACITY 1024
#define ARRAY_SIZE(xs) (sizeof(xs) / sizeof((xs)[0]))
typedef int64_t Word; 

// INST INSTANCE
typedef enum {
    INST_PUSH,
    INST_PLUS, 
    INST_MINUS, 
    INST_MULT, 
    INST_DIV, 
    INST_JMP, 
    INST_JMP_IF, 
    INST_HALT,
} Inst_Type; 

typedef struct {
    Inst_Type type; 
    Word operand; 
} Inst; 

#define MAKE_INST_PUSH(value)   { .type = INST_PUSH, .operand = (value)}
#define MAKE_INST_JMP(addr)     {.type = INST_JMP, .operand = (addr)}
#define MAKE_INST_PLUS          { .type = INST_PLUS}
#define MAKE_INST_MINUS         { .type = INST_MINUS}
#define MAKE_INST_MULT          { .type = INST_MULT}
#define MAKE_INST_DIV           { .type = INST_DIV}
#define MAKE_INST_HALT          { .type = INST_HALT}

// Inst inst_push(Word operand) {
//     return (Inst) {
//         .type = INST_PUSH,
//         .operand = operand,
//     };
// }
// Inst inst_plus(void) {
//     return (Inst) {
//         .type = INST_PLUS,
//     };
// }

const char *inst_type_as_cstr(Inst_Type type) 
{
    switch(type) {
        case INST_PUSH: return "INST_PUSH";
        case INST_PLUS: return "INST_PLUS";
        case INST_MINUS: return "INST_MINUS";
        case INST_MULT: return "INST_MULT";
        case INST_DIV: return "INST_DIV";
        case INST_JMP: return "INST_JMP"; 
        case INST_HALT: return "INST_HALT";
        default: assert(0 && "inst_type_as_cstr: Unreachable");
    }
}

// ERROR INSTANCE 
typedef enum {
    ERR_OK = 0, 
    ERR_STACK_OVERFLOW, 
    ERR_STACK_UNDERFLOW,
    ERR_ILLEGAL_INST, 
    ERR_ILLEGAL_INST_ACCESS,
    ERR_DIV_BY_ZERO,
} Err;

// BM INSTANCE 
typedef struct {
    Word stack[BM_STACK_CAPACITY];
    Word stack_size; 

    Inst program[BM_PROGRAM_CAPACITY];
    Word program_size; 
    Word ip; 

    int halt; 
} Bm; 

 
const char *Err_as_cstr(Err err) 
{
    switch(err) {
        case ERR_OK: return "ERR_OK";
        case ERR_STACK_OVERFLOW: return "ERR_STACK_OVERFLOW";
        case ERR_STACK_UNDERFLOW: return "ERR_STACK_UNDERFLOW";
        case ERR_ILLEGAL_INST: return "ERR_ILLEGAL_INST";
        case ERR_DIV_BY_ZERO: return "ERR_DIV_BY_ZERO";
        case ERR_ILLEGAL_INST_ACCESS: return "ERR_ILLEGAL_INST_ACCESS";
        default: assert(0 && "err_as_cstr: Unreachable");
    }
}



Err bm_execute_inst(Bm *bm) 
{
    if(bm->ip < 0 || bm->ip >= bm->program_size) {
        return ERR_ILLEGAL_INST_ACCESS;
    }

    Inst inst = bm->program[bm->ip];

    switch(inst.type) {
        case INST_PUSH: 
            if(bm->stack_size >= BM_STACK_CAPACITY) {
                return ERR_STACK_OVERFLOW; 
            }
            bm->stack[bm->stack_size++] = inst.operand; 
            bm->ip += 1;  
            break; 

        case INST_PLUS: 
            if(bm->stack_size < 2) {
                return ERR_STACK_UNDERFLOW; 
            }
            bm->stack[bm->stack_size-2] += bm->stack[bm->stack_size-1]; 
            bm->stack_size -= 1; 
            bm->ip += 1; 
            break;

        case INST_MINUS: 
            if(bm->stack_size < 2) {
                return ERR_STACK_UNDERFLOW;
            }
            bm->stack[bm->stack_size-2] -= bm->stack[bm->stack_size-1]; 
            bm->stack_size -= 1;
            bm->ip += 1;  
            break;

        case INST_MULT:
            if(bm->stack_size < 2) {
                return ERR_STACK_UNDERFLOW; 
            }
            bm->stack[bm->stack_size-2] *= bm->stack[bm->stack_size-1]; 
            bm->stack_size -= 1; 
            bm->ip += 1; 
            break; 

        case INST_DIV: 
            if(bm->stack_size < 2) {
                return ERR_STACK_UNDERFLOW;
            }
            if(bm->stack[bm->stack_size-1] == 0) {
                return ERR_DIV_BY_ZERO;
            }
            bm->stack[bm->stack_size-2] /= bm->stack[bm->stack_size-1];
            bm->stack_size -= 1; 
            bm->ip += 1; 
            break;

        case INST_JMP:
            bm->ip = inst.operand; 
            break;

        case INST_HALT: 
            bm->halt = 1; 
            break;
            
        default: 
            return ERR_ILLEGAL_INST;
    }
    return ERR_OK; 
}


void bm_dump_stack(FILE* stream, const Bm *bm) 
{
    fprintf(stream, "STACK:\n");
    if(bm->stack_size == 0) {
        fprintf(stream, " [empty]\n");
    }
    else{
        for(Word i = 0; i < bm->stack_size; i++) {
            fprintf(stream, " %lld\n", bm->stack[i]); // lld because word it int64_t 
        }
    }
}



Inst program[] = {
    MAKE_INST_PUSH(0),  //0
    MAKE_INST_PUSH(1),  //1
    MAKE_INST_PLUS,     //2
    MAKE_INST_JMP(1),   //3
};

// void bm_push_inst(Bm *bm, Inst inst) 
// {
//     assert(bm->program_size < BM_PROGRAM_CAPACITY);
//     bm->program[bm->program->size++] = inst;
// } 


void bm_load_program_from_memory(Bm *bm, Inst *program, size_t program_size) 
{
    assert(program_size < BM_PROGRAM_CAPACITY);
    memcpy(bm->program, program, sizeof(program[0]) * program_size);  
    bm->program_size = program_size;
}  
Bm bm = {0};
int main() 
{
   
    bm_dump_stack(stdout, &bm);
    bm_load_program_from_memory(&bm, program, ARRAY_SIZE(program));
    for(int i = 0; i < BM_EXECUTION_LIMIT; i++) {
        Err err = bm_execute_inst(&bm);
        if(err != ERR_OK) {
            fprintf(stderr, "Error: %s\n", Err_as_cstr(err)); 
            bm_dump_stack(stderr, &bm);
            exit(1); 
        }
        bm_dump_stack(stdout, &bm);
    }
    // bm_dump_stack(stdout, &bm);
    return 0; 
}