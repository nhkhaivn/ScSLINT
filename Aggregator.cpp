#include "Resolution.h"
#include <iostream>

FLOAT LinearAggregator::Aggregate(ScoreEntryEx *entry, VECTOR<INT32> *sFOI, VECTOR<FLOAT> *sFOIThreshold)
{
	FLOAT res = 0;
	INT32 valid = 0;
	for (auto id = sFOI->begin(); id != sFOI->end(); id++)
	if ((useBoolean == 0 && entry->Similarity[*id] >= 0) || entry->Similarity[*id] >= sFOIThreshold->at(*id))
	{
		if (K == 1)
			res += entry->Similarity[*id];
		else
		if (K == 2)
			res += entry->Similarity[*id] * entry->Similarity[*id];
		valid++;
	}
	if (res > 0)
	{
		if (useAverage)
			res /= valid;
		if (useWeighting)
			res *= entry->Weight;
	}
	return res;
}

LinearAggregator::LinearAggregator(INT32 mask)
{
	K = (mask >> 3) + 1;
	useBoolean = (mask >> 2) & 1;
	useAverage = (mask >> 1) & 1;
	useWeighting = mask & 1;
}

INT32 LinearAggregator::GetMask()
{
	return ((K - 1) << 3) | (useBoolean << 2) | (useAverage << 1) | (useWeighting);
}

STRING LinearAggregator::ToString()
{
	STRING fID[] = { "LS", "LSW", "LA", "LAW", "LBA", "LBW", "LBA", "LBAW", "QS", "QSW", "QA", "QAW", "QBA", "QBW", "QBA", "QBAW" };
	return STRING(fID[GetMask()]);
}
