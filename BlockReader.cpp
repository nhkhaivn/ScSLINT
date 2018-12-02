#include "Resolution.h"
#include "StreamIO.h"

BlockReader::BlockReader(STRING strBlockFile)
{
	blockFile = strBlockFile;
	fi.open(strBlockFile, ios::in | ios::binary);
	entryCount = ReadINT64(&fi);
	buffer = new INT64[0x1000000];
	ResetPointer();
}

BlockReader::~BlockReader()
{
	fi.close();
	delete[]buffer;
}

INT32 BlockReader::GetBufferSize()
{
	return bufSize;
}

VOID BlockReader::SetBufferSize(INT64 bufSize)
{
	this->bufSize = (INT32)MIN(MAX(bufSize, 0x8000), 0x1000000);
}

VOID BlockReader::ResetPointer() 
{
	fi.seekg(sizeof(INT64), fi.beg); //skip Count variable
	readSize = 0;	
}

INT64 BlockReader::GetEntryCount()
{
	return entryCount;
}

VECTOR<pair<INT32, INT32>> BlockReader::Read()
{
	VECTOR<pair<INT32, INT32>> res;	
	INT32 toRead = (INT32)MIN(entryCount - readSize, bufSize);
	res.reserve(toRead);	
	
	fi.read((INT8*)buffer, toRead * sizeof(INT64));
	for (INT32 i = 0; i < toRead; i++)
	{
		INT32 src = (INT32)(buffer[i] & 0xFFFFFFFF);
		res.push_back(pair<INT32, INT32>(src, (INT32)(buffer[i] >> 32)));
	}
	readSize += toRead;	
	return res;
}

VOID BlockReader::Select(INT64 Start, INT64 Count)
{
	fi.seekg(Start * sizeof(INT64), ios::cur);
	entryCount = Count;
	
	/*INT64 x = 0; //TODO for 32 bit indexed FS
	for (x = 0; x + 0x8000000 < size; x += 0x8000000)
		fi.seekg(0x8000000 * sizeof(INT64), ios::cur);
	fi.seekg((size - x)* sizeof(INT64));
	*/
	
}

BlockReader* BlockReader::Duplicate()
{
	return new BlockReader(blockFile);
}
