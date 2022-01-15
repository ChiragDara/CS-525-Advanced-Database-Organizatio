#include "storage_mgr.h"
#include<stdio.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<unistd.h>
#include<string.h>
#include<stdbool.h>

// OUR ERROR CODE
int RC_GET_NUMBER_OF_BYTES_FAILED = 305;
int  RC_FILE_NOT_CLOSED = 306;

FILE *pointerforFile;

/*
Function 1:
Function Name: initStorageManager
Function Related to: Manipulating Page Files
Function Description: Intializing the storage Manager
Function parameters: void
Function return type: void
*/
void initStorageManager (void){
    printf("\n********************Initialization Storage Manager********************\n");
    pointerforFile = NULL;
}
/*
Function 2:
Function Name: createPageFile
Function Related to: Manipulating Page Files
Function Description: To create empty page in the memory
Function parameters: char *fileName
Function return type: RC
*/
RC createPageFile (char *fileName){
    printf("\n******************* Create Page file Function *******************\n");
    RC returncreatePageFile = RC_OK;
    char chars[PAGE_SIZE];
    bool flag = false;
    //Open the file in "w+" mode. The w+ mode in fopen creates a empty file with "fileName" for reading and writing.
    pointerforFile = fopen(fileName,"w+");

    if(pointerforFile == NULL){
        fclose(pointerforFile);
        flag = true;
    }

    memset(chars, '\0', sizeof(chars));
    int size =1;
    //fwrite() -> returns the size of data written to file.
    int result = fwrite(chars, size, PAGE_SIZE, pointerforFile);
    //Checking for the condition whether the fwrite result is not equal to PAGE_SIZE
    if(result != PAGE_SIZE){
        fclose(pointerforFile);
        destroyPageFile(fileName);
        flag = true;
    }
    if(flag){
        return RC_FILE_HANDLE_NOT_INIT;
    }
    //Closing the file, so that all buffers are flushed
    fclose(pointerforFile);
    printf("\n******************* Create Page file Function %d *******************\n", returncreatePageFile);
    return returncreatePageFile;
}

/*
Function 3:
Function Name: openPageFile
Function Related to: Manipulating Page Files
Function Description: Opens an existing page file. Returns RC FILE NOT FOUND, if the file does not exist
Function parameters: char *fileName, SM_FileHandle *fHandle
Function return type: RC
*/
RC openPageFile (char *fileName, SM_FileHandle *fHandle){
    printf("\n******************* Open Page file Function *******************\n");
    int returnOpenPageFileCode;
    int startPosition = 0;
    //Open the file in read only mode. "r" - open a file in read mode
    pointerforFile = fopen(fileName,"r");

    if(pointerforFile != NULL){
        if(fseek(pointerforFile, startPosition, SEEK_END) == 0){
            fHandle -> fileName = fileName;
            fHandle -> curPagePos = 0;
            if(ftell(pointerforFile) % PAGE_SIZE == 0){
                // set the file information
                fHandle -> totalNumPages = (ftell(pointerforFile)/PAGE_SIZE);
            }else{
                fHandle -> totalNumPages = (ftell(pointerforFile)/PAGE_SIZE + 1);
            }
            fclose(pointerforFile);
            returnOpenPageFileCode = RC_OK;
        }else if(fseek(pointerforFile, startPosition, SEEK_END) != 0 && 
                        ftell(pointerforFile) == -1){   //  ftell() - find out the position of file pointer in the file with respect to starting of the file
            printf("File contain some values, hence closing the file and returning no of bytes fail errror");
            fclose(pointerforFile);
            returnOpenPageFileCode = RC_GET_NUMBER_OF_BYTES_FAILED;
            }
    }else{
            returnOpenPageFileCode =RC_FILE_NOT_FOUND;
        }
        printf("\n******************* Open Page file Function %d *******************\n", returnOpenPageFileCode);
        return returnOpenPageFileCode;
}
/*
Function 4:
Function Name: closePageFile
Function Related to: Manipulating Page Files
Function Description: Close an open page file
Function parameters: SM_FileHandle *fHandle
Function return type: RC
*/
RC closePageFile (SM_FileHandle *fHandle){
    printf("\n******************* Close Page file Function *******************\n");
    int returnclosePageFileCode = RC_OK;
    bool flag = false;
    if(pointerforFile == NULL){
        flag = true;
    }
    else{
        pointerforFile = NULL;
        returnclosePageFileCode = RC_OK;
    }
    if(flag){
        returnclosePageFileCode = RC_FILE_NOT_CLOSED;
    }
    printf("\n******************* Returning Close Page file Function : %d *******************\n", returnclosePageFileCode);
    return returnclosePageFileCode;
}
/*
Function 3:
Function Name: destroyPageFile
Function Related to: Manipulating Page Files
Function Description: Closes an open page file
Function parameters: SM_FileHandle *fHandle
Function return type: RC
*/
RC destroyPageFile (char *fileName){
    printf("\n******************* Destroy Page file Function *******************\n");
    int returndestroyPageFileCode = RC_OK;
    //Deleting the given fileName and checking for the status, whether the file is correctly deleted or not
    if(remove(fileName) != -1){
        returndestroyPageFileCode = RC_OK;
    }
    else if(remove(fileName) == -1){
        returndestroyPageFileCode = RC_FILE_NOT_FOUND;
    }
    printf("\n******************* Returning Destroy Page file Function : %d *******************\n", returndestroyPageFileCode);
    return returndestroyPageFileCode;
}
/*
Function 6:
Function Name: readBlock
Function Related to: Reading blocks from disc
Function Description: 
Function parameters: int pageNum, SM_FileHandle *fileHandle, SM_PageHandle memPage
Function return type: RC
*/
RC readBlock(int pageNum, SM_FileHandle *fileHandle, SM_PageHandle pageHandle){
    printf("\n******************* Read Block Function *******************\n");
    int returnReadBlockCode = RC_READ_NON_EXISTING_PAGE;

    //Condtion for checking page number value
    if(pageNum < 0){
        return returnReadBlockCode;
    }else if (pageNum > fileHandle->totalNumPages){
        return returnReadBlockCode;
    }else{
        //Opening the file in read mode.
        pointerforFile = fopen(fileHandle->fileName , "r");
        //checking condition for file open statement
        if(pointerforFile != NULL){
            //Adjust the location of the pointer filestream, the location is evaluated by pageNum * PAGE_SIZE.
            //seek is complete when it is equal to 0
            fseek(pointerforFile, (pageNum * PAGE_SIZE), SEEK_SET);
            //Read content, store it at a location defined by memPage.
            fread(pageHandle, sizeof(char), PAGE_SIZE, pointerforFile);
            //Setting the current page position to the pointer
            //After reading 1 Block/page what is the current position of the pointer in file. 
            //It return the current location in file stream.
            fileHandle->curPagePos=pageNum;
            //closing the file
            fclose(pointerforFile);
            returnReadBlockCode = RC_OK;
        }
        if(returnReadBlockCode == RC_OK){
            printf("\n******************* Read Block Successfully Read ******************* \n");
        } else{
            printf("\n******************* Error in reading File *******************n");
        }
        printf("\n******************* Returning Read Block Function : %d *******************\n", returnReadBlockCode);
        return returnReadBlockCode;
    }
}
/*
Function 7:
Function Name: getBlockPos
Function Related to: Reading Block from Disc
Function Description: Return the current page position in a file
Function parameters: SM_FileHandle *fHandle
Function return type: int
*/
int getBlockPos(SM_FileHandle *fileHandle){
    printf("\n******************* Get Block Pos Function *******************\n");
    //Returning current position of page, retrieved from the fhandle. It will be the same as file position
    bool flag = true;
    int blockPos;
    if(flag){
        blockPos = fileHandle -> curPagePos;
    }
    printf("\n******************* Returning Get Block Pos Function : %d *******************\n", blockPos);
    return blockPos;
}
/*
Function 8:
Function Name: readFirstBlock
Function Related to: Read block fron the disc
Function Description: read the first page in the file
Function parameters:  SM_FileHandle *fHandle, SM_PageHandle pageHandle0
Function return type: RC
*/
RC readFirstBlock(SM_FileHandle *fileHandle, SM_PageHandle pageHandle){
    printf("\n******************* Read First Block Function *******************\n");
    int pageNum = 0;
    //passing to the readBlock and passing the respective parameter
    int returnreadFirstBlockCode = readBlock(pageNum, fileHandle, pageHandle);
    printf("\n******************* Returning Get Block Pos Function : %d *******************\n", returnreadFirstBlockCode);
    return returnreadFirstBlockCode;
}

/*
Function 9:
Function Name: readPreviousBlock
Function Related to: Reading block fron the disc
Function Description: read the previous page relative to the 'curPagePos of the file
Function parameters:  SM_FileHandle *fileHandle, SM_PageHandle pageHandle
Function return type: RC
*/
RC readPreviousBlock(SM_FileHandle *fileHandle, SM_PageHandle pageHandle){
    printf("\n******************* Read Previous Block Function *******************\n");
    RC returnreadPreviousBlock;
    bool flag = true;
    //calculating current page number by dividing page size to current page position
    int currentPagePos = fileHandle -> curPagePos;
    int currentPageNum = currentPagePos / PAGE_SIZE;
    //setting page number to previous block = current - 1 
    int prevPageNum =currentPageNum  - 1;
    if(flag){
        //calling the readBlock function
        returnreadPreviousBlock = readBlock(prevPageNum, fileHandle, pageHandle);
    }
    printf("\n******************* Read Previous Block Function %d *******************\n", returnreadPreviousBlock);
    return returnreadPreviousBlock;
}

/*
Function 10:
Function Name: readCurrentBlock
Function Related to: Reading block fron the disc
Function Description: read the current page relative to the 'curPagePos of the file
Function parameters:  SM_FileHandle *fileHandle, SM_PageHandle pageHandle
Function return type: RC
*/
RC readCurrentBlock(SM_FileHandle *fileHandle, SM_PageHandle pageHandle){
    printf("\n******************* Read Current Block Function *******************\n");
    RC returnreadCurrentBlockCode;
    int count = 1;
    for(int i=0; i<count; i++){
        //finding the current page number by dividing current page position by size of the page. It will have position in bytes.
        int currentPagePos = fileHandle -> curPagePos;
        int currentPageNum = currentPagePos / PAGE_SIZE;
        //Goot to read, so recursively call readBlock subroutine
        returnreadCurrentBlockCode = readBlock(currentPageNum, fileHandle, pageHandle);
        }
        printf("\n******************* Returning Read Current Block Function %d *******************\n", returnreadCurrentBlockCode);
        return returnreadCurrentBlockCode;
}

/*
Function 11:
Function Name: readNextBlock
Function Related to: Reading block fron the disc
Function Description: read the next page relative to the 'curPagePos of the file
Function parameters:  SM_FileHandle *fileHandle, SM_PageHandle pageHandle
Function return type: RC
*/
RC readNextBlock(SM_FileHandle *fileHandle, SM_PageHandle pageHandle){
    printf("\n******************* Read next Block Function *******************\n");
    RC returnreadNextBlockCode;
    //find the current page number the same was reading the current block
    int currentPagePos = fileHandle -> curPagePos;
    int currentPageNum = currentPagePos / PAGE_SIZE;
    //adding '1' to currentPageNum to find the next block
    int nextBlock = currentPageNum + 1;
    int count = 1;
    for(int i=0; i<count; i++){
        returnreadNextBlockCode = readBlock(nextBlock, fileHandle, pageHandle);
    }
    printf("\n******************* Returning Read Next Block Function %d *******************\n", returnreadNextBlockCode);
    return returnreadNextBlockCode;
}

/*
Function 12:
Function Name: readLastBlock
Function Related to: Reading block fron the disc
Function Description: read the last page in the file
Function parameters:  SM_FileHandle *fileHandle, SM_PageHandle pageHandle
Function return type: RC
*/
RC readLastBlock(SM_FileHandle *fileHandle, SM_PageHandle pageHandle){
    printf("\n******************* Read Last Block Function *******************\n");
    RC returnLastBlockCode;
    //Getting the total number of pages
    int lastBlockPage = fileHandle -> totalNumPages - 1;
    //read the last block by calling the function "readBlock" with lastBlockPage as pageNum
    returnLastBlockCode = readBlock(lastBlockPage, fileHandle, pageHandle);
    printf("\n******************* Returning Write Block Function %d *******************\n",returnLastBlockCode);
    return returnLastBlockCode;
}

/*
Function 13:
Function Name: writeBlock
Function Related to: Writing block to a page file
Function Description: write a page to disk using an absolute position
Function parameters:  int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage
Function return type: RC
*/
RC writeBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){
    printf("\n******************* Write Block Function *******************\n");
    if(ensureCapacity(pageNum, fHandle) == RC_OK){
        FILE *pointerforFile;
        RC readwriteBlockCode;
        //Opening the file in read mode.
        pointerforFile = fopen(fHandle->fileName,"rb+");
        //setting the pointer position of the file stream. The fseek returns 0 if is success
        if(fseek(pointerforFile, pageNum * PAGE_SIZE, SEEK_SET) != 0){
            readwriteBlockCode = RC_READ_NON_EXISTING_PAGE;
        }
        //fwrite() -> returns the size of data written to file. And checking for the condtion it's equal to PAGE_SIZE
        else if(fwrite(memPage, sizeof(char), PAGE_SIZE, pointerforFile) != PAGE_SIZE){
            readwriteBlockCode = RC_WRITE_FAILED;
        } else {
            fHandle->curPagePos = pageNum;
            readwriteBlockCode = RC_OK;
        }
        //Closing the file.
        fclose(pointerforFile);
        printf("\n******************* Returning Write Current Block Function %d *******************\n", readwriteBlockCode);
        return readwriteBlockCode;
    }
}

/*
Function 14:
Function Name: writeCurrentBlock
Function Related to: Writing block to a page file
Function Description: write a page to disk using current position
Function parameters:  int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage
Function return type: RC
*/
RC writeCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage){
    printf("\n******************* Write Current Block Function *******************\n");
    RC flag = RC_OK;
    //Calculate current page no: floor value (already written pages) + 1 (new/current page to write)
    int currPage = fHandle -> curPagePos/PAGE_SIZE;

    int pageNumToWrite = currPage + 1;
    //Calling write block for writing at current block
    int returnwriteCurrentBlockCode = writeBlock(pageNumToWrite, fHandle, memPage);
    if(returnwriteCurrentBlockCode == flag){
        //File write successful, so now update info about file in the handler
        fHandle -> totalNumPages++;
        returnwriteCurrentBlockCode = flag;
    }
    printf("\n******************* Returning Write Current Block Function %d *******************\n", returnwriteCurrentBlockCode);
    return returnwriteCurrentBlockCode;
}

/*
Function 15:
Function Name: appendEmptyBlock
Function Related to: Writing block to a page file
Function Description: Increase the number of pages int the file by one
Function parameters:  int pageNum, SM_FileHandle *fHandle
Function return type: RC
*/
RC appendEmptyBlock (SM_FileHandle *fHandle){
    printf("\n******************* Append Empty Block Function *******************\n");
    int returnappendEmptyBlockCode = RC_OK;
    bool flag = false;
    
    SM_PageHandle emptyBlockStart = (SM_PageHandle)calloc(PAGE_SIZE, sizeof(char));
    //Moving the pointer to the end of the file
    int endPointer = fseek(pointerforFile, 0, SEEK_END);
    if(endPointer != 0){
        returnappendEmptyBlockCode = RC_WRITE_FAILED;
    }
    else{
        // Writing an empty page to the file
        int emptyPageSize = fwrite(emptyBlockStart, sizeof(char), PAGE_SIZE, pointerforFile);
        //Checking for the condition of fwrite result
        if (emptyPageSize == PAGE_SIZE){
            //Setting the new value to total no of pages
            fHandle -> totalNumPages++;
            flag = true;
        }
    }
    if(flag){
        returnappendEmptyBlockCode = RC_OK;
    }
    //Deallocating the memory previously used by "emptyPage"
    free(emptyBlockStart);
    printf("\n******************* Returning Append Empty Block Function %d*******************\n",returnappendEmptyBlockCode);
    return returnappendEmptyBlockCode;

}

/*
Function 16:
Function Name: ensureCapacity
Function Related to: Writing block to a page file
Function Description: To increase the total number of pages in the file to numberpointerforfilepage
Function parameters:  int numberpointerforfilepage, SM_FileHandle *fHandle
Function return type: RC
*/
RC ensureCapacity (int numberpointerforfilepage, SM_FileHandle *fHandle){
    printf("\n******************* Ensure Capacity Function *******************\n");
    //Opening the file in append mode. "a" - opens a file in append mode
    pointerforFile = fopen(fHandle->fileName, "a");
    bool flag = false;
    if(pointerforFile == NULL){
        flag = true;
    }else{
        //append empty pages until the numberpointerforfilepage is equal to the pages in the file
        while(fHandle -> totalNumPages < numberpointerforfilepage){
            appendEmptyBlock(fHandle);
        }
    }
    if(pointerforFile != NULL){
        // Closing the file stream so that all the buffers are flushed
        fclose(pointerforFile);
    }
    if(flag){
        return RC_FILE_NOT_FOUND;
    }
    printf("\n******************* END of Ensure Capacity Function *******************\n");
    return RC_OK;
}