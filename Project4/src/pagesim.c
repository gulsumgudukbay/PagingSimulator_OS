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

//queue
struct node
{
	struct node* next;

	uint32_t addr;
};

struct node* queue_head = 0;
struct node* queue_tail = 0;

void enqueue( uint32_t addr)
{
	struct node* temp;

	if( !queue_head)
	{
		queue_head = (struct node*) malloc( sizeof(struct node));
		queue_head->next = 0;
		queue_head->addr = addr;
		queue_tail = queue_head;
		return;
	}

	temp = queue_tail;
	queue_tail->next = (struct node*) malloc( sizeof(struct node));
	queue_tail = queue_tail->next;
	queue_tail->addr = addr;
}

uint32_t dequeue( int* err)
{
	uint32_t addr;
	struct node* temp;

	*err = 0;
	if( !queue_head)
	{
		*err = 1;
		return 0;
	}

	addr = queue_head->addr;
	temp = queue_head;
	queue_head = queue_head->next;

	free( temp);
	return addr;
}

void memory_access_fifo( uint32_t addr)
{

}

int main(void)
{
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
