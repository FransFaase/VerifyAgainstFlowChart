
int height = 40;
int width = 148;
int lane_width = 200;
int in_lane_height = 100;
int escape_width = 50;
int jump_dist = 24;

typedef enum edge_type_e edge_type_t;
enum edge_type_e 
{
    EDGE_TYPE_NONE,
    EDGE_TYPE_YES,
    EDGE_TYPE_NO,
};

void output_edge(FILE *f, block_p from, block_p to, edge_type_t edge_type)
{
    static int edge_nr = 0;
    fprintf(f, 
        "<edge id=\"e%d\" source=\"n%d\" target=\"n%d\">\n"
        "<data key=\"d10\">\n"
        "<y:PolyLineEdge>\n"
        "<y:Path sx=\"0.0\" sy=\"0.0\" tx=\"0.0\" ty=\"0.0\">\n", edge_nr++, from->output_nr, to->output_nr);
    if (edge_type == EDGE_TYPE_YES)
        fprintf(f, "<y:Point x=\"%d\" y=\"%d\"/>\n", to->x + width / 2, from->y + height / 2);
    else if (from->x > to->x && to->output_nr > from->output_nr)
    {
        bool lane_is_free = TRUE;
        for (block_p next_all = from->next_all; next_all->output_nr < to->output_nr; next_all = next_all->next_all)
            if (next_all->x == from->x)
            {
                lane_is_free = FALSE;
                break;
            }
        if (lane_is_free)
        {
            fprintf(f, "<y:Point x=\"%d\" y=\"%d\"/>\n", from->x + width / 2, to->y - jump_dist);
            fprintf(f, "<y:Point x=\"%d\" y=\"%d\"/>\n", to->x + width / 2, to->y - jump_dist);
        }
        else
        {
            fprintf(f, "<y:Point x=\"%d\" y=\"%d\"/>\n", from->x + width / 2, from->y + height + jump_dist);
            fprintf(f, "<y:Point x=\"%d\" y=\"%d\"/>\n", to->escape, from->y + height + jump_dist);
            fprintf(f, "<y:Point x=\"%d\" y=\"%d\"/>\n", to->escape, to->y - jump_dist);
            fprintf(f, "<y:Point x=\"%d\" y=\"%d\"/>\n", to->x + width / 2, to->y - jump_dist);
        }
    }
    fprintf(f,"</y:Path>\n"
        "<y:LineStyle color=\"#000000\" type=\"line\" width=\"1.0\"/>\n"
        "<y:Arrows source=\"none\" target=\"standard\"/>\n");
    if (edge_type != EDGE_TYPE_NONE)
    {
        fprintf(f, "<y:EdgeLabel alignment=\"center\" configuration=\"AutoFlippingLabel\" distance=\"2.0\" fontFamily=\"Dialog\" fontSize=\"12\" fontStyle=\"plain\" hasBackgroundColor=\"false\" hasLineColor=\"false\" height=\"17.96875\" horizontalTextPosition=\"center\" iconTextGap=\"4\" modelName=\"custom\" preferredPlacement=\"anywhere\" ratio=\"0.5\" textColor=\"#000000\" verticalTextPosition=\"bottom\" visible=\"true\" width=\"24.96484375\" x=\"4.032512273730276\" xml:space=\"preserve\" ");
        if (edge_type == EDGE_TYPE_YES)
            fprintf(f, "y=\"-19.102792401058565\">Yes<y:LabelModel><y:SmartEdgeLabelModel autoRotationEnabled=\"false\" defaultAngle=\"0.0\" defaultDistance=\"10.0\"/></y:LabelModel><y:ModelParameter><y:SmartEdgeLabelModelParameter angle=\"6.283185307179586\" distance=\"1.1340424010585655\" distanceToCenter=\"false\" position=\"left\" ratio=\"-0.9811596012697237\"");
        else
            fprintf(f, "y=\"5.921142578125\">No<y:LabelModel><y:SmartEdgeLabelModel autoRotationEnabled=\"false\" defaultAngle=\"0.0\" defaultDistance=\"10.0\"/></y:LabelModel><y:ModelParameter><y:SmartEdgeLabelModelParameter angle=\"0.0\" distance=\"30.0\" distanceToCenter=\"true\" position=\"right\" ratio=\"0.0\"");
        fprintf(f, " segment=\"0\"/></y:ModelParameter><y:PreferredPlacementDescriptor angle=\"0.0\" angleOffsetOnRightSide=\"0\" angleReference=\"absolute\" angleRotationOnRightSide=\"co\" distance=\"-1.0\" frozen=\"true\" placement=\"anywhere\" side=\"anywhere\" sideReference=\"relative_to_edge_flow\"/></y:EdgeLabel>\n");
    }
    fprintf(f,
        "<y:BendStyle smoothed=\"false\"/>\n"
        "</y:PolyLineEdge>\n"
        "</data>\n"
        "</edge>\n");
}

int max_x_offset = 0;

void set_x_offset(block_p block, int offset)
{
    if (block == NULL || block->x != -1)
        return;
    
    if (offset > max_x_offset)
        max_x_offset = offset;
    block->x = offset;
    set_x_offset(block->next, offset);
    set_x_offset(block->alt, offset + lane_width);
}

void output_text(FILE *f, const char *s)
{
    if (s == NULL)
        return;
    for (; *s != '\0'; s++)
        if (*s == '<')
            fprintf(f, "&lt;");
        else if (*s == '>')
            fprintf(f, "&gt;");
        else if (*s == '&')
            fprintf(f, "&amp;");
        else if (*s == ' ' || *s == '\t')
        {
            while (s[1] == ' ' || s[1] == ' ')
                s++;
            if (s[1] != '\0')
                fprintf(f, " ");
        }
        else
            fprintf(f, "%c", *s);
}

void output_flowchart(const char *filename)
{
    FILE *f = fopen(filename, "w");
    if (f == NULL)
        return;
    fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n"
            "<graphml xmlns=\"http://graphml.graphdrawing.org/xmlns\" xmlns:java=\"http://www.yworks.com/xml/yfiles-common/1.0/java\" xmlns:sys=\"http://www.yworks.com/xml/yfiles-common/markup/primitives/2.0\" xmlns:x=\"http://www.yworks.com/xml/yfiles-common/markup/2.0\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:y=\"http://www.yworks.com/xml/graphml\" xmlns:yed=\"http://www.yworks.com/xml/yed/3\" xsi:schemaLocation=\"http://graphml.graphdrawing.org/xmlns http://www.yworks.com/xml/schema/graphml/1.1/ygraphml.xsd\">\n"
            "<!--Created by yEd 3.24-->\n"
            "<key attr.name=\"Description\" attr.type=\"string\" for=\"graph\" id=\"d0\"/>\n"
            "<key for=\"port\" id=\"d1\" yfiles.type=\"portgraphics\"/>\n"
            "<key for=\"port\" id=\"d2\" yfiles.type=\"portgeometry\"/>\n"
            "<key for=\"port\" id=\"d3\" yfiles.type=\"portuserdata\"/>\n"
            "<key attr.name=\"url\" attr.type=\"string\" for=\"node\" id=\"d4\"/>\n"
            "<key attr.name=\"description\" attr.type=\"string\" for=\"node\" id=\"d5\"/>\n"
            "<key for=\"node\" id=\"d6\" yfiles.type=\"nodegraphics\"/>\n"
            "<key for=\"graphml\" id=\"d7\" yfiles.type=\"resources\"/>\n"
            "<key attr.name=\"url\" attr.type=\"string\" for=\"edge\" id=\"d8\"/>\n"
            "<key attr.name=\"description\" attr.type=\"string\" for=\"edge\" id=\"d9\"/>\n"
            "<key for=\"edge\" id=\"d10\" yfiles.type=\"edgegraphics\"/>\n"
            "<graph edgedefault=\"directed\" id=\"G\">\n"
                "<data key=\"d0\" xml:space=\"preserve\"/>\n");
    int output_nr = 0;
    for (block_p block = all_blocks; block != NULL; block = block->next_all)
        block->output_nr = output_nr++;

    int y = 0;
    for (block_p block = all_blocks; block != NULL; block = block->next_all)
    {
        if (block->in_trans == NULL)
        {
            y = 0;
            set_x_offset(block, max_x_offset);
            max_x_offset += lane_width;
            block_p escape_lanes[20];
            int nr_escape_lanes = 0;
            int max_nr_escape_lanes = 0;
            // Calculate escapes
            for (block_p from = block->next; from != NULL && from->in_trans != NULL; from = from->next_all)
            {
                for (int i = 0; i < nr_escape_lanes; i++)
                    if (escape_lanes[i] == from)
                    {
                        escape_lanes[i] = NULL;
                        while (nr_escape_lanes > 0 && escape_lanes[nr_escape_lanes - 1] == NULL)
                            nr_escape_lanes--;
                        break;
                    }
                block_p to = from->next;
                if (to == NULL)
                    continue;
                bool search = to->output_nr > from->output_nr + 1;
                for (int i = 0; search && i < nr_escape_lanes; i++)
                    if (escape_lanes[i] == to)
                        search = FALSE;
                if (!search)
                    ; //printf("Do not search %d to %d\n", from->output_nr, to->output_nr);
                else
                {
                    bool lane_is_free = TRUE;
                    for (block_p next_all = from->next_all; next_all->output_nr < to->output_nr; next_all = next_all->next_all)
                        if (next_all->x == from->x)
                        {
                            lane_is_free = FALSE;
                            break;
                        }
                    if (lane_is_free)
                        ; //printf("lane from %d to %d is free\n", from->output_nr, to->output_nr);
                    else
                    {
                        //printf("lane from %d to %d is not free\n", from->output_nr, to->output_nr);
                        bool inside = FALSE;
                        int last_empty = -1;
                        for (int i = 0; i < nr_escape_lanes; i++)
                            if (escape_lanes[i])
                            {
                                if (last_empty == -1)
                                    last_empty = i;
                            }
                            else
                            {
                                last_empty = -1;
                                if (to->output_nr < escape_lanes[i]->output_nr)
                                {
                                    inside = TRUE;
                                    if (last_empty != -1)
                                    {
                                        escape_lanes[i] = to;
                                        escape_lanes[i]->escape = max_x_offset + i * escape_width;
                                    }
                                    else
                                    {
                                        block_p to_insert = to;
                                        while (to_insert != NULL)
                                        {
                                            block_p next_to_insert = i < nr_escape_lanes ? escape_lanes[i] : NULL;
                                            to_insert->escape = max_x_offset + i * escape_width;
                                            escape_lanes[i] = to_insert;
                                            to_insert = next_to_insert;
                                            i++;
                                        }
                                        if (i >= nr_escape_lanes)
                                            nr_escape_lanes = i + 1;
                                    }
                                    break;
                                }
                            }
                        if (!inside)
                        {
                            to->escape = max_x_offset + nr_escape_lanes * escape_width;
                            escape_lanes[nr_escape_lanes++] = to;
                        }
                        if (nr_escape_lanes > max_nr_escape_lanes)
                            max_nr_escape_lanes = nr_escape_lanes;
                    }
                }
            }
            max_x_offset += max_nr_escape_lanes * escape_width + lane_width;
        }
        block->y = y;
        y += in_lane_height;
    }

    for (block_p block = all_blocks; block != NULL; block = block->next_all)
    {
        fprintf(f, "<node id=\"n%d\">\n"
            "<data key=\"d6\">\n"
            "<y:GenericNode configuration=\"com.yworks.flowchart.%s\">\n"
            "<y:Geometry height=\"%d\" width=\"%d\" x=\"%d\" y=\"%d\"/>\n"
            "<y:Fill color=\"#E8EEF7\" color2=\"#B7C9E3\" transparent=\"false\"/>\n"
            "<y:BorderStyle color=\"#000000\" type=\"line\" width=\"1.0\"/>\n"
            "<y:NodeLabel alignment=\"center\" autoSizePolicy=\"content\" fontFamily=\"Dialog\" fontSize=\"12\" fontStyle=\"plain\" hasBackgroundColor=\"false\" hasLineColor=\"false\" height=\"17.96875\" horizontalTextPosition=\"center\" iconTextGap=\"4\" modelName=\"custom\" textColor=\"#000000\" verticalTextPosition=\"bottom\" visible=\"true\" width=\"104.453125\" x=\"21.2734375\" xml:space=\"preserve\" y=\"11.015625\">",
                block->output_nr, 
                block->in_trans == NULL ? "start1" :
                block->next == NULL ? "terminator" :
                block->alt != NULL ? "decision" : "process",
                height, width, block->x, block->y);
        output_text(f, block->comment);
        fprintf(f,
            "<y:LabelModel><y:SmartNodeLabelModel distance=\"4.0\"/></y:LabelModel><y:ModelParameter><y:SmartNodeLabelModelParameter labelRatioX=\"0.0\" labelRatioY=\"0.0\" nodeRatioX=\"0.0\" nodeRatioY=\"0.0\" offsetX=\"0.0\" offsetY=\"0.0\" upX=\"0.0\" upY=\"-1.0\"/></y:ModelParameter></y:NodeLabel>\n"
            "</y:GenericNode>\n"
            "</data>\n"
            "</node>\n");
    }
    for (block_p block = all_blocks; block != NULL; block = block->next_all)
    {
        if (block->next != NULL)
            output_edge(f, block, block->next, block->alt != NULL ? EDGE_TYPE_NO : EDGE_TYPE_NONE);
        if (block->alt != NULL)
            output_edge(f, block, block->alt, EDGE_TYPE_YES);
    }
    fprintf(f, "</graph>\n"
            "<data key=\"d7\">\n"
            "<y:Resources/>\n"
            "</data>\n"
            "</graphml>\n");
    fclose(f);
}