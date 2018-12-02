#include "Resolution.h"
#include "StreamIO.h"

VOID IndexedStream::Load(STRING strDataFile, STRING strIndexFile)
{
	input.open(strDataFile, ios::in | ios::binary); //data
	INT8 *buf, *ptr; //index
	ptr = buf = ReadAllBytes(strIndexFile);
	Count = (INT32)FileLength(strIndexFile) / sizeof(INT64)-1;
	index.assign((INT64*)buf, (INT64*)buf + Count + 1);
	delete[] buf;
}

INT8* IndexedStream::Read(INT32 ID)
{
	ifstream *ptr = GetStream(ID);
	INT32 length = (INT32)(index[current + 1] - index[current]); current++;	
	if (length < 0) //TODO large data
		return 0;

	INT8 *buf = new INT8[length];
	ptr->read(buf, length);
	return buf;
}

ifstream* IndexedStream::GetStream(INT32 ID)
{
	if (ID != -1 && current != ID)
	{
		current = ID;
		input.seekg(index[current], input.beg);
	}
	return &input;	
}

IndexedStream::~IndexedStream()
{
	input.close();
}

VOID IndexedStream::ResetPointer()
{
	input.seekg(0, input.beg);
	current = 0;
}
