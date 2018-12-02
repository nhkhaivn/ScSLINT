#include "Evaluator.h"
#include "StreamIO.h"
#include "Utility.h"
#include <stdlib.h>

SET<INT64> LoadReference(STRING strFile)
{
	SET<INT64> reference;
	ifstream fi(strFile, ios::in);
	INT8 line[1024];
	while (fi.getline(line, 1024))
	{
		auto id = ReadSeparatedLine(line, (const INT8*)"\t");
		INT32 src = atoi(id[0].c_str());
		INT32 des = atoi(id[1].c_str());
		INT64 pr = ((INT64)des << 32) | src;
		reference.insert(pr);
	}
	fi.close();
	return reference;
}

SET<INT64> AdaptToSplit(SET<INT64> *reference, SET<INT32> *instance)
{
	SET<INT64> ref;
	ref.reserve(reference->size());
	for (auto r : *reference)
	if (RefContains(instance, (INT32)(r & 0xFFFFFFFF)))
		ref.insert(r);
	return ref;
}

RPF Evaluate(VECTOR<pair<INT64, FLOAT>> *detected, SET<INT64> *reference)
{
	RPF eval = { 0 };
	eval.Expected = reference->size();
	eval.Detected = detected->size();
	for (auto pr = detected->begin(); pr != detected->end(); pr++)
		eval.Correct += RefContains(reference, pr->first);
	if (eval.Correct)
	{
		eval.Recall = (FLOAT)eval.Correct / eval.Expected;
		eval.Precision = (FLOAT)eval.Correct / eval.Detected;
		eval.F1 = 2 * eval.Recall * eval.Precision / (eval.Recall + eval.Precision);
	}
	return eval;
}

RPF Evaluate(STRING strBlockFile, SET<INT64> *reference)
{
	ifstream fi(strBlockFile, ios::in | ios::binary);
	RPF eval = { 0 };
	eval.Detected = ReadINT64(&fi);
	eval.Expected = reference->size();
	Loop(eval.Detected);
	INT64 cur, prv = -1;
	INT32 orderingFailed = 0;	
	for (INT64 i = 0; i < eval.Detected; i++)
	{
		StrReadINT64(fi, cur);
		eval.Correct += RefContains(reference, cur);
		if (prv >= cur)
			orderingFailed++;
		prv = cur;
		Loop(); 
	}
	if (eval.Correct)
	{
		eval.Recall = (FLOAT)eval.Correct / reference->size();
		eval.Precision = (FLOAT)eval.Correct / eval.Detected;
		eval.F1 = 2 * eval.Recall * eval.Precision / (eval.Recall + eval.Precision);
	}
	if (orderingFailed)
		LOGALL(endl << "Unordered entries: " << orderingFailed << endl);
	return eval;
}
