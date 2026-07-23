#define MAX_TAG_LEN 100
#define MAX_VALUE_LEN 1000

typedef struct xmliter_s xmliter_t, *xmliter_p;
struct xmliter_s
{
    unsigned char *input;
    char state;
    char tag[MAX_TAG_LEN+1];
    char value[MAX_VALUE_LEN+1];
    int level;
    //int line;
    //int column;
};

void xmliter_next(xmliter_p xmliter);

bool xmliter_accept_tag(xmliter_p xmliter, const char *name)
{
    if (xmliter->state == 'o' && strcmp(xmliter->tag, name) == 0)
    {
        xmliter_next(xmliter);
        return TRUE;
    }
    return FALSE;
}

void xmliter_skip_tag(xmliter_p xmliter)
{
    if (xmliter->state != 'o')
        return;
    xmliter_next(xmliter);    
    int cur_level = xmliter->level;
    while (xmliter->level >= cur_level && xmliter->state != '\0')
    {
        xmliter_next(xmliter);
    }
}

char nextchar(FILE *f)
{
    int ch = fgetc(f);
    return ch < 0 ? '\0' : (char)ch;
}

void xmliter_init(xmliter_p xmliter, FILE *f)
{
    int fh = fileno(f);
    unsigned long len = lseek(fh, 0L, SEEK_END);
    lseek(fh, 0L, SEEK_SET);
    xmliter->input = (unsigned char*)malloc(len + 1);
    len = read(fh, xmliter->input, len);
    xmliter->input[len] = '\0';

    xmliter->state = ' ';
    xmliter->level = 0;
    xmliter_next(xmliter);
}

void xmliter_parse_string(xmliter_p xmliter, char terminator)
{
	int i = 0;
	while (*xmliter->input != '\0' && *xmliter->input != terminator)
	{
		if (*xmliter->input == '&')
		{
			if (strncmp((const char*)xmliter->input, "&lt;", 4) == 0)
			{
                if (i < MAX_VALUE_LEN) xmliter->value[i++] = '<';
				xmliter->input += 4;
			}
			else if (strncmp((const char*)xmliter->input, "&gt;", 4) == 0)
			{
				if (i < MAX_VALUE_LEN) xmliter->value[i++] =  '>';
				xmliter->input += 4;
			}
			else if (strncmp((const char*)xmliter->input, "&amp;", 5) == 0)
			{
				if (i < MAX_VALUE_LEN) xmliter->value[i++] = '&';
				xmliter->input += 5;
			}
			else
			{
				if (i < MAX_VALUE_LEN) xmliter->value[i++] =  '?';
				xmliter->input++;
				while (*xmliter->input != '\0' && *xmliter->input != ';')
					xmliter->input++;
				if (*xmliter->input != '\0' && *xmliter->input == ';')
					xmliter->input++;
			}
		}
		else
		{
			if (i < MAX_VALUE_LEN) xmliter->value[i++] =  *xmliter->input++;
		}
	}
	xmliter->value[i] = '\0';
}

void xmliter_next(xmliter_p xmliter)
{
	if (xmliter->state == '\0' || xmliter->state == 'e')
		return;
	
    while (*xmliter->input != '\0' && *xmliter->input <= ' ')
    {
        xmliter->input++;
    }

	if (*xmliter->input == '\0')
	{
		xmliter->state = '\0';
		return;
	}
	
	if (xmliter->state == 'o' || xmliter->state == 'a')
	{
		if (*xmliter->input == '/')
		{
			xmliter->input++;
			while (*xmliter->input != '\0' && *xmliter->input <= ' ')
                xmliter->input++;
			if (*xmliter->input != '>')
			{
				xmliter->state = 'e';
				return;
			}
			xmliter->input++;
            xmliter->level--;
			xmliter->state = 'c';
			return;
		}
		else if (*xmliter->input == '>')
		{
			xmliter->input++;
			while (*xmliter->input != '\0' && *xmliter->input <= ' ')
                xmliter->input++;
			if (*xmliter->input == '\0')
			{
				xmliter->state = '\0';
				return;
			}
		}
		else
		{
			//line = _text.line();
			//column = _text.column();

			int i = 0;
			while (*xmliter->input > ' ' && *xmliter->input != '>' && *xmliter->input != '/' && *xmliter->input != '=')
			{
				if (i < MAX_TAG_LEN)
					xmliter->tag[i++] = *xmliter->input;
				xmliter->input++;
			}
			xmliter->tag[i] = '\0';
			if (*xmliter->input == '\0' || *xmliter->input != '=')
			{
				xmliter->state = 'e';
				return;
			}
			xmliter->input++;
			if (*xmliter->input == '\0' || *xmliter->input != '"')
			{
				xmliter->state = 'e';
				return;
			}
			xmliter->input++;
			xmliter_parse_string(xmliter, '"');
			if (*xmliter->input == '\0' || *xmliter->input != '"')
			{
				xmliter->state = 'e';
				return;
			}
			xmliter->input++;
			xmliter->state = 'a';
			return;
		}
	}
	
	//line = _text.line();
	//column = _text.column();

	if (*xmliter->input == '<')
	{
		if (xmliter->input[1] == '/')
		{
			xmliter->input += 2;
			int i = 0;
			while (*xmliter->input > ' ' && *xmliter->input != '>')
			{
				if (i < MAX_TAG_LEN)
					xmliter->tag[i++] = *xmliter->input;
				xmliter->input++;
			}
			xmliter->tag[i] = '\0';
			if (*xmliter->input != '>')
			{
				xmliter->state = 'e';
				return;
			}
			xmliter->input++;
            xmliter->level--;
			xmliter->state = 'c';
			return;
		}
		if (xmliter->input[1] == '?')
		{
			xmliter->input += 2;
            int i = 0;
			while (*xmliter->input != '\0' && *xmliter->input != '?')
			{
				if (i < MAX_VALUE_LEN) xmliter->value[i++] = *xmliter->input;
				xmliter->input++;
			}
			xmliter->value[i] = '\0';
			if (*xmliter->input != '\0' && *xmliter->input == '?')
				xmliter->input++;
			if (*xmliter->input != '\0' && *xmliter->input == '>')
				xmliter->input++;
			xmliter->state = 'm';
			return;
		}
		if (xmliter->input[1] == '!')
		{
			xmliter->input += 2;
            int i = 0;
			while (*xmliter->input != '\0' && *xmliter->input != '>')
			{
				if (i < MAX_VALUE_LEN) xmliter->value[i++] = *xmliter->input;
				xmliter->input++;
			}
			xmliter->value[i] = '\0';
			if (*xmliter->input != '\0' && *xmliter->input == '>')
				xmliter->input++;
			xmliter->state = '!';
			return;
		}
		xmliter->input++;
		int i = 0;
		while (*xmliter->input > ' ' && *xmliter->input != '>' && *xmliter->input != '/')
		{
			if (i < MAX_TAG_LEN)
				xmliter->tag[i++] = *xmliter->input;
			xmliter->input++;
		}
		xmliter->tag[i] = '\0';
        while (*xmliter->input != '\0' && *xmliter->input <= ' ')
            xmliter->input++;
        xmliter->level++;
		xmliter->state = 'o';
		return;
	}
	
	xmliter_parse_string(xmliter, '<');
	xmliter->state = 't';
}


