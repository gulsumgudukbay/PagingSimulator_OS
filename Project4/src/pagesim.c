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
	int used;
};

struct in_table_entry
{
	uint32_t frame_number;
	int valid;
	int used;
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

	if( out_table[outer_table].valid)
	{
		if( out_table[outer_table].inner_table[inner_table].valid)
		{
			uint32_t fnum = out_table[outer_table].inner_table[inner_table].frame_number;
			uint32_t phys_addr = ((fnum << 12) & 0xFFFFF000) | page_offset;
			printf( "%x\n", phys_addr);
			requeue(addr);
		}
		else
		{
			if( next_empty < M)
			{
				out_table[outer_table].inner_table[inner_table].frame_number = next_empty;
				out_table[outer_table].inner_table[inner_table].valid = 1;
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
					out_table[(victim & (0xFFC00000)) >> 12].inner_table[(victim & (0x003FF000)) >> 22].valid = 0;
					out_table[outer_table].inner_table[inner_table].frame_number = victim;
					out_table[outer_table].inner_table[inner_table].valid = 1;
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
		out_table[outer_table].inner_table = (struct in_table_entry*) malloc( sizeof(struct in_table_entry) * 1024);
		for( int i = 0; i < 1024; i++)
			out_table[outer_table].inner_table[i].valid = 0;

		if( next_empty < M)
		{
			out_table[outer_table].inner_table[inner_table].frame_number = next_empty;
			out_table[outer_table].inner_table[inner_table].valid = 1;
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
				out_table[(victim & (0xFFC00000)) >> 12].inner_table[(victim & (0x003FF000)) >> 22].valid = 0;
				out_table[outer_table].inner_table[inner_table].frame_number = victim;
				out_table[outer_table].inner_table[inner_table].valid = 1;
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

struct range_node{
	uint32_t X;
	uint32_t Y;
	struct range_node *next;
};


void init( FILE* input1)
{
	uint32_t Xin, Yin;
	uint32_t Xout, Yout;
	uint32_t X, Y;

	struct range_node *ranges, *cur;
	ranges = 0;

	while(fscanf(input1, "%x %x", &X, &Y) != EOF)
	{
		cur = (struct range_node*)malloc(sizeof(struct range_node));
		cur->X = X;
		cur->Y = Y;
		cur->next = ranges;
		ranges = cur;
	}

	for(cur = ranges; cur; cur = cur->next)
	{
		X = cur->X;
		Y = cur->Y;

		//evil bit level integer hacking
		Xin = (X & (0x003FF000)) >> 12;
		Xout = (X & (0xFFC00000)) >> 22;
		Yin = (Y & (0x003FF000)) >> 12;
		Yout = (Y & (0xFFC00000)) >> 22;

		printf("%8x %8x: %x %x %x %x\n", X, Y, Xout, Yout, Xin, Yin);

		//yarraaan basi
		out_table[Xout].used = 1;
		out_table[Xout].valid = 0;
		out_table[Xout].inner_table = (struct in_table_entry*) malloc( sizeof(struct in_table_entry) * 1024);
		for( uint32_t k = 0; k < 1024; k++)
		{
			out_table[Xout].inner_table[k].used = ( k >= Xin ? 1 : 0);
			out_table[Xout].inner_table[k].valid = 0;
		}

		//ortasi
		for( uint32_t j = Xout+1; j < Yout-1 && Yout > 0; j++)
		{
			out_table[j].used = 1;
			out_table[j].valid = 0;
			out_table[j].inner_table = (struct in_table_entry*) malloc( sizeof(struct in_table_entry) * 1024);
			for( uint32_t k = 0; k < 1024; k++)
			{
				out_table[j].inner_table[k].used = 1;
				out_table[j].inner_table[k].valid = 0;
			}
		}

		//yarraaan sapi
		if( Yout > 0)
		{
			out_table[Yout-1].used = 1;
			out_table[Yout-1].valid = 0;
			out_table[Yout-1].inner_table = (struct in_table_entry*) malloc( sizeof(struct in_table_entry) * 1024);
			for( uint32_t k = 0; k < 1024; k++)
			{
				out_table[Yout-1].inner_table[k].used = ( k < Yin ? 1 : 0);
				out_table[Yout-1].inner_table[k].valid = 0;
			}
		}
	}
}

int main(void)
{
	FILE* fp = fopen("in1.txt", "r+");
	init(fp);

	for( uint32_t i = 0; i < 1024; i++)
	{
		if( out_table[i].used)
		{
			for( uint32_t j = 0; j < 1024; j++)
			{
				if( out_table[i].inner_table[j].used)
					printf( "%d: %d \n", i, j);
			}
		}
	}

	return 0;
}
