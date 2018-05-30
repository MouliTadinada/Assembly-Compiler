#include <malloc.h>

typedef struct stack {
	void **data;
	int size;
	int top;
};

stack *create_stack(int size) {
	// CREATE A STACK
	stack *s = (stack *)malloc(sizeof(stack));
	s->size = size;
	s->data = (void **)calloc(s->size, 4);
	s->top = -1;
	return s;
}

void *top_of_stack(stack *s) {
	if (s->top == -1) return NULL;
	return s->data[s->top];
}

int push_stack(stack *s, void *data) {
	// PUSH DATA INTO STACK
	if (s->top == s->size) return -1;
	s->top++;
	s->data[s->top] = (void *)malloc(4);
	s->data[s->top] = data;
	return 1;
}

void *pop_stack(stack *s) {
	// POP DATA FROM STACK
	if (s->top == -1) return NULL;
	return s->data[s->top--];
}

void print_stack(stack *s, void (*print_method) (void *)){
	// PRINT DATA OF STACK
	if (s->top == -1) {
		printf("No Elements!\n");
		return;
	}
	printf("Elements:");
	for (int i = 0; i <= s->top; i++) {
		print_method(s->data[i]);
	}
	printf("\n");
}