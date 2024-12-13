#include <stdio.h>
#include <stdlib.h>


int main(int argc, char **argv)
{

	int *array = (int *) malloc(20*sizeof(int));
	
	array[0] = 1;
	array[1] = 1;
	
	for(int i=2; i<20; i++) {
        	array[i] = array[i-1] + array[i-2];
	}
	array[20] = 20;
	
	/* No se libera la memoria antes de salir */
	return 0;
}
