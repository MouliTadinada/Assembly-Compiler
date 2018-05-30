#include "ObjectCodeGenerator.h"

int main() {
	char file_name[100] = "random";
	create_intermediate_code(file_name);
	save_intermediate_code(file_name);
	compiler_intermediate_code(file_name);
	printf("\n"); system("pause");
	return 0;
}