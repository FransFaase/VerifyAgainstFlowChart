
#include "xmliter.c"

typedef enum node_type_e node_type_t;
typedef struct node_s *node_p;
typedef struct edge_s *edge_p;

enum node_type_e {
    NODE_TYPE_NONE,
    NODE_TYPE_START,
    NODE_TYPE_TERMINATOR,
    NODE_TYPE_PROCESS,
    NODE_TYPE_DECISION,
    NODE_TYPE_PREDEFINED_PROCESS,
};

struct node_s
{
    int id;
    char *name;
    node_type_t node_type;
    edge_p in_edges;
    edge_p out_edges;
    node_p next;
};

struct edge_s
{
    char *name;
    node_p from;
    node_p to;
    edge_p next_in_edge;
    edge_p next_out_edge;
};

node_p all_nodes = 0;

node_p add_node(int id, char *name, node_type_t node_type)
{
    node_p *ref_node = &all_nodes;
    for (; *ref_node != NULL; ref_node = &(*ref_node)->next)
        if ((*ref_node)->id == id)
        {
            if (name != NULL && (*ref_node)->name == NULL)
                (*ref_node)->name = name;
            if (node_type != NODE_TYPE_NONE && (*ref_node)->node_type == NODE_TYPE_NONE)
                (*ref_node)->node_type = node_type;
            return *ref_node;
        }
    node_p node = (node_p)malloc(sizeof(struct node_s));
    node->id = id;
    node->name = name;
    node->node_type = node_type;
    node->in_edges = NULL;
    node->out_edges = NULL;
    node->next = NULL;
    (*ref_node) = node;
    ref_node = &node->next;
    return node;
}

void add_edge(int from, int to, char *name)
{
    node_p from_node = add_node(from, NULL, NODE_TYPE_NONE);
    node_p to_node = add_node(to, NULL, NODE_TYPE_NONE);
    edge_p edge = (edge_p)malloc(sizeof(struct edge_s));
    edge->name = name;
    edge->from = from_node;
    edge->to = to_node;
    edge->next_in_edge = to_node->in_edges;
    to_node->in_edges = edge;
    edge->next_out_edge = from_node->out_edges;
    from_node->out_edges = edge;
}

int parse_node_id(const char *s)
{
    if (*s == 'n')
    {
        s++;
        int id = 0;
        while ('0' <= *s && *s <= '9')
            id = 10 * id + *s++ - '0';
        if (*s == '\0' && id >= 0)
            return id;
    }
    return -1;
}

void parse_flowchart(const char *filename)
{
    FILE *f = fopen(filename, "r");
    xmliter_t xmliter;
    xmliter_init(&xmliter, f);
    fclose(f);

    while (xmliter.state != '\0')
    {
        //printf("%*.*s", xmliter.level, xmliter.level, "");
        if (xmliter_accept_tag(&xmliter, "node"))
        {
            int id = -1;
            char *name = NULL;
            node_type_t node_type = NODE_TYPE_NONE;
            int cur_level = xmliter.level;
            for (; xmliter.state == 'a'; xmliter_next(&xmliter))
                if (eqstr(xmliter.tag, "id"))
                    id = parse_node_id(xmliter.value);

            while (xmliter.level >= cur_level)
                if (xmliter_accept_tag(&xmliter, "y:GenericNode"))
                {
                    for (; xmliter.state == 'a'; xmliter_next(&xmliter))
                        if (eqstr(xmliter.tag, "configuration"))
                        {
                            if (     eqstr(xmliter.value, "com.yworks.flowchart.start1")
                                  || eqstr(xmliter.value, "com.yworks.flowchart.start2"))
                                node_type = NODE_TYPE_START;
                            else if (eqstr(xmliter.value, "com.yworks.flowchart.terminator"))
                                node_type = NODE_TYPE_TERMINATOR;
                            else if (eqstr(xmliter.value, "com.yworks.flowchart.process"))
                                node_type = NODE_TYPE_PROCESS;
                            else if (eqstr(xmliter.value, "com.yworks.flowchart.decision"))
                                node_type = NODE_TYPE_DECISION;
                            else if (eqstr(xmliter.value, "com.yworks.flowchart.predefinedProcess"))
                                node_type = NODE_TYPE_PREDEFINED_PROCESS;
                            else
                                printf("Warning: Unknown %s\n", xmliter.value);
                        }
                }
                else if (xmliter_accept_tag(&xmliter, "y:NodeLabel"))
                {
                    for (; xmliter.state == 'a'; xmliter_next(&xmliter))
                        {}
                    if (xmliter.state == 't')
                        name = copystr(xmliter.value);
                }
                else
                    xmliter_next(&xmliter);
            if (id >= 0 && node_type != NODE_TYPE_NONE)
            {
                //printf("ADD_NODE %d %d '%s'\n", id, node_type, name);
                add_node(id, name, node_type);
            }
        }
        else if (xmliter_accept_tag(&xmliter, "edge"))
        {
            int source = -1;
            int target = -1;
            char *name = NULL;
            int cur_level = xmliter.level;
            for (; xmliter.state == 'a'; xmliter_next(&xmliter))
                if (eqstr(xmliter.tag, "source"))
                    source = parse_node_id(xmliter.value);
                else if (eqstr(xmliter.tag, "target"))
                    target = parse_node_id(xmliter.value);
            while (xmliter.level >= cur_level)
                if (xmliter_accept_tag(&xmliter, "y:EdgeLabel"))
                {
                    for (; xmliter.state == 'a'; xmliter_next(&xmliter))
                    {}
                    if (xmliter.state == 't')
                        name = copystr(xmliter.value);
                }
                else
                    xmliter_next(&xmliter);
            if (source != -1 && target != -1)
            {
                //printf("ADD_EDGE %d %d '%s'\n", source, target, name);
                add_edge(source, target, name);
            }
        }
        else if (xmliter.state == '!')
            ; //printf("Open %s\n", xmliter.value);
        else if (xmliter.state == '?')
            ; //printf("Open %s\n", xmliter.value);
        else if (xmliter.state == 'o')
            ; //printf("Open %s\n", xmliter.tag);
        else if (xmliter.state == 'c')
            ; //printf("Close\n");
        else if (xmliter.state == 'a')
            ; //printf("Attr %s='%s'\n", xmliter.tag, xmliter.value);
        else if (xmliter.state == 't')
            ; //printf("Text '%s'\n", xmliter.value);
        else
            ; //printf("#%c \n", xmliter.state);
        xmliter_next(&xmliter);
    }
}

void print_flowchart(void)
{
    for (node_p node = all_nodes; node != NULL; node = node->next)
    {
        printf("Node %d %d:", node->id, node->node_type);
        if (node->name != NULL)
            printf(" '%s'", node->name);
        printf("\n");
        for (edge_p edge = node->in_edges; edge != NULL; edge = edge->next_in_edge)
        {
            printf("  - Edge from %d", edge->from->id);
            if (edge->name != NULL)
                printf(" '%s'", edge->name);
            printf("\n");
        }
        for (edge_p edge = node->out_edges; edge != NULL; edge = edge->next_out_edge)
        {
            printf("  - Edge to %d", edge->to->id);
            if (edge->name != NULL)
                printf(" '%s'", edge->name);
            printf("\n");
        }
    }
}
