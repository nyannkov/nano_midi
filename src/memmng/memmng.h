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
#ifndef MEMMNG_H_
#define MEMMNG_H_

typedef struct memblock 
{
	int size;
	unsigned char magic;
	unsigned char occupied;
	struct memblock *next;
	struct memblock *prev;
}memblock_t;

typedef struct memmng 
{
	memblock_t block_chain;
	unsigned char *heap;
	int			heap_size;
}memmng_t;


extern int memmng_init( memmng_t *memmng, unsigned char *heap, int heap_size );
extern void *memmng_alloc( memmng_t *memmng, int size );
extern void memmng_free( memmng_t *memmng, void *block );



#endif /* MEMMNG_H_ */
