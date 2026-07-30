// Options

bool opt_trace_parser = FALSE;

// Preprocessor

// The preprocessor is implemented with the use of iterators.
// There are character iterators and token iterators.

// The base character iterator:

typedef struct char_iterator_s char_iterator_t;
typedef struct char_iterator_s* char_iterator_p;
typedef void (*char_next_p)(char_iterator_p);
struct char_iterator_s
{
	char ch;
	long line;
	long column;
	char* filename;
	char_next_p next;
};

// The file iterator

// On creation it points to the first character of the file
// and on each call to the next function it returns the next
// character of the file, where the file is thought to be
// extended with an infinite number of null ('\0') characters
// and carriage return characters ('\r') are skipped.
// The line and column members are updated according to
// line feed ('\n') characters in the file.

struct file_iterator_s
{
	char_iterator_t base;
	FILE* _f;
};
typedef struct file_iterator_s* file_iterator_p;

void file_iterator_next(char_iterator_p char_it)
{
	file_iterator_p it = (file_iterator_p)char_it;
	if (it->base.ch == 0)
	{
		return;
	}
	if (it->base.ch == '\n')
	{
		it->base.line = it->base.line + 1;
		it->base.column = 0;
	}
	it->base.column = it->base.column + 1;
	if (it->_f == NULL)
	{
		it->base.ch = 0;
		return;
	}		
	it->base.ch = fgetc(it->_f);
	if (feof(it->_f))
	{
		it->base.ch = '\0';
		fclose(it->_f);
		it->_f = NULL;
	}
	else if (it->base.ch == '\r')
	{
		file_iterator_next(char_it);
	}
}

file_iterator_p new_file_iterator(const char* fn, const char *alt_fn)
{
	file_iterator_p it = malloc(sizeof(struct file_iterator_s));
	it->_f = fopen(fn, "r");
	if (it->_f == 0 && alt_fn != 0)
		it->_f = fopen(alt_fn, "r");
	it->base.ch = '\n';
	if (it->_f == 0)
	{
		printf("Could not open file %s", fn);
		if (alt_fn != 0 && alt_fn[0] != '\0')
			printf(" or %s", alt_fn);
		printf("\n");
		it->base.ch = 0;
	}
	it->base.filename = copystr(fn);
	it->base.line = 0;
	it->base.column = 0;
	it->base.next = file_iterator_next;
	file_iterator_next(&it->base);
	return it;
}

// The line splice iterator

// The line splice iterator takes care that lines that end
// with a back-slash ('\\') character are joined with the
// next line. On creation the iterator points to the first
// character in the file, assuming that pairs of a
// back-slash charater and line-feed characters are ignored.
// On each call of the next function, the following character
// ignoring such pairs is returned.

struct line_splice_iterator_s
{
	char_iterator_t base;
	char_iterator_p _source_it;
	char _a;
};
typedef struct line_splice_iterator_s* line_splice_iterator_p;

void line_splice_iterator_next(char_iterator_p char_it)
{
	line_splice_iterator_p it = (line_splice_iterator_p)char_it;
	char_next_p _source_it_next; _source_it_next = it->_source_it->next;
	//fprintf(stderr, "line_splice_iterator_next\n");
	if (it->_a == 0)
	{
		it->base.ch = 0;
		return;
	}
	it->base.ch = it->_a;
	it->base.filename = it->_source_it->filename;
	it->base.line = it->_source_it->line;
	it->base.column = it->_source_it->column;
	_source_it_next(it->_source_it);
	it->_a = it->_source_it->ch;
	while (it->base.ch == '\\' && it->_a == '\n')
	{
		_source_it_next(it->_source_it);
		it->base.ch = it->_source_it->ch;
		it->base.filename = it->_source_it->filename;
        it->base.line = it->_source_it->line;
        it->base.column = it->_source_it->column;
        _source_it_next(it->_source_it);
        it->_a = it->_source_it->ch;
    }
}

line_splice_iterator_p new_line_splice_iterator(char_iterator_p source_it)
{
	line_splice_iterator_p it = malloc(sizeof(struct line_splice_iterator_s));
	it->_source_it = source_it;
	it->base.filename = source_it->filename;
	it->base.next = line_splice_iterator_next;
	it->_a = source_it->ch;
	line_splice_iterator_next(&it->base);
	return it;
}


// The comment strip iterator

// The comment strip iterator removes all comments from the input.
// It supports both the original comments which are delimited by
// '/*' and '*/' and the new comments with are delimited by '//'
// and a new-line character. The original comments are replaced
// by a space character and the new comments are replace by a
// new-line character.
// The function comment_strip_iterator_next is implemented as a
// co-routine with the help of goto statements.

struct comment_strip_iterator_s
{
	char_iterator_t base;
	char_iterator_p _source_it;
	char _a;
	int _state;
};
typedef struct comment_strip_iterator_s* comment_strip_iterator_p;

#define MAX_LINE_COMMENT_LEN 200
char last_line_comment[MAX_LINE_COMMENT_LEN+1] = { '\0' };

void clear_last_line_comment(void)
{
	last_line_comment[0] = '\0';
}

char *get_last_line_comment(void)
{
	if (last_line_comment[0] == '\0')
		return NULL;
	char *result = copystr(last_line_comment);
	clear_last_line_comment();
	return result;
}

void comment_strip_iterator_next(char_iterator_p char_it)
{
	comment_strip_iterator_p it = (comment_strip_iterator_p)char_it;
	char_next_p _source_it_next; _source_it_next = it->_source_it->next;

    switch (it->_state)
    {
        case 0: goto s0;
        case 1: goto s1;
        case 2: goto s2;
        case 3: goto s3;
        case 4: goto s4;
        case 5: goto s5;
        case 6: goto s6;
        case 7: goto s7;
        case 8: goto s8;
        case 9: goto s9;
        default:
    }
    s0:
    if (it->_a == '\0')
    {
        it->base.ch = '\0';
        return;
    }
    it->base.ch = it->_a;
    it->base.filename = it->_source_it->filename;
    it->base.line = it->_source_it->line;
    it->base.column = it->_source_it->column;
    _source_it_next(it->_source_it);
    it->_a = it->_source_it->ch;
    if (it->base.ch == '/' && (it->_a == '/' || it->_a == '*'))
    {
        if (it->_a == '/')
        {
			do
			{
                _source_it_next(it->_source_it);
                it->_a = it->_source_it->ch;
			} while (it->_a == ' ');
			int i = 0;
            while (it->_a != '\0' && it->_a != '\n')
            {
				if (i < MAX_LINE_COMMENT_LEN)
					last_line_comment[i++] = it->_a;
                _source_it_next(it->_source_it);
                it->_a = it->_source_it->ch;
            }
			last_line_comment[i] = '\0';
        }
        else
        {
            it->base.ch = ' ';
            it->_state = 1; return; s1:
            _source_it_next(it->_source_it);
            it->base.ch = it->_source_it->ch;
            _source_it_next(it->_source_it);
            it->_a = it->_source_it->ch;
            while (it->base.ch != '\0' && (it->base.ch != '*' || it->_a != '/'))
            {
                it->base.ch = it->_a;
                _source_it_next(it->_source_it);
                it->_a = it->_source_it->ch;
            }
            if (it->base.ch != '\0')
            {
                _source_it_next(it->_source_it);
                it->_a = it->_source_it->ch;
                it->base.line = it->_source_it->line;
                it->base.column = it->_source_it->column;
            }
        }
        it->_state = 0;
        goto s0;
    }
    if (it->base.ch == '"')
    {
        it->_state = 2; return; s2:
        it->base.ch = it->_a;
        it->base.line = it->_source_it->line;
        it->base.column = it->_source_it->column;
        
        while (it->base.ch != '\0' && it->base.ch != '"')
        {
            if (it->base.ch == '\\')
            {
                it->_state = 3; return; s3:
                _source_it_next(it->_source_it);
                it->base.ch = it->_source_it->ch;
                it->base.line = it->_source_it->line;
                it->base.column = it->_source_it->column;
            }
            
            it->_state = 4; return; s4:
            _source_it_next(it->_source_it);
            it->base.ch = it->_source_it->ch;
            it->base.line = it->_source_it->line;
            it->base.column = it->_source_it->column;
        }
        if (it->base.ch == '"')
        {
            it->_state = 5; return; s5:
            _source_it_next(it->_source_it);
            it->_a = it->_source_it->ch;
            it->base.line = it->_source_it->line;
            it->base.column = it->_source_it->column;
        }
        it->_state = 0;
        goto s0;
    }
    if (it->base.ch == '\'')
    {
        it->_state = 6; return; s6:
        it->base.ch = it->_a;
        it->base.line = it->_source_it->line;
        it->base.column = it->_source_it->column;
        if (it->base.ch == '\\')
        {
            it->_state = 7; return; s7:
            _source_it_next(it->_source_it);
            it->base.ch = it->_source_it->ch;
            it->base.line = it->_source_it->line;
            it->base.column = it->_source_it->column;
        }
        it->_state = 8; return; s8:
        _source_it_next(it->_source_it);
        it->base.ch = it->_source_it->ch;
        it->base.line = it->_source_it->line;
        it->base.column = it->_source_it->column;
        it->_state = 0;
        if (it->base.ch == '\'')
        {
            it->_state = 9; return; s9:
            _source_it_next(it->_source_it);
            it->_a = it->_source_it->ch;
            it->base.line = it->_source_it->line;
            it->base.column = it->_source_it->column;
        }
        it->_state = 0;
        goto s0;
    }
}

comment_strip_iterator_p new_comment_strip_iterator(char_iterator_p source_it)
{
	comment_strip_iterator_p it = malloc(sizeof(struct comment_strip_iterator_s));
	it->_source_it = source_it;
	it->base.filename = source_it->filename;
	it->base.next = comment_strip_iterator_next;
	it->_a = source_it->ch;
	it->_state = 0;	
	comment_strip_iterator_next(&it->base);
	return it;
}

// The include iterator

// The include iterator allows character of another character
// iterator to be included in an existing stream by calling
// the function 'include_iterator_add'. Recursive including
// is supported.

struct include_iterator_s
{
	char_iterator_t base;
	char_iterator_p _source_it;
	char_iterator_p _parent_source_its[10];
	int _nr_parents;
};
typedef struct include_iterator_s* include_iterator_p;

void include_iterator_next(char_iterator_p char_it)
{
	include_iterator_p it = (include_iterator_p)char_it;
	char_next_p _source_it_next; _source_it_next = it->_source_it->next;
	_source_it_next(it->_source_it);
	it->base.ch = it->_source_it->ch;
	if (it->base.ch == 0)
	{
		if (it->_nr_parents == 0)
		{
			it->base.ch = 0;
			return;
		}
		it->base.ch = '\n';
		it->_nr_parents = it->_nr_parents - 1;
		it->_source_it = it->_parent_source_its[it->_nr_parents];
		it->base.filename = it->_source_it->filename;
	}
	it->base.filename = it->_source_it->filename;
	it->base.line = it->_source_it->line;
	it->base.column = it->_source_it->column;
}			

void include_iterator_add(include_iterator_p it, char_iterator_p include_it)
{
	it->_parent_source_its[it->_nr_parents] = it->_source_it;
	it->_nr_parents = it->_nr_parents + 1;
	it->_source_it = include_it;
	it->base.filename = it->_source_it->filename;
	it->base.line = it->_source_it->line;
	it->base.column = it->_source_it->column;
	it->base.ch = it->_source_it->ch;
}

include_iterator_p new_include_iterator(char_iterator_p include_it)
{
	include_iterator_p it = malloc(sizeof(struct include_iterator_s));
	it->base.filename = include_it->filename;
	it->base.line = include_it->line;
	it->base.column = include_it->column;
	it->base.ch = include_it->ch;
	it->base.next = include_iterator_next;
	it->_source_it = include_it;
	it->_nr_parents = 0;
	return it;
}

// Token definitions

// Definition of defines for tokens that are not represented
// by a single character. 

#define TK_D_HASH	 500
#define TK_EQ		 501
#define TK_NE		 502
#define TK_LE		 503
#define TK_GE		 504
#define TK_INC		 505
#define TK_DEC		 506
#define TK_ARROW	 507
#define TK_DPERIOD   508
#define TK_DASHES    509

#define TK_ASS		600
#define TK_MUL_ASS  (600 + '*')
#define TK_DIV_ASS	(600 + '/')
#define TK_MOD_ASS	(600 + '%')
#define TK_ADD_ASS	(600 + '+')
#define TK_SUB_ASS	(600 + '-')
#define TK_SHL_ASS	(600 + '<')
#define TK_SHR_ASS	(600 + '>')
#define TK_XOR_ASS  (600 + '^')
#define TK_BAND_ASS (600 + '&')
#define TK_BOR_ASS  (600 + '|')

#define TK_DD_OPER	800
#define TK_SHL		(800 + '<')
#define TK_SHR		(800 + '>')
#define TK_AND		(800 + '&')
#define TK_OR		(800 + '|')

#define TK_KEYWORD		1000
#define TK_BREAK        1000
#define TK_CASE			1001
#define TK_CHAR			1002
#define TK_CONST		1003
#define TK_CONTINUE     1004
#define TK_DEFAULT		1005
#define TK_DO			1006
#define TK_DOUBLE		1007
#define TK_ELSE			1008
#define TK_ENUM			1009
#define TK_EXTERN		1010
#define TK_FLOAT        1011
#define TK_FOR			1012
#define TK_GOTO			1013
#define TK_IF			1014
#define TK_INLINE		1015
#define TK_INT			1016
#define TK_LONG			1017
#define TK_RETURN       1018
#define TK_SHORT		1019
#define TK_SIZEOF		1020
#define TK_STATIC		1021
#define TK_STRUCT		1022
#define TK_SWITCH		1023
#define TK_TYPEDEF		1024
#define TK_UNION		1025
#define TK_UNSIGNED		1026
#define TK_VOID			1027
#define TK_WHILE		1028
#define TK_H_ELSE		1029
#define TK_H_ELIF		1030
#define TK_H_ENDIF		1031
#define TK_H_DEFINE		1032
#define TK_DEFINED		1033
#define TK_H_IF			1034
#define TK_H_IFDEF		1035
#define TK_H_IFNDEF		1036
#define TK_H_INCLUDE	1037
#define TK_H_UNDEF		1038
#define TK_H_ERROR		1039

// The base token iterator

typedef struct token_iterator_s token_iterator_t;
typedef struct token_iterator_s* token_iterator_p;
typedef token_iterator_p (*token_next_p)(token_iterator_p, bool);
struct token_iterator_s
{
	int kind;
	char *token;
	unsigned int length;
	char *filename;
	int line;
	int column;
	token_next_p next;
};

// The tokenizer iterator

// The tokenizer iterator recognizes the C tokens from the
// stream of characters returned by a character iterator.
// On creation it is on the first recognized token and
// on each call to the next function it returns the next
// recognized token.

struct tokenizer_s
{
	token_iterator_t base;
	char_iterator_p _char_iterator;
};
typedef struct tokenizer_s *tokenizer_p;

void tokenizer_skip_white_space(tokenizer_p tokenizer, bool skip_nl)
{
	char_iterator_p it; it = tokenizer->_char_iterator;
	char_next_p it_next = it->next;
	char ch = it->ch;

	while (ch != 0 && ch <= ' ' && (skip_nl || ch != '\n'))
	{
		it_next(it); ch = it->ch;
	}
}

char tokenizer_parse_char_literal(tokenizer_p tokenizer)
{
	char_iterator_p it = tokenizer->_char_iterator;
	char_next_p it_next; it_next = it->next;
	char ch = it->ch;
	it_next(it);
	if (ch != '\\')
	{
		 return ch;
	}
	ch = it->ch;
	it_next(it);
	if (ch == '0' || ch == '1')
	{
		int val = ch - '0';
		ch = it->ch;
		if ('0' <= ch && ch <= '7')
		{
			val = 8 * val + ch - '0';
			it_next(it);
			ch = it->ch;
			if ('0' <= ch && ch <= '7')
			{
				val = 8 * val + ch - '0';
				it_next(it);
			}
			return val;
		}
		else if (val == 0)
		{
			return 0;
		}
		else
		{
			return '1';
		}
	}
	if (ch == 'a')
	{
		ch = '\a';
	}
	else if (ch == 'b')
	{
		ch = '\b';
	}
	else if (ch == 'f')
	{
		ch = '\f';
	}
	else if (ch == 'n')
	{
		ch = '\n';
	}
	else if (ch == 'r')
	{
		ch = '\r';
	}
	else if (ch == 't')
	{
		ch = '\t';
	}
	else if (ch == 'v')
	{
		ch = '\v';
	}
	else if (ch == 'x')
	{
		int v = 0;
		ch = it->ch;
		if ('0' <= ch && ch <= '9')
			v = 16 * (ch - '0');
		else if ('A' <= ch && ch <= 'F')
			v = 16 * (ch - 'A' + 10);
		else if ('a' <= ch && ch <= 'f')
			v = 16 * (ch - 'a' + 10);
		else
			printf("%s:%d.%d Warning: Invalid character '%c' after \\x\n", it->filename, (int)it->line, (int)it->column, ch);
		it_next(it);
		ch = it->ch;
		if ('0' <= ch && ch <= '9')
			v += ch - '0';
		else if ('A' <= ch && ch <= 'F')
			v += ch - 'A' + 10;
		else if ('a' <= ch && ch <= 'f')
			v += ch - 'a' + 10;
		else
			printf("%s:%d.%d Warning: Invalid character '%c' after \\x\n", it->filename, (int)it->line, (int)it->column, ch);
		ch = v;
		it_next(it);
	}
	else if (ch == '\'' || ch == '"' || ch == '\\')
		;
	else
		printf("%s:%d.%d Warning: Unknown escape sequence \\%c\n", it->filename, (int)it->line, (int)it->column, ch);
	return ch;
}

#define MAX_TOKEN_LEN 6000

token_iterator_p tokenizer_next(token_iterator_p token_it, bool skip_nl)
{
	tokenizer_p tokenizer = (tokenizer_p)token_it;
	tokenizer_skip_white_space(tokenizer, skip_nl);
	char_iterator_p it = tokenizer->_char_iterator;
	char_next_p it_next; it_next = it->next;
	int i = 0;
	char ch = it->ch;
	tokenizer->base.kind = 0;
	if (ch == 0)
	{
		goto done;
	}
	bool at_start_of_line =    token_it->filename != tokenizer->_char_iterator->filename
							|| token_it->line != tokenizer->_char_iterator->line;
	token_it->filename = it->filename;
	token_it->line = it->line;
	token_it->column = it->column;
	if (ch == '\n')
	{
		token_it->kind = '\n';
	}
	else if (ch == '#' && at_start_of_line)
	{
		token_it->token[i] = '#'; i = i + 1; it_next(it);
		tokenizer_skip_white_space(tokenizer, TRUE);
		ch = it->ch;
		while ('a' <= ch && ch <= 'z')
		{
			token_it->token[i] = ch; i = i + 1; it_next(it); ch = it->ch;
		}
		token_it->token[i] = '\0';
		     if (eqstr("#else",    token_it->token)) token_it->kind = TK_H_ELSE;
		else if (eqstr("#elif",    token_it->token)) token_it->kind = TK_H_ELIF;
		else if (eqstr("#endif",   token_it->token)) token_it->kind = TK_H_ENDIF;
		else if (eqstr("#define",  token_it->token)) token_it->kind = TK_H_DEFINE;
		else if (eqstr("#if",      token_it->token)) token_it->kind = TK_H_IF;
		else if (eqstr("#ifdef",   token_it->token)) token_it->kind = TK_H_IFDEF;
		else if (eqstr("#ifndef",  token_it->token)) token_it->kind = TK_H_IFNDEF;
		else if (eqstr("#include", token_it->token)) token_it->kind = TK_H_INCLUDE;
		else if (eqstr("#undef",   token_it->token)) token_it->kind = TK_H_UNDEF;
		else if (eqstr("#error",   token_it->token)) token_it->kind = TK_H_ERROR;
	}
	else if (('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || ch == '_')
	{
		token_it->kind = 'i';
		token_it->token[i] = ch; i = i + 1; it_next(it); ch = it->ch;
		while (('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || ('0' <= ch && ch <= '9') || ch == '_')
		{
			token_it->token[i] = ch; i = i + 1; it_next(it); ch = it->ch;
		}
		token_it->token[i] = '\0';
		     if (eqstr("break",    token_it->token)) token_it->kind = TK_BREAK;
		else if (eqstr("case",     token_it->token)) token_it->kind = TK_CASE;
		else if (eqstr("char",     token_it->token)) token_it->kind = TK_CHAR;
		else if (eqstr("continue", token_it->token)) token_it->kind = TK_CONTINUE;
		else if (eqstr("const",    token_it->token)) token_it->kind = TK_CONST;
		else if (eqstr("default",  token_it->token)) token_it->kind = TK_DEFAULT;
		else if (eqstr("defined",  token_it->token)) token_it->kind = TK_DEFINED;
		else if (eqstr("do",       token_it->token)) token_it->kind = TK_DO;
		else if (eqstr("double",   token_it->token)) token_it->kind = TK_DOUBLE;
		else if (eqstr("else",     token_it->token)) token_it->kind = TK_ELSE;
		else if (eqstr("enum",     token_it->token)) token_it->kind = TK_ENUM;
		else if (eqstr("extern",   token_it->token)) token_it->kind = TK_EXTERN;
		else if (eqstr("float",    token_it->token)) token_it->kind = TK_FLOAT;
		else if (eqstr("for",      token_it->token)) token_it->kind = TK_FOR;
		else if (eqstr("goto",     token_it->token)) token_it->kind = TK_GOTO;
		else if (eqstr("if",       token_it->token)) token_it->kind = TK_IF;
		else if (eqstr("inline",   token_it->token)) token_it->kind = TK_INLINE;
		else if (eqstr("int",      token_it->token)) token_it->kind = TK_INT;
		else if (eqstr("long",     token_it->token)) token_it->kind = TK_LONG;
		else if (eqstr("return",   token_it->token)) token_it->kind = TK_RETURN;
		else if (eqstr("short",    token_it->token)) token_it->kind = TK_SHORT;
		else if (eqstr("sizeof",   token_it->token)) token_it->kind = TK_SIZEOF;
		else if (eqstr("static",   token_it->token)) token_it->kind = TK_STATIC;
		else if (eqstr("struct",   token_it->token)) token_it->kind = TK_STRUCT;
		else if (eqstr("switch",   token_it->token)) token_it->kind = TK_SWITCH;
		else if (eqstr("typedef",  token_it->token)) token_it->kind = TK_TYPEDEF;
		else if (eqstr("union",    token_it->token)) token_it->kind = TK_UNION;
		else if (eqstr("unsigned", token_it->token)) token_it->kind = TK_UNSIGNED;
		else if (eqstr("void",     token_it->token)) token_it->kind = TK_VOID;
		else if (eqstr("while",    token_it->token)) token_it->kind = TK_WHILE;
	}
	else if ('0' <= ch && ch <= '9')
	{
		token_it->kind = '0';
		if (ch == '0')
		{
			token_it->token[i] = ch; i = i + 1; it_next(it); ch = it->ch;
			if (ch == 'x')
			{
				token_it->token[i] = ch; i = i + 1; it_next(it); ch = it->ch;
				while (('0' <= ch && ch <= '9') || ('a' <= ch && ch <= 'f') || ('A' <= ch && ch <= 'F'))
				{
					token_it->token[i] = ch; i = i + 1; it_next(it); ch = it->ch;
				}
			}
			else
			{
				while ('0' <= ch && ch <= '7')
				{
					token_it->token[i] = ch; i = i + 1; it_next(it); ch = it->ch;
				}
			}
		}
		else
		{
			while ('0' <= ch && ch <= '9')
			{
				token_it->token[i] = ch; i = i + 1; it_next(it); ch = it->ch;
			}
		}
		if (ch == 'U')
		{
			token_it->token[i] = ch; i = i + 1; it_next(it); ch = it->ch;
		}
		while (ch == 'L')
		{
			token_it->token[i] = ch; i = i + 1; it_next(it); ch = it->ch;
		}
	}
	else if (ch == '\'')
	{
		token_it->kind = ch;
		it_next(it);
		token_it->token[0] = tokenizer_parse_char_literal(tokenizer);
		i = 1;
		if (it->ch == '\'')
		{
			it_next(it);
		}
	}
	else if (ch == '"')
	{
		token_it->kind = ch; it_next(it); ch = it->ch;
		while (ch != '\0' && ch != '\n' && ch != '"')
		{
			if (i == MAX_TOKEN_LEN)
			{
				fprintf(stderr, "MAX_TOKEN_LEN %d exceeded\n", i);
				exit(-1);
			}
			token_it->token[i] = tokenizer_parse_char_literal(tokenizer);
			i++;
			ch = it->ch;
		}
		if (ch == '"')
		{
			it_next(it);
		}
	}
	else
	{
		token_it->kind = ch;
		token_it->token[0] = ch;
		i = 1;
		if (ch == '#')
		{
			it_next(it);
			if (it->ch == '#')
			{
				it_next(it);
				token_it->kind = TK_D_HASH;
				token_it->token[1] = '#';
				i = 2;
			}
		}
		else if (ch == '=')
		{
			it_next(it);
			if (it->ch == '=')
			{
				it_next(it);
				token_it->kind = TK_EQ;
				token_it->token[1] = '=';
				i = 2;
			}
		}
		else if (ch == '*' || ch == '/' || ch == '%' || ch == '^')
		{
			it_next(it);
			if (it->ch == '=')
			{
				it_next(it);
				token_it->kind = TK_ASS + ch;
				token_it->token[1] = '=';
				i = 2;
			}
		}
		else if (ch == '-')
		{
			it_next(it); ch = it->ch;
			if (ch == '-')
			{
				it_next(it);
				token_it->kind = TK_DEC;
				token_it->token[1] = ch;
				i = 2;
			}
			else if (ch == '=')
			{
				it_next(it);
				token_it->kind = TK_SUB_ASS;
				token_it->token[1] = ch;
				i = 2;
			}
			else if (ch == '>')
			{
				it_next(it);
				token_it->kind = TK_ARROW;
				token_it->token[1] = ch;
				i = 2;
			}
		}			
		else if (ch == '+' || ch == '-')
		{
			it_next(it);
			ch = it->ch;
			if (ch == token_it->token[0] || ch == '=')
			{
				it_next(it);
				token_it->token[1] = ch;
				if (ch == '+')
				{
					token_it->kind = TK_INC;
				}
				else if (ch == '-')
				{
					token_it->kind = TK_DEC;
				}
				else
				{
					token_it->kind = TK_ASS + token_it->token[0];
				}
				i = 2;
			}
		}
		else if (ch == '<' || ch == '>' || ch == '|' || ch == '&')
		{
			it_next(it);
			if (it->ch == ch)
			{
				it_next(it);
				token_it->kind = TK_DD_OPER + ch;
				token_it->token[1] = ch;
				i = 2;
			}
			if (it->ch == '=' && (ch == '<' || ch == '>' || ch == '&' || ch == '|')) 
			{
				it_next(it);
				if (i == 2)
				{
					token_it->kind = TK_ASS + ch;
				}
				else if (ch == '&')
				{
					token_it->kind = TK_BAND_ASS;
				}
				else if (ch == '|')
				{
					token_it->kind = TK_BOR_ASS;
				}
				else if (ch == '<')
				{
					token_it->kind = TK_LE;
				}
				else
				{
					token_it->kind = TK_GE;
				}
				token_it->token[i] = '=';
				i = i + 1;
			}
		}
		else if (ch == '!')
		{
			it_next(it);
			if (it->ch == '=')
			{
				it_next(it);
				token_it->kind = TK_NE;
				token_it->token[1] = '=';
				i = 2;
			}
		}
		else if (ch == '.')
		{
			it_next(it);
			if (it->ch == '.')
			{
				it_next(it);
				token_it->kind = TK_DPERIOD;
				token_it->token[1] = '.';
				i = 2;
				if (it->ch == '.')
				{
					it_next(it);
					token_it->kind = TK_DASHES;
					token_it->token[2] = '.';
					i = 3;
				}
			}
		}
		else
		{
			it_next(it);
		}
	}
	done:
	token_it->token[i] = '\0';
	token_it->length = i;
	if (opt_trace_parser)
		printf("tokenizer_next %d: %d '%s'\n", token_it->line, token_it->kind, token_it->token);
	return token_it;
}

int string_int_value(const char *s)
{
	int int_value = 0;
	if (*s == '0')
	{
		s++;
		if (*s == 'x')
		{
			s++;
			while (1)
			{
				if ('0' <= *s && *s <= '9')
					int_value = 16 * int_value + *s - '0';
				else if ('a' <= *s && *s <= 'f')
					int_value = 16 * int_value + *s - 'a' + 10;
				else if ('A' <= *s && *s <= 'F')
					int_value = 16 * int_value + *s - 'A' + 10;
				else
					break;
				s++;
			}
		}
		else
		{
			while ('0' <= *s && *s <= '7')
			{
				int_value = 8 * int_value + *s - '0';
				s++;
			}
		}
	}
	else
	{
		while ('0' <= *s && *s <= '9')
		{
			int_value = 10 * int_value + *s - '0';
			s++;
		}
	}
	if (*s == 'U')
	{
		s++;
	}
	while (*s == 'L')
	{
		s++;
	}
	return int_value;
}

int token_it_int_value(token_iterator_p it)
{
	return string_int_value(it->token);
}

tokenizer_p new_tokenizer(char_iterator_p char_iterator)
{
	tokenizer_p tokenizer = malloc(sizeof(struct tokenizer_s));
	tokenizer->_char_iterator = char_iterator;
	tokenizer->base.token = malloc(MAX_TOKEN_LEN);
	tokenizer->base.filename = NULL;
	tokenizer->base.next = tokenizer_next;
	return tokenizer;
}

// List of tokens

struct tokens_s
{
	int kind;
	char *token;
	int length;
	char *filename;
	int line;
	int column;
	struct tokens_s* next;
};
typedef struct tokens_s* tokens_p;

tokens_p new_token(int kind)
{
	tokens_p token = malloc(sizeof(struct tokens_s));
	token->kind = kind;
	token->token = 0;
	token->length = 0;
	token->filename = "<env>";
	token->line = 0;
	token->column = 0;
	token->next = 0;
	return token;
}
tokens_p new_int_token(const char *str)
{
	tokens_p token = new_token('0');
	token->token = copystr(str);
	return token;
}
tokens_p new_str_token(char *str)
{
	tokens_p token = new_token('"');
	token->token = copystr(str);
	token->length = strlen(str);
	return token;
}
tokens_p new_token_from_it(token_iterator_p it)
{
	tokens_p token = new_token(it->kind);
	token->token = copystrlen(it->token, it->length);
	token->length = it->length;
	token->filename = it->filename;
	token->line = it->line;
	token->column = it->column;
	return token;
}

// The enironment for defined symbols and macros

struct env_s {
	char* name;
	int nr_args;
	char* args[10];
	tokens_p tokens;
	struct env_s* next;
};
typedef struct env_s env_t;
typedef struct env_s *env_p;

env_p environment = NULL;

env_p get_env(char* name, bool create)
{
	env_p prev_env = NULL;
	env_p env = environment;
	while (env != NULL)
	{
		if (eqstr(env->name, name))
			return env;
		prev_env = env;
		env = env->next;
	}
	if (!create)
		return NULL;
	env = malloc(sizeof(struct env_s));
	env->name = copystr(name);
	env->nr_args = 0;
	env->tokens = NULL;
	env->next = NULL;
	if (prev_env == NULL)
	{
		environment = env;
	}
	else
	{
		prev_env->next = env;
	}
	return env;
}

void del_env(char* name)
{
	env_p prev_env = NULL;
	env_p env = environment;
	while (env != 0)
	{
		if (eqstr(env->name, name))
		{
			if (prev_env == NULL)
			{
				environment = environment->next;
			}
			else
			{
				prev_env->next = env->next;
			}
			return;
		}
		prev_env = env;
		env = env->next;
	}
}

// The conditional iterator

// The conditional iterator deals with all conditional compilation
// directives, such as '#if', and also the directives
// `#define`, `#undef`, `#include`, and `#error`.

typedef struct conditional_iterator_s* conditional_iterator_p;
typedef int (*parse_expr_function_p)(conditional_iterator_p it);

struct conditional_iterator_s
{
	token_iterator_t base;
	include_iterator_p _source_it;
	token_iterator_p _token_it;
	parse_expr_function_p _parse_or_expr;
	
	bool _skip_level;
	bool _if_done[40];
	int _if_level;
};

/*
	conditional_primary
		: '(' conditional_or_exp ')'
		| 'defined' ( '(' ident ')' |  ident )
		| integer
		| ident
		.
*/

int conditional_iterator_parse_primary(conditional_iterator_p it)
{
	token_next_p token_it_next = it->_token_it->next;
	int result = 0;
	if (it->_token_it->kind == '(')
	{
		token_it_next(it->_token_it, FALSE);
		parse_expr_function_p parse_or_expr = it->_parse_or_expr;
		result = parse_or_expr(it);
		if (it->_token_it->kind == ')')
		{
			token_it_next(it->_token_it, FALSE);
		}
	}
	else if (it->_token_it->kind == TK_DEFINED)
	{
		token_it_next(it->_token_it, FALSE);
		if (it->_token_it->kind == '(')
		{
			token_it_next(it->_token_it, FALSE);
			//printf("defined(%s) = %d\n", it->_token_it->token, get_env(it->_token_it->token, FALSE) != NULL);
			if (get_env(it->_token_it->token, FALSE) != NULL)
			{
				result = 1;
			}
			token_it_next(it->_token_it, FALSE);
			if (it->_token_it->kind == ')')
			{
				token_it_next(it->_token_it, FALSE);
			}
		}
		else
		{
			if (get_env(it->_token_it->token, FALSE) != NULL)
			{
				result = 1;
			}
			token_it_next(it->_token_it, FALSE);
		}
	}
	else if (it->_token_it->kind == '0')
	{
		result = token_it_int_value(it->_token_it);
		token_it_next(it->_token_it, FALSE);
	}
	else if (it->_token_it->kind == 'i')
	{
		// The code assumes that expansion always leads to a single numeric value
		env_p env = get_env(it->_token_it->token, FALSE);
		while (env != NULL && env->tokens != NULL && env->tokens->kind == 'i')
			env = get_env(env->tokens->token, FALSE);
		if (env != NULL && env->tokens != NULL && env->tokens->kind == '0')
		{
			result = string_int_value(env->tokens->token);
		}
		token_it_next(it->_token_it, FALSE);
	}
	return result;
}

/*
	conditional_unary_expr
		: '!' conditional_primary
		| conditional_primary
		.
*/

int conditional_iterator_parse_unary_expr(conditional_iterator_p it)
{
	token_next_p token_it_next = it->_token_it->next;
	if (it->_token_it->kind == '!')
	{
		token_it_next(it->_token_it, FALSE);
		int result = conditional_iterator_parse_primary(it);
		return !result; //!conditional_iterator_parse_primary(it);
	}
	return conditional_iterator_parse_primary(it);
}

/*
	conditional_compare_expr
		: conditional_unary_expr
		  ('==' conditional_unary_expr 
		  |'!=' conditional_unary_expt ) OPT
		.
*/

int conditional_iterator_parse_compare_expr(conditional_iterator_p it)
{
	token_next_p token_it_next = it->_token_it->next;
	int value = conditional_iterator_parse_unary_expr(it);
	if (it->_token_it->kind == TK_EQ)
	{
		token_it_next(it->_token_it, FALSE);
		return value == conditional_iterator_parse_unary_expr(it);
	}
	if (it->_token_it->kind == TK_NE)
	{
		token_it_next(it->_token_it, FALSE);
		return value != conditional_iterator_parse_unary_expr(it);
	}
	return value;
}

/*
	conditional_and_expr : conditional_compare_expr CHAIN '&&' .
*/

int conditional_iterator_parse_and_expr(conditional_iterator_p it)
{
	token_next_p token_it_next = it->_token_it->next;
	int value = conditional_iterator_parse_compare_expr(it);
	while (it->_token_it->kind == TK_AND)
	{
		token_it_next(it->_token_it, FALSE);
		int value2 = conditional_iterator_parse_compare_expr(it);
		value = value && value2;
	}
	return value;
}

/*
	conditional_or_expr : conditional_and_expr CHAIN '||' .
*/

int conditional_iterator_parse_or_expr(conditional_iterator_p it)
{
	token_next_p token_it_next = it->_token_it->next;
	int value = conditional_iterator_parse_and_expr(it);
	while (it->_token_it->kind == TK_OR)
	{
		token_it_next(it->_token_it, FALSE);
		int value2 = conditional_iterator_parse_and_expr(it);
		value = value || value2;
	}
	return value;
}

char *include_path = 0;
char *end_include_prefix = 0;
char alt_include_path[1000];
char *end_alt_include_prefix = 0;
char std_include_path[1000];
char *end_std_include_prefix = 0;

token_iterator_p conditional_iterator_next(token_iterator_p token_it, bool dummy)
{
	conditional_iterator_p it = (conditional_iterator_p)token_it;
	//printf("conditional_iterator_next\n");
	token_next_p token_it_next = it->_token_it->next;
	int kind;
	int value = 0;
	tokens_p prev_token;
	env_p env;
	tokens_p token;
	file_iterator_p input_it;
	line_splice_iterator_p splice_it;
	comment_strip_iterator_p comment_it;
	
	token_it_next(it->_token_it, TRUE);
	while (TRUE)
	{
		//if (it->_token_it->token[0] == '#')
		//	printf("%s.%d %s (%d)\n", it->_token_it->filename, it->_token_it->line, it->_token_it->token, it->_skip_level);
		kind = it->_token_it->kind;
		if (it->_skip_level > 0)
		{
			if (kind == TK_H_IF || kind == TK_H_IFDEF || kind == TK_H_IFNDEF)
			{
				it->_skip_level = it->_skip_level + 1;
			}
			else if (kind == TK_H_ENDIF)
			{
				it->_skip_level = it->_skip_level - 1;
				if (it->_skip_level == 0)
				{
					it->_if_level = it->_if_level - 1;
				}
			}
			else if (kind == TK_H_ELSE)
			{
				if (it->_skip_level == 1)
				{
					if (it->_if_done[it->_if_level] == FALSE)
					{
						it->_skip_level = 0;
					}
				}
			}
			else if (kind == TK_H_ELIF)
			{
				token_it_next(it->_token_it, FALSE);
				value = conditional_iterator_parse_or_expr(it);
				if (it->_skip_level == 1)
				{
					if (it->_if_done[it->_if_level] == FALSE)
					{
						token_it_next(it->_token_it, FALSE);
						if (value)
						{
							it->_skip_level = 0;
							it->_if_done[it->_if_level] = TRUE;
						}
					}
				}
			}
			token_it_next(it->_token_it, TRUE);
		}
		else if (kind == TK_H_IFDEF || kind == TK_H_IFNDEF)
		{
			token_it_next(it->_token_it, TRUE);
			it->_if_level = it->_if_level + 1;
			if (it->_token_it->kind == 'i')
			{
				//int defined = value = get_env(it->_token_it->token, FALSE) != 0;
				//printf("for %s is defined %d\n", it->_token_it->token, defined);
				if ((get_env(it->_token_it->token, FALSE) != 0) == (kind == TK_H_IFDEF))
				{
					it->_if_done[it->_if_level] = TRUE;
				}
				else
				{
					it->_if_done[it->_if_level] = FALSE;
					it->_skip_level = 1;
				}
				token_it_next(it->_token_it, TRUE);
			}
		}
		else if (kind == TK_H_IF)
		{
			token_it_next(it->_token_it, TRUE);
			it->_if_level = it->_if_level + 1;
			if (conditional_iterator_parse_or_expr(it))
			{
				it->_if_done[it->_if_level] = TRUE;
			}
			else
			{
				it->_if_done[it->_if_level] = FALSE;
				it->_skip_level = 1;
			}
			token_it_next(it->_token_it, TRUE);
		}
		else if (kind == TK_H_ELSE || kind == TK_H_ELIF)
		{
			it->_skip_level = 1;
			while (it->_token_it->kind == '\n')
			{
				token_it_next(it->_token_it, FALSE);
			}
			token_it_next(it->_token_it, TRUE);
		}
		else if (kind == TK_H_ENDIF)
		{	
			it->_if_level = it->_if_level - 1;
			token_it_next(it->_token_it, TRUE);
		}
		else if (kind == TK_H_DEFINE)
		{
			prev_token = NULL;
			token_it_next(it->_token_it, FALSE);
			env = get_env(it->_token_it->token, TRUE);
			env->nr_args = 0;
			if (it->_source_it->base.ch == '(')
			{
				token_it_next(it->_token_it, FALSE);
				while (TRUE)
				{
					token_it_next(it->_token_it, FALSE);
					env->args[env->nr_args] = copystr(it->_token_it->token);
					env->nr_args = env->nr_args + 1;
					token_it_next(it->_token_it, FALSE);
					if (it->_token_it->kind != ',')
					{
						break;
					}
				}
				if (it->_token_it->kind == ')')
				{
					token_it_next(it->_token_it, FALSE);
				}
			}
			else
			{
				token_it_next(it->_token_it, FALSE);
			}
			while (it->_token_it->kind != '\n')
			{
				token = new_token_from_it(it->_token_it);
				if (prev_token == NULL)
				{
					env->tokens = token;
				}
				else
				{
					prev_token->next = token;
				}
				prev_token = token;
				token_it_next(it->_token_it, FALSE);
			}
			token_it_next(it->_token_it, TRUE);
		}
		else if (kind == TK_H_UNDEF)
		{
			token_it_next(it->_token_it, TRUE);
			del_env(it->_token_it->token);
			token_it_next(it->_token_it, TRUE);
		}
		else if (kind == TK_H_INCLUDE)
		{
			token_it_next(it->_token_it, TRUE);
			if (it->_token_it->kind == '"')
			{
				strcpy(end_include_prefix, it->_token_it->token);
				if (end_alt_include_prefix != 0)
					strcpy(end_alt_include_prefix, it->_token_it->token);
				while (it->_token_it->kind != '\n')
				{
					token_it_next(it->_token_it, FALSE);
				}
				//printf("INCLUDE '%s'\n", include_path);
				input_it = new_file_iterator(include_path, alt_include_path);
				if (input_it->base.ch != 0)
				{
					splice_it = new_line_splice_iterator(&input_it->base);
					comment_it = new_comment_strip_iterator(&splice_it->base);
					include_iterator_add(it->_source_it, &comment_it->base);
					token_it_next(it->_token_it, TRUE);
				}
				else
					token_it_next(it->_token_it, TRUE);
			}
			else if (it->_token_it->kind == '<' && end_std_include_prefix != 0)
			{
				token_it_next(it->_token_it, FALSE);
				*end_std_include_prefix = '\0';
				while (it->_token_it->kind != '\n' && it->_token_it->kind != '>')
				{
					strcat(std_include_path, it->_token_it->token);
					token_it_next(it->_token_it, FALSE);
				}

				while (it->_token_it->kind != '\n')
					token_it_next(it->_token_it, FALSE);

				input_it = new_file_iterator(std_include_path, 0);
				if (input_it->base.ch != 0)
				{
					splice_it = new_line_splice_iterator(&input_it->base);
					comment_it = new_comment_strip_iterator(&splice_it->base);
					include_iterator_add(it->_source_it, &comment_it->base);
					token_it_next(it->_token_it, TRUE);
				}
				else
					token_it_next(it->_token_it, TRUE);
			}
			else
			{
				while (it->_token_it->kind != '\n')
					token_it_next(it->_token_it, FALSE);
				if (it->_token_it->kind == '\n')
					token_it_next(it->_token_it, TRUE);
			}
		}
		else if (kind == TK_H_ERROR)
		{
			it->base.kind = it->_token_it->kind;
			it->base.token = it->_token_it->token;
			it->base.length = it->_token_it->length;
			it->base.filename = it->_token_it->filename;
			it->base.line  = it->_token_it->line;
			it->base.column = it->_token_it->column;
			return token_it;
		}
		else
		{
			it->base.kind = it->_token_it->kind;
			it->base.token = it->_token_it->token;
			it->base.length = it->_token_it->length;
			it->base.filename = it->_token_it->filename;
			it->base.line  = it->_token_it->line;
			it->base.column = it->_token_it->column;
			return token_it;
		}
	}
	return token_it;
}	

conditional_iterator_p new_conditional_iterator(include_iterator_p source_it, token_iterator_p token_it)
{
	conditional_iterator_p it = malloc(sizeof(struct conditional_iterator_s));
	it->_source_it = source_it;
	it->_token_it = token_it;
	it->_parse_or_expr = conditional_iterator_parse_or_expr;
	it->_skip_level = 0;
	it->_if_level = 0;
	it->base.next = conditional_iterator_next;
	return it;
}

// The expand macro iterator

// This iterator implements the expansion of a macro with a list
// of tokens for the arguments. It also implements processing of
// the '#' and `##` operators during expansion. After expansion
// has been completed, it returns the token iterator for the
// remaining part.

#define APPENDED_TOKEN_LEN 50

typedef struct expand_macro_iterator_s* expand_macro_iterator_p;
struct expand_macro_iterator_s
{
	token_iterator_t base;
	tokens_p param_tokens;
	tokens_p tokens;
	token_iterator_p _rest_it;
	bool stringify;
	char appended_token[APPENDED_TOKEN_LEN];
	env_p _macro;
	tokens_p args[10];
};

token_iterator_p expand_macro_iterator_next(token_iterator_p token_it, bool dummy)
{
	expand_macro_iterator_p it = (expand_macro_iterator_p)token_it;
	if (it->param_tokens != 0)
	{
		token_it->kind = it->stringify ? '"' : it->param_tokens->kind;
		token_it->token = it->param_tokens->token;
		token_it->length = it->param_tokens->length;
		token_it->filename = it->param_tokens->filename;
		token_it->line = it->param_tokens->line;
		token_it->column = it->param_tokens->column;
		if (opt_trace_parser)
			printf("token from arg %d %s\n", token_it->kind, token_it->token == 0 ? "?" : token_it->token);
		it->param_tokens = it->param_tokens->next;
		return token_it;
	}
	
	for (;;)
	{
		tokens_p token = it->tokens;
		if (token == 0)
		{
			token_iterator_p rest_it = it->_rest_it;
			free(it);
			return rest_it->next(rest_it, dummy);
		}
		
		if (opt_trace_parser)
			printf("token from macro %d %s\n", token->kind, token->token == 0 ? "?" : token->token);
		it->tokens = token->next;
		it->stringify = FALSE;
		
		if (token->next != NULL && token->next->kind == TK_D_HASH)
		{
			token_it->kind = 'i';
			token_it->filename = token->filename;
			token_it->line = token->line;
			token_it->column = token->column;
			int p = 0;
			for (;;)
			{
				const char *s = token->token;
				if (token->kind == 'i')
				{
					int nr_args = it->_macro->nr_args;
					int i = 0;
					for (; i < nr_args; i++)
						if (strcmp(token->token, it->_macro->args[i]) == 0)
							break;
					if (i < nr_args)
					{
						tokens_p tokens = it->args[i];
						if (tokens == NULL)
						{
							if (opt_trace_parser)
								printf("INFO: append arg no value\n");
							s = "";
						}
						else if (tokens->next != NULL)
						{
							printf("ERROR: append arg more than one value\n");
							s = "";
						}
						else
							s = tokens->token;
					}
				}
				for (; *s != '\0'; s++)
					if (p < APPENDED_TOKEN_LEN - 1)
						it->appended_token[p++] = *s;
				token = token->next;
				if (token == NULL || token->kind != TK_D_HASH)
					break;
				token = token->next;
			}
			it->tokens = token;
			it->appended_token[p] = '\0';
			if (opt_trace_parser)
				printf("INFO: Appended token '%s'\n", it->appended_token);
			token_it->token = it->appended_token;
			token_it->length = p;
			
			return token_it;
		}
		else
		{
			if (token->kind == '#')
			{
				it->stringify = TRUE;
				it->tokens = token = token->next;
				if (it->tokens == 0)
					return it->_rest_it->next(it->_rest_it, dummy);
			}
			
			if (token->kind == 'i')
			{
				int nr_args = it->_macro->nr_args;
				int i = 0;
				for (; i < nr_args; i++)
					if (strcmp(token->token, it->_macro->args[i]) == 0)
						break;
				if (i < nr_args)
				{
					tokens_p tokens = it->args[i];
					if (tokens == 0)
						continue;
					token_it->kind = it->stringify ? '"' : tokens->kind;
					token_it->token = tokens->token;
					token_it->length = tokens->length;
					token_it->filename = tokens->filename;
					token_it->line = tokens->line;
					token_it->column = token->column;
					it->param_tokens = tokens->next;
					it->tokens = token->next;
					return token_it;
				}
			}
			token_it->kind = it->stringify ? '"' : token->kind;
			token_it->token = token->token;
			token_it->length = token->length;
			token_it->filename = token->filename;
			token_it->line = token->line;
			token_it->column = token->column;
			
			return token_it;
		}
	}
}

token_iterator_p new_exapnd_macro_iterator(env_p macro, tokens_p *args, token_iterator_p rest_it)
{
	expand_macro_iterator_p it = malloc(sizeof(struct expand_macro_iterator_s));
	it->base.next = expand_macro_iterator_next;
	it->param_tokens = NULL;
	it->tokens = macro->tokens;
	it->_rest_it = rest_it;
	it->_macro = macro;
	for (int arg_nr = 0; arg_nr < macro->nr_args; arg_nr++)
		it->args[arg_nr] = args[arg_nr];
	return expand_macro_iterator_next(&it->base, FALSE);
}

// The expand iterator

// This iterator takes care of expanding all defined symbols and macros.
// For the expansion of these the expand macro iterator is used.

typedef struct expand_iterator_s* expand_iterator_p;
struct expand_iterator_s
{
	token_iterator_t base;
	token_iterator_p _source_it;
};

token_iterator_p expand_iterator_next(token_iterator_p token_it, bool dummy)
{
	expand_iterator_p it = (expand_iterator_p)token_it;
	token_iterator_p source_it = it->_source_it;

	source_it = source_it->next(source_it, dummy);
	
	for (bool go = TRUE; go;)
	{
		go = FALSE;
		if (source_it->kind == 'i')
		{
			env_p macro = get_env(source_it->token, FALSE);
			if (macro != NULL)
			{
				go = TRUE;
				if (opt_trace_parser)
					printf("Expand token %s %d: ", source_it->token, macro->nr_args);
				tokens_p args[10];
				int nr_args = 0;
				if (macro->nr_args > 0)
				{
					source_it = source_it->next(source_it, dummy);
					if (source_it->kind != '(')
					{
						printf("ERROR: No arguments for %s when parameters (%d) are expected\n", macro->name, macro->nr_args);
						continue;
					}
					do
					{
						source_it = source_it->next(source_it, dummy);
						args[nr_args] = NULL;
						tokens_p *ref_tokens = &args[nr_args];
						nr_args++;
						char stack[20];
						int stack_depth = 0;
						while (stack_depth > 0 || (source_it->kind != 0 && source_it->kind != ',' && source_it->kind != ')'))
						{
							*ref_tokens = new_token_from_it(source_it);
							ref_tokens = &(*ref_tokens)->next;
							if (source_it->kind == '(')
								stack[stack_depth++] = ')';
							else if (source_it->kind == '{')
								stack[stack_depth++] = '}';
							else if (source_it->kind == '[')
								stack[stack_depth++] = ']';
							else if (stack_depth > 0 && source_it->kind == stack[stack_depth - 1])
								stack_depth--;
							source_it = source_it->next(source_it, dummy);
						}
					}
					while (source_it->kind == ',');
				}
				if (macro->nr_args != nr_args)
				{
					printf("ERROR: Number arguments (%d) for %s does not match parameters (%d)\n", nr_args, macro->name, macro->nr_args);
					source_it = source_it->next(source_it, dummy);
				}
				else if (macro->tokens == NULL)
					source_it = source_it->next(source_it, dummy);
				else						
					source_it = new_exapnd_macro_iterator(macro, args, source_it);
			}
		}
		it->_source_it = source_it;
	}
	
	token_it->kind = source_it->kind;
	token_it->token = source_it->token;
	token_it->length = source_it->length;
	token_it->filename = source_it->filename;
	token_it->line = source_it->line;
	token_it->column = source_it->column;
			
	return token_it;
}

expand_iterator_p new_expand_iterator(token_iterator_p source_it)
{
	expand_iterator_p it = malloc(sizeof(struct expand_iterator_s));
	it->_source_it = source_it;
	it->base.next = expand_iterator_next;
	return it;
}

token_iterator_p token_it = NULL;

// Output the C-preprocessor output.

void output_preprocessor(const char *filename)
{
	FILE *fout = fopen(filename, "w");
	if (fout == NULL)
		return;
	token_it = token_it->next(token_it, TRUE);
	
	char *prev_file = 0;
	int prev_line = 0;
	while (token_it->kind != 0)
	{
		if (token_it->filename != prev_file || token_it->line != prev_line)
		{
			fprintf(fout, "\n%s: %d", token_it->filename, token_it->line);
			for (int i = 0; i < token_it->column; i++)
				fputc(' ', fout);
			prev_file = token_it->filename;
			prev_line = token_it->line;
		}
		fputc(' ', fout);
		if (token_it->kind == 'i')
		{
			fputs(token_it->token, fout);
		}
		else if (token_it->kind == '"' || token_it->kind == '\'')
		{
			char strsep = token_it->kind;
			fputc(strsep, fout);
			for (int i = 0; i < token_it->length ; i = i + 1)
			{
				char ch = token_it->token[i];
				if (ch == '\n')
					fputs("\\n", fout);
				else if (ch == '\r')
					fputs("\\r", fout);
				else if (ch == '\t')
					fputs("\\t", fout);
				else if (ch == '\0')
					fputs("\\0", fout);
				else if (' ' <= ch && ch < 127)
					fputc(ch, fout);
				else
				{
					fputs("\\", fout);
					fputc(ch, fout);
				}
			}
			fputc(strsep, fout);
		}
		else if (token_it->kind == '0')
		{
			fputs(token_it->token, fout);
		}
		else if (token_it->kind < 127)
		{
			fputc(token_it->kind, fout);
		}
		else if (token_it->kind >= TK_KEYWORD)
			fputs(token_it->token, fout);
		else if (token_it->kind == TK_D_HASH) 	fputs("##", fout);
		else if (token_it->kind == TK_EQ) 		fputs("==", fout);
		else if (token_it->kind == TK_NE)		fputs("!=", fout);
		else if (token_it->kind == TK_LE)		fputs("<=", fout);
		else if (token_it->kind == TK_GE)		fputs(">=", fout);
		else if (token_it->kind == TK_INC)		fputs("++", fout);
		else if (token_it->kind == TK_DEC)		fputs("--", fout);
		else if (token_it->kind == TK_ARROW)	fputs("->", fout);
		else if (token_it->kind == TK_MUL_ASS)	fputs("*=", fout);
		else if (token_it->kind == TK_DIV_ASS)	fputs("/=", fout);
		else if (token_it->kind == TK_MOD_ASS)	fputs("%%=", fout);
		else if (token_it->kind == TK_ADD_ASS)	fputs("+=", fout);
		else if (token_it->kind == TK_SUB_ASS)	fputs("-=", fout);
		else if (token_it->kind == TK_SHL_ASS)	fputs("<<=", fout);
		else if (token_it->kind == TK_SHR_ASS)	fputs(">>=", fout);
		else if (token_it->kind == TK_XOR_ASS)	fputs("^=", fout);
		else if (token_it->kind == TK_BAND_ASS)	fputs("&=", fout);
		else if (token_it->kind == TK_BOR_ASS)	fputs("|=", fout);
		else if (token_it->kind == TK_SHL)		fputs("<<", fout);
		else if (token_it->kind == TK_SHR)		fputs(">>", fout);
		else if (token_it->kind == TK_AND)		fputs("&&", fout);
		else if (token_it->kind == TK_OR)		fputs("||", fout);
		else fputs("????", fout);

		token_it = token_it->next(token_it, TRUE);
	}

	fprintf(fout, "\n\nDone\n");	
	fclose(fout);
}

// Types

#define BT_UNSIGNED_INT (1)
#define BT_SIGNED_INT (1 | 2)
#define BT_TYPE(X)  ((X) << 2)
#define BT_GET_TYPE(X) ((X) >> 2)

typedef enum
{
	BT_VOID = 0,
	BT_S8  = BT_SIGNED_INT   | BT_TYPE(1),
	BT_U8  = BT_UNSIGNED_INT | BT_TYPE(1),
	BT_S16 = BT_SIGNED_INT   | BT_TYPE(2),
	BT_U16 = BT_UNSIGNED_INT | BT_TYPE(2),
	BT_S32 = BT_SIGNED_INT   | BT_TYPE(3),
	BT_U32 = BT_UNSIGNED_INT | BT_TYPE(3),
	BT_S64 = BT_SIGNED_INT   | BT_TYPE(4),
	BT_U64 = BT_UNSIGNED_INT | BT_TYPE(4),
	BT_F   = BT_SIGNED_INT   | BT_TYPE(5),  // Treat float and double as integer
	BT_DF  = BT_SIGNED_INT   | BT_TYPE(6),
	BT_JMP_BUF =               BT_TYPE(7),
	BT_FILE    =               BT_TYPE(8),
	BT_TIME_T  =               BT_TYPE(9),
	BT_BOOL    =               BT_TYPE(10),
} base_type_e;

typedef enum
{
	TYPE_KIND_BASE,
	TYPE_KIND_STRUCT,
	TYPE_KIND_UNION,
	TYPE_KIND_POINTER,
	TYPE_KIND_ARRAY,
	TYPE_KIND_FUNCTION,
	TYPE_KIND_ENUM,
} type_kind_e;

typedef struct decl_s *decl_p;

typedef struct type_s *type_p;
struct type_s
{
	type_kind_e kind;
	base_type_e base_type;
	int nr_members;  // TYPE_KIND_STRUCT or TYPE_KIND_UNION
	type_p *members;
	int nr_decls;    // TYPE_KIND_FUNCTION
	decl_p *decls;
	int nr_elems;    // TYPE_KIND_ARRAY
	decl_p typedef_decl;
};

type_p new_type(type_kind_e kind, int nr_members)
{
	type_p type = (type_p)malloc(sizeof(struct type_s));
	type->kind = kind;
	type->base_type = 0;
	type->nr_members = nr_members;
	if (nr_members > 0)
	{
		type->members = (type_p*)malloc(nr_members * sizeof(type_p));
		for (int i = 0; i < nr_members; i++)
			type->members[i] = NULL;
	}
	else
		type->members = NULL;
	type->nr_decls = 0;
	type->decls = NULL;
	type->nr_elems = 0;
	type->typedef_decl = NULL;
	return type;
}

type_p new_ptr_type(type_p base_type)
{
	type_p ptr_type = new_type(TYPE_KIND_POINTER, 1);
	ptr_type->members[0] = base_type;
	return ptr_type;
}

bool type_is_integer(type_p type)
{
	return type != NULL && type->kind == TYPE_KIND_BASE && ((int)type->base_type & 1) == 1;
}

bool type_is_signed_integer(type_p type)
{
	return type != NULL && type->kind == TYPE_KIND_BASE && ((int)type->base_type & 3) == 3;
}

bool type_is_pointer(type_p type)
{
	return type != NULL && (type->kind == TYPE_KIND_POINTER || type->kind == TYPE_KIND_ARRAY);
}

void type_set_decls(type_p type, int nr_decls, decl_p* decls)
{
	if (nr_decls >= type->nr_decls)
	{
		type->nr_decls = nr_decls;
		type->decls = (decl_p*)malloc(nr_decls * sizeof(decl_p));
	}
	for (int i = 0; i < nr_decls; i++)
		type->decls[i] = decls[i];
}

type_p base_type_void = NULL;
type_p base_type_S8 = NULL;
type_p base_type_U8 = NULL;
type_p base_type_S16 = NULL;
type_p base_type_U16 = NULL;
type_p base_type_S32 = NULL;
type_p base_type_U32 = NULL;
type_p base_type_S64 = NULL;
type_p base_type_U64 = NULL;
type_p base_type_float = NULL;
type_p base_type_double = NULL;
type_p base_type_jmp_buf = NULL;
type_p base_type_file = NULL;
type_p base_type_time_t = NULL;
type_p base_type_bool = NULL;
type_p type_char_ptr = NULL;

type_p new_base_type(base_type_e base)
{
	type_p type = new_type(TYPE_KIND_BASE, 0);
	type->base_type = base;
	return type;
}

void define_base_types(void)
{
	base_type_void = new_base_type(BT_VOID);
	base_type_S8 = new_base_type(BT_S8);
	base_type_U8 = new_base_type(BT_U8);
	base_type_S16 = new_base_type(BT_S16);
	base_type_U16 = new_base_type(BT_U16);
	base_type_S32 = new_base_type(BT_S32);
	base_type_U32 = new_base_type(BT_U32);
	base_type_S64 = new_base_type(BT_S64);
	base_type_U64 = new_base_type(BT_U64);
	base_type_float = new_base_type(BT_F);
	base_type_double = new_base_type(BT_DF);
	base_type_jmp_buf = new_base_type(BT_JMP_BUF);
	base_type_file = new_base_type(BT_FILE);
	base_type_time_t = new_base_type(BT_TIME_T);
	base_type_bool = new_base_type(BT_BOOL);
	type_char_ptr = new_ptr_type(base_type_S8);
}

type_p base_signed_type(type_p int_type)
{
	if (int_type == base_type_U8)
		return base_type_S8;
	if (int_type == base_type_U16)
		return base_type_S16;
	if (int_type == base_type_U32)
		return base_type_S32;
	if (int_type == base_type_U64)
		return base_type_S64;
	return int_type;
}

// Expressions

#define LOCATION_IN_EXPR 

// The expression kind uses the token type or one of the following:

#define OPER_POST_INC    2000
#define OPER_POST_DEC    2001
#define OPER_PRE_INC     2002
#define OPER_PRE_DEC     2003
#define OPER_PLUS        2004
#define OPER_MIN         2005
#define OPER_STAR        2006
#define OPER_ADDR        2007

typedef struct expr_s *expr_p;
struct expr_s
{
	int kind;
	int int_val;
	char *str_val;
	type_p type;
#ifdef LOCATION_IN_EXPR
	const char *filename;
	int line;
	int column;
#endif
	int nr_children;
	expr_p children[0];
};

char token_it_pos_buffer[101];

const char *token_it_pos(void)
{
	snprintf(token_it_pos_buffer, 100, "%s: %d.%d", token_it->filename, token_it->line, token_it->column);
	token_it_pos_buffer[100] = '\0';
	return token_it_pos_buffer;
}

typedef struct location_s location_t, *location_p;
struct location_s
{
	const char *filename;
	int line;
	int column;
};

void save_location(location_p location, token_iterator_p token_it)
{
	location->filename = token_it->filename;
	location->line = token_it->line;
	location->column = token_it->column;
}

#ifdef LOCATION_IN_EXPR
location_t location_for_expr;

void store_pos_for_expr(void)
{
	save_location(&location_for_expr, token_it);
}

static char expr_pos_buffer[101];

const char *expr_pos(expr_p expr)
{
	if (expr->kind > ' ' && expr->kind < 127)
		snprintf(expr_pos_buffer, 100, "%s: %d.%d expr '%c ", expr->filename, expr->line, expr->column, expr->kind);
	else
		snprintf(expr_pos_buffer, 100, "%s: %d.%d expr %d ", expr->filename, expr->line, expr->column, expr->kind);
	expr_pos_buffer[100] = '\0';
	return expr_pos_buffer;
}
#else
void store_pos_for_expr(void) {}
const char *expr_pos(expr_p expr) {
	static char buffer[101];
	if (expr->kind > ' ' && expr->kind < 127)
		snprintf(buffer, 100, "%sexpr '%c ", token_pos(), expr->kind);
	else
		snprintf(buffer, 100, "%s expr %d ", token_pos(), expr->kind);
	buffer[100] = '\0';
	return buffer;
}
#endif

expr_p new_expr(int kind, int nr_children)
{
	expr_p expr = (expr_p)malloc(sizeof(struct expr_s) + nr_children * sizeof(expr_p));
	expr->kind = kind;
	expr->int_val = 0;
	expr->str_val = NULL;
	expr->nr_children = nr_children;
	for (int i = 0; i < nr_children; i++)
		expr->children[i] = NULL;
	expr->type = NULL;
#ifdef LOCATION_IN_EXPR
	expr->filename = location_for_expr.filename;
	expr->line = location_for_expr.line;
	expr->column = location_for_expr.column;
#endif
	return expr;
}

expr_p new_expr_int_value(int value)
{
	expr_p expr = new_expr('0', 0);
	expr->int_val = value;
	expr->type = base_type_U32;
	return expr;
}

void expr_print(FILE *f, expr_p expr)
{
	if (expr == NULL)
	{
		fprintf(f, "NULL");
		return;
	}
	if (' ' < expr->kind && expr->kind < 127)
		fprintf(f, "%c(", expr->kind);
	else
		fprintf(f, "%d(", expr->kind);
	for (int i = 0; i < expr->nr_children; i++)
	{
		if (i > 0)
			fprintf(f, ", ");
		expr_print(f, expr->children[i]);
	}
	fprintf(f, ")");
}

// Declarations

typedef enum {
	DK_IDENT,
	DK_STRUCT,
	DK_UNION,
	DK_ENUM,
} decl_kind_e;

typedef enum {
	ST_NONE,
	ST_STATIC,
	ST_TYPEDEF,
	ST_CONST,
} storage_type_e;

struct decl_s
{
	decl_kind_e kind;
	char *name;
	type_p type;
	storage_type_e storage_type;
	expr_p value;
	int su_nr;
	decl_p prev;
};

bool trace_decls = FALSE;

decl_p cur_ident_decls = NULL;
decl_p cur_decls = NULL;

decl_p add_decl(decl_kind_e kind, const char *name, type_p type)
{
	if (trace_decls)
	{
		printf("add_decl '%s' %d", name, kind);
		if (type != NULL)
			printf(" Type %d", type->kind);
		printf("\n");
	}
	decl_p decl = (decl_p)malloc(sizeof(struct decl_s));
	decl->kind = kind;
	decl->name = copystr(name);
	decl->type = type;
	decl->storage_type = ST_NONE;
	decl->value = NULL;
	decl->su_nr = 0;
	if (kind == DK_IDENT)
	{
		decl->prev = cur_ident_decls;
		cur_ident_decls = decl;
	}
	else
	{
		decl->prev = cur_decls;
		cur_decls = decl;
	}
	return decl;
}

void remove_decl(decl_p decl)
{
	for (decl_p *ref_decl = &cur_decls; *ref_decl != 0; ref_decl = &(*ref_decl)->prev)
		if (*ref_decl == decl)
		{
			*ref_decl = (*ref_decl)->prev;
			break;
		}
}

void add_decl_clone(decl_p decl)
{
	decl_p decl_clone = add_decl(decl->kind, decl->name, decl->type);
	decl_clone->storage_type = decl->storage_type;
}

decl_p find_decl(decl_kind_e kind, char *name)
{
	for (decl_p decls = kind == DK_IDENT ? cur_ident_decls : cur_decls; decls != NULL; decls = decls->prev)
		if (strcmp(decls->name, name) == 0)
			return decls;
	return NULL;
}

decl_p find_or_add_decl(decl_kind_e kind, char *name)
{
	decl_p decl = find_decl(kind, name);
	if (decl != NULL)
		return decl;
	return add_decl(kind, name, NULL);
}

// Labels

typedef struct label_s *label_p;
typedef struct statement_s *statement_p;
struct label_s
{
	char *name;
	statement_p statement;
	label_p next;
};

label_p cur_labels = NULL;

label_p find_label(const char *name)
{
	for (label_p label = cur_labels; label != NULL; label = label->next)
		if (strcmp(label->name, name) == 0)
			return label;
	return NULL;
}

label_p find_or_add_label(const char *name)
{
	label_p label = find_label(name);
	if (label != NULL)
		return label;
	label = (label_p)malloc(sizeof(struct label_s));
	label->name = copystr(name);
	label->next = cur_labels;
	label->statement = 0;
	cur_labels = label;
	return label;
}

// Eval expression

int expr_eval(expr_p expr)
{
	switch (expr->kind)
	{
		case '0': return expr->int_val;
		case '+': return expr_eval(expr->children[0]) + expr_eval(expr->children[1]);
		case '-': return expr_eval(expr->children[0]) - expr_eval(expr->children[1]);
		case '/': return expr_eval(expr->children[0]) / expr_eval(expr->children[1]);
		case '|': return expr_eval(expr->children[0]) | expr_eval(expr->children[1]);
		case OPER_MIN: return -expr_eval(expr->children[0]);
		case TK_SHL: return expr_eval(expr->children[0]) << expr_eval(expr->children[1]);
		case 'i':
		{
			decl_p expr_decl = find_decl(DK_IDENT, expr->str_val);
			if (   expr_decl->storage_type == ST_CONST
				&& type_is_integer(expr_decl->value->type))
				return expr_decl->value->int_val;
			printf("Error: storage type of %s = %d\n", expr->str_val, expr_decl->storage_type);
			break;
		}
	}
	printf("%s Error: expr_eval\n", expr_pos(expr));
	exit(1);
	return 0;
}

// Type functions

type_p expr_type_member(expr_p expr, int nr)
{
	if (expr->type != NULL && nr < expr->type->nr_members)
		return expr->type->members[nr];
	if (expr->kind == 'i')
		printf("%s: Error identifier '%s': ", expr_pos(expr), expr->str_val);
	else
		printf("%s: Error expression: ", expr_pos(expr));
	if (expr->type == NULL)
		printf("Has no type\n");
	else
		printf("Type has no member %d (%d)\n", nr, expr->type->nr_members);
	return NULL;
}

decl_p type_decl(type_p type, const char *name)
{
	for (int i = 0; i < type->nr_decls; i++)
		if (type->decls[i] != NULL && strcmp(type->decls[i]->name, name) == 0)
			return type->decls[i];
	fprintf(stderr, "%s: Error Type %p has no field %s\n", token_it_pos(), type, name);
	return NULL;
}

decl_p expr_type_decl(expr_p expr, const char *name)
{
	if (expr->type != NULL)
	{
		decl_p decl = type_decl(expr->type, name);
		if (decl != NULL)
			return decl;
	}
	printf("%s: Error expression: ", expr_pos(expr));
	if (expr->type == NULL)
		printf("Has no type\n");
	else
	{
		printf("Type %p has no field %s\n", expr->type, name);
	}
	return NULL;
}

// Statements

struct statement_s
{
	int kind;
	expr_p expr;
	decl_p decl;
	label_p label;
	label_p goto_label;
	char *comment;
#ifdef LOCATION_IN_EXPR
	const char *filename;
	int line;
	int column;
#endif
	int nr_children;
	statement_p children[0];
};

statement_p cur_statements[1000];
int cur_nr_statements = 0;

statement_p add_statement(int kind, label_p label, location_p location, char *comment, int from)
{
	int nr_children = 0;
	if (from != -1 && from < cur_nr_statements)
	{
		nr_children = cur_nr_statements - from;
		cur_nr_statements -= nr_children;
	}
	statement_p statement = (statement_p)malloc(sizeof(struct statement_s) + nr_children * sizeof(statement_p));
	statement->kind = kind;
	statement->expr = 0;
	statement->decl = 0;
	statement->label = label;
	if (label != NULL)
		label->statement = statement;
	statement->goto_label = 0;
	statement->comment = comment;
#ifdef LOCATION_IN_EXPR
	if (location == NULL)
		location = &location_for_expr;
	statement->filename = location->filename;
	statement->line = location->line;
	statement->column = location->column;
#endif
	statement->nr_children = nr_children;
	for (int i = 0; i < nr_children; i++)
		statement->children[i] = cur_statements[cur_nr_statements + i];
	cur_statements[cur_nr_statements++] = statement;
	return statement;
}

void print_statement(statement_p statement, int indent)
{
	printf("%*.*s", indent, indent, "");
#ifdef LOCATION_IN_EXPR
	printf("%s:%d,%d ", statement->filename, statement->line, statement->column);
#endif
	if (statement == NULL)
	{
		printf("NULL");
		return;
	}
	if (statement->label != NULL)
		printf("%s: ", statement->label->name == NULL ? "NULL" : statement->label->name);
	if (' ' < statement->kind && statement->kind < 127)
		printf("%c", statement->kind);
	else switch(statement->kind)
	{
		case TK_IF:      printf("if "); break;
		case TK_SWITCH:  printf("switch "); break;
		case TK_CASE:    printf("case "); break;
		case TK_DEFAULT: printf("default "); break;
		case TK_BREAK:   printf("break "); break;
		case TK_RETURN:  printf("return "); break;
		default: printf("#%d", statement->kind);
	}
		
	if (statement->decl != NULL)
		printf(" decl %s", statement->decl->name);
	if (statement->expr != NULL)
	{
		printf(" ");
		expr_print(stdout, statement->expr);
	}
	if (statement->goto_label != NULL)
		printf(" goto %s", statement->goto_label->name);
	if (statement->comment != NULL)
		printf(" '%s'", statement->comment);
	printf("\n");
	for (int i = 0; i < statement->nr_children; i++)
		print_statement(statement->children[i], indent + 4);
}

// Parse functions

void next_token(void)
{
	store_pos_for_expr();
	token_it = token_it->next(token_it, FALSE);
}

bool accept_term(int kind)
{
	if (token_it->kind == kind)
	{
		next_token();
		return TRUE;
	}
	return FALSE;
}

#define FAIL_FALSE { if (opt_trace_parser) fprintf(stderr, "Fail in %s at %d\n", __func__, __LINE__); return FALSE; }
#define FAIL_NULL  { if (opt_trace_parser) fprintf(stderr, "Fail in %s at %d\n", __func__, __LINE__); return NULL; }

expr_p parse_expr(void);
type_p parse_type_specifier(storage_type_e *ref_storage_type);
expr_p parse_unary_expr(void);

#define MAX_CONST_STRLEN 8000

static char strbuf[MAX_CONST_STRLEN];

/*
	primary_expr 
		: identifier
		| integer
		| char
		| string
		| '(' expr ')'
		.
*/

expr_p parse_primary_expr(void)
{
	if (token_it->kind == 'i')
	{
		decl_p decl = find_decl(DK_IDENT, token_it->token);
		if (decl != NULL && decl->storage_type != ST_TYPEDEF)
		{
			store_pos_for_expr();
			expr_p expr = new_expr('i', 0);
			expr->str_val = copystr(token_it->token);
			expr->type = decl->type;
			if (decl->type == NULL)
				printf("Identifier %s has no type\n", expr->str_val);
			next_token();
			return expr;
		}
	}
	if (token_it->kind == '0')
	{
		store_pos_for_expr();
		expr_p expr = new_expr_int_value(token_it_int_value(token_it));
		int nr_L = 0;
		for (char *s = token_it->token; *s != '\0'; s++)
			if (*s == 'L')
				nr_L++;
		if (nr_L > 1 && expr->int_val > 0xFFFFFFFF)
			fprintf(stderr, "%s Warning: long long const not supported\n", token_it_pos());
		next_token();
		return expr;
	}
	if (token_it->kind == '\'')
	{
		store_pos_for_expr();
		expr_p expr = new_expr_int_value(token_it->token[0]);
		expr->type = base_type_S8;
		next_token();
		return expr;
	}
	if (token_it->kind == '"')
	{
		store_pos_for_expr();
		int len = 0;
		while (token_it->kind == '"')
		{
			if (len + token_it->length < MAX_CONST_STRLEN - 1)
				memcpy(strbuf + len, token_it->token, token_it->length);
			len += token_it->length;
			next_token();
		}
		if (len >= MAX_CONST_STRLEN - 1)
		{
			printf("MAX_CONST_STRLEN < %d\n", len + 1);
			exit(1);
		}
		strbuf[len] = '\0';
		expr_p expr = new_expr('"', 0);
		expr->str_val = (char*)malloc(len + 1);
		expr->type = type_char_ptr;
		memcpy(expr->str_val, strbuf, len + 1);
		expr->int_val = len;
		return expr;
	}
	if (accept_term('('))
	{
		type_p type = parse_type_specifier(NULL);
		if (type != NULL)
		{
			if (opt_trace_parser)
				printf("Cast expr\n");
			while (accept_term('*'))
			{
				type = new_ptr_type(type);
			}
			if (!accept_term(')'))
				FAIL_NULL
			expr_p subj_expr = parse_unary_expr();
			if (subj_expr == NULL)
				FAIL_NULL;
			expr_p expr = new_expr('c', 1);
			expr->children[0] = subj_expr;
			expr->type = type;
			return expr;
		}
		expr_p expr = parse_expr(); 
		if (expr == NULL)
			FAIL_NULL
		if (!accept_term(')'))
			FAIL_NULL
		return expr;
	}
	FAIL_NULL
}

expr_p parse_assignment_expr(void);

/*
	postfix_expr
		: primary_expr
		| postfix_expr '[' expr ']'
		| postfix_expr '(' assignment_expr LIST ')'
		| postfix_expr '.' ident
		| postfix_expr '->' ident
		| postfix_expr '++'
		| postfix_expr '--'
		.
*/

expr_p parse_postfix_expr(void)
{
	expr_p expr = parse_primary_expr();
	if (expr == NULL)
		FAIL_NULL
	for (;;)
	{
		expr_p subj_expr = expr;
		if (accept_term('['))
		{
			expr_p index_expr = parse_expr();
			if (index_expr == NULL)
				FAIL_NULL
			if (!accept_term(']'))
				FAIL_NULL
			expr = new_expr('[', 2);
			expr->children[0] = subj_expr;
			expr->children[1] = index_expr;
			expr->type = expr_type_member(subj_expr, 0);
		}
		else if (accept_term('('))
		{
			int nr_children = 1;
			expr_p children[20];
			children[0] = expr;
			if (token_it->kind != ')')
			{
				do
				{
					expr_p child = parse_assignment_expr();
					if (child == NULL)
						FAIL_NULL
					children[nr_children++] = child;
				} while (accept_term(','));
			}
			if (!accept_term(')'))
				FAIL_NULL
			expr = new_expr('(', nr_children);
			for (int i = 0; i < nr_children; i++)
				expr->children[i] = children[i];
			expr->type = expr_type_member(subj_expr, 0);
		}
		else if (accept_term('.'))
		{
			if (token_it->kind != 'i')
				FAIL_NULL
			expr = new_expr('.', 1);
			expr->str_val = copystr(token_it->token);
			expr->children[0] = subj_expr;
			decl_p mem_decl = expr_type_decl(subj_expr, token_it->token);
			if (mem_decl != NULL)
			{
				expr->type = mem_decl->type;
				expr->int_val = mem_decl->su_nr;
			}
			next_token();
		}
		else if (accept_term(TK_ARROW))
		{
			if (token_it->kind != 'i')
				FAIL_NULL
			expr = new_expr(TK_ARROW, 1);
			expr->str_val = copystr(token_it->token);
			expr->children[0] = subj_expr;
			if (subj_expr->type != NULL && subj_expr->type->kind == TYPE_KIND_POINTER)
			{
				decl_p mem_decl = type_decl(subj_expr->type->members[0], token_it->token);
				if (mem_decl != NULL)
				{
					expr->type = mem_decl->type;
					expr->int_val = mem_decl->su_nr;
				}
			}
			else
				printf("-> does not have pointer type\n");
			next_token();
		}
		else if (accept_term(TK_INC))
		{
			expr = new_expr(OPER_POST_INC, 1);
			expr->children[0] = subj_expr;
			expr->type = subj_expr->type;
		}
		else if (accept_term(TK_DEC))
		{
			expr = new_expr(OPER_POST_DEC, 1);
			expr->children[0] = subj_expr;
			expr->type = subj_expr->type;
		}
		else
			break;
	}
	if (expr->type == NULL) printf("parse_postfix_expr has no type\n");
	return expr;
}

type_p parse_sizeof_type(void);

/*
	unary_expr
		: '++' unary_expr
		| '--' unary_expr
		| '&' cast_expr
		| '*' cast_expr
		| '+' cast_expr
		| '-' cast_expr
		| '~' cast_expr
		| '!' cast_expr
		| 'sizeof' '(' sizeof_type ')'
		| 'sizeof' unary_expr
		| postfix_expr
		.
*/

expr_p parse_unary_expr(void)
{
	if (accept_term(TK_INC))
	{
		expr_p expr = parse_unary_expr();
		if (expr == NULL)
			FAIL_NULL
		expr_p pre_oper_expr = new_expr(OPER_PRE_INC, 1);
		pre_oper_expr->children[0] = expr;
		pre_oper_expr->type = expr->type;
		return pre_oper_expr;
	}
	if (accept_term(TK_DEC))
	{
		expr_p expr = parse_unary_expr();
		if (expr == NULL)
			FAIL_NULL
		expr_p pre_oper_expr = new_expr(OPER_PRE_DEC, 1);
		pre_oper_expr->children[0] = expr;
		pre_oper_expr->type = expr->type;
		return pre_oper_expr;
	}
	if (accept_term('&'))
	{
		expr_p expr = parse_unary_expr();
		if (expr == NULL)
			FAIL_NULL
		expr_p pre_oper_expr = new_expr(OPER_ADDR, 1);
		pre_oper_expr->children[0] = expr;
		pre_oper_expr->type = new_ptr_type(expr->type);
		return pre_oper_expr;
	}
	if (accept_term('*'))
	{
		expr_p expr = parse_unary_expr();
		if (expr == NULL)
			FAIL_NULL
		expr_p pre_oper_expr = new_expr(OPER_STAR, 1);
		pre_oper_expr->children[0] = expr;
		pre_oper_expr->type = expr_type_member(expr, 0);
		return pre_oper_expr;
	}
	if (accept_term('+'))
	{
		expr_p expr = parse_unary_expr();
		if (expr == NULL)
			FAIL_NULL
		expr_p pre_oper_expr = new_expr(OPER_PLUS, 1);
		pre_oper_expr->children[0] = expr;
		pre_oper_expr->type = expr->type;
		return pre_oper_expr;
	}
	if (accept_term('-'))
	{
		expr_p expr = parse_unary_expr();
		if (expr == NULL)
			FAIL_NULL
		if (expr->kind == '0')
		{
			expr_p neg_num = new_expr_int_value(-expr->int_val);
			neg_num->type = base_signed_type(expr->type);
			return neg_num; 
		}
		expr_p pre_oper_expr = new_expr(OPER_MIN, 1);
		pre_oper_expr->children[0] = expr;
		pre_oper_expr->type = expr->type;
		return pre_oper_expr;
	}
	if (accept_term('~'))
	{
		expr_p expr = parse_unary_expr();
		if (expr == NULL)
			FAIL_NULL
		expr_p pre_oper_expr = new_expr('~', 1);
		pre_oper_expr->children[0] = expr;
		pre_oper_expr->type = expr->type;
		return pre_oper_expr;
	}
	if (accept_term('!'))
	{
		expr_p expr = parse_unary_expr();
		if (expr == NULL)
			FAIL_NULL
		expr_p pre_oper_expr = new_expr('!', 1);
		pre_oper_expr->children[0] = expr;
		pre_oper_expr->type = base_type_bool;
		return pre_oper_expr;
	}
	if (accept_term(TK_SIZEOF))
	{
		type_p type = NULL;
		if (accept_term('('))
		{
			type = parse_sizeof_type();
			if (type == NULL)
			{
				expr_p sizeof_expr = parse_expr();
				if (sizeof_expr == NULL)
					FAIL_NULL
				type = sizeof_expr->type;
			}
			if (!accept_term(')'))
				FAIL_NULL
		}
		else
		{
			expr_p sizeof_expr = parse_unary_expr();
			if (sizeof_expr == NULL)
				FAIL_NULL
			type = sizeof_expr->type;
		}
		if (type == NULL)
			printf("%s Error: sizeof expression has not size\n", token_it_pos());
		return new_expr_int_value(0);
	}
	
	return parse_postfix_expr();
}

/*
	sizeof_type
		: 'char'
		| 'int'
		| 'unsigned' ('int') OPT
		| 'double'
		| 'void' ('*') OPT
		| 'struct' ident
		| ident
		| sizeof_type '*'
		.
*/

type_p parse_sizeof_type(void)
{
	type_p type = NULL;
	if (accept_term(TK_CHAR))
		type = base_type_S8;
	else if (accept_term(TK_INT))
		type = base_type_S32;
	else if (accept_term(TK_UNSIGNED))
	{
		if (accept_term(TK_INT))
			type = base_type_U32;
		type = base_type_U32;
	}
	else if (accept_term(TK_DOUBLE))
		type = base_type_double;
	else if (accept_term(TK_VOID))
	{
		if (accept_term('*'))
			type = type_char_ptr; // Missuse char pointer here
	}
	else if (accept_term(TK_STRUCT))
	{
		if (token_it->kind == 'i')
		{
			decl_p decl = find_decl(DK_STRUCT, token_it->token);
			if (decl != NULL && decl->type != NULL)
			{
				type = decl->type;
				next_token();
			}
		}
	}
	else if (token_it->kind == 'i')
	{
		decl_p decl = find_decl(DK_IDENT, token_it->token);
		if (decl != NULL && decl->type != NULL && decl->storage_type == ST_TYPEDEF)
		{
			type = decl->type;
			next_token();
		}
	}
	if (type == NULL)
		return NULL;
	while (accept_term('*'))
		type = type_char_ptr; // Missuse char pointer here
	return type;
}

/*
	cast_expr
		: '(' abstract_declaration ')' cast_expr
		| unary_expr
		.
*/

void expr_dioper_set_type(expr_p expr)
{
	type_p type_lhs = expr->children[0]->type;
	type_p type_rhs = expr->children[1]->type;
	if (type_is_integer(type_lhs) && type_is_integer(type_rhs))
	{
		bool signed_int = (((int)type_lhs->base_type | (int)type_rhs->base_type) & 2) == 2;
		expr->type = signed_int ? base_type_S32 : base_type_U32;
	}
	else if (type_is_pointer(type_lhs) && type_is_integer(type_rhs))
		expr->type = type_lhs;
	else if (type_is_integer(type_lhs) && type_is_pointer(type_rhs))
		expr->type = type_rhs;
	else if (expr->kind == '-' && type_is_pointer(type_lhs) && type_is_pointer(type_rhs))
		expr->type = base_type_S32;
	if (expr->type == NULL)
	{
		printf("%s: Error dioper expression: Has no type.", expr_pos(expr));
		if (!type_is_integer(type_lhs)) printf(" LHS not an integer %d", type_lhs == NULL ? -1 : type_lhs->kind);
		if (!type_is_integer(type_rhs)) printf(" RHS not an integer %d", type_rhs == NULL ? -1 : type_rhs->kind);
		printf("\n");
	}
}

/*
	l_expr1 : cast_expr (('*' | '/' | '%') cast_expr) SEQ OPT .
*/

expr_p parse_expr1(void)
{
	expr_p expr = parse_unary_expr();
	for (;;)
	{
		int kind = token_it->kind;
		if (accept_term('*') || accept_term('/') || accept_term('%'))
		{
			expr_p lhs = expr;
			expr_p rhs = parse_unary_expr();
			if (rhs == NULL)
				FAIL_NULL
			expr = new_expr(kind, 2);
			expr->children[0] = lhs;
			expr->children[1] = rhs;
			expr_dioper_set_type(expr);
		}
		else
			break;
	}
	
	if (expr != NULL && expr->type == NULL) printf("parse_expr1 has no type\n");
	return expr;
}

/*
	l_expr2 : l_expr1 (('+' | '-') l_expr1) SEQ OPT .
*/

expr_p parse_expr2(void)
{
	expr_p expr = parse_expr1();
	for (;;)
	{
		int kind = token_it->kind;
		if (accept_term('+') || accept_term('-'))
		{
			expr_p lhs = expr;
			expr_p rhs = parse_expr1();
			if (rhs == NULL)
				FAIL_NULL
			expr = new_expr(kind, 2);
			expr->children[0] = lhs;
			expr->children[1] = rhs;
			expr_dioper_set_type(expr);
		}
		else
			break;
	}
	
	return expr;
}

/*
	l_expr3 : l_expr2 (('<<' | '>>') l_expr2) SEQ OPT .
*/

expr_p parse_expr3(void)
{
	expr_p expr = parse_expr2();
	for (;;)
	{
		int kind = token_it->kind;
		if (accept_term(TK_SHL) || accept_term(TK_SHR))
		{
			expr_p lhs = expr;
			expr_p rhs = parse_expr2();
			if (rhs == NULL)
				FAIL_NULL
			expr = new_expr(kind, 2);
			expr->children[0] = lhs;
			expr->children[1] = rhs;
			expr->type = expr->children[0]->type;
		}
		else
			break;
	}
	
	return expr;
}

/*
	l_expr4 : l_expr3 (('<=' | '>=' | '<' | '>' | '==' | '!=') l_expr3) SEQ OPT .
*/

expr_p parse_expr4(void)
{
	expr_p expr = parse_expr3();
	for (;;)
	{
		int kind = token_it->kind;
		if (   accept_term(TK_EQ) || accept_term(TK_NE)
			|| accept_term(TK_LE) || accept_term(TK_GE)
			|| accept_term('<')   || accept_term('>'))
		{
			expr_p lhs = expr;
			expr_p rhs = parse_expr3();
			if (rhs == NULL)
				FAIL_NULL
			expr = new_expr(kind, 2);
			expr->children[0] = lhs;
			expr->children[1] = rhs;
			expr->type = base_type_bool;
		}
		else
			break;
	}
	
	return expr;
}

/*
	l_expr5 : l_expr4 ('^' l_expr4) SEQ OPT .
*/

expr_p parse_expr5(void)
{
	expr_p expr = parse_expr4();
	while (accept_term('^'))
	{
		expr_p lhs = expr;
		expr_p rhs = parse_expr4();
		if (rhs == NULL)
			FAIL_NULL
		expr = new_expr('^', 2);
		expr->children[0] = lhs;
		expr->children[1] = rhs;
		expr_dioper_set_type(expr);
	}
	
	return expr;
}

/*
	l_expr6 : l_expr5 ('&' l_expr5) SEQ OPT .
*/

expr_p parse_expr6(void)
{
	expr_p expr = parse_expr5();
	while (accept_term('&'))
	{
		expr_p lhs = expr;
		expr_p rhs = parse_expr5();
		if (rhs == NULL)
			FAIL_NULL
		expr = new_expr('&', 2);
		expr->children[0] = lhs;
		expr->children[1] = rhs;
		expr_dioper_set_type(expr);
	}
	
	return expr;
}

/*
	l_expr7 : l_expr6 ('|' l_expr6) SEQ OPT .
*/

expr_p parse_expr7(void)
{
	expr_p expr = parse_expr6();
	while (accept_term('|'))
	{
		expr_p lhs = expr;
		expr_p rhs = parse_expr6();
		if (rhs == NULL)
			FAIL_NULL
		expr = new_expr('|', 2);
		expr->children[0] = lhs;
		expr->children[1] = rhs;
		expr_dioper_set_type(expr);
	}
	
	return expr;
}

/*
	l_expr8 : l_expr7 ('&&' l_expr7) SEQ OPT .
*/

expr_p parse_expr8(void)
{
	expr_p expr = parse_expr7();
	while (accept_term(TK_AND))
	{
		expr_p lhs = expr;
		expr_p rhs = parse_expr7();
		if (rhs == NULL)
			FAIL_NULL
		expr = new_expr(TK_AND, 2);
		expr->children[0] = lhs;
		expr->children[1] = rhs;
		expr->type = base_type_bool;
	}
	
	return expr;
}

/*
	l_expr9 : l_expr8 ('||' l_expr8) SEQ OPT .
*/

expr_p parse_expr9(void)
{
	expr_p expr = parse_expr8();
	while (accept_term(TK_OR))
	{
		expr_p lhs = expr;
		expr_p rhs = parse_expr8();
		if (rhs == NULL)
			FAIL_NULL
		expr = new_expr(TK_OR, 2);
		expr->children[0] = lhs;
		expr->children[1] = rhs;
		expr->type = base_type_bool;
	}
	
	return expr;
}

/*
	conditional_expr
		: l_expr9 '?' l_expr9 ':' conditional_expr
		| l_expr9
		.
*/

expr_p parse_conditional_expr(void)
{
	expr_p expr = parse_expr9();
	if (accept_term('?'))
	{
		expr_p cond_expr = expr;
		expr_p then_expr = parse_expr9();
		if (then_expr == NULL)
			FAIL_NULL
		if (!accept_term(':'))
			FAIL_NULL
		expr_p else_expr = parse_conditional_expr();
		if (else_expr == NULL)
			FAIL_NULL
		expr = new_expr('?', 3);
		expr->children[0] = cond_expr;
		expr->children[1] = then_expr;
		expr->children[2] = else_expr;
		expr->type = then_expr->type;
	}
	
	return expr;
}

/*
	assignment_expr
		: conditional_expr 
		  (( '=' | '*=' | '/=' | '%=' | '+=' | '-=' | '<<=' | '>>=' | '&=' | '|=' | '^=')
		   assignment_expr
		  ) SEQ
		.
*/

expr_p parse_assignment_expr(void)
{
	expr_p expr = parse_conditional_expr();
	if (expr == NULL)
		FAIL_NULL
	for (;;)
	{
		if (token_it->kind == '=' || (token_it->kind >= TK_ASS && token_it->kind < TK_ASS + 127))
		{
			int kind = token_it->kind;
			next_token();
			expr_p lhs = expr;
			expr_p rhs = parse_assignment_expr();
			if (rhs == NULL)
				FAIL_NULL
			expr = new_expr(kind, 2);
			expr->children[0] = lhs;
			expr->children[1] = rhs;
			expr->type = lhs->type;
			//fprintf(fcode, "# %s %d: type of = is %d\n", token_it->filename, token_it->line, expr->type->base_type);
		}
		else
			break;
	}
	
	return expr;
}

/*
	expr : assignment_expr LIST
*/

expr_p parse_expr(void)
{
	expr_p expr = parse_assignment_expr();
	if (expr == NULL)
		FAIL_NULL
	if (accept_term(','))
	{
		int nr_children = 1;
		expr_p children[20];
		children[0] = expr;
		do
		{
			expr_p child = parse_assignment_expr();
			if (child == NULL)
				FAIL_NULL
			children[nr_children++] = child;
		} while (accept_term(','));
		expr = new_expr(',', nr_children);
		for (int i = 0; i < nr_children; i++)
			expr->children[i] = children[i];
		expr->type = expr->children[nr_children-1]->type;
	}
	return expr;
}

bool parse_statements(location_p location);
expr_p parse_initializer(void);

typedef struct struct_or_union_constructor_s struct_or_union_constructor_t;
struct struct_or_union_constructor_s
{
	bool is_struct;
};

struct_or_union_constructor_t *cur_struct_or_union = NULL;

int save_decl_depth = 0;
bool inside_function = FALSE;
bool inside_argument_list = FALSE;

/*
	array_indexes : expr ']' ('[' array_indexes) OPT .
*/

bool parse_array_indexes(type_p type, type_p *arr_type)
{
	expr_p expr = parse_expr();
	if (expr == NULL)
		return FALSE;
	int nr_elems = expr_eval(expr);
	if (!accept_term(']'))
		FAIL_FALSE
	if (accept_term('['))
	{
		type_p result_type = NULL;
		if (!parse_array_indexes(type, &result_type))
			FAIL_FALSE
		type = result_type;
	}
	(*arr_type) = new_type(TYPE_KIND_ARRAY, 1);
	(*arr_type)->members[0] = type;
	(*arr_type)->nr_elems = nr_elems;
	return TRUE;
}

/*
	declaration
		: ('typedef' | 'extern' | 'inline' | 'static' ) SEQ OPT
		  type_specifier
		  ( ';'
		  | func_declarator '(' declaration LIST (',' '...') OPT ')'
		    ( ';'
			| '{' statements '}'
			)
		  | ( declarator ( '=' initializer ) OPT ) LIST ";"
		  )
		.
	func_declarator
		: '*' OPT SEQ ( ident | '(' '*' ident ')')
		.
	declarator
		: '*' OPT SEQ
		  ( ident | '(' '*' ident ')')
		  ( ('[' ']') OPT '[' array_indexes ) OPT
		.
*/

bool parse_declaration(bool is_param)
{
	storage_type_e storage_type = ST_NONE;
	for (;;)
	{
		if (accept_term(TK_TYPEDEF))
		{
			storage_type = ST_TYPEDEF;
			if (inside_function)
				fprintf(stderr, "%s: typedef inside function\n", token_it_pos());
		}
		else if (accept_term(TK_EXTERN))
		{
		}
		else if (accept_term(TK_INLINE))
		{
		}
		else if (accept_term(TK_STATIC))
		{
			if (inside_function)
				storage_type = ST_STATIC;
		}
		else
			break;
	}
	type_p type_specifier = parse_type_specifier(&storage_type);
	if (type_specifier == NULL)
		FAIL_FALSE
	if (accept_term(';'))
	{
		if (   cur_struct_or_union != NULL
			&& (type_specifier->kind == TYPE_KIND_STRUCT || type_specifier->kind == TYPE_KIND_UNION))
		{
			for (int i = 0; i < type_specifier->nr_decls; i++)
			{
				decl_p decl = type_specifier->decls[i];
				decl_p decl_clone = add_decl(decl->kind, decl->name, decl->type);
				decl_clone->storage_type = decl->storage_type;
			}
		}
		return TRUE;
	}
	do
	{
		location_t location;
		save_location(&location, token_it);
		int from = cur_nr_statements;
		type_p type = type_specifier;
		while (accept_term('*'))
		{
			type = new_ptr_type(type);
		}
		decl_p prev_decl = NULL;
		decl_p decl = NULL;
		bool as_pointer = FALSE;
		if (token_it->kind == 'i')
		{
			if (!inside_function && !inside_argument_list && cur_struct_or_union == NULL)
				prev_decl = find_decl(DK_IDENT, token_it->token);
			decl = add_decl(DK_IDENT, token_it->token, NULL);
			next_token();
		}
		else if (accept_term('('))
		{
			if (accept_term('*'))
			{
				if (token_it->kind == 'i')
				{
					as_pointer = TRUE;
					if (!inside_function && !inside_argument_list && cur_struct_or_union == NULL)
						prev_decl = find_decl(DK_IDENT, token_it->token);
					decl = add_decl(DK_IDENT, token_it->token, NULL);
					next_token();
				}
			}

			if (!accept_term(')'))
				FAIL_FALSE
		}
		if (decl != NULL)
		{
			for (;;)
			{
				type_p subj_type = type;
				if (accept_term('('))
				{
					inside_argument_list = TRUE;
					cur_labels = NULL;
					decl_p save_ident_decls = cur_ident_decls;
					bool var_params = FALSE;
					do
					{
						if (accept_term(TK_DASHES))
						{
							var_params = TRUE;
							break;
						}
						if (!parse_declaration(TRUE))
							FAIL_FALSE;
					} while (accept_term(','));
					if (!accept_term(')'))
						FAIL_FALSE;
					inside_argument_list = FALSE;
					type = new_type(TYPE_KIND_FUNCTION, 1);
					type->members[0] = subj_type;
					int nr_parameters = 0;
					for (decl_p decl1 = cur_ident_decls; decl1 != save_ident_decls; decl1 = decl1->prev)
						nr_parameters++;
					decl_p parameters[20];
					int i = nr_parameters;
					if (var_params)
					{
						nr_parameters++;
						parameters[i] = NULL;
					}
					for (decl_p decl1 = cur_ident_decls; decl1 != save_ident_decls; decl1 = decl1->prev)
						parameters[--i] = decl1;
					type_set_decls(type, nr_parameters, parameters);
					decl->type = type;
					if (accept_term('{'))
					{
						inside_function = TRUE;
						if (!parse_statements(NULL))
							FAIL_FALSE
						if (!accept_term('}'))
							FAIL_FALSE
						statement_p statement = add_statement('D', NULL, &location, NULL, from);
						statement->decl = decl;
						cur_ident_decls = save_ident_decls;
						inside_function = FALSE;
						return TRUE;
					}
					//if (!inside_function && cur_struct_or_union == NULL)
					//	fprintf(fcode, "void %s ;\n", decl->name);
					cur_ident_decls = save_ident_decls;
					break;
				}
				else if (accept_term('['))
				{
					if (accept_term(']'))
					{
						if (accept_term('['))
						{
							type_p result_type = NULL;
							if (!parse_array_indexes(type, &result_type))
								FAIL_FALSE
							type = result_type;
						}
						type = new_ptr_type(type);
					}
					else
					{
						type_p result_type = NULL;
						if (!parse_array_indexes(type, &result_type))
							FAIL_FALSE
						type = result_type;
					}
				}
				else
					break;
			}
			if (as_pointer)
			{
				type = new_ptr_type(type);
			}
			if (prev_decl != NULL && type->kind != TYPE_KIND_FUNCTION)
			{
				fprintf(stderr, "%s: Warning: declaration of %s repeated\n", token_it_pos(), decl->name);
				remove_decl(decl);
			}
			decl->type = type;
			decl->storage_type = storage_type;
			if (accept_term('='))
			{
				decl->value = parse_initializer();
				if (!as_pointer && decl->type->kind == TYPE_KIND_POINTER && decl->value != NULL && decl->value->kind == 'l')
				{
					// Fix type:
					int nr_elems = decl->value->nr_children;
					decl->type->kind = TYPE_KIND_ARRAY;
					//decl->type->size = nr_elems * decl->type->members[0]->size;
					decl->type->nr_elems = nr_elems;
				}
			}
			if (!is_param && cur_struct_or_union == NULL && decl->storage_type != ST_TYPEDEF && type->kind != TYPE_KIND_FUNCTION)
			{
				statement_p statement = add_statement('D', NULL, &location, NULL, from);
				statement->decl = decl;
			}
		}
	} while (!is_param && accept_term(','));

	clear_last_line_comment();
	return is_param || accept_term(';');
}

/*
	type_specifier
		: 'const' OPT
		  ( 'char' 'const' OPT
		  | 'unsigned' ('char' | 'short' | 'long' 'long' OPT | 'int') OPT
		  | 'short'
		  | 'int'
		  | 'long' ('double' | 'long') OPT
		  | 'float'
		  | 'double'
		  | 'void'
		  | 'struct' struct_or_union_specifier
		  | 'union' struct_or_union_specifier
		  | 'enum' enum_specifier
		  | ident
		  )
		.
*/

type_p parse_struct_or_union_specifier(decl_kind_e decl_kind);
type_p parse_enum_specifier(void);

type_p parse_type_specifier(storage_type_e *ref_storage_type)
{
	if (accept_term(TK_CONST))
	{
		if (ref_storage_type != NULL) *ref_storage_type = ST_CONST;
	}
	if (accept_term(TK_CHAR))
	{
		if (accept_term(TK_CONST))
		{
			if (ref_storage_type != NULL) *ref_storage_type = ST_CONST;
		}
		return base_type_S8;
	}
	if (accept_term(TK_UNSIGNED))
	{
		if (accept_term(TK_CHAR))
		{
			return base_type_U8;
		}
		if (accept_term(TK_SHORT))
		{
			return base_type_U16;
		}
		if (accept_term(TK_LONG))
		{
			if (accept_term(TK_LONG))
			{
				return base_type_U64;
			}
			return base_type_U32;
		}
		if (accept_term(TK_INT))
		{
			return base_type_U32;
		}
		return base_type_U32;
	}
	if (accept_term(TK_SHORT))
	{
		return base_type_U16;
	}
	if (accept_term(TK_INT))
	{
		return base_type_S32;
	}
	if (accept_term(TK_LONG))
	{
		if (accept_term(TK_DOUBLE))
		{
			return base_type_double;
		}
		if (accept_term(TK_LONG))
		{
			return base_type_S64;
		}
		return base_type_S32; 
	}
	if (accept_term(TK_FLOAT))
	{
		return base_type_float;
	}
	if (accept_term(TK_DOUBLE))
	{
		return base_type_double;
	}
	if (accept_term(TK_VOID))
	{
		return base_type_void;
	}
	if (accept_term(TK_STRUCT))
	{
		return parse_struct_or_union_specifier(DK_STRUCT);
	}
	if (accept_term(TK_UNION))
	{
		return parse_struct_or_union_specifier(DK_UNION);
	}
	if (accept_term(TK_ENUM))
	{
		return parse_enum_specifier();
	}
	if (token_it->kind == 'i')
	{
		decl_p decl = find_decl(DK_IDENT, token_it->token);
		if (decl == NULL)
		{
			if (opt_trace_parser)
				printf("Ident %s has no declaration\n", token_it->token);
			FAIL_NULL
		}
		if (decl->type == NULL)
			FAIL_NULL
		if (decl->storage_type == ST_TYPEDEF)
		{
			if (opt_trace_parser)
				printf("Ident %s is typedef\n", decl->name);
			next_token();
			return decl->type;
		}
		else
		{
			if (opt_trace_parser)
				printf("Ident %s is not a typedef\n", token_it->token);
			FAIL_NULL
		}
	}
	FAIL_NULL
}

/*
	struct_or_union_specifier : ident OPT ('{' declaration SEQ OPT "}") OPT .
*/

type_p parse_struct_or_union_specifier(decl_kind_e decl_kind)
{
	type_kind_e type_kind = decl_kind == DK_STRUCT ? TYPE_KIND_STRUCT : TYPE_KIND_UNION;
	type_p type = NULL;
	if (token_it->kind == 'i')
	{
		decl_p decl = find_or_add_decl(decl_kind, token_it->token);
		if (decl->type == NULL)
		{
			decl->type = new_type(type_kind, 0);
		}
		type = decl->type;
		next_token();
	}
	if (accept_term('{'))
	{
		struct_or_union_constructor_t *parent = cur_struct_or_union;
		struct_or_union_constructor_t this_struct_or_union;
		this_struct_or_union.is_struct = decl_kind == DK_STRUCT;
		cur_struct_or_union = &this_struct_or_union;
		decl_p save_ident_decls = cur_ident_decls;
		do
		{
			if (!parse_declaration(FALSE))
				FAIL_NULL
		} while (!accept_term('}'));
		int nr_decls = 0;
		for (decl_p decl1 = cur_ident_decls; decl1 != save_ident_decls; decl1 = decl1->prev)
			nr_decls++;
		if (type == NULL)
		{
			type = new_type(type_kind, 0);
		}
		//else
		//	type->size = this_struct_or_union.size;
		decl_p decls[200];
		int i = nr_decls;
		for (decl_p decl1 = cur_ident_decls; decl1 != save_ident_decls; decl1 = decl1->prev)
			decls[--i] = decl1;
		{
			//gen_start_struct_or_union();
			//for (int i = 0; i < nr_decls; i++)
			//	gen_struct_or_union_member(decls[i]);
		}
		type_set_decls(type, nr_decls, decls);
		cur_ident_decls = save_ident_decls;
		cur_struct_or_union = parent;
	}
	return type;
}

/*
	enum_specifier : ident OPT ('{' ident ('=' constant_expr) OPT ) LIST '}') OPT .
*/

type_p parse_enum_specifier(void)
{
	type_p type = NULL;
	if (token_it->kind == 'i')
	{
		decl_p decl = find_or_add_decl(DK_ENUM, token_it->token);
		if (decl->type == NULL)
		{
			decl->type = new_type(DK_ENUM, 0);
		}
		type = decl->type;
		next_token();
	}
	int next_enum_val = 0;
	if (accept_term('{'))
	{
		for (;;)
		{
			if (token_it->kind != 'i')
				FAIL_NULL
			decl_p const_decl = add_decl(DK_IDENT, token_it->token, base_type_S32);
			next_token();
			if (accept_term('='))
			{
				expr_p expr = parse_conditional_expr();
				if (expr == NULL)
					FAIL_NULL
				next_enum_val = expr_eval(expr);
			}
			const_decl->value = new_expr_int_value(next_enum_val);
			const_decl->storage_type = ST_CONST;
			//gen_enum_decl(const_decl);
			next_enum_val++;
			if (!accept_term(','))
				break;
			if (token_it->kind == '}')
				break;
		}
		if (!accept_term('}'))
			FAIL_NULL
	}
	if (type == NULL)
		type = new_type(TYPE_KIND_ENUM, 0);
	return type;
}

/*
	initializer
		: '{' initializer LIST OPT ',' OPT '}'
		| assignment_expr
		.
*/

expr_p parse_initializer(void)
{
	if (accept_term('{'))
	{
		expr_p exprs[2000];
		int nr_exprs = 0;
		for (;;)
		{
			if (accept_term('.'))
			{
				if (token_it->kind == 'i')
					next_token();
				accept_term('=');
			}
			exprs[nr_exprs++] = parse_initializer();
			if (!accept_term(','))
				break;
			if (token_it->kind == '}')
				break;
		}
		if (!accept_term('}'))
			FAIL_NULL
		expr_p expr_list = new_expr('l', nr_exprs);
		for (int i = 0; i < nr_exprs; i++)
			expr_list->children[i] = exprs[i];
		return expr_list;
	}
	return parse_assignment_expr();
}

bool add_tracing = FALSE;

/*
	statement
		: (ident ':') SEQ OPT
		  ( 'if' '(' expr ')' statement ('else' statement) OPT
		  | 'while' '(' expr ')' statement
		  | 'do' statement 'while' '(' expr ')' ';'
		  | 'for' '(' (declaration | expr ';') expr ';' expr ')' statement
		  | 'break' ';'
		  | 'continue' ';'
		  | 'switch' '(' expr ')' '{'
		  	 ( ( 'case' expr ':' | 'default' ':') SEQ statement SEQ ) SEQ '}'
		  | 'return' expr OPT ';'
		  | 'goto' ident ';'
		  | '{' statements '}'
		  | expr ';'
		  )
		.
*/

int default_case_nr = 0;
int fall_through_case_nr = 0;
bool label_statement = FALSE;

bool parse_statement(bool in_block)
{
	char *comment = get_last_line_comment();
	label_p label = 0;
	for (;;)
	{
		if (token_it->kind == 'i')
		{
			label_p n_label = find_label(token_it->token);
			if (n_label == NULL && find_decl(DK_IDENT, token_it->token) == NULL)
				n_label = find_or_add_label(token_it->token);
			if (n_label != NULL)
			{
				if (label != NULL)
					printf("Error: Double label %s and %s\n", label->name, n_label->name);
				label = n_label;
				label_statement = TRUE;
				next_token();
				if (!accept_term(':'))
					FAIL_FALSE
				continue;
			}
		}
		break;
	}
	location_t location;
	save_location(&location, token_it);
	if (accept_term(TK_IF))
	{
		if (!accept_term('('))
			FAIL_FALSE
		expr_p cond = parse_expr();
		if (cond == NULL)
			FAIL_FALSE
		if (!accept_term(')'))
			FAIL_FALSE
		int from = cur_nr_statements;
		if (!parse_statement(TRUE))
			FAIL_FALSE
		if (accept_term(TK_ELSE))
		{
			if (!parse_statement(TRUE))
				FAIL_FALSE
		}
		statement_p statement = add_statement(TK_IF, label, &location, comment, from);
		statement->expr = cond;
		return TRUE;
	}
	if (accept_term(TK_WHILE))
	{
		if (!accept_term('('))
			FAIL_FALSE
		expr_p cond = parse_expr();
		if (cond == NULL)
			FAIL_FALSE
		if (!accept_term(')'))
			FAIL_FALSE
		int from = 0;
		if (!parse_statement(TRUE))
			FAIL_FALSE
		statement_p statement = add_statement(TK_WHILE, label, &location, comment, from);
		statement->expr = cond;
		return TRUE;
	}
	if (accept_term(TK_DO))
	{
		int from = 0;
		if (!parse_statement(TRUE))
			FAIL_FALSE
		if (!accept_term(TK_WHILE))
			FAIL_FALSE
		if (!accept_term('('))
			FAIL_FALSE
		expr_p cond = parse_expr(); 
		if (cond == NULL)
			FAIL_FALSE
		if (!accept_term(')'))
			FAIL_FALSE
		if (!accept_term(';'))
			FAIL_FALSE
		statement_p statement = add_statement(TK_DO, label, &location, comment, from);
		statement->expr = cond;
		return TRUE;
	}
	if (accept_term(TK_FOR))
	{
		decl_p save_ident_decls = cur_ident_decls;
		if (!accept_term('('))
			FAIL_FALSE
		int from = cur_nr_statements;
		statement_p init_stat = add_statement('i', 0, &location, NULL, -1);
		if (parse_declaration(FALSE))
			init_stat->decl = cur_decls;
		else
		{
			init_stat->decl = cur_decls;
			init_stat->expr = parse_expr();
			if (!accept_term(';'))
				FAIL_FALSE
		}
		expr_p cond = parse_expr();
		if (!accept_term(';'))
			FAIL_FALSE
		statement_p next_state = add_statement('n', 0, &location, NULL, -1);
		next_state->expr = parse_expr();
		if (!accept_term(')'))
			FAIL_FALSE
		if (!parse_statement(TRUE))
			FAIL_FALSE
		statement_p statement = add_statement(TK_FOR, label, &location, comment, from);
		statement->expr = cond;
		cur_ident_decls = save_ident_decls;
		return TRUE;
	}
	if (accept_term(TK_BREAK))
	{
		if (!accept_term(';'))
			FAIL_FALSE
		add_statement(TK_BREAK, label, &location, comment, -1);
		return TRUE;
	}
	if (accept_term(TK_CONTINUE))
	{
		if (!accept_term(';'))
			FAIL_FALSE
		add_statement(TK_CONTINUE, label, &location, comment, -1);
		return TRUE;
	}
	if (accept_term(TK_SWITCH))
	{
		if (!accept_term('('))
			FAIL_FALSE
		expr_p switch_expr = parse_expr();
		if (switch_expr == NULL)
			FAIL_FALSE
		if (!accept_term(')'))
			FAIL_FALSE
		if (!accept_term('{'))
			FAIL_FALSE
		int from = cur_nr_statements;
		bool fall_through = FALSE;
		//bool has_default = FALSE;
		while (token_it->kind == TK_CASE || token_it->kind == TK_DEFAULT)
		{
			char *case_comment = get_last_line_comment();
			//int case_labels[100];
			//int nr_case_labels = 0;
			//bool default_case = FALSE;
			for (;;)
			{
				location_t case_location;
				save_location(&case_location, token_it);
				if (accept_term(TK_CASE))
				{
					expr_p expr = parse_expr(); 
					if (expr == NULL)
						FAIL_FALSE
					if (!accept_term(':'))
						FAIL_FALSE
					statement_p case_stat = add_statement(TK_CASE, 0, &case_location, case_comment, -1);
					case_stat->expr = expr;
					//if (nr_case_labels < 100)
					//	case_labels[nr_case_labels++] = expr_eval(expr);
					//else
					//	printf("Error: more than 50 case label\n");
				}
				else if (accept_term(TK_DEFAULT))
				{
					if (!accept_term(':'))
						FAIL_FALSE
					add_statement(TK_DEFAULT, 0, &case_location, case_comment, -1);
					//default_case = TRUE;
					//has_default = TRUE;
				}
				else
					break;
			}
			bool has_break = FALSE;
			while (token_it->kind != '}' && token_it->kind != TK_CASE && token_it->kind != TK_DEFAULT)
			{
				if (token_it->kind == TK_BREAK || token_it->kind == TK_RETURN || token_it->kind == TK_GOTO)
					has_break = TRUE;
				label_statement = FALSE;
				bool go = parse_statement(FALSE);
				if (label_statement)
					has_break = FALSE;
				if (!go)
					break;
			}
			if (token_it->kind == '}' && !has_break)
			{
				has_break = TRUE;
			}
			fall_through = !has_break;
			if (fall_through)
			{
				fall_through_case_nr++;
			}
		}
		if (!accept_term('}'))
			FAIL_FALSE
		//if (has_default)
		//{
		//	default_case_nr++;
		//}
		//else
		//{
			//fprintf(fcode, "; break\n");
		//}
		statement_p statement = add_statement(TK_SWITCH, label, &location, comment, from);
		statement->expr = switch_expr;
		return TRUE;
	}
	if (accept_term(TK_RETURN))
	{
		statement_p statement = add_statement(TK_RETURN, label, &location, comment, -1);
		statement->expr = parse_expr();
		if (!accept_term(';'))
			FAIL_FALSE
		return TRUE;
	}
	if (accept_term(TK_GOTO))
	{
		statement_p statement = add_statement(TK_GOTO, label, &location, comment, -1);
		statement->goto_label = find_or_add_label(token_it->token);
		next_token();
		if (!accept_term(';'))
			FAIL_FALSE
		return TRUE;
	}
	if (accept_term('{'))
	{
		if (!parse_statements(&location))
			FAIL_FALSE;
		if (!accept_term('}'))
			FAIL_FALSE
		return TRUE;
	}
	expr_p expr = parse_expr();
	if (expr != NULL || label != NULL)
	{
		statement_p statement = add_statement('e', label, &location, comment, -1);
		statement->expr = expr;
	}
	if (!accept_term(';'))
		FAIL_FALSE
	return TRUE;
}

/*
	statements : (declaration | statement) SEQ OPT .
*/

bool parse_statements(location_p location)
{
	int from = cur_nr_statements;
	decl_p save_ident_decls = cur_ident_decls;
	do
	{
	} while (token_it->kind != '}' && (parse_declaration(FALSE) || parse_statement(FALSE)));
	if (location != NULL)
		add_statement('{', NULL, location, NULL, from);
	cur_ident_decls = save_ident_decls;
	return TRUE;
}



void add_base_type(const char *name, type_p base_type)
{
	add_decl(DK_IDENT, name, base_type);
	cur_ident_decls->storage_type = ST_TYPEDEF;
}

void add_function(const char *name, type_p result_type)
{
	type_p type = new_type(TYPE_KIND_FUNCTION, 1);
	type->members[0] = result_type;
	add_decl(DK_IDENT, name, type);
}

void add_predefined_types(void)
{
/*
	add_base_type("uint64_t", base_type_U64);
	add_base_type("int64_t", base_type_S64);
	add_base_type("uint32_t", base_type_U32);
	add_base_type("int32_t", base_type_S32);
	add_base_type("uint16_t", base_type_U16);
	add_base_type("uint8_t", base_type_U8);
	add_base_type("int8_t", base_type_S8);
	add_base_type("size_t", base_type_U32);
	add_base_type("ssize_t", base_type_S32);
	add_base_type("jmp_buf", base_type_jmp_buf);
	add_base_type("bool", base_type_bool);
*/
}

bool parse_file(const char *input_filename, bool only_preprocess)
{
	strcpy(include_path, input_filename);
	end_include_prefix = include_path;
	for (char *s = include_path; *s != '\0'; s++)
		if (*s == '/')
			end_include_prefix = s + 1;

	file_iterator_p input_it = new_file_iterator(input_filename, 0);
	if (input_it->base.ch == '\0')
	{
		printf("ERROR: Could not open file %s or it was empty\n", input_filename);
		return FALSE;
	}
	line_splice_iterator_p splice_it = new_line_splice_iterator(&input_it->base);
	comment_strip_iterator_p comment_it = new_comment_strip_iterator(&splice_it->base);
	include_iterator_p include_it = new_include_iterator(&comment_it->base);
	tokenizer_p tokenizer_it = new_tokenizer(&include_it->base);
	conditional_iterator_p conditional_it = new_conditional_iterator(include_it, &tokenizer_it->base);
	expand_iterator_p expand_it = new_expand_iterator(&conditional_it->base);
	
	token_it = (token_iterator_p)expand_it;

	token_it = token_it->next(token_it, TRUE);

	if (only_preprocess)
	{
		output_preprocessor("cc_p.c");
		return TRUE;
	}

	while (parse_declaration(FALSE))
	{
	}

	if (token_it != 0 && token_it->kind != 0)
	{
		printf("Parsed %s till %s:%d.%d: kind:%d token:'%s'\n", input_filename, token_it->filename, token_it->line, token_it->column, token_it->kind, token_it->token);
		return FALSE;
	}
	return TRUE;
}

