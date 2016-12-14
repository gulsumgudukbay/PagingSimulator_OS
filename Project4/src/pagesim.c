/*
 ============================================================================
 Name        : pagesim.c
 Author      : Gulsum Gudukbay & Dogukan Yigit Polat
 Version     :
 Copyright   :
 ============================================================================
 */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <getopt.h>

int next_empty = 0;

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

	printf( "enqueue 0x%08x\n", addr);
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

			printf( "r-");
			if( cur == queue_tail)
				queue_tail = prev;


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
int M;

void memory_access( uint32_t addr, int isLRU, FILE* fp)
{
	uint32_t page_offset;
	uint32_t inner_table;
	uint32_t outer_table;
	//evil bit level integer hacking
	page_offset = addr & (0x00000FFF);
	inner_table = (addr & (0x003FF000)) >> 12;
	outer_table = (addr & (0xFFC00000)) >> 22;

	if( !out_table[outer_table].used)
	{
		printf("SEGMANTATION FAULT\n");
		return;
	}
	else if( !out_table[outer_table].inner_table[inner_table].used)
	{
		printf("SEGMANTATION FAULT\n");
		return;
	}

	if( out_table[outer_table].inner_table[inner_table].valid)
	{
		if( isLRU)
			requeue(addr);

		uint32_t fnum = out_table[outer_table].inner_table[inner_table].frame_number;
		uint32_t phys_addr = ((fnum << 12) & 0xFFFFF000) | page_offset;
		printf( "0x%08x\n", phys_addr);
	  fprintf(fp,  "0x%08x\n", phys_addr);
	}
	else
	{
		if( next_empty < M)
		{
			out_table[outer_table].inner_table[inner_table].frame_number = next_empty;
			out_table[outer_table].inner_table[inner_table].valid = 1;
			next_empty++;
			enqueue(addr);

			uint32_t fnum = out_table[outer_table].inner_table[inner_table].frame_number;
			uint32_t phys_addr = ((fnum << 12) & 0xFFFFF000) | page_offset;
			printf( "0x%08x x\n", phys_addr);
			fprintf(fp,  "0x%08x x\n", phys_addr);

		}
		else
		{
			int err;
			uint32_t victim = dequeue( &err);
			if( !err)
			{
				printf( "victim is 0x%08x\n", victim);
				out_table[(victim & (0xFFC00000)) >> 22].inner_table[(victim & (0x003FF000)) >> 12].valid = 0;
				out_table[outer_table].inner_table[inner_table].frame_number = victim >> 12;
				out_table[outer_table].inner_table[inner_table].valid = 1;
				enqueue(addr);

				uint32_t fnum = out_table[outer_table].inner_table[inner_table].frame_number;
				uint32_t phys_addr = ((fnum << 12) & 0xFFFFF000) | page_offset;
				printf( "0x%08x x\n", phys_addr);
				fprintf(fp,  "0x%08x x\n", phys_addr);

			}
			else
			{
				printf("what is going on?\n");
				return;
			}
		}
	}

	out_table[outer_table].inner_table[inner_table].valid = 1;

}

struct range_node{
	uint32_t X;
	uint32_t Y;
	struct range_node *next;
};

struct in_table_entry* create_inner_table()
{
	struct in_table_entry* table = (struct in_table_entry*) malloc(sizeof(struct in_table_entry)*1024);
	for(int i = 0; i < 1024; i++)
	{
		table[i].valid = 0;
		table[i].used = 0;
	}
	return table;
}

struct range_node * read_ranges( FILE* input1)
{
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

	return ranges;
}

void init( struct range_node * ranges)
{
	uint32_t Xin, Yin;
	uint32_t Xout, Yout;
	uint32_t X, Y;
	struct range_node *cur;


	for(int i = 0; i < 1024; i++)
	{
		out_table[i].valid = 0;
		out_table[i].used = 0;
		out_table[i].inner_table = 0;
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

		out_table[Xout].used = 1;


		if(!out_table[Xout].inner_table)
			out_table[Xout].inner_table = create_inner_table();

		if(Yout - Xout > 0)
		{

			for(int i = Xin; i < 1024; i++)
			{
				out_table[Xout].inner_table[i].used = 1;
			}

			for(int i = Xout+1; i < Yout; i++)
			{
				out_table[i].used = 1;

				if(!out_table[i].inner_table)
					out_table[i].inner_table = create_inner_table();

				for(int j = 0; j < 1024; j++)
				{
					out_table[i].inner_table[j].used = 1;
				}
			}

			out_table[Yout].used = 1;

			if(!out_table[Yout].inner_table)
				out_table[Yout].inner_table = create_inner_table();

			for(int i = 0; i < Yin; i++)
			{
				out_table[Yout].inner_table[i].used = 1;
			}

		}
		else
		{
			for(int i = Xin; i < Yin; i++)
			{
				out_table[Xout].inner_table[i].used = 1;
			}
		}
	}
}

int arguments( int argc, char** argv, uint32_t* vmsize, int* M,
							 char** input1, char** input2, char** avutbud, int* algo)
{
	int isRandom;
	int r_index;
	int a_index;

	isRandom = 0;
	for( int i = 0; i < argc; i++)
	{
		if( strcmp(argv[i], "-r") == 0)
		{
			isRandom = 1;
			r_index = i;
		}

		if( strcmp(argv[i], "-a") == 0)
		{
			a_index = i;
		}
	}

	if( isRandom)
	{
		*input2 = argv[1];
		*M = atoi(argv[2]);
		*avutbud = argv[3];
		*algo = atoi(argv[a_index + 1]);
		*vmsize = atoi(argv[r_index + 1]);
	}
	else
	{
		*input1 = argv[1];
		*input2 = argv[2];
		*M = atoi(argv[3]);
		*avutbud = argv[4];
		*algo = atoi(argv[a_index + 1]);
	}

	return isRandom;
}

int main(int argc, char** argv)
{
	FILE* in1;
	FILE* in2;
	FILE* out;

	uint32_t vmsize;
	int a;
	char* out_name;
	char* in1_name;
	char* in2_name;
	int isRandom;

	if( argc != 8 && argc != 7)
	{
		if( argc != 8)
		{
			printf("involix inbudu\n");
			return 1;
		}
	}

	isRandom = arguments( argc, argv, &vmsize, &M, &in1_name,
												&in2_name, &out_name, &a);



	in2 = fopen( in2_name, "r+");
	out	= fopen( out_name, "w+");
	if( isRandom)
	{
		if( argc != 8)
		{
			printf("involix inbudu\n");
			return 1;
		}

		printf("\nin2: %s \nout: %s \nM: %d \nalgo: %d \nvmsize: %d \n", in2_name, out_name, M, a, vmsize);

		struct range_node myrange;
		myrange.next = 0;
		myrange.X = 0;
		myrange.Y = vmsize;
		init(&myrange);

	}
	else
	{
		if( argc != 7)
		{
			printf("involix inbudu\n");
			return 1;
		}

		printf("\nin1: %s \nin2: %s \nout: %s \nM: %d \nalgo: %d \n", in1_name, in2_name, out_name, M, a);

		in1 = fopen( in1_name, "r+");
		struct range_node* rng = read_ranges( in1);
		init(rng);

	}


	//init(&myrange);
	uint32_t inputtanokunancisim;

	while(fscanf(in2, "%x", &inputtanokunancisim) != EOF)
	{
		memory_access(inputtanokunancisim, a, out);
	}

	fclose(out);
	return 0;
}
