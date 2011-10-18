/*
 * main.c
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tower.h"
#include "state_printer.h"
#include "analyser.h"
#include "processor.h"


int main(int argc, char *argv[]) {
	Tower *towers;
	int discsCount, towersCount, destTower;
	static const char filename[] = "enter.txt";
	static const char discDelimiter[] = ",";
	FILE *file = fopen(filename, "r");

	if ( file != NULL ) {
	      char line [ 128 ]; /* max line size */

	      int i;
	      fgets(line, sizeof line, file);
	      sscanf(line, "%i", &discsCount);
	      fgets(line, sizeof line, file);
	      sscanf(line, "%i", &towersCount);
	      fgets(line, sizeof line, file);
	      sscanf(line, "%i", &destTower);

	      printf("Towers of Hanoi: %i towers, %i discs, %i dest tower\n", towersCount, discsCount, destTower);

	      towers = (Tower*) malloc (towersCount * sizeof(*towers));

	      for (i = 0; i < towersCount; i++) {
	    	  char towerLine [ 128 ];
	    	  Tower tower = {0, NULL};
	    	  char *disc;

	    	  tower.number = i+1;

	    	  if(fgets(towerLine, sizeof towerLine, file) == NULL) {
	    		  return 1; /* bad enter */
	    	  }

	    	  printf("Created new tower: %i\n", tower.number);

	    	  disc = strtok(towerLine, discDelimiter);
	    	  while (1) {
	    		  int discSize = 0;

	    		  if (disc == NULL) {
	    			  break;
	    		  }

	    		  sscanf(disc, "%i", &discSize);

	    		  if (discSize == 0) {
	    			  break;
	    		  }

	    		  insertDics(discSize, &tower);
	    		  printf("Inserted disc of size: %i to tower %i\n", discSize, tower.number);
	    		  disc = strtok(NULL, discDelimiter);
	    	  }
	    	  towers[i] = tower;
	      }
	      fclose(file);

	      printState(towers, towersCount);

	      printf("min steps: %i\n", process(towers, towersCount, discsCount, destTower));

	      for(i = 0; i < towersCount; i++) {
	    	  freeDiscs(&towers[i]);
	      }
	      free(towers);
	      return 0;

	}
	perror ("enter.txt could not be opened" );
	return 1;
}
