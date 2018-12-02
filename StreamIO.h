#pragma once

#include "Core.h"
#include <fstream>

using namespace std;

//Utility.cpp	==============================================================================================================
INT8* ReadAllBytes(STRING strFile);
VOID WriteAllBytes(STRING strFile, INT8* buf, INT32 LENGTH);
INT64 FileLength(STRING strFile);
INT32 ParseNumber(STRING str, FLOAT &value);
INT32 ReadINT32(INT8* &ptr);
INT64 ReadINT64(INT8* &ptr);
FLOAT ReadFLOAT(INT8* &ptr);
DOUBLE ReadDOUBLE(INT8* &ptr);
STRING ReadSTRING(INT8* &ptr);
INT32 ReadINT32(ifstream* fi);
INT32 ReadFLOAT(ifstream* fi);
INT64 ReadINT64(ifstream* fi);
STRING ReadSTRING(ifstream* fi);
VECTOR<STRING> ReadSeparatedLine(INT8* line, const INT8* sep);
INT32 IsExist(STRING fileName);
VOID Write(ofstream* fo, INT8* firstElement, INT32 size, INT32 count);

#define StrReadINT32(FILESTREAM, VARIABLE) {FILESTREAM.read((INT8*)&VARIABLE, sizeof(INT32));}
#define StrReadINT64(FILESTREAM, VARIABLE) {FILESTREAM.read((INT8*)&VARIABLE, sizeof(INT64));}
#define BufReadINT32(BUF, VARIABLE) {memcpy(&VARIABLE, BUF, sizeof(INT32)); BUF += sizeof(INT32);}
#define BufReadINT64(BUF, VARIABLE) {memcpy(&VARIABLE, BUF, sizeof(INT64)); BUF += sizeof(INT64);}
#define BufReadFLOAT(BUF, VARIABLE) {memcpy(&VARIABLE, BUF, sizeof(FLOAT)); BUF += sizeof(FLOAT);}
#define BufReadDOUBLE(BUF, VARIABLE) {memcpy(&VARIABLE, BUF, sizeof(DOUBLE)); BUF += sizeof(DOUBLE);}

