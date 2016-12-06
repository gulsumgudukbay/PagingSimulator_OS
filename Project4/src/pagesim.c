/*
 ============================================================================
 Name        : Project4.c
 Author      : Gulsum Gudukbay & Dogukan Yigit Polat
 Version     :
 Copyright   :
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

void memory_access_fifo( uint32_t addr)
{
	int can_insert;
	int ins_frame;
	int rem_frame;

	can_insert = 1;
	ins_frame = 0;


}

int main(void)
{
	int M = 5;

	int* valid_frames = (int*) malloc( sizeof(int)*M);
	memset( valid_frames, 0, sizeof(int)*M);



	int err = 0;
	enqueue(31);
	enqueue(69);

	printf("%d\n", dequeue(&err));
	printf("%d\n", dequeue(&err));

	printf("%d\n", dequeue(&err));
	printf("%d\n", dequeue(&err));
	enqueue(3169);
	enqueue(6931);
	enqueue(3131);
	printf("%d\n", dequeue(&err));
	printf("%d\n", dequeue(&err));
	printf("%d\n", dequeue(&err));

	return EXIT_SUCCESS;
}
