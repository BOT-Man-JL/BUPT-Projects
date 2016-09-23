
#pragma once
#include <string>
#include <list>
#include <unordered_map>
#include "DataStructure.h"
#include <vector> // add by xz
#include <climits>

// Todo: Algorithm - Implement it
int GenerateMTPath(std::list<std::string> &Route, std::unordered_map <int, Trsp> &SolutionVector, const Plan & plan,
	unsigned int time, const Passenger::RouteInfoSet & via, std::list<Passenger::RouteInfo> &path,int &cost, int &t_time);

bool GenerateMinPricePath(const std::string &from, const std::string &to,
	const Passenger::RouteInfoSet &via,
	const Plan &plan, unsigned int time,
	std::list<Passenger::RouteInfo> &path,int &cost,int &t_time);

bool GenerateMinTimePath(const std::string &from, const std::string &to,
	const Passenger::RouteInfoSet &via,
	const Plan &plan, unsigned int time,
	std::list<Passenger::RouteInfo> &path, int &cost, int &t_time);

bool Generate3rdstrategy(const std::string &from, const std::string &to,
	const Passenger::RouteInfoSet &via,
	const Plan &plan, unsigned int time,
	unsigned int limit,
	std::list<Passenger::RouteInfo> &path,int &cost, int &t_time);

//add by xuzhu

struct TrspNode {
	std::string start;
	std::string end;
	Trsp vehicle;

	bool operator < (const struct TrspNode &a) const { return vehicle.cPrice > a.vehicle.cPrice; }
};

struct Trsp3rdNode {
	int Price;
	unsigned int time;
	std::string end;
	std::list<TrspNode> Path;

	bool operator < (const struct Trsp3rdNode &a) const { return Price > a.Price; }
};

class Table {
private:
	std::unordered_map<std::string, std::unordered_map<std::string, std::pair<unsigned int, std::list<TrspNode>>>> _MinPriceTable;
	std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_map<unsigned int, std::pair<unsigned int, std::list<TrspNode>>>>> _MinTimeTable;
	std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_map<unsigned int, std::pair<unsigned int, std::list<TrspNode>>>>> _OneStepTimeTable;
public:
	void Table::Init(const Plan &plan, const std::list<std::string> &g_city);
	void ShowMinPrice(const Plan &plan, const std::list<std::string> &g_city);
	void ShowMinTime(const Plan &plan, const std::list<std::string> &g_city);

	bool Table::GenerateMinPricePath(const std::string &from, const std::string &to,
		const Passenger::RouteInfoSet &via,
		const Plan &plan, unsigned int time,
		std::list<Passenger::RouteInfo> &path,int &cost, int &t_time);
	bool GenerateMinTimePath(const std::string &from, const std::string &to,
		const Passenger::RouteInfoSet &via,
		const Plan &plan, unsigned int time,
		std::list<Passenger::RouteInfo> &path, int &cost, int &t_time);
	bool Generate3rdstrategy(const std::string &from, const std::string &to,
		const Passenger::RouteInfoSet &via,
		const Plan &plan, unsigned int time,
		unsigned int limit,
		std::list<Passenger::RouteInfo> &path, int &cost, int &t_time);
	bool GenerateMinTimePath_ViaCity(const std::string &from, const std::string &to,
		const Passenger::RouteInfoSet &via,
		const Plan &plan, unsigned int time,
		std::list<Passenger::RouteInfo> &path, int &cost, int &t_time);
	bool GenerateMinPricePath_ViaCity(std::string from, std::string to,
		const Passenger::RouteInfoSet &via,
		const Plan &ref_plan, unsigned int time,
		std::list<Passenger::RouteInfo> &path, int &cost, int &t_time);
};


void InitTable(const Plan &g_plan, const std::list<std::string> g_city);
bool JudgeConnect(const Plan & g_plan, std::string ArrCity[], int m, std::string from,
	std::string to
	);
bool NeedSwap(const Plan &g_plan, std::string  ArrCity[], int i, int j, int m);
void Swap(std::string & a, std::string &b);
void FindPath(const Plan &g_plan, std::string  ArrCity[], int k, int m, std::list<std::list<std::string>> &All_path,
	std::string from, std::string to
	);
bool ExistArc(const Plan &g_plan, std::string  ArrCity[], int i, int j);
int  Searchable(const std::unordered_map<int, int>&SVFlag,
	const std::unordered_map<int, std::list<Trsp>> &SolutionVS,
	const int &layer);
bool PathPermutaion(const Plan &g_plan, std::string  ArrCity[], int k, int m, std::list<std::list<std::string>> & All_path,
	std::string from, std::string to
	);