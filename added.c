#include "added.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//helper functions that not in added.h
void maintain_symbol_table(int update_flag) {
    if (update_flag == 0) return;
    symbol_tb.next_index += 1;
}
void maintain_reg_table(int update_flag) {
    if (update_flag == 0) return;
    reg_tb.next_save = -1;
    reg_tb.next_temp = -1;
    for (int i = 0; i < 8; i++) {
        if (reg_tb.reg_list[i].if_busy != 2) {
            reg_tb.next_save = i;
            break;
        }
    }
    for (int i = 8; i < 16; i++) {
        if (reg_tb.reg_list[i].if_busy != 2) {
            reg_tb.next_temp = i;
            break;
        }
    }
}
int search_symbol_table(char *ID) {
    int index = -1;
    for (int i = 0; i < symbol_tb.size; i++) {
        if (symbol_tb.symbol_list[i].status != 1 && symbol_tb.symbol_list[i].status != 2) continue;
        if (strcmp(ID, symbol_tb.symbol_list[i].id) == 0) {
            index = i;
            break;
        }
    }
    return index;
}

//functions in added.h
//statement
void assignment(char *ID, struct expr_struct* expr) {
    int s_id = symbol_tb.next_index;
    int r_id = reg_tb.next_save;
    struct symbol* s_target = &(symbol_tb.symbol_list[symbol_tb.next_index]);
    struct reg* r_target = &(reg_tb.reg_list[18]);
    if (r_id >= 0 && r_id < 8) r_target = &(reg_tb.reg_list[reg_tb.next_save]);
    int memory_flag = 0; //if the ID is stored in memory, it will be 1
    int update_flag = 1; //if the symbol table and register table need to be updated, it will be 1
    FILE *output = fopen("./output.asm", "a");

    //first find if the ID is already saved in register, 
    s_id = search_symbol_table(ID);
    // if found, update s_id, r_id
    if (s_id >= 0) {
        update_flag = 0;
        s_target = &(symbol_tb.symbol_list[s_id]);
        //if the symbol is saved in register
        if(s_target->status == 1) {
            r_target = &(reg_tb.reg_list[s_target->reg_id]);
            r_id = s_target->reg_id;
        }
        //if the symbol is saved in memory
        else if (s_target->status == 2) {
            memory_flag = 1;
        }
    }
    //if not, use a new save register to save its value
    if (s_id < 0) s_id = symbol_tb.next_index;

    sscanf(ID, "%s", s_target->id);
    //if the expression is an integer, and the saved register is not full, and the ID is not in memory, found/not found
    if (expr->my_type == 1 && r_id != -1 && memory_flag == 0) {
        //save symbol
        s_target->reg_id = r_id;
        s_target->status = 1;
        //save register
        r_target->symbol_id = s_id;
        r_target->if_busy = 2;
        r_target->value = expr->my_value;
        fprintf(output, "addi $s%d $zero %d\n", r_id, expr->my_value);
    }
    //if the expression is in register, and the saved register is not full, and the ID is not in memory, found/not found
    else if (expr->my_type == 2 && r_id != -1 && memory_flag == 0) {
        //save symbol
        s_target->reg_id = r_id;
        s_target->status = 1;
        //save register
        r_target->symbol_id = s_id;
        r_target->if_busy = 2;
        if (expr->reg_id < 8) fprintf(output, "add $s%d $zero $s%d\n", r_id, expr->reg_id);
        else fprintf(output, "add $s%d $zero $t%d\n", r_id, expr->reg_id - 8);
    }
    //if the expression is in register, and the saved register is not full, and the ID is not in memory, found/not found
    else if (expr->my_type == 3 && r_id != -1 && memory_flag == 0) {
        //save symbol
        s_target->reg_id = r_id;
        s_target->status = 1;
        //load expression value from memory
        fprintf(output, "lw $t8 %d($sp)\n", expr->mem_pos);
        //save register
        r_target->if_busy = 2;
        fprintf(output, "add $s%d $zero $t8\n", r_id);
    }
    //if the expression is integer, and the saved register is full, and the ID is not found in symbol table, do the memory operation
    else if (expr->my_type == 1 && r_id == -1 && memory_flag == 0) {
        //save symbol
        s_target->status = 2;
        s_target->mem_pos = sp_reg;
        //save memory
        fprintf(output, "addi $t8 $zero %d\n", expr->my_value);
        fprintf(output, "sw $t8 %d($sp)\n", sp_reg);
        sp_reg = sp_reg - 4;
    }
    //if the expression is in register, and the saved register is full, and the ID is not found in symbol table, do the memory operation
    else if (expr->my_type == 2 && r_id == -1 && memory_flag == 0) {
        //save symbol
        s_target->status = 2;
        s_target->mem_pos = sp_reg;
        //save memory
        if (expr->reg_id >= 8) fprintf(output, "add $t8 $zero $t%d\n", expr->reg_id - 8);
        else fprintf(output, "add $t8 $zero $s%d\n", expr->reg_id);
        fprintf(output, "sw $t8 %d($sp)\n", sp_reg);
        sp_reg = sp_reg - 4;
    }
    //if the expression is in memory, and the saved register is full, and the ID is not found in symbol table, do the memory operation
    else if (expr->my_type == 3 && r_id == -1 && memory_flag == 0) {
        //save symbol
        s_target->status = 2;
        s_target->mem_pos = sp_reg;
        //load expression value from memory
        fprintf(output, "lw $t8 %d($sp)\n", expr->mem_pos);
        //save memory
        fprintf(output, "sw $t8 %d($sp)\n", sp_reg);
        sp_reg = sp_reg - 4;
    }
    //if the expression is integer, and the ID is found in symbol table in memory, do the memory operation
    else if (expr->my_type == 1 && memory_flag == 1) {
        //save memory
        fprintf(output, "addi $t8 $zero %d\n", expr->my_value);
        fprintf(output, "sw $t8 %d($sp)\n", s_target->mem_pos);
    }
    //if the expression is in register, and the ID is found in symbol table in memory, do the memory operation
    else if (expr->my_type == 2 && memory_flag == 1) {
        //save memory
        if (expr->reg_id >= 8) fprintf(output, "add $t8 $zero $t%d\n", expr->reg_id - 8);
        else fprintf(output, "add $t8 $zero $s%d\n", expr->reg_id);
        fprintf(output, "sw $t8 %d($sp)\n", s_target->mem_pos);
    }
    //if the expression is in memory, and the ID is found in symbol table in memory, do the memory operation
    else if (expr->my_type == 3 && memory_flag == 1) {
        //load expression valu from memory
        fprintf(output, "lw $t8 %d($sp)\n", expr->mem_pos);
        //save memory
        fprintf(output, "sw $t8 %d($sp)\n", s_target->mem_pos);
    }

    //free temp reg of expresion, if needed
    if (expr->my_type == 2 && expr->reg_id >= 8) {
        reg_tb.reg_list[expr->reg_id].if_busy = 1;
    }
    fclose(output);
    maintain_reg_table(update_flag);
    maintain_symbol_table(update_flag);
}

void read_func(struct id_list_struct *id_list) {
    return;
}

void write_func(struct expr_list_struct *expr_list) {
    return;
}

//id list
void id_to_id_list(char *ID, struct id_list_struct *id_list) {
    FILE *output = fopen("output.asm", "a");

    //set v0 be 5, start to read
    fprintf(output, "addi $v0 $zero 5\n");
    //syscall, the read integer stored in $v0
    fprintf(output, "syscall\n");

    char target[32];
    sscanf(ID, "%s", target);
    int s_index = search_symbol_table(target);
    struct symbol *s_target;
    //if the ID is already in symbol table
    if (s_index >= 0) {
        s_target = &(symbol_tb.symbol_list[s_index]);
        if (s_target->status == 1) {
            if (s_target->reg_id < 8) fprintf(output, "add $s%d $zero $v0\n", s_target->reg_id);
        }
        else if (s_target->status == 2) {
            fprintf(output, "sw $v0 %d($sp)\n", s_target->mem_pos);
        }
    }
    //if the ID is not in symbol table
    else {
        s_target = &(symbol_tb.symbol_list[symbol_tb.next_index]);
        sscanf(target, "%s", s_target->id);
        int r_id = reg_tb.next_save;
        //if there are free save register
        if (r_id >= 0) {
            struct reg *r_target = &(reg_tb.reg_list[r_id]);
            r_target->if_busy = 2;
            r_target->symbol_id = s_index;
            s_target->reg_id = r_id;
            s_target->status = 1;
            fprintf(output, "add $s%d $zero $v0\n", r_id);
        }
        // if save register is full, save the symbol in memory
        else {
            s_target->status = 2;
            s_target->mem_pos = sp_reg;
            fprintf(output, "sw $v0 %d($sp)\n", sp_reg);
            sp_reg = sp_reg - 4;
        }
        maintain_reg_table(1);
        maintain_symbol_table(1);
    }

    fclose(output);
}

void id_list_id_to_id_list(struct id_list_struct *id_list, char *ID, struct id_list_struct *result) {
    FILE *output = fopen("output.asm", "a");

    //set v0 be 5, start to read
    fprintf(output, "addi $v0 $zero 5\n");
    //syscall, the read integer stored in $v0
    fprintf(output, "syscall\n");

    char target[32];
    sscanf(ID, "%s", target);
    int s_index = search_symbol_table(target);
    struct symbol *s_target;
    //if the ID is already in symbol table
    if (s_index >= 0) {
        s_target = &(symbol_tb.symbol_list[s_index]);
        if (s_target->status == 1) {
            if (s_target->reg_id < 8) fprintf(output, "add $s%d $zero $v0\n", s_target->reg_id);
        }
        else if (s_target->status == 2) {
            fprintf(output, "sw $v0 %d($sp)\n", s_target->mem_pos);
        }
    }
    //if the ID is not in symbol table
    else {
        s_target = &(symbol_tb.symbol_list[symbol_tb.next_index]);
        sscanf(target, "%s", s_target->id);
        int r_id = reg_tb.next_save;
        //if there are free save register
        if (r_id >= 0) {
            struct reg *r_target = &(reg_tb.reg_list[r_id]);
            r_target->if_busy = 2;
            r_target->symbol_id = s_index;
            s_target->reg_id = r_id;
            s_target->status = 1;
            fprintf(output, "add $s%d $zero $v0\n", r_id);
        }
        // if save register is full, save the symbol in memory
        else {
            s_target->status = 2;
            s_target->mem_pos = sp_reg;
            fprintf(output, "sw $v0 %d($sp)\n", sp_reg);
            sp_reg = sp_reg - 4;
        }
        maintain_reg_table(1);
        maintain_symbol_table(1);
    }

    fclose(output);
}

//expression list
void expr_to_expr_list(struct expr_struct *expr, struct expr_list_struct *expr_list) {
    FILE *output = fopen("./output.asm", "a");

    struct expr_struct *target = expr;
    if (target->my_type == 1) {
        fprintf(output, "addi $v0 $zero 1\n");
        fprintf(output, "addi $a0 $zero %d\n", target->my_value);
        fprintf(output, "syscall\n");
    }
    else if (target->my_type == 2) {
        char reg_type = 's'; int reg_id = target->reg_id;
        if (reg_id >= 8) {
            reg_type = 't';
            reg_id = reg_id - 8;
        }
        fprintf(output, "addi $v0 $zero 1\n");
        fprintf(output, "add $a0 $zero $%c%d\n", reg_type, reg_id);
        fprintf(output, "syscall\n");
    }
    else if (target->my_type == 3) {
        fprintf(output, "addi $v0 $zero 1\n");
        fprintf(output, "lw $a0 %d($sp)\n", target->mem_pos);
        fprintf(output, "syscall\n");
    }

    fclose(output);
}

void expr_list_expr_to_expr_list(struct expr_list_struct *expr_list, struct expr_struct *expr, struct expr_list_struct *result) {
    FILE *output = fopen("./output.asm", "a");

    struct expr_struct *target = expr;
    if (target->my_type == 1) {
        fprintf(output, "addi $v0 $zero 1\n");
        fprintf(output, "addi $a0 $zero %d\n", target->my_value);
        fprintf(output, "syscall\n");
    }
    else if (target->my_type == 2) {
        char reg_type = 's'; int reg_id = target->reg_id;
        if (reg_id >= 8) {
            reg_type = 't';
            reg_id = reg_id - 8;
        }
        fprintf(output, "addi $v0 $zero 1\n");
        fprintf(output, "add $a0 $zero $%c%d\n", reg_type, reg_id);
        fprintf(output, "syscall\n");
    }
    else if (target->my_type == 3) {
        fprintf(output, "addi $v0 $zero 1\n");
        fprintf(output, "lw $a0 %d($sp)\n", target->mem_pos);
        fprintf(output, "syscall\n");
    }

    fclose(output);
}

//expression
void primary_to_expression(struct primary_struct* prim, struct expr_struct* expr) {
    expr->my_type = prim->my_type;
    expr->my_value = prim->my_value;
    expr->reg_id = prim->reg_id;
    expr->mem_pos = prim->mem_pos;
}
//
void expr_op_prim_to_expression(struct expr_struct* expr, int op, struct primary_struct* prim, struct expr_struct* result) {
    FILE *output = fopen("./output.asm", "a");
    int r_id = reg_tb.next_temp - 8;
    struct reg* r_target = &(reg_tb.reg_list[18]);
    if (r_id >= 0) r_target = &(reg_tb.reg_list[r_id + 8]);

    // if all the temp register is full, save in memory
    if (r_id < 0) {
        result->my_type = 3;
        result->mem_pos = sp_reg;
        if (expr->my_type == 2 && prim->my_type == 2) {
            char expr_reg_type = 's'; int expr_reg_id = expr->reg_id;
            char prim_reg_type = 's'; int prim_reg_id = prim->reg_id;
            if (expr->reg_id >= 8) {
                expr_reg_type = 't';
                expr_reg_id = expr_reg_id - 8;
            }
            if (prim->reg_id >= 8) {
                prim_reg_type = 't';
                prim_reg_id = prim_reg_id - 8;
            }
            if (op == 267) fprintf(output, "add $t8 $%c%d $%c%d\n", expr_reg_type, expr_reg_id, prim_reg_type, prim_reg_id);
            else fprintf(output, "sub $t8 $%c%d $%c%d\n", expr_reg_type, expr_reg_id, prim_reg_type, prim_reg_id);
            fprintf(output, "sw $t8 %d($sp)\n", sp_reg);
        }
        if (expr->my_type == 1 && prim->my_type == 2) {
            char prim_reg_type = 's'; int prim_reg_id = prim->reg_id;
            if (prim->reg_id >= 8) {
                prim_reg_type = 't';
                prim_reg_id = prim_reg_id - 8;
            }
            if (op == 267) fprintf(output, "addi $t8 $%c%d %d\n", prim_reg_type, prim_reg_id, expr->my_value);
            else {
                fprintf(output, "sub $t8 $zero $%c%d\n", prim_reg_type, prim_reg_id);
                fprintf(output, "addi $t8 $t8 %d\n", expr->my_value);
            }
            fprintf(output, "sw $t8 %d($sp)\n", sp_reg);
        }
        if (expr->my_type == 2 && prim->my_type == 1) {
            char expr_reg_type = 's'; int expr_reg_id = expr->reg_id;
            if (expr->reg_id >= 8) {
                expr_reg_type = 't';
                expr_reg_id = expr_reg_id - 8;
            }
            
            if (op == 267) fprintf(output, "addi $t8 $%c%d %d\n", expr_reg_type, expr_reg_id, prim->my_value);
            else fprintf(output, "addi $t8 $%c%d %d\n", expr_reg_type, expr_reg_id, -(prim->my_value));
            fprintf(output, "sw $t8 %d($sp)\n", sp_reg);
        }
        if (expr->my_type == 1 && prim->my_type == 1) {
            if (op == 267) {
                fprintf(output, "addi $t8 $zero %d\n", expr->my_value);
                fprintf(output, "addi $t8 $t8 %d\n", prim->my_value);
            }
            else {
                fprintf(output, "addi $t8 $zero %d\n", -prim->my_value);
                fprintf(output, "addi $t8 $t8 %d\n", expr->my_value);
            }
            fprintf(output, "sw $t8 %d($sp)\n", sp_reg);
        }
        if (expr->my_type == 3 && prim->my_type == 1) {
            //load memory
            fprintf(output, "lw $t8 %d($sp)\n", expr->mem_pos);
            //save memory
            if (op == 267) fprintf(output, "addi $t8 $t8 %d\n", prim->my_value);
            else fprintf(output, "addi $t8 $t8 %d\n", -(prim->my_value));
            fprintf(output, "sw $t8 %d($sp)\n", sp_reg);
        }
        if (expr->my_type == 3 && prim->my_type == 2) {
            //load memory
            fprintf(output, "lw $t8 %d($sp)\n", expr->mem_pos);
            //save register
            char prim_reg_type = 's'; int prim_reg_id = prim->reg_id;
            if (prim->reg_id >= 8) {
                prim_reg_type = 't';
                prim_reg_id = prim_reg_id - 8;
            }
            if (op == 267) fprintf(output, "add $t8 $t8 $%c%d\n", prim_reg_type, prim_reg_id);
            else fprintf(output, "sub $t8 $t8 $%c%d\n", prim_reg_type, prim_reg_id);
            fprintf(output, "sw $t8 %d($sp)\n", sp_reg);
        }
        if (expr->my_type == 3 && prim->my_type == 3) {
            //load memory
            fprintf(output, "lw $t8 %d($sp)\n", expr->mem_pos);
            fprintf(output, "lw $t9 %d($sp)\n", prim->mem_pos);
            //save memory
            if (op == 267) fprintf(output, "add $t8 $t8 $t9\n");
            else fprintf(output, "sub $t8 $t8 $t9\n");
            fprintf(output, "sw $t8 %d($sp)\n", sp_reg);
        }
        if (expr->my_type == 2 && prim->my_type == 3) {
            //load memory
            fprintf(output, "lw $t8 %d($sp)\n", prim->mem_pos);
            //save memory
            char expr_reg_type = 's'; int expr_reg_id = expr->reg_id;
            if (expr->reg_id >= 8) {
                expr_reg_type = 't';
                expr_reg_id = expr_reg_id - 8;
            }
            if (op == 267) fprintf(output, "add $t8 $%c%d $t8\n", expr_reg_type, expr_reg_id);
            else fprintf(output, "sub $t8 $%c%d $t8\n", expr_reg_type, expr_reg_id);
            fprintf(output, "sw $t8 %d($sp)\n", sp_reg);
        }
        if (expr->my_type == 1 && prim->my_type == 3) {
            //load memory
            fprintf(output, "lw $t8 %d($sp)\n", prim->mem_pos);
            //save memory
            if (op == 267) fprintf(output, "addi $t8 $t8 %d\n", expr->my_value);
            else {
                fprintf(output, "sub $t8 $zero $t8\n");
                fprintf(output, "addi $t8 $t8 %d\n", expr->my_value);
            }
            fprintf(output, "sw $t8 %d($sp)\n", sp_reg);
        }
        sp_reg = sp_reg - 4;
    }
    // if there are still free temp register
    else {
        r_target->if_busy = 2;
        result->my_type = 2;
        result->reg_id = r_id + 8;
        if (expr->my_type == 2 && prim->my_type == 2) {
            char expr_reg_type = 's'; int expr_reg_id = expr->reg_id;
            char prim_reg_type = 's'; int prim_reg_id = prim->reg_id;
            if (expr->reg_id >= 8) {
                expr_reg_type = 't';
                expr_reg_id = expr_reg_id - 8;
            }
            if (prim->reg_id >= 8) {
                prim_reg_type = 't';
                prim_reg_id = prim_reg_id - 8;
            }
            if (op == 267) fprintf(output, "add $t%d $%c%d $%c%d\n", r_id, expr_reg_type, expr_reg_id, prim_reg_type, prim_reg_id);
            else fprintf(output, "sub $t%d $%c%d $%c%d\n", r_id, expr_reg_type, expr_reg_id, prim_reg_type, prim_reg_id);
        }
        if (expr->my_type == 1 && prim->my_type == 2) {
            char prim_reg_type = 's'; int prim_reg_id = prim->reg_id;
            if (prim->reg_id >= 8) {
                prim_reg_type = 't';
                prim_reg_id = prim_reg_id - 8;
            }
            if (op == 267) fprintf(output, "addi $t%d $%c%d %d\n", r_id, prim_reg_type, prim_reg_id, expr->my_value);
            else {
                fprintf(output, "sub $t%d $zero $%c%d\n", r_id, prim_reg_type, prim_reg_id);
                fprintf(output, "addi $t%d $t%d %d\n", r_id, r_id, expr->my_value);
            }
        }
        if (expr->my_type == 2 && prim->my_type == 1) {
            char expr_reg_type = 's'; int expr_reg_id = expr->reg_id;
            if (expr->reg_id >= 8) {
                expr_reg_type = 't';
                expr_reg_id = expr_reg_id - 8;
            }
            
            if (op == 267) fprintf(output, "addi $t%d $%c%d %d\n", r_id, expr_reg_type, expr_reg_id, prim->my_value);
            else fprintf(output, "addi $t%d $%c%d %d\n", r_id, expr_reg_type, expr_reg_id, -(prim->my_value));
        }
        if (expr->my_type == 1 && prim->my_type == 1) {
            if (op == 267) {
                fprintf(output, "addi $t%d $zero %d\n", r_id, expr->my_value);
                fprintf(output, "addi $t%d $t%d %d\n", r_id, r_id, prim->my_value);
            }
            else {
                fprintf(output, "addi $t%d $zero %d\n", r_id, -prim->my_value);
                fprintf(output, "addi $t%d $t%d %d\n", r_id, r_id, expr->my_value);
            }
        }
        if (expr->my_type == 3 && prim->my_type == 1) {
            //load memory
            fprintf(output, "lw $t8 %d($sp)\n", expr->mem_pos);
            //save register
            if (op == 267) fprintf(output, "addi $t%d $t8 %d\n", r_id, prim->my_value);
            else fprintf(output, "addi $t%d $t8 %d\n", r_id, -(prim->my_value));
        }
        if (expr->my_type == 3 && prim->my_type == 2) {
            //load memory
            fprintf(output, "lw $t8 %d($sp)\n", expr->mem_pos);
            //save register
            char prim_reg_type = 's'; int prim_reg_id = prim->reg_id;
            if (prim->reg_id >= 8) {
                prim_reg_type = 't';
                prim_reg_id = prim_reg_id - 8;
            }
            if (op == 267) fprintf(output, "add $t%d $t8 $%c%d\n", r_id, prim_reg_type, prim_reg_id);
            else fprintf(output, "sub $t%d $t8 $%c%d\n", r_id, prim_reg_type, prim_reg_id);
        }
        if (expr->my_type == 3 && prim->my_type == 3) {
            //load memory
            fprintf(output, "lw $t8 %d($sp)\n", expr->mem_pos);
            fprintf(output, "lw $t9 %d($sp)\n", prim->mem_pos);
            //save register
            if (op == 267) fprintf(output, "add $t%d $t8 $t9\n", r_id);
            else fprintf(output, "sub $t%d $t8 $t9\n", r_id);
        }
        if (expr->my_type == 2 && prim->my_type == 3) {
            //load memory
            fprintf(output, "lw $t8 %d($sp)\n", prim->mem_pos);
            //save register
            char expr_reg_type = 's'; int expr_reg_id = expr->reg_id;
            if (expr->reg_id >= 8) {
                expr_reg_type = 't';
                expr_reg_id = expr_reg_id - 8;
            }
            if (op == 267) fprintf(output, "add $t%d $%c%d $t8\n", r_id, expr_reg_type, expr_reg_id);
            else fprintf(output, "sub $t%d $%c%d $t8\n", r_id, expr_reg_type, expr_reg_id);
        }
        if (expr->my_type == 1 && prim->my_type == 3) {
            //load memory
            fprintf(output, "lw $t8 %d($sp)\n", prim->mem_pos);
            //save register
            if (op == 267) fprintf(output, "addi $t%d $t8 %d\n", r_id, expr->my_value);
            else {
                fprintf(output, "sub $t%d $zero $t8\n", r_id);
                fprintf(output, "addi $t%d $t%d %d\n", r_id, r_id, expr->my_value);
            }
        }
    }
    //free temp reg of expresion and primary, if needed
    if (expr->my_type == 2 && expr->reg_id >= 8) {
        reg_tb.reg_list[expr->reg_id].if_busy = 1;
    }
    if (prim->my_type == 2 && prim->reg_id >= 8) {
        reg_tb.reg_list[prim->reg_id].if_busy = 1;
    }
    maintain_reg_table(1);
    fclose(output);
}

//primary
void lp_expression_rp_to_primary(struct expr_struct* expr, struct primary_struct* prim) {
    prim->my_type = expr->my_type;
    prim->my_value = expr->my_value;
    prim->mem_pos = expr->mem_pos;
    prim->reg_id = expr->reg_id;
}
//
void id_to_primary(char *ID, struct primary_struct* prim) {
    //first find if the ID is already saved in register, 
    FILE *output = fopen("./output.asm", "a");
    int s_id = search_symbol_table(ID);
    int r_id = reg_tb.next_save;
    struct symbol* s_target = &(symbol_tb.symbol_list[symbol_tb.next_index]);
    struct reg* r_target = &(reg_tb.reg_list[18]);
    if (r_id >= 0 && r_id < 8) r_target = &(reg_tb.reg_list[reg_tb.next_save]);
    int memory_flag = 0; //if the ID is stored in memory, it will be 1
    int update_flag = 1; //if the symbol table and register table need to be updated, it will be 1

    // if found, update s_id, r_id
    if (s_id >= 0) {
        update_flag = 0;
        s_target = &(symbol_tb.symbol_list[s_id]);
        //if the symbol is saved in register
        if(s_target->status == 1) {
            r_target = &(reg_tb.reg_list[s_target->reg_id]);
            r_id = s_target->reg_id;
        }
        //if the symbol is saved in memory
        else if (s_target->status == 2) {
            memory_flag = 1;
        }
    }
    //if not, use a new symbol to save it
    if (s_id < 0) {
        s_id = symbol_tb.next_index;
    }

    sscanf(ID, "%s", s_target->id);
    //(not found, not full) or (found, in register)
    if (r_id != -1 && memory_flag == 0) {
        //save symbol
        s_target->reg_id = r_id;
        s_target->status = 1;
        //save register
        r_target->symbol_id = s_id;
        r_target->if_busy = 2;
        //pass primary
        prim->my_type = 2;
        prim->reg_id = r_id;
    }
    // (found, in memory)
    else if (memory_flag == 1) {
        //pass the memory position from ID to primary
        prim->mem_pos = s_target->mem_pos;
        prim->my_type = 3;
    }
    // (not found, full) save the ID(initial 0) in memory
    else if (r_id == -1) {
        s_target->mem_pos = sp_reg;
        s_target->status = 2;
        prim->my_type = 3;
        prim->mem_pos = sp_reg;
        fprintf(output, "add $t8 $zero $zero\n");
        fprintf(output, "sw $t8 %d($sp)\n", sp_reg);
        sp_reg = sp_reg - 4;
    }
    maintain_reg_table(update_flag);
    maintain_symbol_table(update_flag);
    fclose(output);
}

void integer_to_primary(int intliteral, struct primary_struct* prim) {
    // printf("read: %d\n", intliteral);
    prim->my_type = 1;
    prim->my_value = intliteral;
}

void op_primary_to_primary(struct primary_struct* prim, struct primary_struct* op_prim, int op) {
    FILE *output = fopen("./output.asm", "a");
    prim->my_type = op_prim->my_type;
    if (op_prim->my_type == 1) {
        //if the type is integer
        if (op == 267) prim->my_value = op_prim->my_value;
        else if (op == 268) prim->my_value = - op_prim->my_value;
    }
    else if (op_prim->my_type == 2) {
        if (op == 267) {
            prim->reg_id = op_prim->reg_id;
        }
        else if (op == 268) {
            prim->my_type = 3;
            prim->mem_pos = sp_reg;
            char op_prim_reg_type = 's'; int op_prim_reg_id = op_prim->reg_id;
            if (prim->reg_id >= 8) {
                op_prim_reg_type = 't';
                op_prim_reg_id = op_prim_reg_id - 8;
            }
            fprintf(output, "sub $t8 $zero $%c%d\n", op_prim_reg_type, op_prim_reg_id);
            fprintf(output, "sw $t8 %d($sp)\n", sp_reg);
            sp_reg = sp_reg - 4;
            
            if (op_prim->reg_id >= 8) {
                reg_tb.reg_list[op_prim->reg_id].if_busy = 1;
            }
        }
    }
    else if (op_prim->my_type == 3) {
        if (op == 267) {
            prim->mem_pos = op_prim->mem_pos;
        }
        else if (op == 268) {
            prim->mem_pos = sp_reg;
            fprintf(output, "lw $t8 %d($sp)\n", op_prim->mem_pos);
            fprintf(output, "sub $t8 $zero $t8\n");
            fprintf(output, "sw $t8 %d($sp)\n", sp_reg);
            sp_reg = sp_reg - 4;
        }
    }
    fclose(output);
}