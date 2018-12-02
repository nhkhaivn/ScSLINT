#include "ScSLINT.h"
#include "StreamIO.h"
#include "cLink.h"
#include "Learning.h"
#include "Utility.h"
#include <algorithm>

MAP<STRING, STRING> ParseParameters(INT32 argc, INT8* argv[]);
VOID ExecuteJob(MAP<STRING, STRING> params);
VOID CollectResult(MAP<STRING, STRING> params);

INT32 main(INT32 argc, INT8* argv[])
{
	auto params = ParseParameters(argc, argv);

	/*params["job"] = "align";
	params["src"] = "E:\\newData\\index\\restaurant-zagats.k";
	params["trg"] = "E:\\newData\\index\\restaurant-fodors.k";			
	params["output"] = "E:\\newData";	*/

	//params["job"] = "clink";
	//params["algorithm"] = "naive";
	//params["topsim"] = "8";
	//params["filtering"]="1";
	//params["name"] = "nytimes-2014-02-nyt-geo-loc.k_geonames-2014-02.k_05_0.80_0.20_000";
	//params["src"] = "D:\\repository\\index\\nytimes-2014-02-nyt-geo-loc.k";
	//params["trg"] = "D:\\repository\\index\\geonames-2014-02.k";
	//params["align"] = "D:\\ScSLINT\\data\\nytimes-2014-02-nyt-geo-loc.k_geonames-2014-02.k.align";
	//params["label"] = "D:\\label\\nytimes-2014-02-nyt-geo-loc.k_geonames-2014-02.k.id";
	//params["split"] = "D:\\ScSLINT\\split\\nytimes-2014-02-nyt-geo-loc.k_geonames-2014-02.k_05_0.80_0.20_000";
	//params["output"] = "D:\\ScSLINT\\output\\nytimes-2014-02-nyt-geo-loc.k_geonames-2014-02.k";
	//params["temp"] = "D:\\ScSLINT\\detail";
	
	//params["job"] = "clink";
	//params["algorithm"] = "heuristic";
	//params["topsim"] = "4";
	//params["filtering"]="1";
	//params["name"] = "nytimes-2014-02-nyt-fr-loc.k_freebase-rdf-2013-09-03-21-57.k_-1_0.95_0.20_000";
	//params["src"] = "D:\\repository\\index\\nytimes-2014-02-nyt-fr-loc.k";
	//params["trg"] = "D:\\repository\\index\\freebase-rdf-2013-09-03-21-57.k";
	//params["align"] = "D:\\ScSLINT\\data\\D:\ScSLINT\data\nytimes-2014-02-nyt-fr-loc.k_freebase-rdf-2013-09-03-21-57.k.align";
	//params["label"] = "D:\\label\\nytimes-2014-02-nyt-fr-loc.k_freebase-rdf-2013-09-03-21-57.k.id";
	//params["split"] = "D:\\ScSLINT\\split\\nytimes-2014-02-nyt-fr-loc.k_freebase-rdf-2013-09-03-21-57.k_-1_0.95_0.20_000";
	//params["output"] = "D:\\ScSLINT\\output\\nytimes-2014-02-nyt-fr-loc.k_freebase-rdf-2013-09-03-21-57.k";
	//params["temp"] = "D:\\ScSLINT\\detail";

	//params["job"] = "clink";
	//params["algorithm"] = "heuristic";
	////params["topsim"] = "4";
	//params["filtering"]="1";
	//params["name"] = "Amazon.k_GoogleProducts.k_05_0.80_0.20_000";
	//params["src"] = "D:\\repository\\index\\Amazon.k";
	//params["trg"] = "D:\\repository\\index\\GoogleProducts.k";
	//params["align"] = "D:\\ScSLINT\\data\\Amazon.k_GoogleProducts.k.align";
	//params["label"] = "D:\\label\\Amazon.k_GoogleProducts.k.id";
	//params["split"] = "D:\\ScSLINT\\split\\Amazon.k_GoogleProducts.k_05_0.80_0.20_000";
	//params["output"] = "D:\\ScSLINT\\output\\Amazon.k_GoogleProducts.k";
	//params["temp"] = "D:\\ScSLINT\\detail";

	/*params["job"] = "ser";
	params["algorithm"] = "heuristic";
	params["filtering"]="1";
	params["name"] = "oaei2010-sider.k_oaei2010-diseasome.k_05_0.80_0.20_000";
	params["src"] = "D:\\repository\\index\\oaei2010-sider.k";
	params["trg"] = "D:\\repository\\index\\oaei2010-diseasome.k";
	params["align"] = "D:\\ScSLINT\\data\\oaei2010-sider.k_oaei2010-diseasome.k.align";
	params["label"] = "D:\\label\\oaei2010-sider.k_oaei2010-diseasome.k.id";
	params["split"] = "D:\\ScSLINT\\split\\oaei2010-sider.k_oaei2010-diseasome.k_05_0.80_0.20_000";
	params["output"] = "D:\\ScSLINT\\output\\oaei2010-sider.k_oaei2010-diseasome.k";
	params["score"] = "D:\\ScSLINT\\output\\oaei2010-sider.k_oaei2010-diseasome.k\\oaei2010-sider.k_oaei2010-diseasome.k_05_0.80_0.20_000.score.training";
	params["temp"] = "D:\\ScSLINT\\detail";
	params["thread"] = "1";*/

	LOGSTREAM.open(params["log"], ios::app);
	LOGSTR(cout, "Start: " << Now() << endl);	
	ExecuteJob(params);
	
	LOGSTR(cout, "Finish: " << Now() << endl);
	LOGSTREAM.close();

	return 0;
}

VOID ExecuteJob(MAP<STRING, STRING> params)
{
	INT32 temp = 1;
	SystemParameters sysParams(params);
	if (Contains(params, "output"))
		CMD(MKDIR + params["output"]);
	//cout << sysParams.Job;
	switch (sysParams.Job)
	{
	case DEFAULT:
		ScSLINT(sysParams);
		break;

	case CLINK:
		cLink(sysParams);
		break;

	case CLEARN:
		cLearn(sysParams);
		break;

	case ALIGN:
		GenerateAlignment(sysParams);
		break;

	case BLOCK:
		if (Contains(params, "split") && Contains(params, "fold"))
		{
			temp = 0;
			if (params["fold"].find("training") != string::npos)
				temp |= TRAININGSET;
			if (params["fold"].find("validation") != string::npos)
				temp |= VALIDATIONSET;
			if (params["fold"].find("test") != string::npos)
				temp |= TESTSET;
			GenerateBlock(sysParams, temp);
		}
		else
			GenerateBlock(sysParams);
		break;

	case MATCH:
		Match(sysParams);
		break;

	case SER:
		ser(sysParams);
		break;

	case FILTER:
		Filter(sysParams);
		break;
	
	case SPLIT:
		if (Contains(params, "repeat"))
			temp = atoi(params["repeat"].c_str());
		GenerateRepositorySplit(sysParams, temp);
		break;

	case COLLECT:
		CollectResult(params);
		break;

	default:
		break;
	}
}

MAP<STRING, STRING> ParseParameters(INT32 argc, INT8* argv[])
{
	MAP<STRING, STRING> params;
	params["startup"] = argv[0];
	for (INT32 i = 1; i < argc; i++)
	{
		auto prs = ReadSeparatedLine(argv[i], "=");
		if (prs.size() > 0)
		{
			transform(prs[0].begin(), prs[0].end(), prs[0].begin(), (INT32(*)(INT32))tolower);
			if (prs.size() == 2)
				params[prs[0]] = prs[1];
			if (prs.size() == 1)
				params[prs[0]] = "true";
		}
	}
	if (!Contains(params, "log"))
		params["log"] = Now() + ".txt";
	return params;
}

