#include <string.h>
#include <math.h>
#include "stdarg.h"
#include "btree_mgr.h"
#include "storage_mgr.h"
#include "buffer_mgr.h"
#include <stdlib.h>
#include "unistd.h"
#define RC_ERROR 5

ElementInfo *initializationTree (int memory){
    ElementInfo *dataBTree = (ElementInfo *) malloc(sizeof(ElementInfo));
    dataBTree -> CAPACITY = memory;
    dataBTree -> COMPLETE = 0;
    dataBTree -> DESC = (int *) malloc(sizeof(int) * memory);
    return dataBTree;
}


BPlusTree *nodeCreate( int memory, int check, int countNode) {
    BPlusTree *dataBTree = (BPlusTree *) malloc(sizeof(BPlusTree));
    dataBTree -> NUMBEROFNODES = countNode;
    dataBTree -> TREERIGHT = NULL;
    dataBTree -> TREELEFT = NULL;
    dataBTree -> CHECKCHILD = check;
    ElementInfo *dList = (ElementInfo *) malloc(sizeof(ElementInfo));
    dList -> DESC = (int *) malloc(sizeof(int) * memory);
    dList -> COMPLETE = 0;
    dList -> CAPACITY = memory;
    dataBTree -> INHERIT = NULL;
    dataBTree -> FIELDS = dList;

    if(check){//do Nothing
    } else{
        ElementInfo *subtree = (ElementInfo *)malloc(sizeof(ElementInfo));
        subtree -> DESC = (int *) malloc(sizeof(int) * (memory + 1));
        subtree -> COMPLETE = 0;
        subtree -> CAPACITY = memory + 1;
        dataBTree -> ARRAYNODE = (BPlusTree *) malloc(sizeof(BPlusTree) * (memory + 1));
        dataBTree -> BRANCH = subtree;
        return dataBTree;    
    }

    dataBTree -> CHILDLID = initializationTree (memory);
    ElementInfo *subtreeInfo = (ElementInfo *)malloc (sizeof(ElementInfo));
    subtreeInfo -> DESC = (int *) malloc(sizeof(int) * memory);
    subtreeInfo -> COMPLETE = 0;
    subtreeInfo -> CAPACITY = memory;
    dataBTree -> CHILDRID = subtreeInfo;
    return dataBTree;
}

int searchKeyInTree(ElementInfo *dList, int number, int *pointer) {
  int endValue = dList -> COMPLETE - 1;
  if (endValue < 0) { 
    (*pointer) = 0;
    return -1;
  }else{
  }
  int value = 0;
  int index = 0;
  while(true) {
    index = (value + endValue) / 2;
    if (dList->DESC[index] == number){
      while(index && dList->DESC[index - 1] == number){ 
        index--;
      }
      (*pointer) = index;
      return index;
    }
    if (endValue <= value ) {
      if (dList->DESC[value] < number  )  value = value + 1;
      (*pointer) = value; 
      return -1; 
    }
    if(dList->DESC[index] <= number ) {
        value = index + 1;
    }else{
        endValue = index - 1;
    }
  }
}


BPlusTree *searchBtreeNode(BTreeHandle *bHandle, int value) {
  int num;
  BPlusTree *val = bHandle -> root;
  bool flag = val != NULL;
  while(flag && !val -> CHECKCHILD) {
    bool check = searchKeyInTree(val -> FIELDS, value, &num) >= 0;
    if (check){
        num = num + 1;
    }
    val = val -> ARRAYNODE[num];
  }
  return val;
}


int insertAtIndex(ElementInfo *dList, int value, int position) {
  bool check1 = position <= dList -> COMPLETE;
  bool check2 = dList -> COMPLETE < dList -> CAPACITY;
  bool check = check1 && check2;
  bool pos = position == dList->COMPLETE;
  if (check){
    if (!pos){
        int comp = dList->COMPLETE;
        while(comp > position){
          dList->DESC[comp] = dList->DESC[comp- 1];
          comp--;
        }
    }
    dList -> DESC[position] = value;
    dList -> COMPLETE = dList -> COMPLETE + 1;
    return position;
  }
  return -1; 
}

int insertTreeIndex(ElementInfo *dList, int value) {
  int index = 0;
  int num = -1;
  if(dList -> CAPACITY > dList -> COMPLETE){
    index = searchKeyInTree(dList, value, &num);
    num = insertAtIndex(dList, value, num);
  }
  return num;
}

void deleteAtIndex (ElementInfo *dList, int value, int temp){
    dList -> COMPLETE = dList -> COMPLETE - temp;
    int comp = dList -> COMPLETE;
    int index = value;
    while(comp > index){
        dList -> DESC[index] = dList -> DESC[index + temp];
        index++;
    }
}

void obtainRightChild (int value, BPlusTree *btree, BPlusTree *rchild){
    value = value + 1;
    int val = rchild -> NUMBEROFNODES;
    insertAtIndex(btree -> BRANCH, val, value);
    int index = btree -> FIELDS -> COMPLETE;
    while (value < index){
        btree -> ARRAYNODE[index] = btree -> ARRAYNODE[index - 1];
        index--;
    }
    btree -> ARRAYNODE[value] = rchild;
}

int updateNodes (BPlusTree *ancestor, int number, BPlusTree *gNode, int value){
    ancestor -> FIELDS -> COMPLETE = number;
    ancestor -> BRANCH -> COMPLETE = gNode -> BRANCH -> COMPLETE - value;
    return ancestor -> BRANCH -> COMPLETE;
}

RC modifyAncestor(BTreeHandle *tHandle, BPlusTree *right, BPlusTree *left, int uniqueIdx) {
  BPlusTree *ancestorNode = left->INHERIT;
  int completeLeft,completeRight; 
  int number,value;
  BPlusTree *ancestorR;

  if(ancestorNode != NULL){//doNothing
  } else {
      ancestorNode = nodeCreate(tHandle->CAPACITY, 0, tHandle->NEXTPGINFO);
       insertAtIndex(ancestorNode->BRANCH, left->NUMBEROFNODES, 0);
       ancestorNode->ARRAYNODE[0] = left;

       for(int k = 0 ; k < 3; k++){
           switch (k)
           {
           case 1:
                tHandle->DEPTH++;
                tHandle->NUMBEROFCHILDREN++;
                break;
           case 0:
                tHandle->INDEXROOT = ancestorNode->NUMBEROFNODES; 
                tHandle->NEXTPGINFO++;
                break;
           default: tHandle->root = ancestorNode;
               break;
           }
       }
  }
    right -> INHERIT = ancestorNode;
    left -> INHERIT = ancestorNode;
    
    int position = insertTreeIndex(ancestorNode->FIELDS, uniqueIdx);

    if(position < 0){

        BPlusTree * node = nodeCreate(tHandle->CAPACITY + 1, 0, -1);
        node -> FIELDS -> COMPLETE = ancestorNode -> FIELDS -> COMPLETE;
        node -> BRANCH -> COMPLETE = ancestorNode -> BRANCH -> COMPLETE;
        int complete = ancestorNode -> BRANCH -> COMPLETE;
        memmove (node -> BRANCH -> DESC, ancestorNode -> BRANCH -> DESC, sizeof(int) * complete);
        memmove (node -> FIELDS -> DESC, ancestorNode -> FIELDS -> DESC, sizeof(int) * complete);
        memmove (node -> ARRAYNODE, ancestorNode -> ARRAYNODE, sizeof(BPlusTree *) * complete);

      position = insertTreeIndex(node->FIELDS, uniqueIdx);
      insertAtIndex(node->BRANCH, right->NUMBEROFNODES, position + 1);
      int k = 0; 
      int p = ancestorNode->BRANCH->COMPLETE;
      while(2 > k || position + 1 < p){
          if(p > position + 1){
            node->ARRAYNODE[p] = node->ARRAYNODE[p - 1];
            p--;  
          }else{
              ++k;
              if(k!=1){
                  tHandle->NUMBEROFCHILDREN = tHandle->NUMBEROFCHILDREN + 1;
                  tHandle->NEXTPGINFO = tHandle->NEXTPGINFO++;
                  ancestorNode->BRANCH->COMPLETE = completeLeft + 1;
                  ancestorNode->FIELDS->COMPLETE = completeLeft;
                  number = ancestorNode->BRANCH->COMPLETE;
              }else{
                  node->ARRAYNODE[++position] = right;
                  completeLeft = node->FIELDS->COMPLETE / 2;
                  completeRight = node->FIELDS->COMPLETE - completeLeft;
                  ancestorR = nodeCreate(tHandle->CAPACITY, 0, tHandle->NEXTPGINFO);
              } 
          }
      }
      
      value = updateNodes(ancestorR, completeRight, node,number);

      for(int pointer = 0; pointer < 2; pointer++){
          if(pointer == 0){
            memcpy(ancestorNode->ARRAYNODE, node->ARRAYNODE, sizeof(BPlusTree *) * number);
            memcpy(ancestorNode->FIELDS->DESC, node->FIELDS->DESC, sizeof(int) * completeLeft);
            memcpy(ancestorNode->BRANCH->DESC, node->BRANCH->DESC, sizeof(int) * number);
          } else {
            memcpy(ancestorR->ARRAYNODE, node->ARRAYNODE + number, sizeof(BPlusTree *) * value);
            memcpy(ancestorR->FIELDS->DESC, node->FIELDS->DESC + completeLeft, sizeof(int) * completeRight);
            memcpy(ancestorR->BRANCH->DESC, node->BRANCH->DESC + number, sizeof(int) * value);
          }
      }
      uniqueIdx = ancestorR->FIELDS->DESC[0];
      deleteAtIndex(ancestorR->FIELDS, 0, 1);
      return modifyAncestor(tHandle, ancestorR, ancestorNode, uniqueIdx);
    } else {
        obtainRightChild(position,ancestorNode,right);
    }
}


RC initIndexManager (void *mgmtData){
    RC returnCode = RC_OK;
    return returnCode;
}


RC shutdownIndexManager(){
    RC returnCode = RC_OK;
    return returnCode;
}

BPlusTree *setPosition (BTreeHandle *tHandle, BPlusTree *child){
    child = nodeCreate (tHandle -> CAPACITY, 1, tHandle -> NEXTPGINFO);
    tHandle -> INDEXROOT = child -> NUMBEROFNODES;
    tHandle -> NEXTPGINFO += 1;
    tHandle -> root = child;
    tHandle -> DEPTH += 1;
    tHandle -> NUMBEROFCHILDREN += 1;
    return child;
}


int modifyIndex(DataType dType, char *value){
    int val = 0;
    memcpy(value, &dType, sizeof(int));
    value += sizeof(int);
    return val;
}

RC createBtree(char *idxId, DataType keyType, int n){
    int returnCode = RC_OK;
    SM_FileHandle *fHandle = (SM_FileHandle *)malloc(sizeof(SM_FileHandle));
    int add = 0;
    char *first = (char *)calloc(PAGE_SIZE, sizeof(char));
    createPageFile(idxId);
    openPageFile(idxId, fHandle);
    memmove(first, &n, sizeof(int));
    int value = modifyIndex(&keyType, first + sizeof(int));
    int nextPg = 1 , height = 0, records = 0, total = 0;
    for(int add = 0; add <4; add++){
        if(add == 0){
            add++;
            memcpy(first + sizeof(int), &value, sizeof(int));
        } else if (add == 1){
            add++; 
            memcpy(first + sizeof(int) + sizeof(int), &total, sizeof(int));
        } else if (add == 2){
            add++;
            memcpy(first + sizeof(int) + sizeof(int) + sizeof(int), &records, sizeof(int));
        } else if (add == 3){
            add++; 
            memcpy(first + sizeof(int) + sizeof(int) + sizeof(int) + sizeof(int), &height, sizeof(int));
        } else {
            memcpy(first + sizeof(int) + sizeof(int) + sizeof(int) + sizeof(int) + sizeof(int), &nextPg, sizeof(int));
        }
    }
    writeBlock(0, fHandle, first);
    closePageFile(fHandle);
    free(first);
    free(fHandle);
    return returnCode;
}

void setFunction(char *ptr, BTreeHandle *tree){
    int sizeOfInt = sizeof(int);
    memmove(&tree->keyType, ptr + sizeOfInt, sizeOfInt);
    memmove(&tree->NEXTPGINFO, ptr + sizeOfInt + sizeOfInt + sizeOfInt + sizeOfInt + sizeOfInt + sizeOfInt,sizeOfInt);
    memmove(&tree->NUMBEROFCHILDREN, ptr + sizeOfInt + sizeOfInt + sizeOfInt, sizeOfInt);
    memmove(&tree->INDEXROOT, ptr + sizeOfInt + sizeOfInt, sizeOfInt);
    memmove(&tree->DEPTH, ptr + sizeOfInt + sizeOfInt + sizeOfInt + sizeOfInt + sizeOfInt, sizeOfInt);
    memmove(&tree->CAPACITY, ptr, sizeOfInt);
    memmove(&tree->NUMBEROFFIELDS, ptr + sizeOfInt + sizeOfInt + sizeOfInt + sizeOfInt, sizeOfInt);
}


int modifyTreeKey(BPlusTree *node, Value *index, RID rid){
    int indexV = index->v.intV;
    int result = insertTreeIndex(node->FIELDS, indexV);
    int val1 = rid.slot;
    int val2 = rid.page;
    insertAtIndex(node->CHILDLID, val1, result);
    insertAtIndex(node->CHILDRID, val2, result);
    return result;
}


RC openBtree(BTreeHandle **tree, char *idxId){
    int x = 0 ;
    BTreeHandle *node = (BTreeHandle *) malloc(sizeof(BTreeHandle));
    BM_BufferPool *bufPool = (BM_BufferPool *) malloc(sizeof(BM_BufferPool));
    initBufferPool(bufPool, idxId, 10, RS_LRU, NULL);
    BM_PageHandle *bmPg = (BM_PageHandle *) malloc(sizeof(BM_PageHandle));
    pinPage(bufPool, bmPg, x);
    node->idxId = idxId;
    node->mgmtData = bufPool;
    setFunction(bmPg->data, node);
    node->root = NULL;
    int nodeDepth = node->DEPTH;
    (BPlusTree *) malloc(sizeof(BPlusTree) * nodeDepth);
    *tree = node;
    return RC_OK;
}


BPlusTree *obtainChildInfo(BTreeHandle *bHandle, BPlusTree *node, int capacity, int valueR, int valueL){
    bHandle->NUMBEROFCHILDREN = bHandle->NUMBEROFCHILDREN + 1;
    bHandle->NEXTPGINFO = bHandle->NEXTPGINFO + 1;
    BPlusTree *child = nodeCreate(bHandle->CAPACITY,1, bHandle->NEXTPGINFO);
    child->FIELDS->COMPLETE = valueR;
    child->CHILDRID->COMPLETE = valueR;child->CHILDLID->COMPLETE = valueR;
    memmove(child->CHILDRID->DESC, node->CHILDRID->DESC + valueL, capacity);
    memmove(child->CHILDLID->DESC, node->CHILDLID->DESC + valueL, capacity);
    memmove(child->FIELDS->DESC, node->FIELDS->DESC + valueL, capacity);
    return child;
}


RC closeBtree(BTreeHandle *tree){
    shutdownBufferPool(tree->mgmtData);
    if(tree -> root == NULL){
    } else{
        BPlusTree *child = tree->root;
        while(!child->CHECKCHILD){
            child = child->ARRAYNODE[0];
        } 
        BPlusTree *ancestor = child->INHERIT;
        while(child != NULL){
            child = child -> TREERIGHT;
        } 
        child = ancestor;
        ancestor = child -> INHERIT;
    }
    return RC_OK;
}


RC deleteBtree(char *idxId){
    RC returnCode = RC_OK;
    unlink(idxId);
    return returnCode;
}


RC getNumNodes(BTreeHandle *tree, int *result){
    RC returnCode = RC_OK;
    *result = tree->NUMBEROFCHILDREN;
    return returnCode;
}

RC getNumEntries(BTreeHandle *tree, int *result){
    RC returnCode = RC_OK;
    *result = tree -> NUMBEROFFIELDS;
    return returnCode;
}

RC getKeyType(BTreeHandle *tree, DataType *result){
    RC returnCode = RC_OK;
    *result = tree->keyType;
    return returnCode;
}

void obtainLeftChild(BPlusTree *child, int valueL, BPlusTree *node){
    child->FIELDS->COMPLETE = valueL;
    child->CHILDRID->COMPLETE = valueL;
    child->CHILDLID->COMPLETE = valueL;
    int sizeValueL = sizeof(int) * valueL;
    memmove(child->CHILDLID->DESC, node->CHILDLID->DESC, sizeValueL);
    memmove(child->CHILDRID->DESC, node->CHILDRID->DESC, sizeValueL);
    memmove(child->FIELDS->DESC, node->FIELDS->DESC,sizeValueL);
}

RC findKey(BTreeHandle *tree, Value *key, RID *result){
    BPlusTree *child = searchBtreeNode(tree, key->v.intV);
    int value;
    int searchKeyTree = searchKeyInTree(child->FIELDS, key->v.intV, &value);
    if(child == NULL){
        return RC_ERROR;
    }
    else if(searchKeyTree < 0){
        return RC_IM_KEY_NOT_FOUND;
    } else {
        result->slot = child->CHILDLID->DESC[searchKeyTree];
        result->page = child->CHILDRID->DESC[searchKeyTree];
    }
    return RC_OK;
}

char *printTree(BTreeHandle *tree){
    int returnCode = RC_OK;

    return returnCode;
}

RC insertKey(BTreeHandle * tree, Value *key, RID rid){
    int val1, index, addVal, subVal;
    addVal =0;
    BPlusTree *child = searchBtreeNode(tree, key -> v.intV);

    for(int addVal = 0; addVal < 2; addVal++){
        if(addVal == 0){
            if (NULL == child || child == "" ) child = setPosition(tree, child);
            addVal = addVal + 1;
        } 
        if (addVal == 1){
            if(searchKeyInTree(child -> FIELDS, key -> v.intV, &val1) < 0){}
            else return RC_IM_KEY_ALREADY_EXISTS;
            addVal = addVal + 1;
        }
    }

    index = insertTreeIndex(child -> FIELDS, key -> v.intV);

    if(index >= 0){
        insertAtIndex(child -> CHILDRID, rid.page, index);
        insertAtIndex(child -> CHILDLID, rid.slot, index);
    } else {
        BPlusTree *nextBTree =  nodeCreate(tree -> CAPACITY +1, 1, -1);

        int addVal = 0;
        while (addVal < 3 )
        {
            if(addVal == 0){
                int COMPLETE = child -> FIELDS -> COMPLETE; 
                memmove(nextBTree -> FIELDS -> DESC, child -> FIELDS -> DESC, sizeof(int) * COMPLETE);
                nextBTree -> FIELDS -> COMPLETE = COMPLETE;
            }else if(addVal > 1){
                int COMPLETE = child -> CHILDLID -> COMPLETE;
                memmove (nextBTree -> CHILDLID -> DESC, child -> CHILDLID -> DESC, sizeof(int) * COMPLETE);
                nextBTree -> CHILDLID -> COMPLETE = COMPLETE;
            }else {
                int COMPLETE = child -> CHILDRID -> COMPLETE;
                memmove(nextBTree -> CHILDRID -> DESC, child -> CHILDRID -> DESC, sizeof(int) * COMPLETE);
                nextBTree -> CHILDRID -> COMPLETE = COMPLETE;
            }

            addVal++;
        }

        index = modifyTreeKey (nextBTree, key, rid);
        subVal = 2;
        
        BPlusTree *childInfoR = obtainChildInfo (tree, nextBTree, sizeof(int) * nextBTree -> FIELDS -> COMPLETE - ceil((float) nextBTree -> FIELDS -> COMPLETE / 2),
            nextBTree -> FIELDS -> COMPLETE - ceil((float) nextBTree -> FIELDS -> COMPLETE / 2), 
            ceil((float) nextBTree -> FIELDS -> COMPLETE / 2));
        obtainLeftChild(child, ceil((float) nextBTree -> FIELDS -> COMPLETE / 2), nextBTree);

        while (subVal > 0)
        {
            if(subVal < 2){
                child -> TREERIGHT = childInfoR;
                childInfoR -> TREELEFT = child;
                modifyAncestor(tree, childInfoR,child,childInfoR -> FIELDS -> DESC[0]);
            }else{
                childInfoR ->  TREERIGHT = child -> TREERIGHT;
                if (childInfoR -> TREERIGHT != NULL) childInfoR -> TREERIGHT -> TREELEFT = childInfoR;
            }
            subVal = subVal - 1;
        }
    }
    tree -> NUMBEROFFIELDS += 1;
    return RC_OK;
}

void changeValues(RID *output, InfoSM *childInfo){
    output -> page = childInfo -> NODECURRENT -> CHILDRID -> DESC[childInfo -> indexSM];
    output -> slot = childInfo -> NODECURRENT -> CHILDLID -> DESC[childInfo -> indexSM];
    int indexSM = childInfo -> indexSM;
    childInfo -> indexSM = indexSM + 1;
}


RC deleteKey (BTreeHandle *tree, Value *key){
    BPlusTree *child = searchBtreeNode (tree, key -> v.intV);
    if (child != NULL){
        int value;
        int gotAns = searchKeyInTree(child -> FIELDS, key -> v.intV, &value);

        if(searchKeyInTree(child -> FIELDS, key -> v.intV, &value) >= 0){
        } else {
            return RC_IM_KEY_NOT_FOUND;
        }
        
        deleteAtIndex(child -> CHILDLID, gotAns, 1);
        deleteAtIndex(child -> FIELDS, searchKeyInTree(child -> FIELDS, key -> v.intV, &value), 1);
        deleteAtIndex(child -> CHILDRID, gotAns, 1);
        tree -> NUMBEROFFIELDS = tree -> NUMBEROFFIELDS - 1;
    } else {
        return RC_ERROR;
    }
    return RC_OK;
}

RC openTreeScan (BTreeHandle *tree, BT_ScanHandle **handle){
    InfoSM *scanInfo = (InfoSM *)malloc(sizeof(InfoSM));
    BT_ScanHandle *bHandle = (BT_ScanHandle *) malloc(sizeof(BT_ScanHandle));
    bHandle -> tree = tree;
    if(NULL != bHandle -> tree){
        scanInfo -> NODECURRENT = tree -> root;
        int value =0;
        while(true){
            if( !scanInfo -> NODECURRENT->CHECKCHILD || value < 1){
                if(!scanInfo -> NODECURRENT -> CHECKCHILD) {
                    scanInfo -> NODECURRENT = scanInfo -> NODECURRENT -> ARRAYNODE[0];
                }
                else{
                    value++;
                    scanInfo -> indexSM = 0;
                    bHandle -> mgmtData = scanInfo;
                    *handle = bHandle;
                    break;
                }
            }
        }
    } else {
        return RC_ERROR;
    }
    return RC_OK;
}

RC nextEntry (BT_ScanHandle *handle, RID *result){
    if(NULL != handle){
        InfoSM *mgrInfo = handle -> mgmtData;
        int value = 0;
        while(value < 2){
            bool info = mgrInfo -> NODECURRENT -> CHILDRID -> COMPLETE <= mgrInfo -> indexSM;
            bool check = mgrInfo -> indexSM == mgrInfo -> NODECURRENT -> FIELDS -> COMPLETE;
            if(false != info){
                if(NULL == mgrInfo -> NODECURRENT -> TREERIGHT){
                    if(true == check){
                        return RC_IM_NO_MORE_ENTRIES;
                    }
                }
                else{
                    mgrInfo -> NODECURRENT = mgrInfo -> NODECURRENT -> TREERIGHT;
                    (*mgrInfo).indexSM = 0;
                }
            }
            value++;
        }
        result -> page = mgrInfo -> NODECURRENT -> CHILDRID -> DESC[mgrInfo -> indexSM];
        result -> slot = mgrInfo -> NODECURRENT -> CHILDLID -> DESC[mgrInfo -> indexSM];
        mgrInfo -> indexSM = mgrInfo -> indexSM + 1;
    } else {
        return RC_ERROR;
    }
    return RC_OK;
}

RC closeTreeScan(BT_ScanHandle *handle){
    BT_ScanHandle *bHandle = (BT_ScanHandle *) malloc(sizeof(BT_ScanHandle));
    if(bHandle -> tree != NULL){
    } else {
        return RC_ERROR;
    }
    if(handle != NULL){
        handle -> tree = NULL;
        handle -> mgmtData = NULL;
    }
    free(handle);
    return RC_OK;
}
