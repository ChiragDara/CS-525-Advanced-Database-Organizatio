#include<stdio.h>
#include<stdlib.h>
#include "buffer_mgr.h"
#include "storage_mgr.h"
#include <math.h>

/* 
-- Global variable declaration --
1. bufferSize - Represents the buffer pool size
2. numOfWrites - A counter variable which strores the count of I/O writes to the disk
3. readIndex - A counter variable which stores the count of pages read from the disk
4. frameCount - A counter, which is incremented when a page frame is added into the buffer pool
5. LFUPointer - Used to store the last frequently used page
6. CLOCKPointer - Used to point to the last added page in the buffer pool
*/
int bufferSize = 0;
int numofWrites = 0;
int readIndex = 0;
int frameCount = 0;
int LFUPointer = 0; int CLOCKPointer = 0;
int noValue = 0, oneValue = 1;
int RC_PINNED_PAGES_IN_BUFFER = 16;
int RC_ERROR = 15;

/* 
-- Global variable declaration --
1. bufferSize - Represents the buffer pool size
2. readIndex - A counter variable which stores the count of pages read from the disk
3. numOfWrites - A counter variable which strores the count of I/O writes to the disk
4. frameCount - A counter, which is incremented when a page frame is added into the buffer pool
5. LFUPointer - Used to store the last frequently used page
6. CLOCKPointer - Used to point to the last added page in the buffer pool
*/
typedef struct Page
{
	SM_PageHandle data; // represents actual data of the page
	PageNumber pageNumber; // Identification integer for each page
	int fixCount; // To specify the number of clients using that page at a given instance
	int hitNo;   //  Utilized by LRU algorithm for least recently used page	
	int refNo;   //  Utilized by LFU algorithm for least frequently used page
	int dirtyBit; // To specify whether the contents of the page has been modified
} FramePage;

void FIFO(BM_BufferPool *const bm, FramePage *page){
	//FIFO begins
	FramePage *framePage = (FramePage *) bm -> mgmtData;

	int index = readIndex % bufferSize;
	int newPageNum = page -> pageNumber;
	int newDirtyBit = page -> dirtyBit;
	int newFixCount = page -> fixCount;

	for(int i = 0 ; i < bufferSize ; i++){
		if(framePage[index].fixCount != 0){
			index++;
			if(index % bufferSize == 0){
				index = 0;
			}
			else{
				index = index;
			}
		}
		else if(framePage[index].fixCount == noValue){
			if(framePage[index].dirtyBit == oneValue){
				int pNum = framePage[index].pageNumber;
				SM_FileHandle fHandle;
				SM_PageHandle memPage;
				//Calling the 'openPageFile()' function with the respective parameters
				openPageFile(bm -> pageFile, &fHandle);
				//Calling the 'writeBlock()' function with the respective parameters
				writeBlock(pNum, &fHandle, framePage[index].data);
				// Increasing numOfWrites which records the number of writes done by the buffer manager.
				numofWrites++;
			}
			// Setting page frame's content to new page's content
			framePage[index].data = page -> data;
			framePage[index].pageNumber = newPageNum;
			framePage[index].dirtyBit = newDirtyBit;
			framePage[index].fixCount = newFixCount;
			break;
		}
	}
}

void LFU(BM_BufferPool *const bm, FramePage *page)
{
	//LFU begins
	FramePage *framePage = (FramePage *) bm->mgmtData;
	int bSize = bufferSize;
	bool flag = false;
	int LFURef;
	int LFUIndex = LFUPointer;	
	
	int newPageNum = page -> pageNumber;
	int newDirtyBit = page -> dirtyBit;
	int newFixCount = page -> fixCount;

	int i = 0; 
	int j = 0;
	// This method iterates through all the page frames in the buffer pool
	for(i = 0 ; i < bSize ; i++){
		if(framePage[LFUIndex].fixCount == 0)
		{
			LFUIndex = (LFUIndex + i) % bufferSize;
			LFURef = framePage[LFUIndex].refNo;
			flag = true;
			if(flag){
				break;
			}
		}
	}
	
	i = (LFUIndex + 1) % bufferSize;

	// This method discovers the page frame, which is used the least frequent.
	for(int j = 0 ; j < bSize ; j++){
		if(framePage[i].refNo < LFURef){
			LFUIndex = i;
			LFURef = framePage[i].refNo;
		}
		i = (i + 1) % bufferSize;		
	}
	
	SM_FileHandle fHandle;
	// This method verifies if the page in the memory has been modified, then write page to disk	
	if(framePage[LFUIndex].dirtyBit == 1){
		int pNum = framePage[LFUIndex].pageNumber;
		SM_PageHandle memPage = framePage[LFUIndex].data;

		//Calling the function 'opnePagefile()'
		openPageFile(bm->pageFile, &fHandle);

		//Calling the function 'writeBlock()'
		writeBlock(pNum, &fHandle, memPage);

		// Increaments the number of writes done by the buffer manager.
		numofWrites++;
	}	
	// This method sets page frame's content to new page's content		
	framePage[LFUIndex].data = page->data;
	framePage[LFUIndex].dirtyBit = newDirtyBit;
	framePage[LFUIndex].fixCount = newFixCount;
	framePage[LFUIndex].pageNumber = newPageNum;
	LFUPointer = LFUIndex + 1;
}

void LRU(BM_BufferPool *const bm, FramePage *page){	
	int newPageNum = page -> pageNumber;
	int newDirtyBit = page -> dirtyBit;
	int newFixCount = page -> fixCount;

	FramePage *framePage = (FramePage *) bm->mgmtData;
	bool flag = false;
	int lHitRecord, lHitCount;

	// Interates through entire page frame in the buffer pool.
	int index  = 0;;
	while(index < bufferSize){
		if(framePage[index].fixCount != 0){
			continue;
		}
		else if(framePage[index].fixCount == 0){
			lHitRecord = index;
			lHitCount=framePage[index].hitNo;
			flag = true;
			if(flag){
				break;
			}
		}
		index++;
	}
	
	for(int i = lHitRecord + oneValue ; i < bufferSize ; i++){
		if(framePage[i].hitNo < lHitCount){
			lHitRecord = i;
			lHitCount = framePage[i].hitNo;
		}
		else{
			continue;
		}
	}

	SM_FileHandle fHandle;
	int pNum = framePage[lHitRecord].pageNumber;
	SM_PageHandle memPage = framePage[lHitRecord].data;
	// If page in memory has been remodeled ie..dirtyBit = 1, then write page to disk
	if(framePage[lHitRecord].dirtyBit == oneValue){
		//Calling the function 'openPageFile()'
		openPageFile(bm->pageFile, &fHandle);
		//Calling the function 'writeBlock()'
		writeBlock(pNum, &fHandle, memPage);
		// Increaments noofWrites which records the number of writes done by the buffer manager.
		numofWrites++;
	}
	
	// Setting page frame's content to new page's content
	framePage[lHitRecord].data = page -> data;
	framePage[lHitRecord].dirtyBit = newDirtyBit;
	framePage[lHitRecord].fixCount = newFixCount;
	framePage[lHitRecord].hitNo = page->hitNo;
	framePage[lHitRecord].pageNumber = newPageNum;
}

void CLOCK(BM_BufferPool *const bm, FramePage *page){	
	//CLOCK begins
	FramePage *framePage = (FramePage *) bm->mgmtData;

	int newPageNum = page -> pageNumber;
	int newDirtyBit = page -> dirtyBit;
	int newFixCount = page -> fixCount;

	bool flag = true;

	while(flag){
		int value = CLOCKPointer % bufferSize;
		if(value == 0){
			CLOCKPointer = noValue;
		}
		else{
			CLOCKPointer = CLOCKPointer;
		}
	}

	if(framePage[CLOCKPointer].hitNo != noValue){
		// Incrementing clockPointer so that we can check the next page frame location.
		framePage[CLOCKPointer++].hitNo = oneValue;
	}
	else{
		SM_FileHandle fHandle;
		int pNum = framePage[CLOCKPointer].pageNumber;
		SM_PageHandle memPage;
		// If page in the memory has been altered (dirtyBit = 1), then write page to disk
		if(framePage[CLOCKPointer].dirtyBit == oneValue){
			//Calling the function 'openPageFile()'
			openPageFile(bm->pageFile, &fHandle);
			//Calling the function 'writeBlock()'
			writeBlock(pNum, &fHandle, memPage);
			// Increaments the noofWrites which records the number of writes done by the buffer manager.
			numofWrites++;
		}
		// Setting page frame's content to new page's content
		framePage[CLOCKPointer].data = page -> data;
		framePage[CLOCKPointer].hitNo = page -> hitNo;
		framePage[CLOCKPointer].pageNumber = newPageNum;
		framePage[CLOCKPointer].dirtyBit = newDirtyBit;
		framePage[CLOCKPointer].fixCount = newFixCount;	
		CLOCKPointer++;
	}
}

/*-----------------------------------------
--------Part 1 - Buffer Functions------------------
-----------------------------------------*/

/*
Function 1:
Function Name: initBufferPool
Function Related to: Buffer Functions
Function Description: Creates a new Buffer Pool
Function parameters: BM_BufferPool *const bm, const char *const pageFileName, const int numPages, ReplacementStrategy strategy, void *stratData
Function return type: RC
*/
 RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName, const int numPages, ReplacementStrategy strategy, void *stratData)
{
	RC returnCode = RC_OK;
	ReplacementStrategy strat = strategy;

	bm -> strategy = strat;
	bm -> pageFile = (char *)pageFileName;
	bm -> numPages = numPages;
	
	// Memory space = number of pages x space required for one page
	FramePage *bufferPool = malloc(numPages * sizeof(FramePage));
	bufferSize = numPages;	

	// Inscribing all the pages in buffer pool. The values of fields (variables) in the page must be either NULL or 0
	int empty = -1;
	for(int i = 0 ; i < bufferSize ; i++){
		bufferPool[i].data = NULL;
		bufferPool[i].pageNumber = empty;
		bufferPool[i].dirtyBit = noValue;
		bufferPool[i].hitNo = noValue;	
		bufferPool[i].refNo = noValue;
		bufferPool[i].fixCount = noValue;
	}
	bm -> mgmtData = bufferPool;
	numofWrites = LFUPointer = CLOCKPointer = noValue;

	return returnCode;
}

/*
Function 2:
Function Name: shutdownBufferPool
Function Related to: Buffer Functions
Function Description: Destroys the Buffer Pool
Function parameters: BM_BufferPool *const bm
Function return type: RC
*/
RC shutdownBufferPool(BM_BufferPool *const bm){
	RC flagTrueReturnCode = RC_PINNED_PAGES_IN_BUFFER;
	FramePage *frame = (FramePage *)bm -> mgmtData;

	// Calling the function 'forceFlushPool()' to write all Altered pages back to disk
	forceFlushPool(bm);

	bool flag = false;
	bool correct = true;
	bool incorrect = false;
	int i = 0;
	for( ; i < bufferSize ; i++){	
		// If fixCount != 0, that is the contents of the pages were altered by some client and has not been written back to disk.
		int check = frame[i].fixCount;
		flag = (check != 0) ? correct : incorrect;
	}
	// unleasing space occupied by the page
	free(frame);
	bm->mgmtData = NULL;

	if(flag){
		return flagTrueReturnCode;
	}
	return RC_OK;
}

/*
Function 3:
Function Name: forceFlushPool
Function Related to: Buffer Functions
Function Description: Causes all dirty pages from the buffer pool to be written to disk
Function parameters: BM_BufferPool *const bm
Function return type: RC
*/
RC forceFlushPool(BM_BufferPool *const bm)
{
	RC returnCode = RC_OK;

	FramePage *framePage = (FramePage *) bm -> mgmtData;
	
	int index = 0;
	// Accumulates all the modified pages in memory to page file on disk
	while(index < bufferSize){
		int pNum = framePage[index].pageNumber;
		SM_FileHandle fHandle;
		SM_PageHandle mempage = framePage[index].data;
		if(framePage[index].dirtyBit == oneValue && framePage[index].fixCount == noValue){
			char *fileName = bm -> pageFile;
			//Calling the function 'openPageFile()'
			openPageFile(fileName, &fHandle);
			//Calling the function 'writeBlock()'
			writeBlock(pNum, &fHandle, mempage);
			framePage[index].dirtyBit = noValue;
			numofWrites++;
		}
		index++;
	}
	return returnCode;
}

/*------------------------------------------------
--------Part 2 - Page Management functions-----------------
-------------------------------------------------*/
/*
Function 4:
Function Name: pinPage
Function Related to: Page Management functions
Function Description: Pins the page with page number pageNum
Function parameters: BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum
Function return type: RC
*/
RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum)
{
	RC returnCode = RC_OK;
	FramePage *framePage = (FramePage *)bm->mgmtData;
	int empty = -1, zeroVal = 0;
	int isLRU = 1, isCLOCK = 2,  isLFU = 3;
	SM_FileHandle fHandle;

	// Examining if buffer pool is empty or not
	if(framePage[zeroVal].pageNumber != empty){
		// Reading page from disk and inscribing page frame's content in the buffer pool
		bool flag = true;
		int index  = 0;
		while(index < bufferSize){
			if(framePage[index].pageNumber != empty){	
				// Examining if page is in memory
				if(framePage[index].pageNumber == pageNum)
				{
					bool strat1 = (bm -> strategy == isLRU) ? true : false; 
					bool strat2 = (bm -> strategy == isCLOCK) ? true : false;
					bool strat3 = (bm -> strategy == isLFU) ? true : false;
					// Increamenting fixCount i.e. now there is one more client accessing this specific page
					framePage[index].fixCount++;
					flag = false;
					frameCount++; 
					if(strat1){
						// LRU algorithm uses the value of hit to determine the least recently used page	
						framePage[index].hitNo = frameCount;
					}
					if(strat3){
						// Increasing the refNo in order to add one more count of number of times the page is used
						framePage[index].refNo++;
					}
					if(strat2){
						// hitNo = 1 in order to represent that this was the last page frame checked
						framePage[index].hitNo = oneValue;
					}
					page->pageNum = pageNum;
					page->data = framePage[index].data;
					CLOCKPointer++;
					break;
				}
				else if(framePage[index].pageNumber != pageNum){
					flag = true;
				}					
			}
			else {
				openPageFile(bm->pageFile, &fHandle);
				framePage[index].data = (SM_PageHandle) malloc(PAGE_SIZE);
				readBlock(pageNum, &fHandle, framePage[index].data);
				framePage[index].refNo = noValue;
				framePage[index].pageNumber = pageNum;
				framePage[index].fixCount = oneValue;
				readIndex++;	
				frameCount++; 
				bool strat1 = (bm -> strategy == isLRU) ? true : false; 
				bool strat2 = (bm -> strategy == isCLOCK) ? true : false;
				if(strat2){
					// LRU algorithm uses the value of hit to determine the least recently used page
					framePage[index].hitNo = oneValue;	
				}			
				if(strat1){
					// hitNo = 1 to indicate that this was the last page frame examined (added to the buffer pool)
					framePage[index].hitNo = frameCount;
				}
				page -> pageNum = pageNum;
				page -> data = framePage[index].data;
				flag = false;
				break;
			}
		index++;
		}
		
	// If flag = true, then it means that the buffer is full and we must replace an existing page using page replacement strategy
		if(flag)
		{
			// Create a new page to store data read from the file.
			FramePage *newPage = (FramePage *) malloc(sizeof(FramePage));		
			
			// Reading page from disk and initializing page frame's content in the buffer pool
			SM_FileHandle fHandle;
			SM_PageHandle memPage = newPage -> data;
			//Calling the function 'openPageFile'
			openPageFile(bm -> pageFile, &fHandle);
			newPage -> data = (SM_PageHandle) malloc(PAGE_SIZE);
			//Calling the function 'readBlock'
			readBlock(pageNum, &fHandle, newPage->data);
			
			newPage -> dirtyBit = noValue;		
			newPage -> fixCount = oneValue;
			newPage -> refNo = noValue;
			newPage -> pageNumber = pageNum;
			readIndex++;
			frameCount++;
			bool strat1 = (bm -> strategy == isLRU) ? true : false; 
			bool strat2 = (bm -> strategy == isCLOCK) ? true : false;
			if(strat1){
				newPage -> hitNo = frameCount;
			}
				// LRU algorithm uses the value of hit to determine the least recently used page			
			if(strat2){
				// hitNo = 1 to represent that this was the last page frame examined (added to the buffer pool)
				newPage -> hitNo = oneValue;
			}
			page -> pageNum = pageNum;
			page -> data = newPage->data;			

		// Calls respective algorithm's function depending on the page replacement strategy selected (passed through parameters)
			switch(bm -> strategy){			
				case 0: // Using FIFO algorithm
					FIFO(bm, newPage);
					break;
				case 1: // Using LRU algorithm
					LRU(bm, newPage);
					break;
				case 2: // Using CLOCK algorithm
					CLOCK(bm, newPage);
					break;
				case 3: // Using LFU algorithm
					LFU(bm, newPage);
					break;
				case 4:
					printf("\nLRU-K algorithm is not implemented");
					break;
				default:
					printf("\nAlgorithm is not Implemented\n");
			}				
		}		
		return returnCode;		
	}
	else
	{	
		SM_FileHandle fHandle;
		openPageFile(bm->pageFile, &fHandle);
		framePage[zeroVal].data = (SM_PageHandle) malloc(PAGE_SIZE);
		ensureCapacity(pageNum, &fHandle);
		readBlock(pageNum, &fHandle, framePage[0].data);
		framePage[zeroVal].pageNumber = pageNum;
		framePage[zeroVal].fixCount++;
		readIndex = frameCount = zeroVal;
		framePage[zeroVal].hitNo = frameCount;	
		framePage[zeroVal].refNo = zeroVal;
		page->pageNum = pageNum;
		page->data = framePage[zeroVal].data;
		
		return returnCode;
	}	
}

/*
Function 5:
Function Name: unpinPage
Function Related to: Page Management functions
Function Description: Unpins the page 'page'
Function parameters: BM_BufferPool *const bm, BM_PageHandle *const page
Function return type: RC
*/
RC unpinPage(BM_BufferPool *const bm, BM_PageHandle *const page)
{	
	RC returnCode = RC_OK;
	int count = 0; bool flag = false;
	FramePage *resultArray = (FramePage *)bm->mgmtData;

	int pageCount = page -> pageNum;
	// Iterating through all the pages in the buffer pool
	for(int i = 0 ; i < bufferSize ; i++){
		int pNum = resultArray[i].pageNumber;
		if(pNum == pageCount){
			count++;
		}
		if(count != 0){
			resultArray[i].fixCount--;
			if(!flag){
				break;
			}
		}
	}
	return returnCode;
}

/*
Function 6:
Function Name: markDirty
Function Related to: Page Management functions
Function Description: Marks a page as dirty
Function parameters: BM_BufferPool *const bm, BM_PageHandle *const page
Function return type: RC
*/
RC markDirty(BM_BufferPool *const bm, BM_PageHandle *const page)
{
	RC returnCode = RC_ERROR;
	FramePage *resultArray = (FramePage *)bm->mgmtData;
	
	int index = 0;
	bool flag = false;
	do{
		int pNum = resultArray[index].pageNumber;
  		if(pNum == page->pageNum){
		  	resultArray[index].dirtyBit = oneValue;
			flag = true;
			break;
		}
  		index++;
	} while(!flag);

	if(flag){
		return RC_OK;
	}	
	return returnCode;
}

/*
Function 7:
Function Name: forcePage
Function Related to: Page Management functions
Function Description: should write the current content of the page back to the page file on disk
Function parameters: BM_BufferPool *const bm, BM_PageHandle *const page
Function return type: RC
*/
RC forcePage(BM_BufferPool *const bm, BM_PageHandle *const page)
{
	RC returnCodeResult = RC_OK;
	
	FramePage *resultArray = (FramePage *)bm->mgmtData;
	
	for(int i = 0 ; i < bufferSize ; i++){
		int pageCount = page -> pageNum;
		int pNum = resultArray[i].pageNumber;
		SM_FileHandle fHandle;
		SM_PageHandle memPage = resultArray[i].data;
		if(pNum == pageCount){	
			//Calling the function 'openPageFile'
			openPageFile(bm->pageFile, &fHandle);
			//Calling the function 'writeBlock()'
			writeBlock(pNum, &fHandle, memPage);
			resultArray[i].dirtyBit = noValue;
			numofWrites++;
		}
	}
	return returnCodeResult;
}

/*--------------------------------------
---------Part 3 - Statistical Functions----------
--------------------------------------*/

/*
Function 8:
Function Name: getFrameContents
Function Related to: Statistical Functions
Function Description: Returns an array of PageNumbers
Function parameters: BM_BufferPool *const bm
Function return type: PageNumber
*/
PageNumber *getFrameContents(BM_BufferPool *const bm){
	PageNumber *resultArray = malloc(bufferSize * sizeof(PageNumber));
	FramePage *framePage = (FramePage *) bm->mgmtData;

	int flag = -1;
	int index = 0;
	while(index < bufferSize){
		if(framePage[index].pageNumber != flag){
			resultArray[index] = framePage[index].pageNumber;
		}
		else{
			//An empty page frame 'NO_PAGE'
			resultArray[index] = NO_PAGE; 
		}
		index++;
	}
	return resultArray;
}

/*
Function 9:
Function Name: getDirtyFlags
Function Related to: Statistical Functions
Function Description: Returns an array of bools
Function parameters: BM_BufferPool *const bm
Function return type: bool
*/
bool *getDirtyFlags (BM_BufferPool *const bm){
	bool *isdirtyFlags = malloc(bufferSize * sizeof(bool));
	FramePage *framePage = (FramePage *)bm -> mgmtData;

	bool correct = true;
	bool incorrect = false;
	// Iterating through all the pages in the buffer pool and setting dirtyFlags' value to TRUE if page is dirty else FALSE
	int flag = 1;
	for(int i = 0 ; i < bufferSize ; i++){
		isdirtyFlags[i] = (framePage[i].dirtyBit == flag) ? correct : incorrect;
	}	
	return isdirtyFlags;
}

/*
Function 10:
Function Name: getFixCounts
Function Related to: Statistical Functions
Function Description: Returns an array of ints
Function parameters: BM_BufferPool *const bm
Function return type: int
*/
int *getFixCounts (BM_BufferPool *const bm){
	int *fixCounts = malloc(sizeof(int) * bufferSize);
	FramePage *framePage= (FramePage *)bm -> mgmtData;

	int count = 0;
	int flag = -1;
	while(count < bufferSize){
		if(framePage[count].fixCount != flag){
			fixCounts[count] = framePage[count].fixCount;
		}
		else{
			fixCounts[count] = noValue;
		}
		count++;
	}
	return fixCounts;
}

/*
Function 11:
Function Name: getNumReadIO
Function Related to: Statistical Functions
Function Description: Returns the number of pages that have been read from disk since a buffer pool has been initialized
Function parameters: BM_BufferPool *const bm
Function return type: int
*/
int getNumReadIO(BM_BufferPool *const bm){
	int numReadPages = readIndex + 1;
	return numReadPages;
}

/*
Function 12:
Function Name: getNumWriteIO
Function Related to: Statistical Functions
Function Description: Returns the number of pages written to the page file since the buffer pool has been initialized.
Function parameters: BM_BufferPool *const bm
Function return type: int
*/
int getNumWriteIO (BM_BufferPool *const bm){
	int numPagesWritten = numofWrites;
	return numPagesWritten;
}