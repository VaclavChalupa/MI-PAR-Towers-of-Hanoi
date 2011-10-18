/*
 * stack.c
 *
 */

#include "stack.h"
#include <stdio.h>
#include <stdlib.h>

Stack * stack;
int x = 0;
static void pushItem(StackItem* stackItem, int step);

Stack * initializeStack() {
	stack = malloc(sizeof(* stack));
	stack->top = NULL;
	stack->num = 0;
	return stack;
}

void freeStack() {
	StackItem* stackItem = stack->top;
	while(stackItem != NULL) {
		StackItem* tmp = stackItem->next;
		free(stackItem->data);
		free(stackItem);
		stackItem = tmp;
	}
	free(stack);
}

void pushItem(StackItem* stackItem, int step) {
	stackItem->step = step;
	if(stack->top != NULL)
		stackItem->num = stack->top->num + 1;
	else
		stackItem->num = 0;
	stackItem->next = stack->top;
	stack->top = stackItem;
	stack->num++;

	printf("INSERT %i\n", stackItem->num);

}

void push(int* data, int step) {
	StackItem* stackItem;
	stackItem = (StackItem*) malloc(sizeof(* stackItem));

	stackItem->data = data;
	stackItem->next = NULL;
	pushItem(stackItem, step);
}

int* top(int* step) {
	if(isStackEmpty()) {
		perror("ERROR: stack is empty for top");
		return NULL;
	}
	*step = stack->top->step;
	return stack->top->data;
}

void pop() {

	printf("\nPOP\n");

	StackItem* tmp;
	if(isStackEmpty()) {
		perror("ERROR: stack is empty for pop");
		return;
	}


	if(stack->top->next != NULL) {
		printf("\n %i\n", stack->top->next->num);
	}
	stack->num--;
	tmp = stack->top;
	stack->top = stack->top->next;

	if(stack->top != NULL) {
			printf("\n %i\n", stack->top->num);
	}

	free(tmp);
}

int isStackEmpty() {
	if(stack->top != NULL) {
		return 0;
	}
	return 1;
}
