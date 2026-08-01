//#include "c_parser.c"

typedef struct block_s *block_p;
typedef struct transition_s *transition_p;

struct block_s
{
    int start_line;
    char *comment;
    int nr_statements;
    statement_p *statements;
    expr_p cond;
    block_p next;
    block_p alt;
    transition_p in_trans;
    node_p node;
    // For generating a flowchart
    int output_nr;
    int x;
    int y;
    int escape;
    block_p next_all;
};

struct transition_s
{
    block_p from;
    transition_p next_in_trans;
};

block_p new_block(void)
{
    block_p block = (block_p)malloc(sizeof(struct block_s));
    block->start_line = 0;
    block->comment = 0;
    block->nr_statements = 0;
    block->statements = NULL;
    block->cond = NULL;
    block->next = NULL;
    block->alt = NULL;
    block->in_trans = NULL;
    block->node = NULL;
    block->output_nr = 0;
    block->x = -1;
    block->y = 0;
    block->escape = -1;
    block->next_all = NULL;
    return block;
}

block_p all_blocks = NULL;

void add_block_at_start_line(block_p block)
{
    block_p *ref_block = &all_blocks;
    while (*ref_block != NULL && (*ref_block)->start_line <= block->start_line)
        ref_block = &(*ref_block)->next_all;
    block->next_all = *ref_block;
    *ref_block = block;
}

void add_block(block_p block)
{
    if (block->nr_statements == 0)
    {
        printf("Error: block has no statements\n");
        return;
    }
    if (block->start_line != 0)
    {
        printf("Warning: block already added on line %d\n", block->start_line);
        return;
    }
    block->start_line = block->statements[0]->line;
    add_block_at_start_line(block);
}

void block_add_transition(block_p from, block_p to)
{
    if (to != NULL)
    {
        transition_p transition = (transition_p)malloc(sizeof(struct transition_s));
        transition->from = from;
        transition->next_in_trans = to->in_trans;
        to->in_trans = transition;
    }
}

void block_set_next(block_p block, block_p next)
{
    block->next = next;
    block_add_transition(block, next);
}

void block_set_alt(block_p block, block_p alt)
{
    block->alt = alt;
    block_add_transition(block, alt);
}

expr_p new_diop_expr(int kind, expr_p left, expr_p right)
{
    expr_p expr = new_expr(kind, 2);
    expr->children[0] = left;
    expr->children[1] = right;
    return expr;
}

bool empty_statements(statement_p *statements, int nr_statements);
bool empty_statement(statement_p statement)
{
    if (statement == NULL)
        return TRUE;
    if (statement->kind == 'D' && (statement->decl->storage_type == ST_STATIC || statement->decl->value == NULL))
        return TRUE;
    if (statement->kind == '{')
        return empty_statements(statement->children, statement->nr_children);
    return FALSE;
}
bool empty_statements(statement_p *statements, int nr_statements)
{
    for (int i = 0; i < nr_statements; i++)
        if (!empty_statement(statements[i]))
            return FALSE;
    return TRUE;
}

//int indent = 0;

void construct_blocks_from_statements(statement_p *statement, int nr_statements, block_p block, block_p next, block_p next_break);

bool case_or_default_statement(statement_p statement) { return statement->kind == TK_CASE || statement->kind == TK_DEFAULT; }

void construct_blocks_from_statement(statement_p *statements, block_p block, block_p next, block_p next_break)
{
    //printf("%*.*sconstruct_blocks_from_statement %d %d\n", indent, indent, "", block->nr, next == NULL ? -1 : next->nr);
    statement_p statement = statements[0];
    if (empty_statement(statement))
        return;
    
    if (statement->kind == '{')
    {
        if (block->comment == NULL)
            block->comment = statement->children[0]->comment;
        construct_blocks_from_statements(statement->children, statement->nr_children, block, next, next_break);
        return;
    }
    
    if (block->nr_statements == 0)
    {
        block->statements = statements;
        block->nr_statements = 1;
    }

    if (statement->kind == TK_IF)
    {
        add_block(block);
        block->comment = statement->comment;
        block->cond = statement->expr;
        if (empty_statement(statement->children[0]))
            block_set_alt(block, next);
        else
        {
            block_set_alt(block, new_block());
            construct_blocks_from_statement(&statement->children[0], block->alt, next, next_break);
        }
        if (statement->nr_children == 1 || empty_statement(statement->children[1]))
            block_set_next(block, next);
        else
        {
            block_set_next(block, new_block());
            construct_blocks_from_statement(&statement->children[1], block->next, next, next_break);
        }
    }
    else if (statement->kind == TK_WHILE)
    {
        add_block(block);
        block->comment = statement->comment;
        block->cond = statement->expr;
        if (empty_statement(statement->children[0]))
            printf("Warning: Empty while\n");
        else
        {
            block_set_alt(block, new_block());
            construct_blocks_from_statement(&statement->children[0], block->alt, block, next);
        }
        block_set_next(block, next);
    }
    else if (statement->kind == TK_SWITCH)
    {
        block->comment = NULL; //statement->comment;
        
        for (int i = 0; i < statement->nr_children; i++)
        {
            block->statements = &statement->children[i];
            block->nr_statements = 0;
            for (; i < statement->nr_children && case_or_default_statement(statement->children[i]); i++)
            {
                if (block->comment == NULL)
                    block->comment = statement->children[i]->comment;
                block->nr_statements++;
                expr_p compare = NULL;
                if (statement->children[i]->kind == TK_CASE)
                    compare = new_diop_expr(TK_EQ, statement->expr, statement->children[i]->expr);
                else
                {
                    compare = new_expr('0', 1);
                    compare->int_val = 1;
                }
                block->cond = block->cond == NULL ? compare : new_diop_expr(TK_OR, block->cond, compare);
            }
            add_block(block);
            int nr_case_stat = 0;
            while (i + nr_case_stat < statement->nr_children && !case_or_default_statement(statement->children[i + nr_case_stat]))
                nr_case_stat++;
            block_p next_block = NULL;
            if (i + nr_case_stat < statement->nr_children)
                next_block = new_block();
            block_set_next(block, next_block != NULL ? next_block : next);
            
            block_p child_block = new_block();
            block_set_alt(block, child_block);

            construct_blocks_from_statements(&statement->children[i], nr_case_stat, child_block, next_block, next);
            i += nr_case_stat - 1;

            block = next_block;
        }
    }
    else if (statement->kind == TK_RETURN)
    {
        add_block(block);
    }
    else if (statement->kind == TK_BREAK)
    {
        add_block(block);
        if (next_break == NULL)
            printf("Error: break outside loop or switch\n");
        block_set_next(block, next_break);
    }
    else if (statement->kind == TK_FOR)
    {
        block->comment = statement->comment;
        if (statement->children[0] != NULL)
        {
            block_p for_init_block = block;
            if (for_init_block->comment == NULL)
                for_init_block->comment = "(For init)";
            for_init_block->statements = &statement->children[0];
            for_init_block->nr_statements = 1;
            add_block(for_init_block);
            block = new_block();
            block_set_next(for_init_block, block);
        }
        block->cond = statement->expr;
        if (block->comment == NULL)
            block->comment = "(For condition)";
        block->statements = statements;
        block->nr_statements = 1;
        block_set_next(block, next);
        add_block(block);
        block_p for_next_block = block;
        if (statement->children[1] != NULL)
        {
            for_next_block = new_block();
            for_next_block->comment = "(For next)";
            for_next_block->statements = &statement->children[1];
            for_next_block->nr_statements = 1;
            block_set_next(for_next_block, block);
        }
        if (empty_statement(statement->children[2]))
            printf("Warning: Empty for\n");
        else
        {
            block_set_alt(block, new_block());
            construct_blocks_from_statement(&statement->children[2], block->alt, for_next_block, next);
        }
        if (for_next_block != block)
        {
            for (block_p body_block = block->alt; body_block != for_next_block; body_block = body_block->next)
                for_next_block->start_line = body_block->start_line;
            add_block_at_start_line(for_next_block);
        }
    }
    else
        printf("Statement type %d is not supported\n", statement->kind);
}

bool complext_statement_kind(int kind)
{
    return kind == TK_IF || kind == TK_WHILE || kind == TK_FOR || kind == TK_DO || kind == TK_SWITCH || kind == TK_RETURN;
}

void construct_blocks_from_statements(statement_p *statements, int nr_statements, block_p block, block_p next, block_p next_break)
{
    //printf("%*.*sstart construct_blocks_from_statements %d block %d\n", indent, indent, "", nr_statements, block->nr);
    //indent += 3;
    for (int i = 0; i < nr_statements; i++)
    {
        statement_p child = statements[i];
        //printf("%*.*s- Process statement %s:%d.%d %d %s\n", indent, indent, "", child->filename, child->line, child->column, child->kind, child->comment == NULL ? "" : child->comment);
        block->statements = &statements[i];
        block->nr_statements = 1;
        block->comment = child->comment;
        if (!complext_statement_kind(child->kind))
        {
            while (   i + 1 < nr_statements 
                   && !complext_statement_kind(statements[i + 1]->kind)
                   && statements[i + 1]->comment == NULL)
            {
                block->nr_statements++;
                i++;
            }
        }
        block_p next_block = NULL;
        for (int j = i + 1; j < nr_statements; j++)
        {
            if (!empty_statement(statements[j]))
            {
                next_block = new_block();
                break;
            }
        }
        block_p next_for_block = next_block != NULL ? next_block : next;
        if (complext_statement_kind(child->kind))
            construct_blocks_from_statement(&statements[i], block, next_for_block, next_break);
        else
        {
            add_block(block);
            bool has_break = FALSE;
            for (int j = 0; j < block->nr_statements; j++)
                if (block->statements[j]->kind == TK_BREAK)
                {
                    has_break = TRUE;
                    block_set_next(block, next_break);
                    break;
                }
            if (!has_break)
                block_set_next(block, next_for_block);
        }
        block = next_block;
    }
    //indent -= 3;
    //printf("%*.*send construct_blocks_from_statements\n", indent, indent, "");
}

void construct_blocks(void)
{
    for (int i = 0; i < cur_nr_statements; i++)
    {
        statement_p statement = cur_statements[i];
        if (statement->kind == 'D' && statement->decl->type->kind == TYPE_KIND_FUNCTION)
        {
            block_p function_block = new_block();
            function_block->nr_statements = 1;
            function_block->statements = &cur_statements[i];
            add_block(function_block);
            function_block->comment = statement->decl->name;
            block_p block = new_block();
            block_set_next(function_block, block);
            construct_blocks_from_statements(statement->children, statement->nr_children, block, NULL, NULL);
        }
    }
}

void print_blocks(void)
{
    printf("\n\n");
    for (block_p block = all_blocks; block != NULL; block = block->next_all)
    {
        printf("block %d", block->start_line);
        if (block->comment != NULL)
            printf(" '%s'", block->comment);
        if (block->in_trans != NULL)
        {
            const char *s = " from";
            for (transition_p trans = block->in_trans; trans != NULL; trans = trans->next_in_trans)
            {
                printf("%s %d", s, trans->from->start_line);
                //s = ",";
            }
        }
        if (block->alt != 0)
            printf(" alt %d", block->alt->start_line);
        if (block->next != 0)
            printf(" next %d", block->next->start_line);
        printf(" : %d", block->nr_statements);
        for (int j = 0; j < block->nr_statements; j++)
            printf(" stat %d", block->statements[j]->line);
            //printf("   %s:%d\n", block->statements[j]->filename, block->statements[j]->line);
        printf("\n");
    }
}

