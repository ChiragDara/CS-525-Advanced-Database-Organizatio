# Fall 2021 - ADO Assignment - 2 : Buffer Manager

The purpose of this assignment is to implement a Buffer Manager. The buffer manager manages a fixed number of pages in memory that represent pages
from a page file managed by the storage manager implemented in assignment 1. The memory pages managed by the buffer manager are called page
frames or frames for short. We call the combination of a page file, and the page frames storing pages from that file a Buffer Pool.

Buffer manager should be able to handle more than one open buffer pool at the same time. However, there can only be one buffer pool for
each page file. Each buffer pool uses a one-page replacement strategy that is determined when the buffer pool is initialized. You should at least implement two replacement strategies FIFO and LRU. Your solution should implement all the methods defined in the buffer_mgr.h
## Authors

- Chirag Dara   - cdara1@hawk.iit.edu A20493550
- Malhar Hunge  - mhunge@hawk.iit.edu A20492858
- Shishir Kondai- skondai@hawk.iit.edu A20492225


## RUNNING THE SCRIPT:

```bash
  Step 1: Go to the project root using terminal 
  Step 2: Type ls to review the files and make sure that we are in the correct directory. 
  Step 3: Once ensure, Run make clean command 
  Step 4: Run make Step 
  5: Run ./test_assign1
```

## Functions and Description

1. initBufferPool : This method sets up the buffer pool.
- It validates the input parameters passed.
- If the page file entered matches a legitimate file.
- It sets the default values for the BM BufferPool and BufferManagerInfo object fields.
- It generates frameNodes based on the page size input.
- It closes the File page.
- If the buffer pool is correctly initialized, it returns RC OK.

2. shutdownBufferPool: This method terminates the buffer pool and releases all resources that have been acquired.
- It validates the input parameters passed.
- It checks to see whether any pages have been pinned.
- If both of the preceding conditions are met, it will write all dirty pages back to the disk.
- It releases the memory occupied by all FrameNodes associated with this buffer pool.
- It releases the buffer manager information.
- If the buffer pool is successfully shut off, it will return RC OK.

3. forceFlushPool: This method flushes the pool by writing all dirty pages to disk.
- It verifies the input parameters' correctness.
- It then opens the page file.
- It loops through the frame nodes, checking for dirty frames.
- When a frame becomes dirty, the contents are written back to the disk.
- Marks the frame as clean.
- Indicates that it is not dirty in the array dirtyFlagPerFrame.
- Once all of the dirty frames have been written, the page file is closed.
- If successful, returns RC OK.

4. markDirty: This technique indicates that the supplied page is filthy.
- It verifies the inputs' correctness.
- It iterates across the buffer pool, looking for the frame that contains this page.
- If the page cannot be located, it returns an error.
- Otherwise, it flags the FrameNode that is storing this page as unclean.
- It modifies the "dirtyFlagPerFrame" array to designate the frame associated with the page as dirty.
- On success, returns RC OK.

5. unpinPage: This method removes the page's pin.
- It verifies the input for correctness.
- It locates the FrameNode that corresponds to the page provided.
- It determines whether the fixed count is greater than zero. If this is the case, decrement the fixed count. The array pinCountsPerFrame will be updated with the new fix count.
- Otherwise, an error message will be returned. RC BM INVALID UNPIN.
- If successful, returns RC OK.

6. Force page: This method will write the page's contents back to disk.
- It will locate the frame node that corresponds to the page number.
- If the page is not found in the frame node list, the error "RC BM INVALID PAGE" is returned.
- Otherwise, it will open the pageFile
- It saves the data from that page to disk.
- Increase the numOfDiskWrites value.
- Set the dirty flag to false if the fix count is zero.
- Closes the page file.
- If successful, returns RC OK.

7. pinPage: Implements FIFO and LRU logic.
   FIFO :
- It determines whether the page exists in the buffer pool.
- If affirmative, this frame is returned, raising the pin count.
- If not, it will check to see if there is an empty frame in the pool that can be used.
- If yes, that frame node will be used to read data from the disk into that frame. Place that frame at the top of the list and increase the fix count.
- Otherwise, it will look for a frame to replace. It will begin at the tail and iterate until it finds a frame with a fix count of zero.
  It will then write the contents of this frame back to the disk if it is dirty and read the contents of the page to be returned from the disk.

LRU :
- It checks if the page exists in the buffer pool.
- If yes, it returns this frame increasing the pin count. This frame is also moved to the top of the list.
- Else, it will check if an empty frame in the pool can be used.
- If yes, it will use that frame node, read data from the disk into that frame. Move that frame to the head of the list, and increment the fix count.
- Else, it will check for a frame to replace. It will navigate from the tail and keep iterating till it finds a frame that has a fix count of zero.
- It will then write the contents of this frame back to the disk if it is dirty and read the contents of the page to be returned from the disk. It will then increment the fix count.

CLOCK :
- It checks if the page exists in the buffer pool.
- If yes, it returns this frame increasing the pin count. And sets the reference bit of this node to 1 and marks this node as a reference node.
- Else, it will check if there is an empty frame in the pool that the empty frame can use.
- If yes, it will use that frame node, read data from the disk into that frame. Increment the fix count. Mark the node as reference node and set reference bit to 1.
- Else, it will check for a frame to replace. It will navigate from the reference node and keep iterating till it finds a frame that has a fix count of zero and a reference bit of zero.
  In the way, it marks all other reference bits to 0.
  When it finds the replacement node, It will write the contents of this frame back to the disk if it is dirty and read the contents of the page to be returned from the disk. It will then increment the fix count. And mark this as a reference node and set its reference bit to 1.


8. getFrameContents: Returns an array of PageNumbers (numberPages size) where the ith element is the number of the page stored in the ith page frame.
   An empty page frame is represented using the constant NO_PAGE.

9. getDirtyFlags: Returns an array of bools (numberPages size) where the ith item is TRUE if the page stored in the ith page frame is dirty.
   Blank page frames are treated as clean.

10. getFixCounts: Returns an array of ints (of size numPages) where the ith element is the fixed count of the page stored in the ith page frame.
    Return 0 for empty page frames.

11. getNumReadIO: Returns the number of pages that have been read from the disk since a buffer pool has been initialized.

12. getNumWriteIO: Returns the number of pages written to the page file since the buffer pool has been initialized.