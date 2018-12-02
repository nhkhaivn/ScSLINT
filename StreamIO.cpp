#include "StreamIO.h"
#include <stdlib.h>
#include <string.h>
#include <cerrno>
#include <sys/stat.h>

#include <iostream>

INT8* ReadAllBytes(STRING strFile)
{
	ifstream fs(strFile, ios::in | ios::binary);
	if (!fs.is_open())
		return 0;

	fs.seekg(0, fs.end);
	INT32 count = (INT32)fs.tellg();
	INT8* buf = new INT8[count];
	fs.seekg(0, fs.beg);
	fs.read(buf, count);
	fs.close();
	return buf;
}

VOID WriteAllBytes(STRING strFile, INT8* buf, INT32 LENGTH)
{
	ofstream fs(strFile, ios::out | ios::binary);
	fs.write(buf, LENGTH);
	fs.close();
}

INT64 FileLength(STRING strFile)
{
	ifstream fs(strFile, ios::in | ios::binary);
	fs.seekg(0, fs.end);
	INT64 count = fs.tellg();
	fs.close();
	return count;
}

INT32 ParseNumber(STRING str, FLOAT &value)
{
	INT8 *e;
	errno = 0;
	value = strtof(str.c_str(), &e);
	return *e == 0 && errno == 0;
}

VECTOR<INT32> ToUTF32(STRING utf8)
{
	VECTOR<INT32> utf32;
	INT8* p = &utf8[0];
	INT8* q = p + utf8.length();
	utf32.reserve(utf8.length());
	while (p != q)
	{
		INT32 val = 0;
		if ((*p & 0xF0) == 0xF0) //4
		{
			val = (((*p) & 0x07) << 18) | ((*(p + 1) & 0x3F) << 12) | ((*(p + 2) & 0x3F) << 6) | (*(p + 3) & 0x3F);
			p += 4;
		}
		else if ((*p & 0xE0) == 0xE0) //3
		{
			val = (((*p) & 0x0F) << 12) | ((*(p + 1) & 0x3F) << 6) | (*(p + 2) & 0x3F);
			p += 3;
		}
		else if ((*p & 0xC0) == 0xC0) //2
		{
			val = (((*p) & 0x1F) << 6) | (*(p + 1) & 0x3F);
			p += 2;
		}
		else //1
		{
			val = *p & 0x7F;
			p += 1;
		}

		utf32.push_back(val);
	}
	return utf32;
}

INT32 ReadINT32(INT8* &ptr)
{
	INT32 x = 0;
	memcpy(&x, ptr, sizeof(INT32));
	ptr += sizeof(INT32);
	return x;
}

INT64 ReadINT64(INT8* &ptr)
{
	INT64 x = 0;
	memcpy(&x, ptr, sizeof(INT64));
	ptr += sizeof(INT64);
	return x;
}

FLOAT ReadFLOAT(INT8* &ptr)
{
	FLOAT x=0;
	memcpy(&x, ptr, sizeof(FLOAT));
	ptr += sizeof(FLOAT);
	return x;
}

DOUBLE ReadDOUBLE(INT8* &ptr)
{
	DOUBLE x = 0;
	memcpy(&x, ptr, sizeof(DOUBLE));
	ptr += sizeof(DOUBLE);
	return x;
}

STRING ReadSTRING(INT8* &ptr)
{
	INT32 length = 0;
	INT32 shift = 0;
	do
	{
		length |= (*ptr & 0x7F) << shift;
		shift += 7;
	} while (*ptr++ & 0x80);

	STRING str(ptr, length);
	ptr += length;
	return str;
}

INT32 ReadINT32(ifstream* fi)
{
	INT32 x=0;
	fi->read((INT8*)&x, sizeof(INT32));
	return x;
}

INT64 ReadINT64(ifstream* fi)
{
	INT64 x=0;
	fi->read((INT8*)&x, sizeof(INT64));
	return x;
}

INT32 ReadFLOAT(ifstream* fi)
{
	FLOAT x = 0;
	fi->read((INT8*)&x, sizeof(FLOAT));
	return x;
}

STRING ReadSTRING(ifstream* fi)
{
	INT32 length = 0;
	INT32 shift = 0;
	INT8 ptr;
	do
	{
		fi->read(&ptr, sizeof(INT8));
		length |= (ptr & 0x7F) << shift;
		shift += 7;
	} while (ptr & 0x80);
	INT8 *buf = new INT8[length];
	fi->read(buf, sizeof(INT8)* length);
	STRING str(buf, buf + length);
	delete[] buf;
	return str;
}

VECTOR<STRING> ReadSeparatedLine(INT8* line, const INT8* sep)
{
	VECTOR<STRING> set;
	INT8 *p = strtok(line, sep);
	while (p != 0)
	{
		STRING str(p);
		set.push_back(str);
		p = strtok(0, sep);
	}
	return set;
}

INT32 IsExist(STRING fileName)
{
	struct stat fileStat;	
	return !stat(fileName.c_str(), &fileStat);
}

VOID Write(ofstream* fo, INT8* firstElement, INT32 size, INT32 count)
{
	fo->write((INT8*)&count, sizeof(INT32));
	if (count > 0)
		fo->write(firstElement, size * count);
}
