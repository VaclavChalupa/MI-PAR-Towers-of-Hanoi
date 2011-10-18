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

	/*
	for(i = 0; i < discsCount; i++) {
		printf("DES: \n\n\n%i - %i\n", i+1, data[i]+1);
	}*/

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
	stack = initializeStack();

	/* initial state */
	push(serializeState(towers), 0);

	while(!isStackEmpty()) {
		int step;
		int i;
		int* stack_data;
		Tower* _towers;

		stack_data = top(&step);
		printf("STEP %i, num = %i\n", step, stack->num);

		if(step > max || loopDetected(stack)) {
			/* not a perspective branch solution */
			pop();
			continue;
		}

		/* deserialize */
		_towers = deserializeState(stack_data);

		if (isDestTowerComplete(&_towers[destTower - 1], discsCount)) {
			printf("FOUND");
			printState(_towers, towersCount);
			if (step < minSteps) {
				minSteps = step;
			}
			inspectStack(stack, sq);

			pop();
			if (step <= min) {
				break;
			} else {
				continue;
			}
		}

		for(i = 0; i < towersCount; i++) {
			int j = 0;
			for(j = 0; j < towersCount; j++) {
				int resultDisc;
				if(i == j) {
					continue;
				}

				resultDisc = move(&_towers[i],&_towers[j]);

				if(resultDisc > 0) {
					push(serializeState(_towers), step+1);
					undoMove(&_towers[i],&_towers[j]);
				}
			}
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
			*sourceTower = prevState[i];
			*destTower = currentState[i];
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
	ProcessItem ** wr, * n;
	n = NULL;

	tmp = stack->top;
	wr = &sq->head;
	currentState = stack->top->data;

	printf("\nstep: %i", tmp->step);

	for(tmp = tmp->next; tmp; tmp = tmp->next) {
		n = (ProcessItem*) malloc(sizeof(* n));
		describeMove(tmp->data, currentState, &n->disc, &n->sourceTower, &n->destTower);
		currentState = tmp->data;
		*wr = n;
		wr = &n->next;
	}
	*wr = NULL;
}


int process(Tower *_towers, int _size, int _discsCount, int _destTower) {
	struct SolutionQueue sq;

	printf("\nHAHA: PROCESS\n");
	towersCount = _size;
	towers = _towers;
	discsCount = _discsCount;
	currentSteps = 0;
	destTower = _destTower;
	minSteps = 0;

	sq.head = NULL;

	min = minMoves(towers, towersCount, discsCount, destTower);
	max = maxMoves(discsCount, towersCount);
	max = 1;

	processStepWithStack(&sq);

	if(minSteps > -1) {
		ProcessItem* pi;
		pi = sq.head;
		printf("\nDONE, Steps: %i\n\n", minSteps);
		while(pi != NULL) {
			printProcessItem(pi);
			pi = pi->next;
		}
		if(sq.head == NULL) printf("KURVA");
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
