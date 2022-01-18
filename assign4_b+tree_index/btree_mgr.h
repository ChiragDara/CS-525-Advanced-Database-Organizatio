#ifndef BTREE_MGR_H
#define BTREE_MGR_H

#include "dberror.h"
#include "tables.h"
#include "dt.h"

// structure for accessing btrees
typedef struct ElementInfo{
  int COMPLETE;
  int *DESC;
  int CAPACITY;
}ElementInfo;

typedef struct BPlusTree{
  int NUMBEROFNODES;
  struct BPlusTree *INHERIT;
  int CHECKCHILD;
  int CAPACITY;
  ElementInfo *FIELDS;
  ElementInfo *BRANCH;
  struct BPlusTree *TREERIGHT;
  ElementInfo *CHILDRID;
  struct BPlusTree **ARRAYNODE;
  ElementInfo *CHILDLID;
  struct BPlusTree *TREELEFT;
}BPlusTree;

typedef struct InfoSM{
  BPlusTree *NODECURRENT;
  int indexSM;
}InfoSM;

typedef struct BTreeHandle {
  DataType keyType;
  char *idxId;
  void *mgmtData;
  int NUMBEROFCHILDREN;
  int NUMBEROFFIELDS;
  int DEPTH;
  int INDEXROOT;
  int NEXTPGINFO;
  BPlusTree *root;
  int CAPACITY;
} BTreeHandle;

typedef struct BT_ScanHandle {
  BTreeHandle *tree;
  void *mgmtData;
} BT_ScanHandle;


// init and shutdown index manager
extern RC initIndexManager (void *mgmtData);
extern RC shutdownIndexManager ();

// create, destroy, open, and close an btree index
extern RC createBtree (char *idxId, DataType keyType, int n);
extern RC openBtree (BTreeHandle **tree, char *idxId);
extern RC closeBtree (BTreeHandle *tree);
extern RC deleteBtree (char *idxId);

// access information about a b-tree
extern RC getNumNodes (BTreeHandle *tree, int *result);
extern RC getNumEntries (BTreeHandle *tree, int *result);
extern RC getKeyType (BTreeHandle *tree, DataType *result);

// index access
extern RC findKey (BTreeHandle *tree, Value *key, RID *result);
extern RC insertKey (BTreeHandle *tree, Value *key, RID rid);
extern RC deleteKey (BTreeHandle *tree, Value *key);
extern RC openTreeScan (BTreeHandle *tree, BT_ScanHandle **handle);
extern RC nextEntry (BT_ScanHandle *handle, RID *result);
extern RC closeTreeScan (BT_ScanHandle *handle);

// debug and test functions
extern char *printTree (BTreeHandle *tree);

#endif // BTREE_MGR_H
