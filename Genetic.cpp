#include "cLink.h"
#include "StreamIO.h"
#include "Utility.h"
#include <thread>
#include <algorithm>

#define PROBABILITY(X,Y) rand()%Y<X
struct Individual
{
	VECTOR<INT8> offString;
	INT32 configIndex;
	FLOAT score;
};

INT32 GreaterThan(Individual a, Individual b)
{
	return a.score > b.score;
}

VOID Check(Individual* newIndividual, VECTOR<Configuration>* allConfig, Configuration &cfg, VECTOR<ScoreEntryEx> *ens, SET<INT64> *refLinks)
{
	for (INT32 i = 0; i < newIndividual->offString.size(); i++)
	if (newIndividual->offString[i])
		cfg.sFOI.push_back(i);	
	FindThreshold(ens, &cfg, refLinks);
	newIndividual->score = cfg.TrainingPerformance.F1;
	newIndividual->configIndex = allConfig->size();
	allConfig->push_back(cfg);
}

VOID Genetic(VECTOR<ScoreEntryEx> *ensTrain, SET<INT64> *refLinksTrain, VECTOR<ScoreEntryEx> *ensValidation, SET<INT64> *refLinksValidation,
	Configuration* config, SystemParameters params, STRING strOutputFile, INT32 outID)
{
	VECTOR<Configuration> allConfig; 
	INT32 maxLoop = params.MaxLoopCount;
	INT32 popSize = params.PopulationSize;
	INT32 topSim = params.TopSimFunctionCount;

	allConfig.reserve(maxLoop * popSize);
	INT8 outBuf[1024];
	sprintf(outBuf, "%d", outID);
	ofstream fo(strOutputFile + outBuf);

	//Init population
	VECTOR<Individual> pop;  pop.reserve(params.PopulationSize);
	INT32 size = config->Similarity.size();
	topSim = MIN(topSim, size / 4);
	popSize = MIN(popSize, 1 << size);

	for (INT32 j = 0; j < popSize; j++)
	{
		Individual newIndividual;
		newIndividual.offString.reserve(size);
		for (INT32 i = 0; i < size; i++)
			newIndividual.offString.push_back(PROBABILITY(topSim, size));
		Configuration cfg(*config);
		Check(&newIndividual, &allConfig, cfg, ensTrain, refLinksTrain);
		pop.push_back(newIndividual);
		fo << cfg.ToString() << endl << flush;		
	}
	std::sort(pop.begin(), pop.end(), GreaterThan);
	
	sprintf(outBuf, "[%-2d] checked:%-2d | best:%.5f\n", outID, allConfig.size(), allConfig[pop[0].configIndex].TrainingPerformance.F1);
	LOGSTRSAFE(cout, outBuf);
	INT32 uselessLoop = 0;
	
	while (maxLoop--)
	{
		//selection
		VECTOR<Individual> nextPop; nextPop.reserve(popSize * 2);
		VECTOR<Individual> prvPop; prvPop.reserve(popSize * 2);
		for (INT32 i = 0; i < popSize; i++)
		{
			INT32 slBase = popSize * 100;
			INT32 slProb = pow(0.95, i) * slBase;			
			if (PROBABILITY(slProb, slBase))
				nextPop.push_back(pop[i]);
			else
				prvPop.push_back(pop[i]);
		}
		
		INT32 parentCount = nextPop.size();
		INT32 wise = parentCount * parentCount;
		
		//Cross over
		for (INT32 i = 0; i < wise; i++)
		if (PROBABILITY(parentCount, wise))
		{
			auto father = nextPop[i / parentCount].offString;
			auto mother = nextPop[i % parentCount].offString;
			INT32 crossPoint = 0;
			while (crossPoint == 0) crossPoint = rand() % (size - 1);
			Individual sister, brother;
			sister.offString.resize(size);
			brother.offString.resize(size);
			std::copy(father.begin(), father.begin() + crossPoint, sister.offString.begin());
			std::copy(mother.begin(), mother.begin() + crossPoint, brother.offString.begin());
			std::copy(father.begin() + crossPoint, father.end(), brother.offString.begin() + crossPoint);
			std::copy(mother.begin() + crossPoint, mother.end(), sister.offString.begin() + crossPoint);			
			nextPop.push_back(sister);
			nextPop.push_back(brother);
		}

		//Mutation		
		INT32 length = (nextPop.size() - parentCount) * size;
		INT32 mtProb = (INT32)(length * 0.05F); 
		for (INT32 i = parentCount; i < nextPop.size(); i++)
		for (INT32 j = 0; j < size; j++)
		if (PROBABILITY(mtProb, length))
			nextPop[i].offString[j] = !nextPop[i].offString[j];
		
		//Compute fitness		
		for (INT32 i = parentCount; i < nextPop.size(); i++)
		{
			Configuration cfg(*config);
			Check(&nextPop[i], &allConfig, cfg, ensTrain, refLinksTrain);
			fo << cfg.ToString() << endl << flush;
		}

		FLOAT best = pop[0].score;
		INT32 nextSize = nextPop.size();
		nextPop.resize(nextPop.size() + prvPop.size());
		std::copy(prvPop.begin(), prvPop.end(), nextPop.begin() + nextSize);
		std::sort(nextPop.begin(), nextPop.end(), GreaterThan);
		pop.clear(); pop.resize(pop.capacity());
		std::copy(nextPop.begin(), nextPop.begin() + popSize, pop.begin());

		sprintf(outBuf, "[%-2d] checked:%-2d | best:%.5f\n", outID, allConfig.size(), allConfig[pop[0].configIndex].TrainingPerformance.F1);
		LOGSTRSAFE(cout,outBuf);

		//early-stop 
		if (pop[0].score >= best && pop[0].score - best <= 0.0001)
		{
			if (++uselessLoop == 3)
				break;
		}
		else
			uselessLoop = 0;
	}

	INT32 maxID = pop[0].configIndex;
	config->FilteringThreshold = allConfig[maxID].FilteringThreshold;
	config->TrainingPerformance = allConfig[maxID].TrainingPerformance;
	config->sFOI.assign(allConfig[maxID].sFOI.begin(), allConfig[maxID].sFOI.end());


	config->ValidationPerformance = Validate(ensValidation, refLinksValidation, config);
	sprintf(outBuf, "[%-2d] Finished: %s", outID, config->ToString().c_str());  LOGSTRSAFE(cout, outBuf << endl);
	fo << outBuf << endl;
	fo.close();
}
