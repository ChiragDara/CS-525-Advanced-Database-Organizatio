# ********************************************************************* #
# ------  Fall 2021 - ADO Assignment - 1 : Storage Manager ------
# ********************************************************************* #

## TEAM MEMBER PERSONAL INFORMATION

1. Chirag Dara      - cdara1@hawk.iit.edu       A20493550
2. Malhar Hunge     - mhunge@hawk.iit.edu       A20492858
3. Shishir Kondai   - skondai@hawk.iit.edu      A20492225

# ********************************************************************* #

## RUNNING THE SCRIPT:

Step 1: Go to the project root using terminal
Step 2: Type ls to review the files and make sure that we are in the correct directory.
Step 3: Once ensure, Run `make clean` command
Step 4: Run `make`
Step 5: Run ./test_assign1

# ********************************************************************* #

## Functionalities of Storage Manager

# Manipulation of Files */

1. initStorageManager : 
	The function used to initialize the file pointer variable to NULL.


2. createPageFile (char *fileName) :
        - Create a new page fileName. The initial file size must be one page. This method must fill in this unique page with ' 0' bytes.
        - The file is opened using fopen() function in C in w+ mode to enable both read and write operations.


3. openPageFile (char *fileName, SM_FileHandle *fHandle) :
        - Opens an existing page file. Expected to return RC_FILE_NOT_FOUND if the file does not exist. 
        - The second argument is an existing file handle. 
        - If the file is successfully opened, the fields in that file manager must be initialized with the file information open.
        For instance, you'll have to read the total number of pages that are stored in the file from disk.
        - The file opened using fopen() function of C in r mode to enable only read operation.
        

4. closePageFile (SM_FileHandle *fHandle), destroyPageFile (char *fileName) :
        - Close an open page file or destroy (delete) a page file.
        - In case the file is not closed RC_FILE_NOT_CLOSED error is thrown.
         

# ****************************************************************************************************************************************** #

# Reading Blocks From Disc

1. readBlock RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) :
	- This function reads a block from disk
	- The file is opened using fopen() function of C in r mode to enable only read operation.
	- Using the fseek() method and a file pointer, navigate to a specified location. 
	- If we found the desired location, read the data from the specified page number and store it beginning at the location pageHandle. 
	- Checked whether the page number is valid or not by verify if its not negative or greater than total number of pages.


2. getBlockPos int getBlockPos (SM_FileHandle *fHandle):
	- This function returns the current page position in the file.


3. readFirstBlock RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage):
	- This function reads first page from the file.
	- readBlock() function is used to read the page, pageNum attribute is set to 0.


4. readPreviousBlock RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage):
	- This function read the previous page relative to the 'curPagePos' of the file.
	- readBlock() function is used to read page, prePageNum attribute is set to current page -1.
	
	
5. readCurrentBlock RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage):
	- Read the current page relative to the curPagePos of the file.
	- readBlock() function is used to read page, currentPageNum is calculated by dividing currentPagePos to PAGE_SIZE.


6. readNextBlock extern RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage):
	- We call the readBlock(...) function by providing the pageNum argument as (current page position + 1)


7. readLastBlock RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage):
	- We used readBlock() function by passing the pageNum argument as total number of pages - 1.



# writing blocks to a page file

1. writeBlock RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage):
	- Firstly we ensure that there is enough capacity to write block
	- We navigate to correct position using fseek() function
	- If the fseek() function is successful, we use the fwrite() function to write the data to the appropriate position and put it in the memPage parameter.


2. writeCurrentBlock RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage):
	- Write a page to disk using the current position.
	- writeBlock function is used by setting pageNumtoWrite as currPage + 1.


3. appendEmptyBlock RC appendEmptyBlock (SM_FileHandle *fHandle):
	- Make an empty block with size = PAGE_SIZE.
	- Move the pointer from the file flow to the last page.
	- Increase in total number of pages by 1.

4. ensureCapacity RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle):
	- First verify whether the number of pages needed is greater that the total number of pages.
	- Calculate the required page count and add as many empty blocks.
	- Add the empty block using appendEmptyBlock().

# ****************************************************************************************************************************************** #