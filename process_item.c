/*
 * stack_item.c
 *
 */

#include "process_item.h"
#include <stdio.h>

void printProcessItem(ProcessItem* processItem) {
	printf("MOVE: disc(%i), source tower: %i, dest tower: %i", processItem->disc, processItem->sourceTower, processItem->destTower);
}

