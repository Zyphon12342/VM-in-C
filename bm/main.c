#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#define BM_STACK_CAPACITY 1024
#define ARRAY_SIZE(xs) (sizeof(xs) / sizeof((xs)[0]))
typedef int64_t Word; 

typedef enum {
    Err_OK = 0, 
    Err_STACK_OVERFLOW, 
    Err_STACK_UNDERFLOW,
    Err_ILLEGAL_INST, 
} Err;

typedef struct {
    Word stack[BM_STACK_CAPACITY];
    size_t stack_size; 
} Bm; 

typedef enum {
    INST_PUSH,
    INST_PLUS, 
    INST_MINUS, 
    INST_MULT, 
    INST_DIV, 
} Inst_Type; 

typedef struct {
    Inst_Type type; 
    Word operand; 
} Inst; 

#define MAKE_INST_PUSH(value) { .type = INST_PUSH, .operand = (value)}
#define MAKE_INST_PLUS { .type = INST_PLUS}
#define MAKE_INST_MINUS { .type = INST_MINUS}
#define MAKE_INST_MULT { .type = INST_MULT}
#define MAKE_INST_DIV { .type = INST_DIV}

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
const char *Err_as_cstr(Err err) 
{
    switch(err) {
        case Err_OK: return "Err_OK";
        case Err_STACK_OVERFLOW: return "Err_STACK_OVERFLOW";
        case Err_STACK_UNDERFLOW: return "Err_STACK_UNDERFLOW";
        case Err_ILLEGAL_INST: return "Err_ILLEGAL_INST";
        default: assert(0 && "Err_as_cstr: Unreachable");
    }
}
const char *inst_type_as_cstr(Inst_Type type) 
{
    switch(type) {
        case INST_PUSH: return "INST_PUSH";
        case INST_PLUS: return "INST_PLUS";
        case INST_MINUS: return "INST_MINUS";
        case INST_MULT: return "INST_MULT";
        case INST_DIV: return "INST_DIV";
        default: assert(0 && "inst_type_as_cstr: Unreachable");
    }
}
Err bm_execute_inst(Bm *bm, Inst inst) {
    switch(inst.type) {
        case INST_PUSH: 
            if(bm->stack_size >= BM_STACK_CAPACITY) {
                return Err_STACK_OVERFLOW; 
            }
            bm->stack[bm->stack_size++] = inst.operand; 
            break; 
        case INST_PLUS: 
            if(bm->stack_size < 2) {
                return Err_STACK_UNDERFLOW; 
            }
            bm->stack[bm->stack_size-2] += bm->stack[bm->stack_size-1]; 
            bm->stack_size -= 1; 
            break; 
        case INST_MINUS: 
            if(bm->stack_size < 2) {
                return Err_STACK_UNDERFLOW;
            }
            bm->stack[bm->stack_size-2] -= bm->stack[bm->stack_size-1]; 
            bm->stack_size -= 1; 
            break;
        case INST_MULT:
            if(bm->stack_size < 2) {
                return Err_STACK_UNDERFLOW; 
            }
            bm->stack[bm->stack_size-2] *= bm->stack[bm->stack_size-1]; 
            bm->stack_size -= 1; 
            break; 
        case INST_DIV: 
            if(bm->stack_size < 2) {
                return Err_STACK_UNDERFLOW;
            }
            bm->stack[bm->stack_size-2] /= bm->stack[bm->stack_size-1];
            bm->stack_size -= 1; 
            break;
        default: 
            return Err_ILLEGAL_INST;
    }
    return Err_OK; 
}


void bm_dump(FILE* stream, const Bm *bm) {
    fprintf(stream, "STACK:\n");
    if(bm->stack_size == 0) {
        fprintf(stream, " [empty]\n");
    }
    else{
        for(size_t i = 0; i < bm->stack_size; i++) {
            fprintf(stream, " %lld\n", bm->stack[i]); // lld because word it int64_t 
        }
    }
}

Bm bm = {0};

Inst program[] = {
    MAKE_INST_PUSH(69), 
    MAKE_INST_PUSH(420),
    MAKE_INST_PLUS,
    MAKE_INST_PUSH(42),
    MAKE_INST_MINUS, 
};


int main() 
{
    bm_dump(stdout, &bm);
    for(size_t i = 0; i < ARRAY_SIZE(program); i++) {
        fprintf(stdout, "%s\n", inst_type_as_cstr(program[i].type));
        Err err = bm_execute_inst(&bm, program[i]);
        if(err != Err_OK) {
            fprintf(stderr, "Error: %s\n", Err_as_cstr(err)); 
            bm_dump(stderr, &bm);
            exit(1); 
        }
        bm_dump(stdout, &bm);
    }
    bm_dump(stdout, &bm);
    return 0; 
}