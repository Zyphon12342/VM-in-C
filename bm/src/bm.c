#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <errno.h> 
//BM SETUP 
#define BM_EXECUTION_LIMIT 69
#define BM_STACK_CAPACITY 1024
#define BM_PROGRAM_CAPACITY 1024
#define ARRAY_SIZE(xs) (sizeof(xs) / sizeof((xs)[0]))

typedef int64_t Word; 

// INST INSTANCE
typedef enum {
    INST_NOP = 0, 
    INST_PUSH,
    INST_DUP,
    INST_PLUS, 
    INST_MINUS, 
    INST_MULT, 
    INST_DIV, 
    INST_JMP, 
    INST_JMP_IF,
    INST_EQ, 
    INST_HALT,
    INST_PRINT_DEBUG, 
} Inst_Type; 

typedef struct {
    Inst_Type type; 
    Word operand; 
} Inst; 

#define MAKE_INST_PUSH(value)   { .type = INST_PUSH, .operand = (value)}
#define MAKE_INST_DUP(addr)     { .type = INST_DUP, .operand = (addr)}
#define MAKE_INST_JMP(addr)     { .type = INST_JMP, .operand = (addr)}
#define MAKE_INST_PLUS          { .type = INST_PLUS}
#define MAKE_INST_MINUS         { .type = INST_MINUS}
#define MAKE_INST_MULT          { .type = INST_MULT}
#define MAKE_INST_DIV           { .type = INST_DIV}
#define MAKE_INST_HALT          { .type = INST_HALT, .operand = (addr)}
#define MAKE_INST_NOP           {.type = INST_NOP, .operand = 0} 
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
        case INST_NOP:         return "INST_NOP"; 
        case INST_PUSH:        return "INST_PUSH";
        case INST_PLUS:        return "INST_PLUS";
        case INST_MINUS:       return "INST_MINUS";
        case INST_MULT:        return "INST_MULT";
        case INST_DIV:         return "INST_DIV";
        case INST_JMP:         return "INST_JMP"; 
        case INST_JMP_IF:      return "INST_JMP_IF"; 
        case INST_EQ:          return "INST_EQ"; 
        case INST_HALT:        return "INST_HALT";
        case INST_PRINT_DEBUG: return "INST_PRINT_DEBUG"; 
        case INST_DUP:         return "INST_DUP";
        default:            assert(0 && "inst_type_as_cstr: Unreachable");
    }
}

// ERROR INSTANCE  
typedef enum {
    ERR_OK = 0, 
    ERR_STACK_OVERFLOW, 
    ERR_STACK_UNDERFLOW,
    ERR_ILLEGAL_INST, 
    ERR_ILLEGAL_INST_ACCESS,
    ERR_ILLEGAL_OPERAND, 
    ERR_DIV_BY_ZERO,
} Err;

// BM INSTANCE 
typedef struct {
    //GOAL SEPERATE SPACE FOR PROGRAM STACK AND MEMORY FOR SECURITY
    Word stack[BM_STACK_CAPACITY];
    //current stack size 
    Word stack_size; 

    Inst program[BM_PROGRAM_CAPACITY];

    Word program_size; 
    //instruction pointer 
    Word ip; 

    int halt; 
} Bm; 

 
const char *Err_as_cstr(Err err) 
{
    switch(err) {
        case ERR_OK:                  return "ERR_OK";
        case ERR_STACK_OVERFLOW:      return "ERR_STACK_OVERFLOW";
        case ERR_STACK_UNDERFLOW:     return "ERR_STACK_UNDERFLOW";
        case ERR_ILLEGAL_INST:        return "ERR_ILLEGAL_INST";
        case ERR_DIV_BY_ZERO:         return "ERR_DIV_BY_ZERO";
        case ERR_ILLEGAL_INST_ACCESS: return "ERR_ILLEGAL_INST_ACCESS";
        case ERR_ILLEGAL_OPERAND:     return "ERR_ILLEGAL_OPERAND";
        default:                      assert(0 && "err_as_cstr: Unreachable");
    }
}




Err bm_execute_inst(Bm *bm) 
{
    if(bm->ip < 0 || bm->ip >= bm->program_size) {
        return ERR_ILLEGAL_INST_ACCESS;
    }

    Inst inst = bm->program[bm->ip];

    switch(inst.type) {
        case INST_NOP: 
            bm->ip += 1; 
            break; 
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

        case INST_EQ:
            if(bm->stack_size < 2) {
                return ERR_STACK_UNDERFLOW;
            }
            bm->stack[bm->stack_size-2] = (
                bm->stack[bm->stack_size-2] == bm->stack[bm->stack_size-1]
            ); 
            bm->stack_size -= 1; 
            bm->ip += 1; 
            break;

        case INST_JMP_IF: 
            if(bm->stack_size < 1) {
                return ERR_STACK_UNDERFLOW; 
            }
            if(bm->stack[bm->stack_size - 1]) {
                bm->stack_size -= 1; 
                bm->ip = inst.operand; 
            } else {
                bm->ip += 1; 
            }
            break;

        case INST_PRINT_DEBUG: 
            if(bm->stack_size < 1) {
                return ERR_STACK_UNDERFLOW; 
            }
            fprintf(stdout, "%lld", bm->stack[bm->stack_size - 1]); 
            bm->stack_size -= 1; 
            bm->ip += 1; 
            break; 
        case INST_DUP: 
            if(bm->stack_size - inst.operand <= 0) {
                return ERR_STACK_UNDERFLOW; 
            }
            if(bm->stack_size >= BM_STACK_CAPACITY) {
                return ERR_STACK_OVERFLOW; 
            }
            if(inst.operand < 0) {
                return ERR_ILLEGAL_OPERAND; 
            }
            bm->stack[bm->stack_size] = bm->stack[bm->stack_size - 1 - inst.operand]; 
            bm->stack_size += 1; 
            bm->ip += 1; 
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
            fprintf(stream, " %lld\n", bm->stack[i]); // lld because word is int64_t 
        }
    }
}

void bm_save_program_to_file(Inst *program, size_t program_size, const char *file_path) 
{
    FILE *f = fopen(file_path, "wb"); 
    if(f == NULL) {
        fprintf(stderr, "ERROR: Could not open file '%s': %s\n", file_path, strerror(errno)); 
        exit(1); 
    }
    fwrite(program, sizeof(program[0]), program_size, f); 
    
    if(ferror(f)) {
        fprintf(stderr, "ERROR: Could not open file '%s': %s\n", file_path, strerror(errno));
        exit(1); 
    }
    fclose(f); 
}

// Inst program[] = {
//     MAKE_INST_PUSH(0),  //0
//     MAKE_INST_PUSH(1),  //1
//     MAKE_INST_DUP(1),   //2
//     MAKE_INST_DUP(1),   //3
//     MAKE_INST_PLUS,     //4
//     MAKE_INST_JMP(2),   //5
// };

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


void bm_load_program_from_file(Bm *bm, const char * file_path) 
{
    FILE *f = fopen(file_path, "rb"); 
    if(f == NULL) {
        fprintf(stderr, "ERROR: Could not open file '%s': %s\n", file_path, strerror(errno)); 
        exit(1);
    }
    if (fseek(f,0,SEEK_END) < 0) { // seeks to end of file -1 means error 
        fprintf(stderr, "ERROR: Could not read file '%s': %s\n", file_path, strerror(errno)); 
        exit(1);
    }

    long m = ftell(f); //tells position of cursor 
    if(m < 0) {
        fprintf(stderr, "ERROR: Could not read file '%s': %s\n", file_path, strerror(errno)); 
        exit(1);
    }
     // ensures file program and bm program are of same type?
    assert(m % sizeof(bm->program[0]) == 0); //seek to end + cursor to get size of program 
    
    assert((size_t) m <= BM_PROGRAM_CAPACITY * sizeof(bm->program[0])); 
    //returning cursor to start of file to read 
    if(fseek(f, 0, SEEK_SET) < 0) { 
        fprintf(stderr, "ERROR: Could not read file '%s': %s\n", file_path, strerror(errno)); 
        exit(1);
    }
    // char * buffer; 
    // fread(buffer, sizeof(bm->program[0]), m / sizeof(bm->program[0]), f);
    // bm_load_program_from_memory(bm, buffer, ARRAY_SIZE(program)); 
    bm->program_size = fread(bm->program, sizeof(bm->program[0]), m / sizeof(bm->program[0]), f);
    if(ferror(f)) {
        fprintf(stderr, "ERROR: Could not open file '%s': %s\n", file_path, strerror(errno)); 
        exit(1);
    }
    fclose(f);
}
//INSTANCE OF BM 
Bm bm = {0};

// int main(void) 
// {
//     bm_save_program_to_file(program, ARRAY_SIZE(program), "fib.bm"); 
//     return 0; 
// }

// char* source_code = 
//     "push 0\n"
//     "push 1\n"
//     "dup 1\n"
//     "dup 1\n"
//     "plus\n"
//     "jmp 2\n"; 


// String_View implmenetation 

typedef struct {
    size_t count; 
    const char *data; 
} String_View; 

String_View cstr_as_sv(const char *cstr) 
{
    return (String_View) {
        .count = strlen(cstr), 
        .data = cstr, 
    };
}


String_View sv_trim_left(String_View sv) 
{
    size_t i = 0; 
    while(i < sv.count && isspace(sv.data[i])) {
        i += 1; 
    }
    return (String_View) {
        .count = sv.count - i,
        .data = sv.data + i,
    }; 
}

String_View sv_trim_right(String_View sv) 
{
    size_t i = 0; 
    while(i < sv.count && isspace(sv.data[sv.count-1-i])) {
        i += 1; 
    }
    return (String_View) {
        .count = sv.count - i, 
        .data = sv.data, 
    }; 
}

String_View sv_trim(String_View sv)
{
    return sv_trim_left(sv_trim_right(sv));
}
String_View sv_chop_by_delim(String_View *sv, char delim) 
{
    size_t i = 0; 
    while(i < sv->count && sv->data[i] != delim) {
        i += 1; 
    }
    String_View result =  {
        .count = i,
        .data = sv->data,
    };
    if(i < sv->count) {
        sv->count -= i + 1;
        sv->data += i + 1;
    } else {
        sv->count -= i; 
        sv->data += i; 
    }
    return result; 
}

int sv_eq(String_View a, String_View b) 
{
    if(a.count != b.count) {
        return 0; 
    }
    return memcmp(a.data, b.data, a.count) == 0;
}

int sv_to_int(String_View sv) 
{
    int result = 0; 

    for(size_t i = 0; i < sv.count && isdigit(sv.data[i]); i++) {
        result = result * 10 + sv.data[i] - '0'; 
        i += 1; 
    }
    return result;
}

char *trim_left(char *str, size_t str_size) 
{
    for(size_t i = 0; i < str_size; i++) {
        if(!isspace(str[i])) {
            return str + i; 
        }
    }
    return str + str_size; 
}



Inst bm_translate_line(String_View line) 
{
    line = sv_trim_left(line); 
    String_View inst_name = sv_chop_by_delim(&line, ' '); 
    if(sv_eq(inst_name, cstr_as_sv("push"))) {
        line = sv_trim_left(line); 
        int operand = sv_to_int(sv_trim_right(line));
        return (Inst) { .type = INST_PUSH, .operand = operand}; 
    } else if(sv_eq(inst_name, cstr_as_sv("dup"))) {
        line = sv_trim_left(line); 
        int operand = sv_to_int(sv_trim_right(line));
        return (Inst) { .type = INST_DUP, .operand = operand}; 
    } else if(sv_eq(inst_name, cstr_as_sv("plus"))) {
        return (Inst) { .type = INST_PLUS }; 
    } else if(sv_eq(inst_name, cstr_as_sv("jmp"))) {
        line = sv_trim_left(line); 
        int operand = sv_to_int(sv_trim_right(line));
        return (Inst) { .type = INST_JMP, .operand = operand}; 
    } else {
        fprintf(stderr, "ERROR: Unknown instruction '%.*s'\n", (int) inst_name.count, inst_name.data); 
        exit(1); 
    }
}

size_t bm_translate_source(String_View source,  Inst *program, size_t program_capacity) 
{
    size_t program_size = 0; 
    while(source.count > 0) {
        assert(program_size < program_capacity);
        String_View line = sv_chop_by_delim(&source, '\n'); 
        if(line.count > 0) {
            program[program_size++] = bm_translate_line(line);
        }
        printf("#%.*s#\n", (int) line.count, line.data);
    }
    return program_size; 
}

String_View slurp_file(const char *file_path) 
{
    FILE *f = fopen(file_path, "r"); 
    if(f == NULL) {
        fprintf(stderr, "ERROR: Could not read file '%s': %s\n", file_path, strerror(errno)); 
        exit(1);
    }
    if (fseek(f,0,SEEK_END) < 0) { // seeks to end of file -1 means error 
        fprintf(stderr, "ERROR: Could not read file '%s': %s\n", file_path, strerror(errno)); 
        exit(1);
    }

    long m = ftell(f); //tells position of cursor 
    if(m < 0) {
        fprintf(stderr, "ERROR: Could not read file '%s': %s\n", file_path, strerror(errno)); 
        exit(1);
    }
    char *buffer = malloc(m); 
    if(buffer == NULL) {
        fprintf(stderr, "ERROR: Could not allocate memory for file '%s': %s\n", file_path, strerror(errno)); 
        exit(1);
    }
    if(fseek(f, 0, SEEK_SET) < 0) { 
        fprintf(stderr, "ERROR: Could not read file '%s': %s\n", file_path, strerror(errno)); 
        exit(1);
    }
    size_t n = fread(buffer, 1, m, f); 
    if(ferror(f)) {
        fprintf(stderr, "ERROR: Could not read file '%s': %s\n", file_path, strerror(errno)); 
        exit(1);
    }
    fclose(f); 
    return (String_View) {
        .count = n,
        .data = buffer, 
    }; 
}