#include <stdio.h>
#include <malloc.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

// ----- bool -----

typedef char bool;
#define TRUE 1
#define FALSE 0

// ------ string -------

bool eqstr(char* s, char* t)
{
	return strcmp(s, t) == 0;
} 

// ----- Strings ----

char* copystr(const char* str)
{
	int len = strlen(str);
	char* new_str = malloc(len + 1);
	strcpy(new_str, str);

	return new_str;
}

char* copystrlen(const char* str, int len)
{
	char* new_str = malloc(len + 1);
	memcpy(new_str, str, len + 1);

	return new_str;
}

// Include C parser

#include "c_parser.c"

// Include flow chart parser

#include "flowchart_parser.c"

// Main

bool has_extention(const char *filename, const char *ext)
{
	int len_fn = strlen(filename);
	int len_ext = strlen(ext);
	return len_fn > len_ext && strcmp(filename + len_fn - len_ext, ext) == 0; 
}

int main(int argc, char *argv[])
{
	strcpy(std_include_path, argv[0]);
	end_std_include_prefix = std_include_path + strlen(std_include_path);
	while (end_std_include_prefix > std_include_path && end_std_include_prefix[-1] != '/')
		end_std_include_prefix--;
	strcpy(end_std_include_prefix, "include/");
	end_std_include_prefix += 8;

	include_path = malloc(200);

	bool init = FALSE;

	//fcode = stdout;
	bool only_preprocess = FALSE;

	for (int i = 1; i < argc; i++)
		if (strcmp(argv[i], "-E") == 0)
			only_preprocess = TRUE;
		else if (strcmp(argv[i], "-T") == 0)
			add_tracing = TRUE;
		else if (strcmp(argv[i], "-dp") == 0)
			opt_trace_parser = TRUE;
		else if (strcmp(argv[i], "-I") == 0 && i + 1 < argc)
		{
			strcpy(alt_include_path, argv[++i]);
			end_alt_include_prefix = alt_include_path + strlen(alt_include_path);
		}
		else if (strcmp(argv[i], "-D") == 0 && i + 1 < argc)
		{
			i++;
			const char *s = argv[i];
			char name[100];
			int j = 0;
			for (; *s != '\0' && *s != '='; s++)
				if (j < 99)
					name[j++] = *s;
			name[j] = '\0';
			env_p env = get_env(name, TRUE);
			if (*s == '=')
			{
				s++;
				char value[200];
				j = 0;
				if (*s == '"')
				{
					s++;
					for (; *s != '"' && *s != '\0'; s++)
						if (j < 199)
							value[j++] = *s;
					value[j] = '\0';
					env->tokens = new_str_token(value);
				}
				else
				{
					for (; '0' <= *s && *s <= '9'; s++)
						if (j < 199)
							value[j++] = *s;
					value[j] = '\0';
					env->tokens = new_int_token(value);
				}
			}
		}
		else if (has_extention(argv[i], ".c"))
		{
			if (!init)
			{
				define_base_types();
				add_predefined_types();
				init = TRUE;
			}

			if (!parse_file(argv[i], only_preprocess))
				return 1;
			for (int i = 0; i < cur_nr_statements; i++)
				print_statement(cur_statements[i], 0);
		}
		else if (has_extention(argv[i], ".graphml"))
		{
			parse_flowchart(argv[i]);
			print_flowchart();
		}
		else
		{
			printf("Error: Unknown extention for '%s'\n", argv[i]);
			break;
		}

	if (only_preprocess)
		return 0;

	return 0;
}
