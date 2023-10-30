#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <string.h>
#include "debugmalloc.h"
#include "dmhelper.h"
#include <stdio.h>

#define HEADERSIZE sizeof(struct header)
#define FOOTSIZE sizeof(struct foot)
#define TESTNUMBER 0xCCDEADCC
#define EIGHT 8
int allocatedSize = 0;
struct header {
    int checksum;
    size_t size;
    int fence;
};
struct foot {
    int fence;
};
typedef struct block_node   block_list;
struct block_node {
    void* ptr;
    int size;
    char* filename;
    int linenum;
    block_list* next;
};

block_list* headOfList; //the head of the list 
block_list* find_block(void* ptr);//function: find a block

/*
*add the block to the list
*/
int add_block_to_list(void* ptr, int size, char* filename, int linenum) {
    block_list* newblock;
    char* t;

    newblock = (block_list*)malloc(sizeof(struct block_node));
    if (!newblock) {
        return -1;
    }
    t = (char*)malloc(strlen(filename) + 1);
    if (!t) {
        free(newblock);
        return -1;
    }
    strcpy(t, filename);
    newblock->ptr = ptr;
    newblock->size = size;
    newblock->filename = t;
    newblock->linenum = linenum;
    newblock->next = headOfList;
    headOfList = newblock;
    return 0;
}

/*
*
*remove the block in the list
*/
int remove_block_in_list(void* ptr)
{
    block_list* current = headOfList;
    block_list* precurrent = NULL;

    while (current) {
        if (current->ptr == ptr) {
            if (!precurrent) {   //delete the head!
                headOfList = current->next; //the next block of currblock    
            }
            else {
                precurrent->next = current->next;  //the next block of prevblock    
            }
            free(current->filename);
            free(current);
            return 0;
        }
        precurrent = current;     //point to a node before current
        current = current->next;
    }
    return -1;
}

/* returns number of bytes allocated using MyMalloc/MyFree:
    used as a debugging tool to test for memory leaks */
int AllocatedSize() {
    return allocatedSize;
}

/*
*
*check different ERRORS of blocks
*
*/
int checkBlock(void* ptr)
{
    struct header* headptr = (struct header*)ptr - 1;   //-1??
    struct foot* footptr;
    int temp = TESTNUMBER;
    int size = headptr->size, amount;

    if (headptr->fence != TESTNUMBER) {  //check head fence
        return 1;
    }
    if (headptr->checksum != countBit(headptr->size) + countBit(headptr->fence)) {
        return 3;
    }
    size += EIGHT - 1;
    size -= size % EIGHT;//size is times of 8    
    footptr = (struct foot*)((char*)ptr + size);
    if (footptr->fence != TESTNUMBER) {  //the element of fence, ckeck footer   
        return 2;
    }

    /* check area before footer if not part of payload */
    amount = (EIGHT - (headptr->size % EIGHT));
    if (amount > 0) {  //greater than zero    
        if (amount <= 4) {
            if (strncmp((char*)footptr - amount, (char*)&temp, amount) != 0) {
                return 2;
            }
        }
        else {
            if (strncmp((char*)footptr - amount, (char*)&temp, 4) != 0
                || strncmp((char*)footptr - amount + 4, (char*)&temp, amount - 4) != 0) {
                return 2;
            }
        }
    }

    return 0;
}

/*find the block*/
block_list* find_block(void* ptr) {
    block_list* curr = headOfList;
    while (curr) {
        if (curr->ptr == ptr) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;  //return the null pointer    
}
/*count number from a specified number*/
int countBit(int t)
{
    int k = 0x11 | (0x11 << 8);
    int a = k | (k << 16);
    int s = t & a;
    s += t >> 1 & a;
    s += t >> 2 & a;
    s += t >> 3 & a;
    s = s + (s >> 16);

    /* Low order 16 bits now consists of 4 sums,
       each ranging between 0 and 8.
       Split into two groups and sum */
    a = 0xF | (0xF << 8);
    s = (s & a) + ((s >> 4) & a);
    return (s + (s >> 8)) & 0x3F;

}



/* Optional functions */

/* Prints a list of all allocated blocks with the
    filename/line number when they were MALLOC'd */
void PrintAllocatedBlocks() {
    block_list* currblock = headOfList;

    while (currblock) {
        printf("filename: %s, linenumber: %d", currblock->filename, currblock->linenum);
        currblock = currblock->next;
    }
    return;

}

/* Goes through the currently allocated blocks and checks
    to see if they are all valid.
    Returns -1 if it receives an error, 0 if all blocks are
    okay.
*/
int HeapCheck() {

    block_list* currblock = headOfList;
    int ret = 0, err;
    while (currblock) {
        if (err = checkBlock(currblock->ptr)) {
            ret = -1;
            PRINTERROR(err, currblock->filename, currblock->linenum);
        }
        currblock = currblock->next;
    }
    return ret;
}


void* MyMalloc(size_t size, char* filename, int linenumber) {

    struct header h;
    struct foot f;
    char* allptr;
    char* footptr;
    int amount;
    int temp;
    h.size = size;
    h.fence = TESTNUMBER;
    f.fence = TESTNUMBER;
    h.checksum = countBit(size) + countBit(h.fence);/*the numbers it contains 1 in size and fence in head*/
    size += EIGHT - 1;
    size -= size % EIGHT; /*size is 8 times now*/
    allptr = malloc(size + HEADERSIZE + FOOTSIZE);   /*the total space,and allstr is its first address,the space is times of 8*/
    if (!allptr)
        return NULL;
    footptr = allptr + HEADERSIZE + size;

    memcpy(allptr, &h, HEADERSIZE);
    memcpy(footptr, &f, FOOTSIZE);
    amount = EIGHT - size % EIGHT;//the blank in the size
    temp = TESTNUMBER;
    if (amount > 0) {   /*fill in the blank in the size!*/
        strncpy(footptr - amount, &temp, amount);
        if (amount <= 4) {
            strncpy(footptr - amount, (char*)&temp, amount);
        }
        else {
            strncpy(footptr - amount, (char*)&temp, 4);
            strncpy(footptr - amount + 4, (char*)&temp, amount - 4);
        }
    }
    allocatedSize += h.size;
    if (add_block_to_list((void*)(allptr + HEADERSIZE), size, filename, linenumber) == 0)
        return (void*)(allptr + HEADERSIZE);//指向中间的size块
    else
    {
        free(allptr);
        return NULL;
    }
}
/*free a block*/
void MyFree(void* ptr, char* filename, int linenumber) {

    struct header* headptr = (struct header*)ptr - 1;
    block_list* theBlock = NULL;
    int a;
    theBlock = find_block(ptr);
    if (!(theBlock)) {
        error(4, filename, linenumber);
    }

    if (a = checkBlock(ptr)) { /*get different kind of blocks */
        errorfl(a, theBlock->filename, theBlock->linenum, filename, linenumber);
    }
    /*reduce the number of blocks*/
    allocatedSize -= headptr->size;
    free(headptr);
    remove_block_in_list(ptr);
}