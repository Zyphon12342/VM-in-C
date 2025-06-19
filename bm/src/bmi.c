#include "./bm.c"
//FOR RUNNING A BM FILE

int main(int arc, char ** argv) 
{
    if(arc < 2) {
        fprintf(stderr, "Usage: ./bmi <input.bm>\n"); 
        fprintf(stderr, "ERROR: expected input\n"); 
        exit(1);
    }


    // bm_load_program_from_memory(&bm, program, ARRAY_SIZE(program)); 
    bm_load_program_from_file(&bm, argv[1]); 
    bm_dump_stack(stdout, &bm); 
    for(size_t i = 0; i < BM_EXECUTION_LIMIT && !bm.halt; i++) { 
        Err err = bm_execute_inst(&bm);
        bm_dump_stack(stdout, &bm); 
        if(err != ERR_OK) {
            fprintf(stderr, "Error: %s\n", Err_as_cstr(err)); 
            exit(1); 
        }
    }
    // bm_dump_stack(stdout, &bm);
    return 0; 
}   