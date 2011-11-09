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
	int * stack_item_data, i;
	stack_item_data = (int*) malloc(discsCount * sizeof(int));

	for(i = 0; i < discsCount; i++) {
		stack_item_data[i] = -1;
	}

	for(i = 0; i < towersCount; i++) {
		Disc* disc;
		disc = _towers[i].top;
		while(disc != NULL) {
			stack_item_data[disc->size-1] = i; /* disc size -> tower indexed form 0*/
			disc = disc->next;
		}
	}

	for(i = 0; i < discsCount; i++) {
		if(stack_item_data[i] == -1) {
			perror("ERROR: stack_item_data defect by serialization");
			return NULL;
		}
	}

	return stack_item_data;
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
	push(serializeState(towers), 0, -1); /* no disc moved yet */

	while(!isStackEmpty()) {
		int step, iStart, jStart, prevMovedDisc, i, moved = 0;
		int* stack_data;
		Tower* _towers;

		stack_data = top(&step, &iStart, &jStart, &prevMovedDisc);

		_towers = deserializeState(stack_data);

		if(step > max || loopDetected(stack)) {
			/* not a perspective solution branch */
			pop();
			freeTowers(_towers, &towersCount);
			continue;
		}

		if (isDestTowerComplete(&_towers[destTower - 1], discsCount)) {
			printf("\n\n\n\n------------------------------------FOUND %i\n", step);
			printState(_towers, towersCount);

			if (step < minSteps) {
				max = step;
				minSteps = step;
				inspectStack(stack, sq);
				printf("\nMAX NOW: %i\n", max);
			}

			pop();
			freeTowers(_towers, &towersCount);

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
					if(j+1 >= towersCount) {
						setState(i+1, 0);
					} else {
						setState(i, j+1);
					}
					if(moved == 0) {

						if(prevMovedDisc != resultDisc) {
							push(serializeState(_towers), step+1, resultDisc);
							moved++;
							/*printf("\n\n\n DIFF MOVE: %i - %i", prevMovedDisc, resultDisc);*/
						}
					}
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
		/*  */
		freeTowers(_towers, &towersCount);
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
	max = maxMoves(discsCount, towersCount);
	minSteps = max + 1;
	/*max = 30;*/

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

	freeInspectStack(&sq);

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
