/*
 * stack.h
 *
 */

#ifndef STACK_H_
#define STACK_H_

#include "stack_item.h"

typedef struct {
	StackItem *top;
	int num;
} Stack;

Stack * initializeStack();

void push(int* data, int step);

int* top(int* step);

void pop();

int isStackEmpty();

void freeStack();

#endif /* STACK_H_ */
