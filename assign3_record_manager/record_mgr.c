#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "record_mgr.h"
#include "buffer_mgr.h"
#include "storage_mgr.h"

typedef struct FieldMgmtInfo{
	int numberOfFields;
	int numberOfTuples;
	int firstEmptySlot;
	RID fieldIdentifier;
	Expr *findField;
	BM_BufferPool bmBuffPool;
	BM_PageHandle bm_handle;
} FieldMgmtInfo;

FieldMgmtInfo *fInformation;
const int countPage = 100;
const int elementName = 15;
int zeroVal = 0;
int oneVal = 0;
int negVal = -1;

void addData (FieldMgmtInfo *mgr, RID *rid, Record *field, char *pointer, char *value, int fieldCap) {
    BM_BufferPool *const buffManager = &mgr -> bmBuffPool;
    BM_PageHandle *const page = &mgr -> bm_handle;
    //call markDirty method
    markDirty(buffManager, page);
    int temp = (rid -> slot * fieldCap);
    pointer = value + temp;
    *pointer = '+';
    // Calling memcpy method
    memcpy(++pointer, field -> data + 1, fieldCap - 1);
    //Calling unpingPage
    unpinPage(buffManager, page);
    //add value for numberOfFields
    mgr -> numberOfFields = mgr -> numberOfFields + 1;
    //Calling pingPage Method
    pinPage(buffManager,page, 0);
}

//This function performs the file operations.
void fileOp (char *name, SM_FileHandle fileHandle, char *data) {
    SM_FileHandle *fHandle = &fileHandle;
    // Calling createPageFile method
    createPageFile(name);
    //Calling openPageFile method
    openPageFile(name, fHandle);
    //calling writeBlock method
    writeBlock(0, fHandle, data);
    closePageFile(fHandle);
}

//This function will search the empty block
int searchEmptyBlock (int field, char *value){
    for(int i = 0; i < PAGE_SIZE / field; i++){
        char fSize = value[i * field];
        if(fSize != '+'){
            return i;
        } 
    }
    return -1;
}

//This function will set the Schema
SM_PageHandle fixSchema (RM_TableData *rel) {
    Schema *tableInfo;
    tableInfo = (Schema *) malloc(sizeof(Schema));
    SM_PageHandle bm_handle = (char *) fInformation -> bm_handle.data;
    fInformation -> numberOfFields = *(int *) bm_handle;
    bm_handle += sizeof (int);

    fInformation -> firstEmptySlot = *(int *) bm_handle;
    bm_handle += sizeof (int);
    int value = *(int *) bm_handle;

    bm_handle += sizeof(int);
    tableInfo -> typeLength = (int *) malloc (sizeof(int) * value);
    tableInfo -> dataTypes = (DataType*) malloc(sizeof(DataType) *value);
    tableInfo -> attrNames = (char **) malloc(sizeof(char *) *value);
    tableInfo -> numAttr = value;

    int index = 0;
    while(value > index){ 
        tableInfo -> attrNames[index]= (char*) malloc (elementName);
        index++;
    }
    int flag = tableInfo -> numAttr;
    int i;
    for(i = 0; i < flag; i++){
        // Calling strncpy method for updating the schema in RM_TableData structure
        strncpy(tableInfo -> attrNames[i], bm_handle, elementName);
        bm_handle = bm_handle + elementName;
        tableInfo -> dataTypes[i] = *(int *) bm_handle;
        bm_handle = bm_handle + sizeof(int);
        tableInfo -> typeLength[i] = *(int *)bm_handle;
        bm_handle = bm_handle + sizeof(int);
    }
    rel -> schema = tableInfo;
    return bm_handle;
}

//Function: update()
void update (FieldMgmtInfo *structRM){
    int aValue = 1; int val = 0;
    structRM -> numberOfTuples = val;
    structRM -> fieldIdentifier.page = aValue;
    structRM -> fieldIdentifier.slot = zeroVal;
}

//This function will
void paraUpdate(FieldMgmtInfo *structRM, Record *field, int capacity){
    int temp = 0; int slot = 0; int page = 0;
    int size = capacity;
    char *pointer = field -> data;
    slot = field -> id.slot; 
    page = field -> id.page;
    slot = structRM -> fieldIdentifier.slot;
    page = structRM -> fieldIdentifier.page;
    char *value = structRM -> bm_handle.data;
    *pointer = '-';
    temp = (structRM -> fieldIdentifier.slot * size);
    value += temp;
    value++;
    //Calling the memcpy function
    memcpy(++pointer, value, capacity - 1);
}

void functionUpdate(FieldMgmtInfo *structRM, FieldMgmtInfo *rmTable, Record *field,int capacity, int flag){
    int temp = flag;
    BM_BufferPool *const buffManager = &rmTable->bmBuffPool;
    BM_PageHandle *const page = &structRM->bm_handle;
    const PageNumber pNum = structRM->fieldIdentifier.page;
    // Calling pinPage function
    pinPage(buffManager, page, pNum);
    // calling paraUpdate Method
    paraUpdate(structRM, field, capacity);
    structRM->numberOfTuples++;
    temp++;
}

/*-----------------------------------------
--------Part 1 - Table and Manager Functions------------------
-----------------------------------------*/

/*
Function 1:
Function Name: initRecordManager
Function Related to: Table and Record Manager Functions
Function Description: 
Function parameters: void *mgmtData
Function return type: RC
*/
RC initRecordManager(void *mgmtData){
    RC returnCode = RC_OK;
    //initializing the storage manager
    initStorageManager();
    return returnCode;
}

/*
Function 2:
Function Name: shutdownRecordManager
Function Related to: Table and Record Manager Functions
Function Description: Frees the memory of the record manager data
Function parameters: -
Function return type: RC
*/
RC shutdownRecordManager(){
    RC returnCode = RC_OK;
    //the free() function frees the memory of the record manager
    free(fInformation);
    return returnCode;
}

/*
Function 3:
Function Name: createTable
Function Related to: Table and Record Manager Functions
Function Description: Creates a table which contains the underlying pagefile and also stores information about the schema
Function parameters: char *name, Schema *schema
Function return type: RC
*/
RC createTable(char *name, Schema *schema){
    RC returnCode = RC_OK;
    char arr[PAGE_SIZE];
    SM_FileHandle fileHandle;
    char *buffManagerHandle = arr;
    //The Replacement Strategy is LRU
    ReplacementStrategy rStrategy = RS_LRU;
    fInformation = (FieldMgmtInfo*) malloc(sizeof(FieldMgmtInfo));
    const int pages = countPage;
    //Calling the function 'initBufferPool'
    initBufferPool(&fInformation -> bmBuffPool, name, pages, rStrategy, NULL);
    for(int i = 0; i < 2; i++){
        *(int*)buffManagerHandle = i;
        buffManagerHandle = buffManagerHandle + sizeof(int);
    }
    *(int*)buffManagerHandle = schema -> numAttr;
    buffManagerHandle = buffManagerHandle + sizeof(int);
    *(int*)buffManagerHandle = schema -> keySize;
    buffManagerHandle = buffManagerHandle + sizeof(int);
    for(int i = 0; i < schema -> numAttr; i++){
        // calling strncpy inbuild C function
        strncpy(buffManagerHandle, schema->attrNames[i],elementName);
        buffManagerHandle += elementName;
        *(int*)buffManagerHandle = (int)schema -> dataTypes[i];
        buffManagerHandle += sizeof(int);
        *(int*)buffManagerHandle = (int)schema -> typeLength[i];
        buffManagerHandle += sizeof(int);
    }
    // Calling fileOp method
    fileOp(name, fileHandle, arr);
    return returnCode;
}

/*
Function 4:
Function Name: openTable
Function Related to: Table and Record Manager Functions
Function Description: Opens the table for operations to be performed on the table
Function parameters: RM_TableData *rel, char *name
Function return type: RC
*/
RC openTable(RM_TableData *rel, char *name){
    SM_PageHandle bmHandle;
    const PageNumber pageNum = 0;
    
    RC returnCode = RC_OK;
    RC falseReturnCode = RC_ERROR;
    if(name == false || rel == false){
        return falseReturnCode;    //returns the error if the table does not exist
    }   
    else{
        rel -> mgmtData = fInformation;
        rel -> name = name;
        BM_BufferPool *const bufferManager = &fInformation -> bmBuffPool;
        BM_PageHandle *const page = &fInformation -> bm_handle;
        //Calling the function 'pinPage'
        pinPage(bufferManager, page, pageNum);
        bmHandle = fixSchema(rel);
        //Calling the function 'unpinPage'
        unpinPage(bufferManager, page);
        forcePage(bufferManager,page);
    }
    return returnCode;
}

/*
Function 5:
Function Name: closeTable
Function Related to: Table and Record Manager Functions
Function Description: Cause all the outstanding changes to the table to be written to the pagefile
Function parameters: RM_TableData *rel
Function return type: RC
*/
RC closeTable(RM_TableData *rel){
    RC trueReturnCode = RC_OK;
    RC falseReturnCode = RC_ERROR;
    if(rel == true){
        BM_BufferPool *const bufferManager = &fInformation -> bmBuffPool;
        //calling the function 'shutdownbufferPool' to shut down the buffer pool
        shutdownBufferPool(bufferManager);
        rel -> mgmtData = NULL;
        free(rel -> schema);
    }
    else if(rel == false){
        return falseReturnCode;
    }
    return trueReturnCode;
}

/*
Function 6:
Function Name: deleteTable
Function Related to: Table and Record Manager Functions
Function Description: Deletes the table
Function parameters: char *name
Function return type: RC
*/
RC deleteTable(char *name){
    RC trueReturnCode = RC_OK;
    RC falseReturnCode = RC_ERROR;
    if(name != ((char*)0)){
        // destroyingPageFile method
        destroyPageFile(name);
    }  
    //If the name is empty, then return error RC
    else if(name == ((char*)0)){
        return falseReturnCode;
    }
    return trueReturnCode;
}

/*
Function 7:
Function Name: getNumTuples
Function Related to: Table and Record Manager Functions
Function Description: Returns the number of tuples in the table
Function parameters: RM_TableData *rel
Function return type: int
*/
int getNumTuples(RM_TableData *rel){
    //returning the number of the tuples
    return ((FieldMgmtInfo * )rel->mgmtData)->numberOfFields;
}

/*-----------------------------------------
--------Part 2 - Handling Records in a Table Functions------------------
-----------------------------------------*/

/*
Function 8:
Function Name: insertRecord
Function Related to: Handling Records in a Table Functions
Function Description: When a new record is inserted the record manager should assign an RID to this record 
and update the record parameter passed to insertRecord
Function parameters: RM_TableData *rel, Record *record
Function return type: RC
*/
RC insertRecord(RM_TableData *rel, Record *record){
    RC returnCode = RC_OK;
    char *valueQ;
    FieldMgmtInfo *mgr = rel->mgmtData;
    RID *pointer = &record->id;
    pointer -> page = mgr -> firstEmptySlot;
    const PageNumber pageNum = pointer->page;

    BM_BufferPool *const buffManager = &mgr -> bmBuffPool;
    //calling the function 'pinPage'
    pinPage(buffManager, &mgr->bm_handle, pageNum);
    char *valueP = mgr -> bm_handle.data;
    pointer -> slot = searchEmptyBlock(getRecordSize(rel -> schema), valueP);
    while(zeroVal > pointer -> slot){
        // Calling unpingPage method
        unpinPage(buffManager, &mgr->bm_handle);
        pointer->page++;
        const PageNumber pNum = pointer -> page;
        //Calling pingPage method
        pinPage(buffManager, &mgr -> bm_handle, pNum);
        valueP = mgr -> bm_handle.data;
        pointer -> slot = searchEmptyBlock(getRecordSize(rel -> schema), valueP);
    }
    //calling addData method
    addData(mgr, pointer, record, valueQ, valueP, getRecordSize(rel -> schema));
    return returnCode;
}

/*
Function 9:
Function Name: deleteRecord
Function Related to: Handling Records in a Table Functions
Function Description: Deletes the record from the table
Function parameters: RM_TableData *rel, RID id
Function return type: RC
*/
RC deleteRecord(RM_TableData *rel, RID id){
    RC returnCode = RC_OK;
    int uniquePage = 0;
    uniquePage = id.page;
    FieldMgmtInfo *manage = rel->mgmtData;
    BM_BufferPool *const buffManager = &manage->bmBuffPool;
    BM_PageHandle *const currPage = &manage->bm_handle;
    //calling the function 'pinPage'
    pinPage(buffManager, currPage, uniquePage);
    manage -> firstEmptySlot = uniquePage;
    markDirty(buffManager, currPage);
    unpinPage(buffManager, currPage);
    return returnCode;
}

/*
Function 10:
Function Name: updateRecord
Function Related to: Handling Records in a Table Functions
Function Description: Updates the record in the table
Function parameters: RM_TableData *rel, Record *record
Function return type: RC
*/
RC updateRecord(RM_TableData *rel, Record *record) {
    RC returnCode = RC_OK;
    char *currPointer;
    RID recordID = record -> id;
    int capacity = 0; int flag = 0;
    FieldMgmtInfo *manage = rel -> mgmtData;
    BM_BufferPool *const buffManager = &manage -> bmBuffPool;
    BM_PageHandle *const currPage = &manage -> bm_handle;
    //calling the function 'pinPage'
    pinPage(buffManager, currPage, record -> id.page);
    capacity = getRecordSize(rel -> schema);
    currPointer = manage -> bm_handle.data;
    flag = recordID.slot * capacity;
    currPointer = currPointer + flag;
    *currPointer = '+';
    int temp = capacity - 1;
    memcpy(++currPointer, record -> data + 1, temp);
    markDirty(buffManager, currPage);
    //calling the function 'unpinPage'
    unpinPage(buffManager, currPage);
    return returnCode;
}

/*
Function 11:
Function Name: getRecord
Function Related to: Handling Records in a Table Functions
Function Description: Fetches the record from the table
Function parameters: RM_TableData *rel, RID id, Record *record
Function return type: RC
*/
RC getRecord(RM_TableData *rel, RID id, Record *record) {
    FieldMgmtInfo *manage = rel -> mgmtData;
    RC trueReturnCode = RC_OK;
    RC falseReturnCode = RC_ERROR; 
    int uniquePage = 0;
    int capacity = 0; int flag = 0;
    uniquePage= id.page;
    BM_BufferPool *const buffManager = &manage -> bmBuffPool;
    BM_PageHandle *const currPage = &manage -> bm_handle;
    //calling the function 'pinPage'
    pinPage (buffManager, currPage, uniquePage);
    capacity = getRecordSize(rel -> schema);
    char *pointer = manage -> bm_handle.data;
    flag = id.slot * capacity;
    pointer = pointer + flag;
    if (*pointer == '+'){
        record -> id = id;
        char *dPointer = record -> data;
        int temp = capacity - 1;
        memcpy(++dPointer, pointer + 1, temp);
    }
    else {
        return falseReturnCode;
    }
    //calling the function 'unpinPage'
    unpinPage(buffManager, currPage);
    return trueReturnCode;
}

/*-----------------------------------------
--------Part 3 - Scans Functions------------------
-----------------------------------------*/

/*
Function 12:
Function Name: startScan
Function Related to: Scans Functions
Function Description: 
Function parameters: RM_TableData *rel, RM_ScanHandle *scan, Expr *cond
Function return type: RC
*/
RC startScan(RM_TableData *rel, RM_ScanHandle *scan, Expr *cond) {
    FieldMgmtInfo *structRM;
    RC falseReturnCode = RC_ERROR;
    RC trueReturnCode = RC_OK;
    FieldMgmtInfo *manage;
    char *name = "ScanTable";
    if(cond != NULL){
        // calling openTable method
        openTable (rel, name);
        structRM = (FieldMgmtInfo *) malloc(sizeof(FieldMgmtInfo));
        scan -> mgmtData = structRM;
        // calling update method
        update(structRM);
        structRM -> findField = cond;
        manage = rel -> mgmtData;
        manage -> numberOfFields = elementName;
        scan -> rel = rel;
        return trueReturnCode;
    }
    else {
        return falseReturnCode;
    }
}

/*
Function 13:
Function Name: next
Function Related to: Scans Functions
Function Description: Returns the next tuple that fulfills the scan condition
Function parameters: RM_ScanHandle *scan, Record *record
Function return type: RC
*/
RC next(RM_ScanHandle *scan, Record *record) {
    bool flag = false;
    int fieldSize, numRows, blockCount, visited, myBlock;
    
    RC returnCode = RC_OK;
    RC falseReturnCode = RC_ERROR;
    RC tuplesReturnCode = RC_RM_NO_MORE_TUPLES;
    Value *output = (Value *) malloc (sizeof(Value));
    FieldMgmtInfo *structRM = scan ->rel-> mgmtData;
    FieldMgmtInfo *mgr = scan -> mgmtData;
    BM_BufferPool *const buffManager = &structRM -> bmBuffPool;
    BM_PageHandle *const page = &mgr -> bm_handle;
    Schema *tableInfo = scan -> rel -> schema;
    if(mgr -> findField != NULL) {
        if((0 == structRM ->numberOfFields)) {
            return tuplesReturnCode;
        }
        while ((structRM -> numberOfFields) >= (mgr -> numberOfTuples) ) {
            if (0 < (mgr -> numberOfTuples) ) {
                int slot = mgr -> fieldIdentifier.slot;
                mgr -> fieldIdentifier.slot = slot + 1;
                myBlock = mgr -> fieldIdentifier.slot;
                if (myBlock >= PAGE_SIZE / getRecordSize(tableInfo)) {
                    mgr ->  fieldIdentifier.page = mgr -> fieldIdentifier.page + 1;
                    mgr -> fieldIdentifier.slot = zeroVal;
                }
            }
            else {
                flag = true;
                if(flag){
                    update(mgr);
                }
            } 
            int flag = mgr -> numberOfTuples;
            int capacity = getRecordSize(tableInfo);
            functionUpdate(mgr, structRM, record, capacity, flag);
            evalExpr (record, tableInfo, mgr -> findField, &output);
            if(TRUE == output -> v.boolV ) {
                unpinPage(buffManager, page);
                return returnCode;
            }
        }
    } 
    else {
        return falseReturnCode;
    }  
    //calling the function 'unpinPage'
    unpinPage(buffManager, page);
    update(mgr);
    return RC_RM_NO_MORE_TUPLES;
}

/*
Function 14:
Function Name: closeScan
Function Related to: Scans Functions
Function Description: 
Function parameters: RM_ScanHandle *scan
Function return type: RC
*/
RC closeScan(RM_ScanHandle *scan) {
    int val = 0;
    RC returnCode = RC_OK;
    FieldMgmtInfo *manage = scan -> mgmtData;
    val = manage -> numberOfTuples;
    FieldMgmtInfo *structRM = scan -> rel -> mgmtData;
    BM_BufferPool *const buffManage = &structRM -> bmBuffPool;
    BM_PageHandle *const page = &manage -> bm_handle;
    if (val > 0) {
        //calling the function 'unpinPage'
        unpinPage(buffManage, page);
        update(manage);
    }
    // Setting up mgmtdata to null
    scan -> mgmtData = NULL;
    free(scan -> mgmtData);
    return returnCode;
}

/*-----------------------------------------
--------Part 4 - Dealing with Schemas Functions------------------
-----------------------------------------*/

/*
Function 15:
Function Name: getRecordSize
Function Related to: Dealing with Schemas Functions
Function Description: Gets the record size
Function parameters: Schema *schema
Function return type: int
*/
int getRecordSize (Schema *schema) {
    int rSize = 0;
    int index = 0;
    int target = schema -> numAttr;
    // Iterating through all attributes in the schema
    for(int index = 0; index < target; index++){
        // check condition dataType == 0
        if(DT_INT == schema -> dataTypes[index]) {
            rSize += sizeof(int);
        }
        else if(schema -> dataTypes[index] != DT_INT) {
            rSize += schema -> typeLength[index];
        }
    }
    rSize++;
    return rSize;
}

/*
Function 16:
Function Name: createSchema
Function Related to: Dealing with Schemas Functions
Function Description: This function creates the schema
Function parameters: int numAttr, char **attrNames, DataType *dataTypes, int *typeLength, int keySize, int *keys
Function return type: Schema
*/
Schema *createSchema(int numAttr, char **attrNames, DataType *dataTypes, int *typeLength, int keySize, int *keys) {
    // Allocating memory space to schema
    Schema *tableInfo = (Schema *) malloc (sizeof(Schema)); 
    // Check condition whether memory allocated or not
    if(tableInfo == NULL){
        return NULL;
    }
    else {
        // Setting value to attributed of schema
        tableInfo -> numAttr = numAttr;
        tableInfo -> typeLength = typeLength;
        tableInfo -> keyAttrs = keys;
        tableInfo -> attrNames = attrNames;
        tableInfo -> keySize = keySize;
        tableInfo -> dataTypes = dataTypes;
        // Returning schema
        return tableInfo;
    }
}

/*
Function 17:
Function Name: freeSchema
Function Related to: Dealing with Schemas Functions
Function Description: 
Function parameters: Schema *schema
Function return type: RC
*/
RC freeSchema (Schema *schema) {
    RC returnCode = RC_OK;
    RC noReturnCode = NULL;
    bool flag = false;
    if (schema != NULL) {
        // Deallocating memory space occupied by 'schema'
        free(schema);
        // set flag to true
        flag = true;
    }
    if(flag){
        return returnCode;
    }
    return noReturnCode;
}

/*-----------------------------------------
--------Part 5 - Dealing with records and attribute Functions------------------
-----------------------------------------*/

/*
Function 18:
Function Name: createRecord
Function Related to: Dealing with records and attribute Functions
Function Description: Creates the new record for the given schema
Function parameters: Record **record, Schema *schema
Function return type: RC
*/
RC createRecord(Record **record, Schema *schema) {
    RC returnCode = RC_OK;
    int negVal = -1;
    // Allocating memory space for the new record
    Record *currField = (Record *) malloc(sizeof(Record));
    int currSlot = 0; int currPage = 0;
    currSlot = currField -> id.slot;
    currPage = currField -> id.page;
    // Allocating memory space for the new record
    currField -> data = (char *) malloc (getRecordSize (schema));
    currSlot = negVal;
    currPage = negVal;
    char *currFieldPointer = currField -> data;
    // '-' is used for Tombstone mechanism. We set '-' because the record is empty.
    *currFieldPointer = '-';
    currFieldPointer++;
    // Appending '\0' which means NULL in C Programming to the record after tombstone.
    *currFieldPointer ='\0';
    *record = currField;
    return returnCode;
}

/*
Function 19:
Function Name: freeRecord
Function Related to: Dealing with records and attribute Functions
Function Description: 
Function parameters: Record *record
Function return type: RC
*/
RC freeRecord(Record *record) {
    RC returnCode = RC_OK;
    RC noReturnCode = NULL;
    bool flag = false;
    if(record != NULL){
        // Deallocate memory space allocated to record using free method
        free (record);
        // set flag = true once the space gets free
        flag = true;
    }
    if(flag){
        return returnCode;
    }
    return noReturnCode;
}

/*
Function 20:
Function Name: getAttr
Function Related to: Dealing with records and attribute Functions
Function Description: 
Function parameters: Record *record, Schema *schema, int attrNum, Value **value
Function return type: RC
*/
RC getAttr(Record *record, Schema *schema, int attrNum, Value **value) {
    RC returnCode = RC_OK;
    int diffVal = 0, size = 0;
    int numVal = 0;
    int *output = &diffVal;
    bool flag = true;
    int one = 1;
    *output = 1;
    //itterating till i less than value of attrNum
    for(int i = 0; i < attrNum; i++){
        if(schema -> dataTypes[i] == DT_INT) {
            *output += sizeof (int);
        }
        else {
            *output += schema -> typeLength[i];
        }
    }
    Value *field = (Value*) malloc (sizeof (Value));
    char *datainfo = record -> data;
    datainfo = datainfo + diffVal;
    if(attrNum == one){
        schema -> dataTypes[attrNum] = 1;
    }    
    if(schema -> dataTypes[attrNum] == DT_INT){
        int count = 0;
        memcpy(&count, datainfo, sizeof(int));
        field -> dt = DT_INT;
        field -> v.intV = count;
    }
    else if(schema -> dataTypes[attrNum] != DT_INT){
        int len = schema -> typeLength[attrNum];
        size = schema -> typeLength[attrNum];
        // Allocatting the space for string having size of 'length'
        field -> v.stringV = (char *)malloc(size + 1);
        if(flag){
            // Copying string to location pointed by dataPointer, and also appending '\0' which denotes end of string in C
            strncpy(field -> v.stringV, datainfo, size);
            field -> dt = DT_STRING;
            field -> v.stringV[size] = '\0';
        }
    }
    *value = field;
    return returnCode;
}

/*
Function 21:
Function Name: setAttr
Function Related to: Dealing with records and attribute Functions
Function Description: 
Function parameters: Record *record, Schema *schema, int attrNum, Value *value
Function return type: RC
*/
RC setAttr(Record *record, Schema *schema, int attrNum, Value *value){
    int returnCode = RC_OK;
    int size = 0, diffVal = 0;
    int count = 0;
    int *output = &diffVal;
    bool flag = true;
    *output = 1;
    for(int i = 0; i < attrNum; i++){
        if(schema -> dataTypes[i] == DT_INT){
            *output += sizeof(int);
        }
        else{
            *output += schema->typeLength[i];
        }
    }
    char *datainfo = record->data;
    //setting the value for datainfo
    datainfo += diffVal;
    if(schema -> dataTypes[attrNum] == DT_INT){
        *(int *) datainfo = value -> v.intV;
        datainfo += sizeof(int);
    }
    else if(schema -> dataTypes[attrNum] != DT_INT){
        if(flag){
            size = schema -> typeLength[attrNum];
            strncpy(datainfo, value -> v.stringV, size);
            datainfo = datainfo + schema -> typeLength[attrNum];
        }
    }
    return returnCode;
}
