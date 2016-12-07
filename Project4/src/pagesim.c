/*
 ============================================================================
 Name        : pagesim.c
 Author      : Gulsum Gudukbay & Dogukan Yigit Polat
 Version     :
 Copyright   :
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

	printf("enqweqwe %d\n", addr);

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
		return -1;
	}

	addr = queue_head->addr;
	temp = queue_head;
	queue_head = queue_head->next;

	free( temp);
	return addr;
}

int requeue( uint32_t addr)
{
	struct node* cur, *temp;
	struct node* prev;

	cur = queue_head;
	prev = cur;
	while( cur)
	{
		if( cur->addr == addr)
		{

			if( cur == queue_tail)
				queue_tail = prev;

			//ananı siktim
			if( cur == queue_head)
			{
				queue_head = cur->next;
				free( cur);
				enqueue(addr);
				return 0;
			}

			temp = cur;
			cur = cur->next;
			prev->next = cur;
			free(temp);
			enqueue( addr);
			return 0;
		}
		prev = cur;
		cur = cur->next;
	}

	return -1;
}
///////////////////////////////////////////////////////////////////////////////////
struct out_table_entry
{
	struct in_table_entry* inner_table;
	int valid;
};

struct in_table_entry
{
	uint32_t frame_number;
	int valid;
};

struct out_table_entry out_table[1024];
int next_empty = 0;
int M;

void memory_access_fifo( uint32_t addr)
{
	uint32_t page_offset;
	uint32_t inner_table;
	uint32_t outer_table;

	//evil bit level integer hacking
	page_offset = addr & (0x00000FFF);
	inner_table = (addr & (0x003FF000)) >> 12;
	outer_table = (addr & (0xFFC00000)) >> 22;

	if( out_table[outer_table]->valid)
	{
		if( out_table[outer_table]->inner_table[inner_table]->valid)
		{
			uint32_t fnum = out_table[outer_table]->inner_table[inner_table]->frame_number;
			uint32_t phys_addr = ((fnum << 12) & 0xFFFFF000) | page_offset;
			printf( "%x\n", phys_addr);
			requeue(addr);
		}
		else
		{
			if( next_empty < M)
			{
				out_table[outer_table]->inner_table[inner_table]->frame_number = next_empty;
				out_table[outer_table]->inner_table[inner_table]->valid = 1;
				next_empty++;
				enqueue(addr);
			}
			else
			{
				int err;
				uint32_t victim = dequeue( &err);
				if( !err)
				{
					printf( "victim is %x", victim);
					out_table[(victim & (0xFFC00000)) >> 12]->inner_table[(victim & (0x003FF000)) >> 22]->valid = 0;
					out_table[outer_table]->inner_table[inner_table]->frame_number = victim;
					out_table[outer_table]->inner_table[inner_table]->valid = 1;
					enqueue(addr);
				}
				else
				{
					printf("what is going on?\n");
					return;
				}
			}
		}
	}
	else
	{
		//create inner table.
		out_table[outer_table]->inner_table = (struct in_table_entry*) malloc( sizeof(in_table_entry) * 1024);
		for( int i = 0; i < 1024; i++)
			out_table[outer_table]->inner_table[i]->valid = 0;

		if( next_empty < M)
		{
			out_table[outer_table]->inner_table[inner_table]->frame_number = next_empty;
			out_table[outer_table]->inner_table[inner_table]->valid = 1;
			next_empty++;
			enqueue(addr);
		}
		else
		{
			int err;
			uint32_t victim = dequeue( &err);
			if( !err)
			{
				printf( "victim is %x", victim);
				out_table[(victim & (0xFFC00000)) >> 12]->inner_table[(victim & (0x003FF000)) >> 22]->valid = 0;
				out_table[outer_table]->inner_table[inner_table]->frame_number = victim;
				out_table[outer_table]->inner_table[inner_table]->valid = 1;
				enqueue(addr);
			}
			else
			{
				printf("what is going on?\n");
				return;
			}
		}

	}


}

int main(void)
{
	M = 10;
	for( int i = 0; i < 1024; i++)
		out_table[i]->valid = 0;

	return 0;
}
