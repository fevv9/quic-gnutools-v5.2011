/*****************************************************************
# Copyright (c) $Date$ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date$
*****************************************************************/
#define HASH_TABLE_SIZE 256      

// To enable printing of register names 
typedef struct
{
  char* sName; // reg name
  int   Index; // Offset in reg file
} gdbregtype_t;

// To automatically get all thread registers from arch
/*static gdbregtype_t globalRegs[]={
#include "../../include/global_regs.h"
  {"",0xFFFFFFFF}
};*/

// To automatically get all thread registers from arch
static gdbregtype_t threadRegs[]={
#include "../../../../tools/include/arch/v2/thread_regs.h"    
#include "../../../../tools/include/arch/v3/thread_regs.h"    
{"", 0xFFFFFFFF}
};


typedef struct GDBDollarVar {
  char*          Name;  
  int            HashIndex;           // Index into hash table
  struct GDBDollarVar*  HashNext;     // Next element in hash table
} GDBDollarVar;

GDBDollarVar *HashTable[HASH_TABLE_SIZE];   // Hash table

// ###########################################################################
// GetHashKey
// This function returns the hash key given the name of a resource
// ###########################################################################

int GetHashKey(char *Name);
// ###########################################################################
// ResetHashTable
// This function resets the Hash Table
// ###########################################################################

void ResetHashTable();

// ###########################################################################
// AddToHashTable
// This function adds a GDB $ accessed name to the hash table
// ###########################################################################

void AddToHashTable(GDBDollarVar * gdbvar);

// ###########################################################################
// GetResourceValueByName
// This function finds a Resource value given a name.
// It returns null if not found
// ###########################################################################

GDBDollarVar * LookUPGDBDollarVar(char *Name);

