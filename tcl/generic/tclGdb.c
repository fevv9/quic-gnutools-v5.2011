/*****************************************************************
# Copyright (c) $Date$ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date$
*****************************************************************/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "tclGdb.h"
#include "tcl.h"
#include "tclInt.h"
#define MAXGDBKEYWORDS 12

static char *gdbKeyWords[MAXGDBKEYWORDS] = {
"_",
"__",
"_exitcode",
"bpnum",
"cdir",
"cwdr",
"tpnum",
"trace_file",
"trace_frame",
"trace_func",
"trace_line",
"tracepoint"
};



// ###########################################################################
// GetHashKey
// This function returns the hash key given the name of a resource
// ###########################################################################

int GetHashKey(char *Name)
{
  static int i;                             // Temporary Loop Counter
  static int NameLength;                    // Length of name to hash
  int HashKey;                              // Resulting hash key

  HashKey = 0;
  NameLength = strlen(Name);

  // Truncate to 12 characters
  if (NameLength > 12)
    NameLength = 12;

  // Hash function is simply the sum of the characters in the name
  for (i = 0; i < NameLength; i++)
    HashKey += Name[i];

  return(HashKey % HASH_TABLE_SIZE);

} // GetHashKey

// ###########################################################################
// ResetHashTable
// This function resets the Hash Table
// ###########################################################################

void ResetHashTable()
{
  int i;

  // Initialize Hash Table
  for (i = 0; i < HASH_TABLE_SIZE; i++)
    HashTable[i] = NULL;
} // ResetHashTable


/*
 * AddToHashTable
 * This function adds a GDB $ accessed name to the hash table
 */

void AddToHashTable(GDBDollarVar *NewValue)
{
  // Calculate the hash key
  int HashKey = GetHashKey(NewValue->Name);

  NewValue->HashIndex = HashKey;

  if (HashTable[HashKey] == NULL)
    // If no hash entry, insert at head of list
    {
      HashTable[HashKey] = NewValue;
      NewValue->HashNext = NULL;
    }
  else
    // Otherwise, insert at end of list
    {
       // Pointer to current dollar var
       GDBDollarVar *CurrentPtr;

      // Find end of list
      CurrentPtr = HashTable[HashKey];
      while (CurrentPtr->HashNext != NULL)
	   CurrentPtr = CurrentPtr->HashNext;

      // Do the insertion
      CurrentPtr->HashNext = NewValue;
      NewValue->HashNext = NULL;
    }

} /* AddToHashTable */



/*
 * GetResourceValueByName
 * This function finds a Resource value given a name.
 * It returns null if not found
 */

GDBDollarVar * LookUPGDBDollarVar(char *namePtr)
{
  // Pointer to current dollar var
  GDBDollarVar *CurrentPtr;
  /* Snip out the $ from the var name */
  //char         *Name  = namePtr+1;
  char         *Name  = namePtr;
  //if((namePtr == NULL) || namePtr[0] != '$')
  if(namePtr == NULL)
      return (NULL);

  // Calculate the hash key
  CurrentPtr = HashTable[GetHashKey(Name)];

  // Cycle until end of hash table list
  while (CurrentPtr != NULL)
    {
      // Check if name matches
      if (strcmp(CurrentPtr->Name, Name) == 0)
	    return(CurrentPtr);
      // Cycle to the next one on the list
      CurrentPtr = CurrentPtr->HashNext;
    }


  // Name not found
  return (NULL);

} /* LookUPGDBDollarVar */

/*
 * Register GDB's local registers in a look up hash table
 */
static GDBDollarVar* SetUpGDBRegHashData(int regPos)
{

  GDBDollarVar* tmpPtr =
      (GDBDollarVar*)(malloc(sizeof(GDBDollarVar)));
  tmpPtr->Name      = (char*)malloc(sizeof(char) *25);
  tmpPtr->HashIndex = 0;
  tmpPtr->HashNext  = NULL;

  if((tmpPtr != NULL) && (tmpPtr->Name != NULL))
  {
      if(regPos < 10)
         sprintf (tmpPtr->Name , "r%d", regPos);
      else
         strcpy(tmpPtr->Name,threadRegs[regPos].sName);
  }

  return tmpPtr;

}


/*
 * Register GDB's dollar variables in a look up hash table
 */
static GDBDollarVar* SetUpGDBVarHashData(int idx)
{

  GDBDollarVar* tmpPtr =
      (GDBDollarVar*)(malloc(sizeof(GDBDollarVar)));
  tmpPtr->Name      = (char*)malloc(sizeof(char) *25);
  tmpPtr->HashIndex = 0;
  tmpPtr->HashNext  = NULL;

  if((tmpPtr != NULL) && (tmpPtr->Name != NULL))
    strcpy(tmpPtr->Name,gdbKeyWords[idx]);

  return tmpPtr;

}

/*
 * Register GDB's dollar variables in a look up hash table
 */
void AddGdbDollarVarsToHash()
{
    GDBDollarVar *gdbPtr = NULL;
    int i;

    ResetHashTable();

    for (i=0; i<(sizeof(threadRegs)/sizeof(gdbregtype_t));
         i++)
    {
       gdbPtr = SetUpGDBRegHashData(i);
       if(gdbPtr != NULL)
        AddToHashTable(gdbPtr);

    }
    for (i=0; i<MAXGDBKEYWORDS; i++)
    {
       gdbPtr = SetUpGDBVarHashData(i);
       if(gdbPtr != NULL)
        AddToHashTable(gdbPtr);

    }
}

/* The $ sign is not followed by an open curly brace.  Then
 *    the variable name is everything up to the next
 *    character that isn't a letter, digit, or underscore.
 */

int ExtractGdbVarName (char* srcPtr, int nBytes)
{
    char c;
    char *src    = srcPtr+1; /* skip the $ */
    char *start  = src;
    int numBytes = nBytes-1; /* account for $ */
    int offset, strSize;
    Tcl_UniChar ch;

	while (numBytes) {
	    if (Tcl_UtfCharComplete(src, numBytes)) {
	        offset = Tcl_UtfToUniChar(src, &ch);
	    } else {
		char utfBytes[TCL_UTF_MAX];
		memcpy(utfBytes, src, (size_t) numBytes);
		utfBytes[numBytes] = '\0';
	        offset = Tcl_UtfToUniChar(utfBytes, &ch);
	    }
	    c = UCHAR(ch);
	    if (isalnum(c) || (c == '_')) { /* INTL: ISO only, UCHAR. */
		src += offset;  numBytes -= offset;
		continue;
	    }
	    if ((c == ':') && (numBytes != 1) && (src[1] == ':')) {
		src += 2; numBytes -= 2;
		while (numBytes && (*src == ':')) {
		    src++; numBytes--;
		}
		continue;
	    }
	    break;
	}
    strSize = src-start;
    return strSize;

}
