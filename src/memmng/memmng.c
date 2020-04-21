/*
  MIT License

  Copyright (c) 2020 nyannkov

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/
#include "memmng.h"

#ifndef NULL
#define NULL ((void *)0)
#endif


#define MEM_HEADER_SIZE 	(sizeof(memblock_t))
#define MEM_DELTA			(20)
#define MEM_MAGIC			(0xa5)
#define MEM_WORD_SIZE		(sizeof(int))
#define MEM_WORD_ALIGN(n)	(( (n) + MEM_WORD_SIZE - 1 ) & ~(MEM_WORD_SIZE - 1) )


int memmng_init( memmng_t *memmng, unsigned char *heap, int heap_size ) {

	if ( (memmng == NULL) || (heap == NULL) ) {
		return -1;
	}
	// register the memory pool as a heap memory.
	memmng->heap 		= heap;
	memmng->heap_size 	= heap_size;

	memblock_t *h = NULL;

	h = (memblock_t *)heap;

	memmng->block_chain.size 		= sizeof(memmng->block_chain);
	memmng->block_chain.magic 		= MEM_MAGIC;
	memmng->block_chain.occupied 	= 1;
	memmng->block_chain.prev 		= h;
	memmng->block_chain.next		= h;

	h->size 		= heap_size;
	h->magic 		= MEM_MAGIC;
	h->occupied 	= 0;
	h->next 		= &memmng->block_chain;
	h->prev			= &memmng->block_chain;

	return 0;
}

void *memmng_alloc( memmng_t *memmng, int size ) {

	int nbytes 			= 0;
	memblock_t *p 		= NULL;
	memblock_t *p_next 	= NULL;

	if ( memmng == NULL ) {
		return NULL;
	}

	nbytes = MEM_WORD_ALIGN(size + MEM_HEADER_SIZE);

	for ( p = memmng->block_chain.next; p != &memmng->block_chain; p = p->next ) {

		if ( !(p->occupied) &&  (p->size >= nbytes) ) {

			if ( ( p->size - nbytes ) > MEM_DELTA ) {

				p_next 			 = (memblock_t *)((char *)p + nbytes);
				p_next->size 	 = p->size - nbytes;
				p_next->magic 	 = MEM_MAGIC;
				p_next->occupied = 0;

				p->next->prev = p_next;
				p_next->next  = p->next;
				p->next = p_next;
				p_next->prev = p;

				p->size 	= nbytes;
				p->occupied = 1;

			}
			else {
				p->occupied = 1;

			}

			return ((char *)p + MEM_HEADER_SIZE);
		}
	}

	return NULL;
}

void memmng_free( memmng_t *memmng, void *block ) {

	if ( ( memmng == NULL ) || ( block == NULL ) ) {
		return;
	}

	memblock_t *mem = NULL;

	mem = (memblock_t *)( (char *)block - MEM_HEADER_SIZE );

	if ( mem->magic != MEM_MAGIC ) {
		return;
	}

	if ( !(mem->prev->occupied) ) {
		mem->next->prev = mem->prev;
		mem->prev->next = mem->next;
		mem->prev->size += mem->size;
		mem = mem->prev;
	}

	if ( !(mem->next->occupied) ) {
		mem->next->next->prev = mem;
		mem->size += mem->next->size;
		mem->next = mem->next->next;
	}

	mem->occupied = 0;
}


