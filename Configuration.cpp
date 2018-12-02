#include "Resolution.h"
#include "StreamIO.h"

Configuration::Configuration(const Configuration &config)
{
	Similarity.assign(config.Similarity.begin(), config.Similarity.end());
	SimilarityThreshold.assign(config.SimilarityThreshold.begin(), config.SimilarityThreshold.end());
	sFOI.assign(config.sFOI.begin(), config.sFOI.end());
	FilteringFactor = config.FilteringFactor;
	FilteringThreshold = config.FilteringThreshold;
	TrainingPerformance = config.TrainingPerformance;
	ValidationPerformance = config.ValidationPerformance;
	Aggregate = config.Aggregate;
}

Configuration::Configuration(VECTOR<SimilarityFunction> &simFunctions)
{
	Similarity = simFunctions;
	sFOI.resize(Similarity.size());
	for (INT32 i = 0; i < sFOI.size(); i++)
		sFOI[i] = i;
	SimilarityThreshold.resize(Similarity.size());
	FilteringFactor = 1.0;
	FilteringThreshold = 0;
	TrainingPerformance.Recall = TrainingPerformance.Precision = TrainingPerformance.F1 = 0;
	ValidationPerformance.Recall = ValidationPerformance.Precision = ValidationPerformance.F1 = 0;
	Aggregate = LinearAggregator(0);
}

Configuration::Configuration(VECTOR<SimilarityFunction> &simFunctions, FLOAT filteringFactor, LinearAggregator LinearAggregator)
{
	Similarity = simFunctions;
	SimilarityThreshold.resize(Similarity.size());
	FilteringFactor = filteringFactor;
	FilteringThreshold = 0;
	TrainingPerformance.Recall = TrainingPerformance.Precision = TrainingPerformance.F1 = 0;
	ValidationPerformance.Recall = ValidationPerformance.Precision = ValidationPerformance.F1 = 0;
	Aggregate = LinearAggregator;
}

STRING Configuration::ToString()
{
	INT8 buf[1024];
	INT8 *pointer = buf;
	pointer += sprintf(pointer, "%s,", Aggregate.ToString().c_str());
	for (INT32 k : sFOI)
		pointer += sprintf(pointer, "%d_", k);
	sprintf(pointer - 1, ",%f,%f,%f,%f,%f,%f,%f", FilteringThreshold,
		TrainingPerformance.Recall, TrainingPerformance.Precision, TrainingPerformance.F1,
		ValidationPerformance.Recall, ValidationPerformance.Precision, ValidationPerformance.F1);
	return STRING(buf);
}

VOID Configuration::Load(STRING strFile)
{
	ifstream fi(strFile, ios::binary);
	INT32 size;
	fi.read((INT8*)&Aggregate, sizeof(LinearAggregator));
	fi.read((INT8*)&FilteringThreshold, sizeof(FLOAT));
	fi.read((INT8*)&FilteringFactor, sizeof(FLOAT));
	fi.read((INT8*)&TrainingPerformance, sizeof(RPF));
	fi.read((INT8*)&ValidationPerformance, sizeof(RPF));
	fi.read((INT8*)&size, sizeof(INT32)); Similarity.resize(size); fi.read((INT8*)&Similarity[0], sizeof(SimilarityFunction) * size);
	fi.read((INT8*)&size, sizeof(INT32)); SimilarityThreshold.resize(size); fi.read((INT8*)&SimilarityThreshold[0], sizeof(FLOAT)* size);
	fi.read((INT8*)&size, sizeof(INT32)); sFOI.resize(size); fi.read((INT8*)&sFOI[0], sizeof(INT32)* size);
	fi.close();
}

VOID Configuration::Save(STRING strFile)
{
	ofstream fo(strFile, ios::binary);
	fo.write((INT8*)&Aggregate, sizeof(LinearAggregator));
	fo.write((INT8*)&FilteringThreshold, sizeof(FLOAT));
	fo.write((INT8*)&FilteringFactor, sizeof(FLOAT));
	fo.write((INT8*)&TrainingPerformance, sizeof(RPF));
	fo.write((INT8*)&ValidationPerformance, sizeof(RPF));
	Write(&fo, (INT8*)&Similarity[0], sizeof(SimilarityFunction), Similarity.size());
	Write(&fo, (INT8*)&SimilarityThreshold[0], sizeof(FLOAT), SimilarityThreshold.size());
	Write(&fo, (INT8*)&sFOI[0], sizeof(INT32), sFOI.size());
	fo.close();
}
