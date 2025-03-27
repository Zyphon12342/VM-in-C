#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#define BM_STACK_CAPACITY 1024

typedef int64_t Word; 

typedef enum {
    TRAP_OK = 0, 
    TRAP_STACK_OVERFLOW, 
    TRAP_STACK_UNDERFLOW,
    TRAP_ILLEGLA_INST, 
} Trap;

typedef struct {
    Word stack[BM_STACK_CAPACITY];
    size_t stack_size; 
} Bm; 

typedef enum {
    INST_PUSH,
    INST_PLUS, 
} Inst_Type; 

typedef struct {
    Inst_Type type; 
    Word operand; 
} Inst; 

Inst inst_push(Word operand) {
    return (Inst) {
        .type = INST_PUSH,
        .operand = operand,
    };
}
Inst inst_plus(Word operand) {
    return (Inst) {
        .type = INST_PLUS, 
        .operand = operand,
    };
}
Bm bm = {0}; 

Trap bm_execute_inst(Bm *bm, Inst inst) {
    switch(inst.type) {
        case INST_PUSH: 
            if(bm->stack_size >= BM_STACK_CAPACITY) {
                return TRAP_STACK_OVERFLOW; 
            }
            bm->stack[bm->stack_size++] = inst.operand; 
            break; 
        case INST_PLUS: 
            if(bm->stack_size < 2) {
                return TRAP_STACK_UNDERFLOW; 
            }
            bm->stack[bm->stack_size-2] += bm->stack[bm->stack_size-1]; 
            bm->stack_size -= 1; 
            break; 
        default: 
            return TRAP_ILLEGLA_INST;
        
    }
    return TRAP_OK; 
}


void bm_dump(const Bm *bm) {
    printf("STACK: "); 
    for(size_t i = 0; i < bm->stack_size; i++) {
        printf(" %lld\n", bm->stack[i]); // ld because word it int64_t 
    }
}



int main() 
{
    printf("hello, world!\n");
    return 0; 
}