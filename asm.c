#include "asm.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_GLOBALS 1000
#define MAX_TEMPS 10000
#define REGISTER_LIMIT 13

static const char *register_map[REGISTER_LIMIT] = {"%ebx",  "%ecx",  "%edx",  "%esi",  "%edi",  "%r8d", "%r9d",
                                                   "%r10d", "%r11d", "%r12d", "%r13d", "%r14d", "%r15d"};

typedef struct
{
    char name[64];
    int offset;
    bool used;
} global_var_t;

typedef struct
{
    int num_temps;
    int first_use[MAX_TEMPS];
    int last_use[MAX_TEMPS];
    int interference_graph[MAX_TEMPS][MAX_TEMPS];
    int color[MAX_TEMPS];
    global_var_t globals[MAX_GLOBALS];
    int num_globals;
} register_allocator_t;

static register_allocator_t allocator;

static void init_allocator(void)
{
    allocator.num_temps = 0;
    allocator.num_globals = 0;
    memset(allocator.first_use, -1, sizeof(allocator.first_use));
    memset(allocator.last_use, -1, sizeof(allocator.last_use));
    memset(allocator.interference_graph, 0, sizeof(allocator.interference_graph));
    memset(allocator.color, -1, sizeof(allocator.color));
    memset(allocator.globals, 0, sizeof(allocator.globals));
}

static bool is_temporary_register(const char *temp)
{
    return temp != NULL && temp[0] == 'r' && strcmp(temp, "rfp") && strcmp(temp, "rbss");
}

static bool is_global_reference(const char *operand)
{
    return operand != NULL && !strcmp(operand, "rbss");
}

static void add_global_variable(int offset)
{
    for (int i = 0; i < allocator.num_globals; i++)
    {
        if (allocator.globals[i].offset == offset)
        {
            allocator.globals[i].used = true;
            return;
        }
    }

    if (allocator.num_globals >= MAX_GLOBALS)
    {
        fprintf(stderr, "Error: Too many global variables\n");
        exit(1);
    }

    snprintf(allocator.globals[allocator.num_globals].name, 64, "global_%d", offset);
    allocator.globals[allocator.num_globals].offset = offset;
    allocator.globals[allocator.num_globals].used = true;
    allocator.num_globals++;
}

static int get_temp_number(const char *temp)
{
    if (!is_temporary_register(temp))
    {
        return -1;
    }
    return atoi(temp + 1);
}

static void mark_as_temporary_use(const char *temp, int line)
{
    if (!is_temporary_register(temp))
    {
        return;
    }

    int reg_num = get_temp_number(temp);
    if (reg_num < 0 || reg_num >= MAX_TEMPS)
    {
        fprintf(stderr, "Error: Invalid register number %d\n", reg_num);
        exit(1);
    }

    if (allocator.first_use[reg_num] == -1)
    {
        allocator.first_use[reg_num] = line;
        allocator.num_temps++;
    }
    allocator.last_use[reg_num] = line;
}

static void build_lifetime_table(code_t *code)
{
    init_allocator();

    code_t *current = code;
    int line = 1;

    while (current != NULL)
    {
        struct iloc_t inst = current->instruction;

        mark_as_temporary_use(inst.arg1, line);
        mark_as_temporary_use(inst.arg2, line);
        mark_as_temporary_use(inst.arg3, line);

        // Track global variable usage
        if (!strcmp(inst.mnemonic, "loadAI") && is_global_reference(inst.arg1))
        {
            add_global_variable(atoi(inst.arg2));
        }
        else if (!strcmp(inst.mnemonic, "storeAI") && is_global_reference(inst.arg2))
        {
            add_global_variable(atoi(inst.arg3));
        }

        current = current->next;
        line++;
    }
}

static bool temporaries_interfere(int temp1, int temp2)
{
    if (allocator.first_use[temp1] == -1 || allocator.first_use[temp2] == -1)
    {
        return false;
    }

    return (allocator.first_use[temp1] <= allocator.first_use[temp2] &&
            allocator.first_use[temp2] <= allocator.last_use[temp1]) ||
           (allocator.first_use[temp2] <= allocator.first_use[temp1] &&
            allocator.first_use[temp1] <= allocator.last_use[temp2]);
}

static void build_interference_graph(void)
{
    for (int temp1 = 0; temp1 < MAX_TEMPS; temp1++)
    {
        if (allocator.first_use[temp1] == -1)
            continue;

        for (int temp2 = temp1 + 1; temp2 < MAX_TEMPS; temp2++)
        {
            if (allocator.first_use[temp2] == -1)
                continue;

            if (temporaries_interfere(temp1, temp2))
            {
                allocator.interference_graph[temp1][temp2] = 1;
                allocator.interference_graph[temp2][temp1] = 1;
            }
        }
    }
}

static int calculate_node_degree(int node)
{
    int degree = 0;
    for (int j = 0; j < MAX_TEMPS; j++)
    {
        if (node != j && allocator.interference_graph[node][j] && allocator.color[j] == -1)
        {
            degree++;
        }
    }
    return degree;
}

static int find_max_degree_node(void)
{
    int max_degree = -1;
    int max_degree_node = -1;

    for (int i = 0; i < MAX_TEMPS; i++)
    {
        if (allocator.first_use[i] == -1 || allocator.color[i] != -1)
        {
            continue;
        }

        int node_degree = calculate_node_degree(i);
        if (node_degree > max_degree)
        {
            max_degree = node_degree;
            max_degree_node = i;
        }
    }

    return max_degree_node;
}

static bool is_color_available(int node, int color_candidate)
{
    for (int adj = 0; adj < MAX_TEMPS; adj++)
    {
        if (allocator.interference_graph[node][adj] && allocator.color[adj] == color_candidate)
        {
            return false;
        }
    }
    return true;
}

static bool assign_color_to_node(int node)
{
    for (int possible_color = 0; possible_color < REGISTER_LIMIT; possible_color++)
    {
        if (is_color_available(node, possible_color))
        {
            allocator.color[node] = possible_color;
            return true;
        }
    }
    return false;
}

static void color_interference_graph(void)
{
    int colored_nodes = 0;

    while (colored_nodes < allocator.num_temps)
    {
        int max_degree_node = find_max_degree_node();

        if (max_degree_node == -1)
        {
            fprintf(stderr, "Error: No uncolored node found\n");
            exit(1);
        }

        int max_degree = calculate_node_degree(max_degree_node);

        if (max_degree >= REGISTER_LIMIT)
        {
            fprintf(stderr, "Error: Program requires more registers than available (%d needed, %d available)\n",
                    max_degree + 1, REGISTER_LIMIT);
            exit(1);
        }

        if (!assign_color_to_node(max_degree_node))
        {
            fprintf(stderr, "Error: Could not assign color to node %d\n", max_degree_node);
            exit(1);
        }

        colored_nodes++;
    }
}

static void allocate_registers(code_t *code)
{
    build_lifetime_table(code);
    build_interference_graph();
    color_interference_graph();
}

static const char *retrieve_register(const char *temp)
{
    if (!is_temporary_register(temp))
    {
        return temp;
    }

    int temp_num = get_temp_number(temp);
    if (temp_num < 0 || temp_num >= MAX_TEMPS)
    {
        fprintf(stderr, "Error: Invalid temporary register %s\n", temp);
        exit(1);
    }

    int color_assigned = allocator.color[temp_num];
    if (color_assigned < 0 || color_assigned >= REGISTER_LIMIT)
    {
        fprintf(stderr, "Error: No register assigned to temporary %s\n", temp);
        exit(1);
    }

    return register_map[color_assigned];
}

static void generate_prev(void)
{
    printf("\t.globl main\n");
    printf("main:\n");
    printf(".LFB0:\n");
    printf("\tpushq\t%%rbp\n");
    printf("\tmovq\t%%rsp, %%rbp\n");
}

static void generate_bss_section(void)
{
    if (allocator.num_globals > 0)
    {
        printf("\t.bss\n");
        for (int i = 0; i < allocator.num_globals; i++)
        {
            if (allocator.globals[i].used)
            {
                printf("%s:\n", allocator.globals[i].name);
                printf("\t.zero\t4\n"); // 4 bytes for 32-bit integers
            }
        }
        printf("\t.text\n");
    }
}

static void generate_pos(void)
{
    printf(".LFE0:\n");
    printf("\t.section .note.GNU-stack,\"\",@progbits\n");
}

static const char *get_global_name(int offset)
{
    for (int i = 0; i < allocator.num_globals; i++)
    {
        if (allocator.globals[i].offset == offset)
        {
            return allocator.globals[i].name;
        }
    }
    fprintf(stderr, "Error: Global variable at offset %d not found\n", offset);
    exit(1);
}

static void translate_load_immediate(struct iloc_t inst)
{
    printf("\tmovl\t$%s, %s\n", inst.arg1, retrieve_register(inst.arg3));
}

static void translate_return(struct iloc_t inst)
{
    printf("\tmovl\t%s, %%eax\n", retrieve_register(inst.arg1));
    printf("\tpopq\t%%rbp\n");
    printf("\tret\n");
}

static void translate_load_ai(struct iloc_t inst)
{
    if (is_global_reference(inst.arg1))
    {
        // Global variable access: loadAI rbss, offset => reg
        int offset = atoi(inst.arg2);
        printf("\tmovl\t%s(%%rip), %s\n", get_global_name(offset), retrieve_register(inst.arg3));
    }
    else
    {
        // Local variable access: loadAI rfp, offset => reg
        printf("\tmovl\t-%d(%%rbp), %s\n", atoi(inst.arg2) + 4, retrieve_register(inst.arg3));
    }
}

static void translate_store_ai(struct iloc_t inst)
{
    if (is_global_reference(inst.arg2))
    {
        // Global variable access: storeAI reg => rbss, offset
        int offset = atoi(inst.arg3);
        printf("\tmovl\t%s, %s(%%rip)\n", retrieve_register(inst.arg1), get_global_name(offset));
    }
    else
    {
        // Local variable access: storeAI reg => rfp, offset
        printf("\tmovl\t%s, -%d(%%rbp)\n", retrieve_register(inst.arg1), atoi(inst.arg3) + 4);
    }
}

static void translate_arithmetic(struct iloc_t inst, const char *op)
{
    printf("\tmovl\t%s, %s\n", retrieve_register(inst.arg1), retrieve_register(inst.arg3));
    printf("\t%s\t%s, %s\n", op, retrieve_register(inst.arg2), retrieve_register(inst.arg3));
}

static void translate_division(struct iloc_t inst)
{
    printf("\tmovl\t%s, %%eax\n", retrieve_register(inst.arg1));
    printf("\tcltd\n");
    printf("\tidivl\t%s\n", retrieve_register(inst.arg2));
    printf("\tmovl\t%%eax, %s\n", retrieve_register(inst.arg3));
}

static void translate_reverse_subtract_immediate(struct iloc_t inst)
{
    printf("\tmovl\t$%s, %s\n", inst.arg2, retrieve_register(inst.arg3));
    printf("\tsubl\t%s, %s\n", retrieve_register(inst.arg1), retrieve_register(inst.arg3));
}

static void translate_comparison(struct iloc_t inst, const char *set_op)
{
    printf("\tcmpl\t%s, %s\n", retrieve_register(inst.arg2), retrieve_register(inst.arg1));
    printf("\t%s\t%%al\n", set_op);
    printf("\tmovzbl\t%%al, %%eax\n");
    printf("\tmovl\t%%eax, %s\n", retrieve_register(inst.arg3));
}

static void translate_logical_and(struct iloc_t inst)
{
    printf("\tmovl\t%s, %s\n", retrieve_register(inst.arg1), retrieve_register(inst.arg3));
    printf("\timull\t%s, %s\n", retrieve_register(inst.arg2), retrieve_register(inst.arg3));
    printf("\ttest\t%s, %s\n", retrieve_register(inst.arg3), retrieve_register(inst.arg3));
    printf("\tsetne\t%%al\n");
    printf("\tmovzbl\t%%al, %%eax\n");
    printf("\tmovl\t%%eax, %s\n", retrieve_register(inst.arg3));
}

static void translate_logical_or(struct iloc_t inst)
{
    printf("\tmovl\t%s, %s\n", retrieve_register(inst.arg1), retrieve_register(inst.arg3));
    printf("\tor\t%s, %s\n", retrieve_register(inst.arg2), retrieve_register(inst.arg3));
    printf("\ttest\t%s, %s\n", retrieve_register(inst.arg3), retrieve_register(inst.arg3));
    printf("\tsetne\t%%al\n");
    printf("\tmovzbl\t%%al, %%eax\n");
    printf("\tmovl\t%%eax, %s\n", retrieve_register(inst.arg3));
}

static void translate_conditional_branch(struct iloc_t inst)
{
    printf("\ttest\t%s, %s\n", retrieve_register(inst.arg1), retrieve_register(inst.arg1));
    printf("\tjne\t%s\n", inst.arg2);
    printf("\tjmp\t%s\n", inst.arg3);
}

static void translate_jump_immediate(struct iloc_t inst)
{
    printf("\tjmp\t%s\n", inst.arg2);
}

static void translate_instruction(struct iloc_t instruction)
{
    if (instruction.label != NULL)
    {
        printf("%s:\n", instruction.label);
    }

    if (!strcmp(instruction.mnemonic, "loadI"))
    {
        translate_load_immediate(instruction);
    }
    else if (!strcmp(instruction.mnemonic, "ret"))
    {
        translate_return(instruction);
    }
    else if (!strcmp(instruction.mnemonic, "loadAI"))
    {
        translate_load_ai(instruction);
    }
    else if (!strcmp(instruction.mnemonic, "storeAI"))
    {
        translate_store_ai(instruction);
    }
    else if (!strcmp(instruction.mnemonic, "add"))
    {
        translate_arithmetic(instruction, "addl");
    }
    else if (!strcmp(instruction.mnemonic, "sub"))
    {
        translate_arithmetic(instruction, "subl");
    }
    else if (!strcmp(instruction.mnemonic, "mult"))
    {
        translate_arithmetic(instruction, "imull");
    }
    else if (!strcmp(instruction.mnemonic, "div"))
    {
        translate_division(instruction);
    }
    else if (!strcmp(instruction.mnemonic, "rsubI"))
    {
        translate_reverse_subtract_immediate(instruction);
    }
    else if (!strcmp(instruction.mnemonic, "cmp_EQ"))
    {
        translate_comparison(instruction, "sete");
    }
    else if (!strcmp(instruction.mnemonic, "cmp_NE"))
    {
        translate_comparison(instruction, "setne");
    }
    else if (!strcmp(instruction.mnemonic, "cmp_LE"))
    {
        translate_comparison(instruction, "setle");
    }
    else if (!strcmp(instruction.mnemonic, "cmp_LT"))
    {
        translate_comparison(instruction, "setl");
    }
    else if (!strcmp(instruction.mnemonic, "cmp_GE"))
    {
        translate_comparison(instruction, "setge");
    }
    else if (!strcmp(instruction.mnemonic, "cmp_GT"))
    {
        translate_comparison(instruction, "setg");
    }
    else if (!strcmp(instruction.mnemonic, "and"))
    {
        translate_logical_and(instruction);
    }
    else if (!strcmp(instruction.mnemonic, "or"))
    {
        translate_logical_or(instruction);
    }
    else if (!strcmp(instruction.mnemonic, "cbr"))
    {
        translate_conditional_branch(instruction);
    }
    else if (!strcmp(instruction.mnemonic, "jumpI"))
    {
        translate_jump_immediate(instruction);
    }
    else if (!strcmp(instruction.mnemonic, "nop"))
    {
        printf("\tnop\n");
    }
    else
    {
        fprintf(stderr, "Warning: Unknown instruction mnemonic: %s\n", instruction.mnemonic);
    }
}

void add_comment(const char *comment)
{
    if (comment == NULL || strlen(comment) == 0)
    {
        return;
    }

    char *comment_copy = strdup(comment);
    char *line = strtok(comment_copy, "\n");

    while (line != NULL)
    {
        if (strlen(line) > 0)
        {
            printf("\t# %s\n", line);
        }
        line = strtok(NULL, "\n");
    }

    free(comment_copy);
}

void generate_asm(code_t *ir_code, const char *comments)
{
    if (comments != NULL && strlen(comments) > 0)
    {
        add_comment(comments);
    }

    allocate_registers(ir_code);
    generate_bss_section();
    generate_prev();

    code_t *current = ir_code;
    while (current != NULL)
    {
        translate_instruction(current->instruction);
        current = current->next;
    }

    generate_pos();
}