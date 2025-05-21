#ifndef MEM_H
#define	MEM_H


/*********************************************************************
 * Segment header data type
 ********************************************************************/

#define NEAR
#define	MAX_HEAP_SIZE		0x50


#define	_MAX_SEGMENT_SIZE	0x7F
#define _MAX_HEAP_SIZE 	MAX_HEAP_SIZE-1



/*********************************************************************
 * Reserve the memory heap
 ********************************************************************/
//#pragma 	udata	_SRAM_ALLOC_HEAP
unsigned char _uDynamicHeap[MAX_HEAP_SIZE];


typedef union _SALLOC
{
	unsigned char byte;
	struct _BITS
	{
		unsigned count:7;
		unsigned alloc:1;	
	}bits;
}SALLOC;


unsigned char * SRAMalloc(unsigned char nBytes);
void SRAMfree(unsigned char * pSRAM);
void SRAMInitHeap(void);



#endif	/* MEM_H */

