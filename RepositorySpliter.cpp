#include "Learning.h"
#include "StreamIO.h"
#include "Utility.h"
#include <stdlib.h>

VOID Shuffle(VECTOR<INT32> &value)
{
	//Shuffle array using Fisher–Yates algorithm	
	for (INT32 i = value.size() - 1; i >= 1; i--)
	{
		INT32 j = rand() % (i + 1);
		INT32 t = value[i];
		value[i] = value[j];
		value[j] = t;
	}
}

RepositorySpliter::RepositorySpliter(STRING strFile)
{
	Load(strFile);
}

VOID RepositorySpliter::BeginSplit(SET<INT64> refLinks, INT32 nSrcInsCount, FLOAT trainingSplit, FLOAT validationSplit, INT32 folds)
{
	//Find positiveSourceInstances and negativeSourceInstances
	SET<INT32> positive;
	for (auto ref : refLinks)
		positive.insert((INT32)(ref & 0xFFFFFFFF));
	positiveSourceInstances.assign(positive.begin(), positive.end());
	negativeSourceInstances.reserve(nSrcInsCount - positive.size());
	for (INT32 i = 0; i < nSrcInsCount; i++)
	if (!Contains(positive,i))
		negativeSourceInstances.push_back(i);

	this->trainingSplit = trainingSplit;
	this->validationSplit = validationSplit;
	this->Folds = folds;
	currentFold = 0;
	srand(time(0) + TickCount());
	if (Folds != -1)
	{
		Shuffle(positiveSourceInstances);
		Shuffle(negativeSourceInstances);
	}
}

VOID RepositorySpliter::Split()
{
	for (INT32 i = 0; i < 3; i++)
		splitedSet[i].clear(); 
	
	INT32 posSize = positiveSourceInstances.size();
	INT32 negSize = negativeSourceInstances.size();

	VECTOR<INT32> posTrain, negTrain; 
	if (Folds == -1) //percentage
	{
		Shuffle(positiveSourceInstances);
		Shuffle(negativeSourceInstances);
		INT32 posSeg = (INT32)(trainingSplit * posSize);
		INT32 negSeg = (INT32)(trainingSplit * negSize);
		posTrain.resize(posSeg); negTrain.resize(negSeg);
		splitedSet[2].resize(posSize + negSize - posSeg - negSeg);
		copy(positiveSourceInstances.begin(), positiveSourceInstances.begin() + posSeg, posTrain.begin());
		copy(negativeSourceInstances.begin(), negativeSourceInstances.begin() + negSeg, negTrain.begin());
		copy(positiveSourceInstances.begin() + posSeg, positiveSourceInstances.end(), splitedSet[2].begin());
		copy(negativeSourceInstances.begin() + negSeg, negativeSourceInstances.end(), splitedSet[2].begin() + (posSize - posSeg));
	}
	else //cross-validation
	{
		INT32 posSeg = posSize / Folds;
		INT32 negSeg = negSize / Folds;
		INT32 posBeg = currentFold * posSeg;
		INT32 negBeg = currentFold * negSeg;
		if (currentFold == Folds - 1)
		{
			posSeg = posSize - posSeg * currentFold;
			negSeg = negSize - negSeg * currentFold;
		}
		posTrain.resize(posSize - posSeg); negTrain.resize(negSize - negSeg);
		splitedSet[2].resize(posSeg + negSeg);
		copy(positiveSourceInstances.begin(), positiveSourceInstances.begin() + posBeg, posTrain.begin());
		copy(positiveSourceInstances.begin() + (posBeg + posSeg), positiveSourceInstances.end(), posTrain.begin() + posBeg);
		copy(negativeSourceInstances.begin(), negativeSourceInstances.begin() + negBeg, negTrain.begin());
		copy(negativeSourceInstances.begin() + (negBeg + negSeg), negativeSourceInstances.end(), negTrain.begin() + negBeg);
		copy(positiveSourceInstances.begin() + posBeg, positiveSourceInstances.begin() + (posBeg + posSeg), splitedSet[2].begin());
		copy(negativeSourceInstances.begin() + negBeg, negativeSourceInstances.begin() + (negBeg + negSeg), splitedSet[2].begin() + posSeg);
		
		currentFold++;
	}
	
	INT32 posSeg = (INT32)((1 - validationSplit) * posTrain.size());
	INT32 negSeg = (INT32)((1 - validationSplit) * negTrain.size());
	splitedSet[0].resize(posSeg + negSeg);
	splitedSet[1].resize(posTrain.size() + negTrain.size() - posSeg - negSeg);
	copy(posTrain.begin(), posTrain.begin() + posSeg, splitedSet[0].begin());
	copy(negTrain.begin(), negTrain.begin() + negSeg, splitedSet[0].begin() + posSeg);
	copy(posTrain.begin() + posSeg, posTrain.end(), splitedSet[1].begin());
	copy(negTrain.begin() + negSeg, negTrain.end(), splitedSet[1].begin() + (posTrain.size() - posSeg));
}

VOID RepositorySpliter::Save(STRING strFile) //Save current split 
{
	ofstream of(strFile, ios::out | ios::binary);
	for (INT32 i = 0; i < 3; i++)
	{
		INT32 size = splitedSet[i].size();
		of.write((INT8*)&size, sizeof(INT32));
		for (auto src : splitedSet[i])
			of.write((INT8*)&src, sizeof(INT32));
	}
	of.close();
}

VOID RepositorySpliter::Load(STRING strFile) //Load saved split 
{
	INT8* buf = ReadAllBytes(strFile);
	INT8 *ptr = buf;
	for (INT32 i = 0; i < 3; i++)
	{
		INT32 size = ReadINT32(ptr);
		splitedSet[i] = VECTOR<INT32>((INT32*)ptr, (INT32*)ptr + size);
		ptr += size * sizeof(INT32);
	}
	delete[]buf;
}

SET<INT32> RepositorySpliter::GetInstanceList(INT32 setID) //Get the split by set ID: TRAININGSET, VALIDATIONSET, TESTSET, FULLSET
{
	VECTOR<INT32> ins;
	for (INT32 i = 0; i < 3; i++)
		if (setID & (1<<i))
		{
			INT32 size = ins.size();
			ins.resize(ins.size() + splitedSet[i].size());
			copy(splitedSet[i].begin(), splitedSet[i].end(), ins.begin() + size);
		}
	return SET<INT32>(ins.begin(), ins.end());
}

