#ifndef EXODIA_LIB_H
#define EXODIA_LIB_H


// Standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


// Boolean type
#define bool _Bool
#define true 1
#define false 0


// Types of fixed size integers
#define uint8 unsigned __int8
#define uint16 unsigned __int16
#define uint32 unsigned __int32
#define uint64 unsigned __int64
#define int8 __int8
#define int16 __int16
#define int32 __int32
#define int64 __int64


// Error message with variadic parameters
#define error(msg,...) ({fprintf(stderr,"COMPILATION ERROR:\n");fprintf(stderr,(msg),##__VA_ARGS__);fprintf(stderr,"\n\nCOMPILATION FAILED!\n");exit(-1);})


// A string of characters, represented like a vector
struct String{
	char* arr;
	int size;
};


// String macros
// The amount of characters we append to the string everytime we format the input string
int _STRING_APPEND=20;
// Concatenate a formatted or non-formatted string to the end of the string
#define pushback_string(d,s,...) \
({\
	(d).arr=(char*)realloc((d).arr,(d).size+strlen((s))+_STRING_APPEND);\
	(d).size+=sprintf((d).arr+(d).size-1,s,##__VA_ARGS__);\
	(d).arr[(d).size-1]='\0';\
})
// Set the string to the value of a formatted or non-formatted string
#define set_string(d,s,...) \
({\
	(d).size=strlen(s)+_STRING_APPEND;\
	(d).arr=(char*)realloc((d).arr,(d).size);\
	(d).size=sprintf((d).arr,s,##__VA_ARGS__);\
	(d).arr=(char*)realloc((d).arr,(d).size);\
})
// Copy the contents of a string to another
#define copy_string(d,s) ({set_string((d),(s));})
// Free the string
#define free_string(s) ({if((s).arr != NULL)free((s).arr);})


// Vector macros
// Push an element at the end of a vector
#define pushback(a,b) ({(a).arr=realloc((a).arr,sizeof(typeof(b))*(++(a).size));(a).arr[(a).size-1]=(b);})
// Pop the element at the end of a vector
#define popback(a) ({(a).arr=realloc((a).arr,sizeof(typeof(*(a).arr))*(--(a).size));})
// Get the n'th element of vector a
#define at(a,n) ((a).arr[(n)])
// Get the last element of a vector
#define at_back(a) ((a).arr[(a).size-1])
// Free a simple vector (those whose elements do not themselves have allocated memory)
#define free_vector(a) ({if((a).arr != NULL)free((a).arr);})


// Straight up arena allocator
struct ArenaAlloc{
	char* ptr;
	size_t pos;
	size_t size;
};
#define arena_free(a) ({if((a).ptr != NULL) free((a).ptr);(a).ptr=NULL;})
void arena_init(struct ArenaAlloc* a, size_t sz){
	if(a->ptr != NULL) free(a->ptr);
	*a=(struct ArenaAlloc){(char*)malloc(sz),0,sz};
	if(a->ptr == NULL) error("Arena allocator : failed to allocate %d bytes of memory!\n",(int)sz);
}
void* arena_alloc(struct ArenaAlloc* a, size_t sz){
	if(a->ptr == NULL || a->size < 1) return NULL;
	a->pos+=sz;
	if(a->pos > a->size) { arena_free(*a); error("Arena allocator : memory overflow of %d bytes!\n",(int)(a->pos-a->size)); }
	return a->ptr+a->pos;
}

// Token handling
#include "tokenization.h"

// Parsing tokens and creating a parse tree
#include "parser.h"

// Generate the code using the parse tree
#include "generation.h"

#endif