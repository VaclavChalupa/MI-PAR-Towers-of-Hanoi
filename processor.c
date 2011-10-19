/*
 * processor.c
 *
 */

#include "state_printer.h";
#include "tower.h"
#include "stack_item.h"
#include "stack.h"
#include "process_item.h"
#include "analyser.h"
#include <stdio.h>
#include <stdlib.h>

int towersCount;
Tower *towers;
int discsCount;
int min;
int max;
int currentSteps;
int minSteps;
int destTower;
struct SolutionQueue {
	ProcessItem *head;
};

static void describeMove(int* prevState, int* currentState, int* disc, int* sourceTower, int* destTower);
static int compareStates(int* prevState, int* currentState);
static void inspectStack(Stack * stack, struct SolutionQueue* sq);
static int*  serializeState(Tower* _towers);
static Tower* deserializeState(int* data);
static int loopDetected(Stack* stack);
static void processStepWithStack(struct SolutionQueue* sq);
void freeInspectStack(struct SolutionQueue* sq);

int*  serializeState(Tower* _towers) {
	int * stack_item, i;
	stack_item = (int*) malloc(discsCount * sizeof(int));

	for(i = 0; i < discsCount; i++) {
		stack_item[i] = -1;
	}

	for(i = 0; i < towersCount; i++) {
		Disc* disc;
		disc = _towers[i].top;
		while(disc != NULL) {
			stack_item[disc->size-1] = i; /* disc size -> tower indexed form 0*/
			disc = disc->next;
		}
	}

	for(i = 0; i < discsCount; i++) {
		if(stack_item[i] == -1) {
			perror("ERROR: stack_item defect");
			return NULL;
		}
	}

	return stack_item;
}

Tower* deserializeState(int* data) {
	int i;
	Tower* _towers;

	_towers = (Tower*) malloc (towersCount * sizeof(Tower));

	for(i = 0; i < towersCount; i++) {
		_towers[i].number = i+1;
		_towers[i].top = NULL;
	}

	for(i = discsCount-1; i >= 0; i--) {
		insertDics(i+1, &_towers[data[i]]);
	}

	return _towers;
}

int loopDetected(Stack* stack) {
	StackItem * current, * tmp;
	int loop;
	current = stack->top;

	if(stack == NULL) {
		return 0;
	}

	for(tmp = current->next, loop = 0; tmp && loop < 20000; tmp = tmp->next, loop++) {
		if(compareStates(current->data, tmp->data)) {
			return 1;
		}
	}
	return 0;
}

void processStepWithStack(struct SolutionQueue* sq) {
	Stack * stack;
	int counter;
	stack = initializeStack();
	counter = 0;

	/* initial state */
	push(serializeState(towers), 0);

	while(!isStackEmpty()) {
		int step, iStart, jStart, i, moved = 0;
		int* stack_data;
		Tower* _towers;

		if(++counter % 10000 == 0) {
			printf(".");
		}

		stack_data = top(&step, &iStart, &jStart);

		/*printf("STEP %i, num = %i\n", step, stack->num);*/
		_towers = deserializeState(stack_data);

		/*printf("\ni: %i; j: %i\n", iStart, jStart);
				printf("----------\n");
				printState(_towers, towersCount);
				printf("----------\n");*/

		if(step > max || loopDetected(stack)) {
			/* not a perspective branch solution */
			pop();
			continue;
		}

		/* deserialize */

		if (isDestTowerComplete(&_towers[destTower - 1], discsCount)) {
			printf("\n\n\n\n\n\n\n\n\n\n\n\n-----------------------------------------------------------------------------------------FOUND %i\n", step);
			printState(_towers, towersCount);

			if (step < minSteps) {
				minSteps = step;
				inspectStack(stack, sq);
			}

			pop();

			if (step <= min) {
				break;
			} else {
				continue;
			}
		}

		for(i = iStart; i < towersCount; i++) {
			int j;
			for(j = jStart; j < towersCount; j++) {
				int resultDisc;
				if(i == j) {
					continue;
				}

				resultDisc = move(&_towers[i],&_towers[j]);

				if(resultDisc > 0) {
					int* d = serializeState(_towers);
					if(j+1 >= towersCount) {
						setState(i+1, 0);
					} else {
						setState(i, j+1);
					}
					if(moved == 0) {
						push(d, step+1);
						moved++;
					}
					/*undoMove(&_towers[i],&_towers[j]);*/
					break;
				}
				jStart = 0;
			}
			if(moved > 0) {
				break;
			}
		}
		if(moved == 0) {
			pop();
		}
		free(_towers);
	}
	freeStack();
}

void describeMove(int* prevState, int* currentState, int* disc, int* sourceTower, int* destTower) {
	int i;
	for(i = 0; i < discsCount; i++) {
		if(prevState[i] != currentState[i]) {
			*disc = i+1;
			*sourceTower = prevState[i] + 1;
			*destTower = currentState[i] + 1;
			return;
		}
	}
	*disc = *sourceTower = *destTower = -1;
}

int compareStates(int* prevState, int* currentState) {
	int i;
	for(i = 0; i < discsCount; i++) {
		if(prevState[i] != currentState[i]) {
			return 0;
		}
	}
	return 1;
}

void inspectStack(Stack * stack, struct SolutionQueue* sq) {
	int* currentState;
	StackItem * tmp;
	ProcessItem * n;
	n = NULL;

	freeInspectStack(sq);
	if(sq->head != NULL) {
		printf("KUA");
	} else {
		printf("OKOK");
	}

	tmp = stack->top;
	currentState = stack->top->data;

	for(tmp = tmp->next; tmp; tmp = tmp->next) {
		n = (ProcessItem*) malloc(sizeof(* n));
		describeMove(tmp->data, currentState, &n->disc, &n->sourceTower, &n->destTower);
		currentState = tmp->data;
		n->next = sq->head;
		sq->head = n;
	}
}

void freeInspectStack(struct SolutionQueue* sq) {
	ProcessItem * next;
	next = sq->head;
	while(next != NULL) {
		ProcessItem * tmp;
		tmp = next->next;
		free(next);
		next = tmp;
	}
	sq->head = NULL;
}


int process(Tower *_towers, int _size, int _discsCount, int _destTower) {
	struct SolutionQueue sq;

	printf("\nPROCESS:\n");
	towersCount = _size;
	towers = _towers;
	discsCount = _discsCount;
	destTower = _destTower;

	sq.head = NULL;

	min = minMoves(towers, towersCount, discsCount, destTower);
	max = minSteps = maxMoves(discsCount, towersCount);
	max = 30;

	printf("\nmin: %i, max: %i , %i", min, max, minSteps);

	processStepWithStack(&sq);

	if(minSteps <= max) {
		ProcessItem* pi;
		pi = sq.head;
		printf("\n\nDONE, Steps: %i\n\n", minSteps);
		while(pi != NULL) {
			printProcessItem(pi);
			pi = pi->next;
		}
	} else {
		printf("\nERROR: No solution found\n");
	}

	return minSteps;
}

/*
static int**  serializeStateMatrix() {
	int ** stack_item, i;
	stack_item = (int**) malloc(discsCount * sizeof(int));
	if (stack_item == NULL) {
		perror("ERROR: stack_item row could not be allocated");
	}
	for (i = 0; i < discsCount ; i++) {
		stack_item[i] = (int *)malloc(sizeof(int) * 2);
		if (stack_item[i] == NULL)
		{
			perror("ERROR: stack_item column could not be allocated");
		}
	}


	return stack_item;
}
*/
