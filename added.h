#ifndef _ADDED_H_
#define _ADDED_H_

struct expr_struct {
    int my_type; //int: 1, in register: 2, in memory: 3
    int my_value; //used for int
    int reg_id; //used for reg
    int mem_pos; //used for memory saved position in $sp
};

struct expr_list_struct {
    int size; //no need for any parameters in this struct
};

struct primary_struct {
    int my_type; //int: 1, in register: 2, in memory: 3
    int my_value;
    int reg_id;
    int mem_pos; //saved position in $sp
};

struct id_list_struct {
    int size; //no need for any parameters in this struct
};

struct reg {
    int value;
    int symbol_id;
    int if_busy; //busy: 2 not busy: 1
};

struct symbol {
    char id[32];
    int reg_id;
    int status; //register: 1, memory: 2, unused: other
    int mem_pos; //saved position in $sp
};

struct symbol_table {
    int size;
    int next_index;
    struct symbol symbol_list[100];
};

struct reg_table {
    int size;
    int next_save; //next index of saved reg, if saved reg is all busy, the value will be -1
    int next_temp; //next index of temp reg
    // 0-7(s0-s7) saved reg 8-15(t0-t7) temp reg 16-18(t8 t9 at) syscall
    struct reg reg_list[19];
};

static struct reg_table reg_tb = {size: 19, next_save: 0, next_temp: 8};

static struct symbol_table symbol_tb = {size: 60, next_index: 0};

static int sp_reg = 0;

//statement
void assignment(char *ID, struct expr_struct* expr);

void read_func(struct id_list_struct *id_list);

void write_func(struct expr_list_struct *expr_list);

//id list
void id_to_id_list(char *ID, struct id_list_struct *id_list);

void id_list_id_to_id_list(struct id_list_struct *id_list, char *ID, struct id_list_struct *result);

//expression list
void expr_to_expr_list(struct expr_struct *expr, struct expr_list_struct *expr_list);

void expr_list_expr_to_expr_list(struct expr_list_struct *expr_list, struct expr_struct *expr, struct expr_list_struct *result);

//expression
void primary_to_expression(struct primary_struct* prim, struct expr_struct* expr);

void expr_op_prim_to_expression(struct expr_struct* expr, int op, struct primary_struct* prim, struct expr_struct* result);

// primary
void lp_expression_rp_to_primary(struct expr_struct* expr, struct primary_struct* prim);

void id_to_primary(char *ID, struct primary_struct* prim);

void integer_to_primary(int intliteral, struct primary_struct* prim);

void op_primary_to_primary(struct primary_struct* prim, struct primary_struct* op_prim, int op);


#endif