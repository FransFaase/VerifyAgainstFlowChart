
void compare(node_p node, block_p block)
{
    if (block->node != NULL)
    {
        if (block->node != node)
            printf("Error: node '%s' to wrong block at %d '%s'\n", 
                node->name == NULL ? "NULL" : node->name,
                block->start_line, block->comment == NULL ? "NULL" : block->comment);
        return;
    }
    block->node = node;
    char *node_name = node->name == NULL ? "" : node->name;
    char *comment = block->comment == NULL ? "" : block->comment;
    if (!eqstr(node_name, comment))
        printf("Error: Name of node '%s' does not match with block at %d '%s'\n", node_name, block->start_line, comment);
    int nr_out_edges = 0;
    edge_p yes_edge = NULL;
    edge_p no_edge = NULL;
    for (edge_p edge = node->out_edges; edge != NULL; edge = edge->next_out_edge)
    {
        nr_out_edges++;
        if (edge->name == NULL)
            ;
        else if (eqstr(edge->name, "yes") || eqstr(edge->name, "Yes"))
            yes_edge = edge;
        else if (eqstr(edge->name, "no") || eqstr(edge->name, "No"))
            yes_edge = edge;
        else
            printf("Error: illegal edge name '%s' going out of node '%s'\n", edge->name, node_name);
    }
    if (node->node_type == NODE_TYPE_DECISION)
    {
        if (nr_out_edges != 2)
            printf("Error: Decision node '%s' should have two edges\n", node_name);
        if (yes_edge == NULL)
            printf("Error: Decision node '%s' should have 'yes' edge\n", node_name);
        else
        {
            if (block->alt == NULL)
                printf("Error: 'yes' edge from node '%s' has no alternative\n", node_name);
            else
                compare(yes_edge->to, block->alt);
        }
        if (no_edge == NULL)
            printf("Error: Decision node '%s' should have 'no' edge\n", node_name);
        else
        {
            if (block->next == NULL)
                printf("Error: 'no' edge from node '%s' has no alternative\n", node_name);
            else
                compare(no_edge->to, block->next);
        }
    }
    else if (node->node_type == NODE_TYPE_TERMINATOR)
    {
        if (nr_out_edges != 0)
            printf("Error: Exit node '%s' should not have edges\n", node_name);
        if (block->next != NULL)
            printf("Error: block '%s' has next block\n", comment);
    }
    else
    {
        if (nr_out_edges != 1)
            printf("Error: Node '%s' should have one edge\n", node_name);
        if (yes_edge != NULL)
            printf("Error: Node '%s' should not have 'yes' edge\n", node_name);
        if (no_edge != NULL)
            printf("Error: Node '%s' should not have 'no' edge\n", node_name);
        if (nr_out_edges == 1)
        {
            if (block->next == NULL)
                printf("Error: block '%s' has no next block\n", node_name);
            else
                compare(node->out_edges->to, block->next);
        }
    }
}

void compare_all(void)
{
    for (block_p block = all_blocks; block != NULL; block = block->next_all)
    {
        if (block->in_trans == NULL)
        {
            node_p start_node = NULL;
            for (node_p node = all_nodes; node != NULL; node = node->next)
                if (node->node_type == NODE_TYPE_START && eqstr(block->comment, node->name))
                {
                    start_node = node;
                    break;
                }
            if (start_node == NULL)
                printf("Error: Did not find start node for '%s'\n", block->comment);
            else
                compare(start_node, block);
            
        }
    }
}