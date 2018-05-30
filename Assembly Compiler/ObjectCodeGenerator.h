#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "myStack.h"

char *strip(char *word) {
	int len = strlen(word);
	if (word[len - 1] == '\n')
		word[len - 1] = '\0';
	return word;
}

char **str_split(char *string, char *delimiter, int *len) {
	char **tokens = (char **)calloc(10, sizeof(char *)), *token;
	int index = 0;
	token = strtok(string, delimiter);
	while (token != NULL) {
		tokens[index] = (char *)malloc(100 * sizeof(char));
		strcpy(tokens[index], token);//strip(token));
		index++;
		token = strtok(NULL, delimiter);
	}
	if (len != NULL) *len = index;
	return tokens;
}

typedef struct symbol_table {
	char *name;
	int address;
	int size;
	int is_constant;
	int value;
};

typedef struct operations {
	int op_code;
	int *parameters;
	int para_len;
};

typedef struct label_manager {
	char name;
	int address;
};

symbol_table **st;
operations **code;
label_manager **LM;
int current_address = 0, symbol_count = 0, op_count = 0, label_count = 0;

int get_symbol_address(char *symbol){
	for (int i = 0; i < symbol_count; i++) {
		if (strcmp(st[i]->name, symbol) == 0) {
			return st[i]->address;
		}
	}
	return -1;
}

void add_symbol(char *symbol, int address, int size, int is_constant, int value) {
	st = (symbol_table **)realloc(st, sizeof(symbol_table *) * (symbol_count + 1));
	st[symbol_count] = (symbol_table *)malloc(sizeof(symbol_table));
	st[symbol_count]->name = (char *)malloc(100 * sizeof(char));
	strcpy(st[symbol_count]->name, symbol);
	st[symbol_count]->address = address;
	st[symbol_count]->size = size;
	st[symbol_count]->is_constant = is_constant;
	st[symbol_count]->value = value;
	symbol_count++;
}

void add_operation(int op_code, int *parameters, int para_count) {
	code = (operations **)realloc(code, sizeof(operations *) * (op_count + 1));
	code[op_count] = (operations *)malloc(sizeof(operations));
	code[op_count]->op_code = op_code;
	code[op_count]->parameters = parameters;
	code[op_count]->para_len = para_count;
	op_count++;
}

void add_label(char label, int line) {
	LM = (label_manager **)realloc(LM, sizeof(label_manager *) * (label_count + 1));
	LM[label_count] = (label_manager *)malloc(sizeof(label_manager));
	LM[label_count]->name = label;
	LM[label_count++]->address = line;
}

int find_label(char label) {
	for (int i = 0; i < label_count; i++) {
		if (LM[i]->name == label) {
			return LM[i]->address;
		}
	}
	return -1;
}

void print_symbol_table() {
	printf("%5s%20s%20s%20s%20s%20s\n", "Sno", "Name", "Address", "Size", "Is Constant", "Value");
	for (int i = 0; i < symbol_count; i++) {
		printf("%5d%20s%20d%20d%20d%20d\n", i + 1, st[i]->name, st[i]->address, st[i]->size, st[i]->is_constant, st[i]->value);
	}
	printf("\n");
}

void print_operations() {
	printf("%5s%20s", "Sno", "Op Code");
	for (int i = 0; i < 4; i++) {
		printf("%20s %d", "Parameter", i + 1);
	}
	printf("\n");
	for (int i = 0; i < op_count; i++) {
		printf("%5d%20d", i + 1, code[i]->op_code);
		for (int j = 0; j < code[i]->para_len; j++) {
			printf("%20d", code[i]->parameters[j]);
		}
		printf("\n");
	}
	printf("\n");
}

void create_intermediate_code(char *file_name) {
	char *new_file_name = (char *)malloc(100 * sizeof(char));
	strcpy(new_file_name, file_name);
	strcat(new_file_name, ".txt");
	st = (symbol_table **)calloc(40, sizeof(symbol_table *));
	code = (operations **)calloc(40, sizeof(operations *));
	LM = (label_manager **)calloc(40, sizeof(label_manager *));
	stack *conditionals = create_stack(20);
	FILE *fp = fopen(new_file_name, "r");
	char buf[100];
	while (fgets(buf, 100, fp) != NULL) {
		//system("pause");
		int len;
		char **tokens = str_split(buf, " ,=\t\n", &len);
		int *parameters = (int *)calloc(5, sizeof(int));
		if (strcmp(tokens[0], "DATA") == 0) {
			if (strlen(tokens[1]) > 1) {
				char **temp = str_split(tokens[1], "[]", NULL);
				add_symbol(temp[0], current_address, atoi(temp[1]), 0, -1);
				current_address += atoi(temp[1]);
			}
			else {
				add_symbol(tokens[1], current_address, 1, 0, -1);
				current_address++;
			}
		}
		else if (strcmp(tokens[0], "CONST") == 0) {
			add_symbol(tokens[1], current_address++, 1, 1, atoi(tokens[2]));
		}
		else if (strcmp(tokens[0], "MOV") == 0) {
			if (strlen(tokens[1]) == 2 && tokens[1][1] == 'X') {
				// memory to register
				int offset = 0;
				if (strlen(tokens[2]) > 1) {
					offset += atoi(str_split(tokens[2], "[]", NULL)[1]);
				}
				parameters[0] = get_symbol_address(tokens[2]) + offset;
				parameters[1] = tokens[1][0] - 'A';
				add_operation(1, parameters, 2);
			}
			else {
				//register to memory
				int offset = 0;
				if (strlen(tokens[1]) > 1) {
					offset += atoi(str_split(tokens[1], "[]", NULL)[1]);
				}
				parameters[0] = get_symbol_address(tokens[1]) + offset;
				parameters[1] = tokens[2][0] - 'A';
				add_operation(2, parameters, 2);
			}
		}
		else if (strcmp(tokens[0], "ADD") == 0) {
			parameters[0] = tokens[2][0] - 'A';
			parameters[1] = tokens[3][0] - 'A';
			parameters[2] = tokens[1][0] - 'A';
			add_operation(3, parameters, 3);
		}
		else if (strcmp(tokens[0], "SUB") == 0) {
			parameters[0] = tokens[2][0] - 'A';
			parameters[1] = tokens[3][0] - 'A';
			parameters[2] = tokens[1][0] - 'A';
			add_operation(4, parameters, 3);
		}
		else if (strcmp(tokens[0], "MUL") == 0) {
			parameters[0] = tokens[2][0] - 'A';
			parameters[1] = tokens[3][0] - 'A';
			parameters[2] = tokens[1][0] - 'A';
			add_operation(5, parameters, 3);
		}
		else if (strlen(tokens[0])==2 && tokens[0][1] == ':') {
			parameters[0] = tokens[0][0];
			add_label(tokens[0][0], op_count + 1);
		}
		else if (strcmp(tokens[0], "IF") == 0 && strcmp(tokens[4], "THEN") == 0) {
			int *index = (int *)malloc(sizeof(int));
			*index = op_count;
			push_stack(conditionals, (void *)index);
			int operator_index;
			if (strcmp(tokens[2], "EQ") == 0) operator_index = 8;
			else if (strcmp(tokens[2], "LT") == 0) operator_index = 9;
			else if (strcmp(tokens[2], "GT") == 0) operator_index = 10;
			else if (strcmp(tokens[2], "LTEQ") == 0) operator_index = 11;
			else if (strcmp(tokens[2], "GTEQ") == 0) operator_index = 12;
			parameters[0] = tokens[1][0] - 'A';
			parameters[1] = tokens[3][0] - 'A';
			parameters[2] = operator_index;
			parameters[3] = -1;
			add_operation(7, parameters, 4);
		}
		else if (strcmp(tokens[0], "ELSE") == 0) {
			operations *if_operation = code[*(int *)pop_stack(conditionals)];
			if_operation->parameters[3] = op_count + 2;
			int *index = (int *)malloc(sizeof(int));
			*index = op_count;
			push_stack(conditionals, (void *)index);
			parameters[0] = -1;
			add_operation(16, parameters, 1);
		}
		else if (strcmp(tokens[0], "ENDIF") == 0) {
			operations *else_operation = code[*(int *)pop_stack(conditionals)];
			for (int i = 0; i < 4; i++) {
				if (else_operation->parameters[i] == -1) {
					else_operation->parameters[i] = op_count + 1;
					break;
				}
			}
		}
		else if (strcmp(tokens[0], "PRINT") == 0) {
			parameters[0] = tokens[1][0] - 'A';
			add_operation(13, parameters, 1);
		}
		else if (strcmp(tokens[0], "READ") == 0) {
			parameters[0] = tokens[1][0] - 'A';
			add_operation(14, parameters, 1);
		}
		else if (strcmp(tokens[0], "JMP") == 0) {
			parameters[0] = find_label(tokens[1][0]);
			add_operation(6, parameters, 1);
		}
		//system("cls");
		//print_operations();
		//print_symbol_table();
	}
	fclose(fp);
}

void save_intermediate_code(char *file_name) {
	print_operations();
	print_symbol_table();
	char *new_file_name = (char *)malloc(100 * sizeof(char));
	strcpy(new_file_name, file_name);
	strcat(new_file_name, ".o");
	FILE *fp = fopen(new_file_name, "wb+");
	if (fp == NULL) return;
	fwrite(&symbol_count, sizeof(int), 1, fp);
	fwrite(st, sizeof(symbol_table *), symbol_count, fp);
	fwrite(&op_count, sizeof(int), 1, fp);
	fwrite(code, sizeof(operations *), op_count, fp);
	fclose(fp);
}

void save_intermediate_code_to_mem() {
	void *memory = malloc(1000);
}

void compiler_intermediate_code(char *file_name) {
	char *new_file_name = (char *)malloc(100 * sizeof(file_name));
	strcpy(new_file_name, file_name);
	strcat(new_file_name, ".o");
	FILE *fp = fopen(new_file_name, "rb");
	if (fp == NULL) {
		return;
	}
	fread(&symbol_count, sizeof(int), 1, fp);
	st = (symbol_table **)calloc(symbol_count, sizeof(symbol_table*));
	fread(st, sizeof(symbol_table *), symbol_count, fp);
	fread(&op_count, sizeof(int), 1, fp);
	code = (operations **)calloc(op_count, sizeof(operations *));
	fread(code, sizeof(operations *), op_count, fp);
	fclose(fp);
	int *data = (int *)calloc(1000, sizeof(int));
	for (int i = 0; i < symbol_count; i++) {
		if (st[i]->is_constant) {
			data[st[i]->address] = st[i]->value;
		}
	}
	int registers[9] = { 0 }, boolean = 0;
	int register_offset, var1_offset, var2_offset, ans_offset, jmp_loc;
	for (int i = 0; i < op_count; i++) {
		switch (code[i]->op_code) {
		case 1:
			register_offset = code[i]->parameters[1];
			var1_offset = code[i]->parameters[0];
			data[var1_offset] = registers[register_offset];
			break;
		case 2:
			register_offset = code[i]->parameters[1];
			var1_offset = code[i]->parameters[0];
			registers[register_offset] = data[var1_offset];
			break;
		case 3:
			var1_offset = code[i]->parameters[0];
			var2_offset = code[i]->parameters[1];
			ans_offset = code[i]->parameters[2];
			data[ans_offset] = data[var1_offset] + data[var2_offset];
			break;
		case 4:
			var1_offset = code[i]->parameters[0];
			var2_offset = code[i]->parameters[1];
			ans_offset = code[i]->parameters[2];
			data[ans_offset] = data[var1_offset] - data[var2_offset];
			break;
		case 5:
			var1_offset = code[i]->parameters[0];
			var2_offset = code[i]->parameters[1];
			ans_offset = code[i]->parameters[2];
			data[ans_offset] = data[var1_offset] * data[var2_offset];
			break;
		case 7:
			var1_offset = code[i]->parameters[0];
			var2_offset = code[i]->parameters[1];
			switch (code[i]->parameters[2]) {
			case 8:
				boolean = data[var1_offset] == data[var2_offset];
				break;
			case 9:
				boolean = data[var1_offset] < data[var2_offset];
				break;
			case 10:
				boolean = data[var1_offset] > data[var2_offset];
				break;
			case 11:
				boolean = data[var1_offset] <= data[var2_offset];
				break;
			case 12:
				boolean = data[var1_offset] >= data[var2_offset];
				break;
			}
			if (boolean == 0) {
				i = code[i]->parameters[3] - 2;
			}
			break;
		case 16:
			if (boolean == 1) {
				i = code[i]->parameters[0] - 2;
				boolean = 0;
			}
			break;
		case 13:
			printf("%d", data[code[i]->parameters[0]]);
			break; 
		case 14:
			scanf("%d", &data[code[i]->parameters[0]]);
			break;
		case 6:
			jmp_loc = code[i]->parameters[0];
			i = jmp_loc - 2;
			break;
		}
	}
}
