/*
#define N 全局时间

一点点伪代码：
两个三维数组: LengOfPath[city0...cityn][city0...cityn][0...5], RecordOfPath[city0...cityn][city0...cityn][0...5];

*/
#include "Algorithm.h"
#include <algorithm>

//add for test
#include <fstream>
#include <iostream>
#include <queue>
#include <unordered_map>

#define N 24 //全局时间片

namespace Algo_impl
{
	Table g_AlgorithmTable;
}
using namespace Algo_impl;

void Table::Init(const Plan &plan, const std::list<std::string> &citylist)
{

	for (auto start : citylist) {
		for (auto end : citylist) {
			if (start == end) continue;
			std::list<Trsp> trans;
			try {
				trans = plan.GetTrsp(start, end);
			}
			catch (std::exception &e) { ; }

			if (trans.empty()) _MinPriceTable[start][end].first = INT_MAX;
			else {
				TrspNode min;
				min.vehicle.cPrice = INT_MAX;
				for (auto i : trans) {
					if (i.cPrice < min.vehicle.cPrice) {
						min.vehicle.cPrice = i.cPrice;
						min.vehicle.tArrival = i.tArrival;
						min.vehicle.tDeparture = i.tDeparture;
						min.vehicle.type = i.type;
						min.start = start;
						min.end = end;
					}
				}
				_MinPriceTable[start][end].first = min.vehicle.cPrice;
				_MinPriceTable[start][end].second.push_back(min);
			}
		}
	}

	for (auto mid : citylist) {
		for (auto start : citylist) {
			for (auto end : citylist) {
				if (start == mid || start == end || mid == end) continue;
				if ((_MinPriceTable.at(start).at(mid).first != INT_MAX) && (_MinPriceTable.at(mid).at(end).first != INT_MAX) && (_MinPriceTable.at(start).at(mid).first + _MinPriceTable.at(mid).at(end).first < _MinPriceTable.at(start).at(end).first)) {
					_MinPriceTable.at(start).at(end).first = _MinPriceTable.at(start).at(mid).first + _MinPriceTable.at(mid).at(end).first;
					_MinPriceTable.at(start).at(end).second.clear();
					_MinPriceTable.at(start).at(end).second.assign(_MinPriceTable.at(start).at(mid).second.begin(), _MinPriceTable.at(start).at(mid).second.end());
					_MinPriceTable.at(start).at(end).second.insert(_MinPriceTable.at(start).at(end).second.end(), _MinPriceTable.at(mid).at(end).second.begin(), _MinPriceTable.at(mid).at(end).second.end());
					/*for (auto i : _MinPriceTable.at(start).at(mid).second)
					_MinPriceTable.at(start).at(end).second.push_back(i);
					for (auto i : _MinPriceTable.at(mid).at(end).second)
					_MinPriceTable.at(start).at(end).second.push_back(i);*/
				}
			}
		}
	}

	for (auto start : citylist) {
		for (auto end : citylist) {
			if (start == end) continue;
			else {
				for (int i = 0; i != N; ++i) _MinTimeTable[start][end][i].first = INT_MAX;
				std::list<Trsp> trans;
				try {
					trans = plan.GetTrsp(start, end);
				}
				catch (std::exception &e) { continue; }
				int min_tDeparture = N - 1, max_tDeparture = 0, minArrPoint;
				for (auto i : trans) {
					if (_MinTimeTable[start][end][i.tDeparture].first > i.tDeparture + i.tArrival) {
						_MinTimeTable[start][end][i.tDeparture].first = i.tDeparture + i.tArrival;
						TrspNode min;
						min.vehicle.cPrice = i.cPrice;
						min.vehicle.tArrival = i.tArrival;
						min.vehicle.tDeparture = i.tDeparture;
						min.vehicle.type = i.type;
						min.start = start;
						min.end = end;
						_MinTimeTable[start][end][i.tDeparture].second.clear();
						_MinTimeTable[start][end][i.tDeparture].second.push_back(min);
						if (i.tDeparture < min_tDeparture) min_tDeparture = i.tDeparture;
						if (i.tDeparture > max_tDeparture) max_tDeparture = i.tDeparture;
					}
				}
				//整形
				if (_MinTimeTable[start][end][min_tDeparture].first != INT_MAX)
					for (int i = N - 1; i != max_tDeparture; --i) {
						_MinTimeTable[start][end][i].first = _MinTimeTable[start][end][min_tDeparture].first + N;
						//Trsp min = _MinTimeTable[start][end][min_tDeparture].second
						_MinTimeTable[start][end][i].second.assign(_MinTimeTable[start][end][min_tDeparture].second.begin(), _MinTimeTable[start][end][min_tDeparture].second.end());
					}
				minArrPoint = _MinTimeTable[start][end][max_tDeparture].first;
				for (int i = max_tDeparture - 1; i != -1; --i) {
					if (_MinTimeTable[start][end][i].first > minArrPoint) {
						_MinTimeTable[start][end][i].first = minArrPoint;
						_MinTimeTable[start][end][i].second.assign(_MinTimeTable[start][end][max_tDeparture].second.begin(), _MinTimeTable[start][end][max_tDeparture].second.end());
					}
					else {
						minArrPoint = _MinTimeTable[start][end][i].first;
						max_tDeparture = i;
					}
				}
			}
		}
	}

	for (auto start : citylist) {
		for (auto end : citylist) {
			if (start == end) continue;
			else {
				for (int i = 0; i != N; ++i) {
					_OneStepTimeTable[start][end][i].first = _MinTimeTable[start][end][i].first;
					_OneStepTimeTable[start][end][i].second.assign(_MinTimeTable[start][end][i].second.begin(), _MinTimeTable[start][end][i].second.end());
				}
			}
		}
	}


	for (auto mid : citylist) {
		for (auto start : citylist) {
			if (mid == start) continue;
			for (auto end : citylist) {
				if (mid == end || start == end) continue;
				for (int i = 0; i != N; ++i) {
					if (_MinTimeTable[start][mid][i].first == INT_MAX) continue;
					else if (_MinTimeTable[mid][end][_MinTimeTable[start][mid][i].first % N].first == INT_MAX) continue;
					else if (_MinTimeTable[start][mid][i].first + _MinTimeTable[mid][end][_MinTimeTable[start][mid][i].first % N].first - _MinTimeTable[start][mid][i].first % N < _MinTimeTable[start][end][i].first) {
						_MinTimeTable[start][end][i].first = _MinTimeTable[start][mid][i].first + _MinTimeTable[mid][end][_MinTimeTable[start][mid][i].first % N].first - _MinTimeTable[start][mid][i].first % N;
						_MinTimeTable[start][end][i].second.clear();
						_MinTimeTable[start][end][i].second.assign(_MinTimeTable[start][mid][i].second.begin(), _MinTimeTable[start][mid][i].second.end());
						_MinTimeTable[start][end][i].second.insert(_MinTimeTable[start][end][i].second.end(), _MinTimeTable[mid][end][_MinTimeTable[start][mid][i].first % N].second.begin(), _MinTimeTable[mid][end][_MinTimeTable[start][mid][i].first % N].second.end());
					}

				}
			}
		}
	}
}

void Table::ShowMinPrice(const Plan &plan, const std::list<std::string> &citylist)
{
		std::ofstream ofstr("minprice.txt", std::ofstream::out);

	for (auto start : citylist) {
		for (auto end : citylist) {
			if (start == end) continue;
			ofstr << start << " -> " << end << ":" << _MinPriceTable.at(start).at(end).first << std::endl;
			for (auto i : _MinPriceTable.at(start).at(end).second)
				ofstr << i.vehicle.cPrice << " ";
			ofstr << std::endl;
		}
		ofstr << std::endl << std::endl;
	}

}

void Table::ShowMinTime(const Plan &plan, const std::list<std::string> &citylist)
{	//test
	std::ofstream ofstr("mintime.txt", std::ofstream::out);

	for (auto start : citylist) {
		ofstr << start << std::endl;
		for (auto end : citylist) {
			if (start == end) continue;
			ofstr << end;
			for (int i = 0; i != N; ++i) {
				ofstr << " " << _MinTimeTable[start][end][i].first << " ";
			}
			ofstr << std::endl;
		}
	}
}

bool Table::GenerateMinPricePath(const std::string &from, const std::string &to,
	const Passenger::RouteInfoSet &via,
	const Plan &plan, unsigned int time,
	std::list<Passenger::RouteInfo> &path, int &cost, int &t_time)
{
	//No via city
	if (!via.size()) {
		if (_MinPriceTable[from][to].first == INT_MAX)
			return false;

		std::list<TrspNode> TrspPath;
		for (auto i : _MinPriceTable.at(from).at(to).second) {
			TrspPath.push_back(i);
		}
		//todo : trans to path
		if (TrspPath.size() == 0)
			return false;

		cost = 0;
		for (auto i : TrspPath)
			cost += i.vehicle.cPrice;

		Passenger::RouteInfo Node;
		Node.city = from;
		Node.tArr = 0;
		Node.tStay = (time % N) <= TrspPath.front().vehicle.tDeparture ? (TrspPath.front().vehicle.tDeparture - time % N) : N - (time % N) + TrspPath.front().vehicle.tDeparture;
		Node.TrspType = TrspPath.front().vehicle.type;
		path.push_back(Node);
		for (std::list<TrspNode>::iterator i = TrspPath.begin(); i != TrspPath.end(); ++i) {
			Passenger::RouteInfo Node;
			Node.city = (*i).end;
			Node.tArr = (*i).vehicle.tArrival;
			Node.TrspType = (*i).vehicle.type;
			std::list<TrspNode>::iterator j = i;
			++j;
			if (j != TrspPath.end()) {
				Node.tStay = (((*i).vehicle.tDeparture + (*i).vehicle.tArrival) % N) <= (*j).vehicle.tDeparture ? (*j).vehicle.tDeparture - (((*i).vehicle.tDeparture + (*i).vehicle.tArrival) % N) : (*j).vehicle.tDeparture + N - (((*i).vehicle.tDeparture + (*i).vehicle.tArrival) % N);
			}
			else Node.tStay = 0;

			path.push_back(Node);
		}

		t_time = 0;
		for (auto i : path)
			t_time += i.tStay + i.tArr;
	}
	else {
		//todo : 旅行商问题
	}
}

bool Table::GenerateMinTimePath(const std::string &from, const std::string &to,
	const Passenger::RouteInfoSet &via,
	const Plan &plan, unsigned int time,
	std::list<Passenger::RouteInfo> &path, int &cost, int &t_time)
{

	//No via city
	if (!via.size()) {
		if (_MinTimeTable[from][to][time].first == INT_MAX)
			return false;

		std::list<TrspNode> TrspPath;
		for (auto i : _MinTimeTable[from][to][time].second)
			TrspPath.push_back(i);
		if (TrspPath.size() == 0)
			return false;

		cost = 0;
		for (auto i : TrspPath)
			cost += i.vehicle.cPrice;

		//todo : trans to path
		Passenger::RouteInfo Node;
		Node.city = from;
		Node.tArr = 0;
		Node.tStay = (time % N) <= TrspPath.front().vehicle.tDeparture ? TrspPath.front().vehicle.tDeparture - (time % N) : N + TrspPath.front().vehicle.tDeparture - (time % N);
		Node.TrspType = TrspPath.front().vehicle.type;
		path.push_back(Node);
		for (std::list<TrspNode>::iterator i = TrspPath.begin(); i != TrspPath.end(); ++i) {
			Passenger::RouteInfo Node;
			Node.city = (*i).end;
			Node.tArr = (*i).vehicle.tArrival;
			Node.TrspType = (*i).vehicle.type;
			std::list<TrspNode>::iterator j = i;
			++j;
			if (j != TrspPath.end()) {
				Node.tStay = (((*i).vehicle.tDeparture + (*i).vehicle.tArrival) % N) <= (*j).vehicle.tDeparture ? (*j).vehicle.tDeparture - (((*i).vehicle.tDeparture + (*i).vehicle.tArrival) % N) : (*j).vehicle.tDeparture + N - (((*i).vehicle.tDeparture + (*i).vehicle.tArrival) % N);
			}
			else Node.tStay = 0;

			path.push_back(Node);
		}

		t_time = 0;
		for (auto i : path)
			t_time += i.tStay + i.tArr;
	}
	else {
		//todo:
	}
}

bool Table::Generate3rdstrategy(const std::string &from, const std::string &to,
	const Passenger::RouteInfoSet &via,
	const Plan &plan, unsigned int time,
	unsigned int limit,
	std::list<Passenger::RouteInfo> &path,int &cost, int &t_time)
{
	std::unordered_map<std::string, bool> via_set;
	std::unordered_map<std::string, unsigned int> via_set_stay;
	if (via.size())
		for (auto i : via) {
			via_set[i.city] = false;
			via_set_stay[i.city] = i.tStay;
		}

	std::priority_queue<Trsp3rdNode> P;
	std::priority_queue<Trsp3rdNode> B;
	std::unordered_map<std::string, unsigned int> Cnt;

	Trsp3rdNode Node;
	Node.Price = 0;
	Node.end = from;
	Node.time = time;
	B.push(Node);
	while (!B.empty()) {
		for (auto i : via_set)
			i.second = false;
		Node.end = B.top().end;
		Node.Price = B.top().Price;
		Node.time = B.top().time;
		Node.Path.assign(B.top().Path.begin(), B.top().Path.end());
		B.pop();
		if (Node.end == to) {
			if (Node.time - time <= limit) {
				if (!via.size()) {
					P.push(Node);
					break;
				}
				if (via.size()) {
					for (auto i : Node.Path)
						if (via_set.count(i.end)) via_set[i.end] = true;
					for (auto i : via_set)
						if (!i.second) continue;
					P.push(Node);
				}
			}
			//else P.pop_back();
		}
		if (Node.time - time <= limit) {
			std::list<std::string> Conj;
			try {
				Conj = plan.GetConj(Node.end);
			}
			catch (std::exception &e) { continue; }
			for (auto v : Conj) {
				bool Is_v_in_NodePath = false;
				int to_v_cost;
				for (auto i : Node.Path) {
					if (v == i.end) {
						Is_v_in_NodePath = true;
					}
				}
				if (!Is_v_in_NodePath) {
					const std::list<Trsp> end_to_v = plan.GetTrsp(Node.end, v);
					for (auto j : end_to_v) {
						Trsp3rdNode newNode;
						TrspNode trspNode;
						newNode.time = (Node.time % N) <= j.tDeparture ? Node.time + j.tDeparture - (Node.time % N) + j.tArrival : Node.time + j.tDeparture + N - (Node.time % N) + j.tArrival;
						if (newNode.time - time > limit) continue;
						trspNode.start = Node.end;
						trspNode.end = v;
						trspNode.vehicle.cPrice = j.cPrice;
						trspNode.vehicle.tArrival = j.tArrival;
						trspNode.vehicle.tDeparture = j.tDeparture;
						trspNode.vehicle.type = j.type;
						newNode.end = v;
						newNode.Price = Node.Price + j.cPrice;
						newNode.Path.assign(Node.Path.begin(), Node.Path.end());
						newNode.Path.push_back(trspNode);
						B.push(newNode);
					}
				}
			}
		}
		else continue;
	}

	bool find = false;

	while (!find && !P.empty()) {
		Node.Price = P.top().Price;
		Node.end = P.top().end;
		Node.time = P.top().time;
		Node.Path.clear();
		Node.Path.assign(P.top().Path.begin(), P.top().Path.end());
		Passenger::RouteInfo pathNode;
		pathNode.city = from;
		pathNode.tArr = 0;
		//bug mabye
		pathNode.tStay = (time % N) <= Node.Path.front().vehicle.tDeparture ? Node.Path.front().vehicle.tDeparture - (time % N) : N + Node.Path.front().vehicle.tDeparture - time % N;
		pathNode.TrspType = Node.Path.front().vehicle.type;
		path.push_back(pathNode);
		for (auto i = P.top().Path.begin(); i != P.top().Path.end(); ++i) {
			Passenger::RouteInfo pathNode;
			pathNode.city = (*i).end;
			pathNode.tArr = (*i).vehicle.tArrival;
			pathNode.TrspType = (*i).vehicle.type;
			auto j = i;
			++j;
			if (j != P.top().Path.end()) {
				pathNode.tStay = (*i).vehicle.tDeparture + (*i).vehicle.tArrival <= (*j).vehicle.tDeparture ? (*j).vehicle.tDeparture - (*i).vehicle.tArrival - (*i).vehicle.tDeparture : (*j).vehicle.tDeparture + N - ((*i).vehicle.tDeparture + (*i).vehicle.tArrival) % N;
			}
			else pathNode.tStay = 0;

			
			if (via_set.count(pathNode.city))
				if (pathNode.tStay < via_set_stay[pathNode.city]) {
					path.clear();
					P.pop();
					break;
				}

			path.push_back(pathNode);
			if (j == P.top().Path.end()) {
				find = true;
				cost = P.top().Price;
				t_time = P.top().time - time;
			}
		}
	}
	if (!find) return false;
	return true;
	
}

bool GenerateMinPricePath(const std::string &from, const std::string &to,
	const Passenger::RouteInfoSet &via,
	const Plan &plan, unsigned int time,
	std::list<Passenger::RouteInfo> &path, int &cost, int &t_time)
{
	if (via.size() != 0)
		return g_AlgorithmTable.GenerateMinPricePath_ViaCity(from, to, via, plan, time, path,cost,t_time);
	else
		return g_AlgorithmTable.GenerateMinPricePath(from, to, via, plan, time, path,cost,t_time);
}

bool GenerateMinTimePath(const std::string &from, const std::string &to,
	const Passenger::RouteInfoSet &via,
	const Plan &plan, unsigned int time,
	std::list<Passenger::RouteInfo> &path, int &cost, int &t_time)
{
	if (via.size() != 0)
		return g_AlgorithmTable.GenerateMinTimePath_ViaCity(from, to, via, plan, time, path,cost,t_time);
	else
		return g_AlgorithmTable.GenerateMinTimePath(from, to, via, plan, time, path,cost,t_time);
}

bool Generate3rdstrategy(const std::string &from, const std::string &to,
	const Passenger::RouteInfoSet &via,
	const Plan &plan, unsigned int time,
	unsigned int limit,
	std::list<Passenger::RouteInfo> &path, int &cost, int &t_time)
{
	return g_AlgorithmTable.Generate3rdstrategy(from, to, via, plan, time, limit, path,cost,t_time);
}


void InitTable(const Plan &g_plan, const std::list<std::string> g_city)
{
	g_AlgorithmTable.Init(g_plan, g_city);
	//g_AlgorithmTable.ShowMinPrice(g_plan, g_city);
	//g_AlgorithmTable.ShowMinTime(g_plan, g_city);
}
bool Table::GenerateMinTimePath_ViaCity(const std::string &from, const std::string &to,
	const Passenger::RouteInfoSet &via,
	const Plan &plan, unsigned int time,
	std::list<Passenger::RouteInfo> &path,int &cost, int &t_time) {

	std::list<std::list<std::string>>	All_path;
	auto citylist = plan.GetCitys();
	int Size_City = citylist.size();//The number of cities in the map
	std::string * ArrCity = new std::string[Size_City];
	int j = 0;

	for (auto i : citylist) {
		ArrCity[j] = i;
		++j;
	}
	j = 0;
	for (int k = 0;k < Size_City;++k) {
		if (ArrCity[k] == from)
			j = k;
	}
	Swap(ArrCity[0], ArrCity[j]);//让起点在ArrCity的首部,只排列后面的城市
	FindPath(plan, ArrCity, 1, citylist.size(), All_path, from, to);//找到所有可行的路径
	if (All_path.size() == 0)//没有可行的路径
		return false;

	std::list<std::string> Routetemp /*= All_path.front()*/;
	std::list<Passenger::RouteInfo> TempPath;
	int min_time = 65536;
	//auto front = Routetemp.begin();
	All_path.unique();//去除重复路径
	for (auto Route : All_path) {//比较所有生成的路径，找到需要时间最少的路径
		TempPath.clear();
		std::list<Trsp> TNode;//每两个节点之间的交通方式
		std::unordered_map<int, std::list<Trsp>> SolutionVS;//解空间Solution Vector Space
		auto front = Route.begin();
		int count = 0;

		int ViaCount = 0;
		for (auto k : Route) {
			for (auto l : via)
				if (k == l.city)
					ViaCount++;
		}
		if (ViaCount != via.size())//当前路未包含全部中间城市
			continue;

		for (auto last = front;front != Route.end();) {//把两点之间的交通方式都添加进入SoultionVectorSpace
			last = front;
			front++;
			if (front == Route.end())
				break;
			TNode = plan.GetTrsp((*last), (*front));
			SolutionVS.emplace(std::make_pair(count, TNode));
			++count;
		}

		std::unordered_map <int, Trsp>SolutionVector;
		std::unordered_map <int, int>SVFlag;
		for (int i = 0;i < SolutionVS.size();++i) {
			SVFlag.emplace(std::make_pair(i, -1));
		}//标记第i层的可搜索空间

		std::list<Passenger::RouteInfo> _Path;
		int layer = 0;
		while (layer >= 0) {
			if (Searchable(SVFlag, SolutionVS, layer) < SolutionVS.at(layer).size())    // t = 1时，f(n, 1) = g(n, 1) = 起始城市1, 只能选起始城市
				for (int i = Searchable(SVFlag, SolutionVS, layer);i < SolutionVS.at(layer).size();i++) {
					auto _tempIterator = SolutionVS.at(layer).begin();
					for (int count = 0;count < i;++count)
						_tempIterator++;
					SolutionVector[layer] = (*_tempIterator);
					SVFlag.at(layer) = i;
					if (layer < SVFlag.size()) { //在满足约束和限界条件下/*constraint(t) && bound(t)*/ 
						if (layer == SVFlag.size() - 1) {		//		_Path.clear();
							int _time;
							_time = GenerateMTPath(Route, SolutionVector, plan, time, via, _Path,cost,t_time);
							if (_time < min_time) {
								min_time = _time;
								path = _Path;
							}
						}      // 得到完全解，结束
						else layer++;
					}
				}
			else {
				SVFlag.at(layer) = -1;
				layer--;  //搜寻完t层各个子结点后未得到完全解，回溯到上一层结点，

			}
		}

	}

	if (path.size() == 0)//没有满足要求的路径
		return false;

	////生成路径
	//auto path_i = Routetemp.begin();
	//int Path_time = time;//路径上花的时间
	//int _cost = 0;//所花费用
	//for (auto path_j = path_i;path_i != Routetemp.end();) {
	//	path_j = path_i;
	//	path_i++;
	//	if (path_i == Routetemp.end())
	//		break;
	//	auto Trsp = plan.GetTrsp((*path_j), (*path_i));
	//	auto temp_Trsp = Trsp.front();

	//	for (auto k : Trsp) {//找到a,b之间费用最少的交通方式temp_Trsp其中包含	
	//						 //_type	type;
	//						 //int		cPrice;
	//						 //int		tDeparture;			// when to departure
	//						 //int		tArrival;			// how long to arrive at
	//		if (k.cPrice < temp_Trsp.cPrice)
	//			temp_Trsp = k;
	//	}
	//	_cost += temp_Trsp.cPrice;
	//	if (path_j == Routetemp.begin()) {//若path_j为出发城市
	//		unsigned int _tStay;
	//		_tStay = (Path_time % N) <= temp_Trsp.tDeparture ? temp_Trsp.tDeparture - (Path_time % N) : N + temp_Trsp.tDeparture - Path_time % N;
	//		for (auto j : via) {
	//			if (j.city == (*path_j))
	//				_tStay = _tStay > j.tStay ? _tStay : j.tStay;
	//		}
	//		Path_time += _tStay;
	//		Passenger::RouteInfo NextCity{ (*path_j),_tStay,0 };
	//		path.emplace_back(NextCity);
	//		_tStay = 0;
	//		Path_time += (_tStay + temp_Trsp.tArrival);
	//		NextCity = { (*path_i),_tStay,static_cast<unsigned>(temp_Trsp.tArrival),temp_Trsp.type };
	//		path.emplace_back(NextCity);
	//	}
	//	else {
	//		auto _temp = path.back();
	//		path.pop_back();
	//		_temp.tStay = (Path_time % N) <= temp_Trsp.tDeparture ? temp_Trsp.tDeparture - (Path_time % N) : N + temp_Trsp.tDeparture - Path_time % N;
	//		for (auto j : via) {
	//			if (j.city == _temp.city)
	//				_temp.tStay = _temp.tStay > j.tStay ? _temp.tStay : j.tStay;
	//		}
	//		path.push_back(_temp);
	//		unsigned int _tStay = 0;
	//		Path_time += (_tStay + temp_Trsp.tArrival);
	//		Passenger::RouteInfo NextCity{ (*path_i),_tStay,temp_Trsp.tArrival,temp_Trsp.type };
	//		path.emplace_back(NextCity);
	//	}
	//}
	//cost = _cost;
	//t_time = Path_time - time;

	return true;
}

int Searchable(const std::unordered_map<int, int>&SVFlag, const std::unordered_map<int, std::list<Trsp>> &SolutionVS, const int &layer) {
	int k = SVFlag.at(layer);
	int j = SolutionVS.at(layer).size();
	if (k < j)
		return k + 1;
	return -1;
}
int GenerateMTPath(std::list<std::string> &Route, std::unordered_map <int, Trsp> &SolutionVector, const Plan & plan,
	unsigned int time, const Passenger::RouteInfoSet & via, std::list<Passenger::RouteInfo> &path,int &cost, int &t_time)
{//利用Route信息及SolutionVector在path中生成最终的路径
	path.clear();
	auto path_i = Route.begin();
	int Path_time = time;//路径上花的时间
	int _cost = 0;//所花费用
	int seq = 0;
	for (auto path_j = path_i;path_i != Route.end();) {
		path_j = path_i;
		path_i++;
		if (path_i == Route.end())
			break;
		auto Trsp = SolutionVector.at(seq);
		seq++;
		_cost += Trsp.cPrice;

		if (path_j == Route.begin()) {//若path_j为出发城市
			unsigned int _tStay;
			_tStay = (Path_time % N) <= Trsp.tDeparture ? Trsp.tDeparture - (Path_time % N) : N + Trsp.tDeparture - Path_time % N;
			for (auto j : via) {
				if (j.city == (*path_j))
					_tStay = _tStay > j.tStay ? _tStay : j.tStay;
			}
			Path_time += _tStay;
			Passenger::RouteInfo NextCity{ (*path_j),_tStay,0 };
			path.emplace_back(NextCity);
			_tStay = 0;
			Path_time += (_tStay + Trsp.tArrival);
			NextCity = { (*path_i),_tStay,static_cast<unsigned>(Trsp.tArrival) ,Trsp.type };
			path.emplace_back(NextCity);
		}
		else {
			auto _temp = path.back();
			path.pop_back();
			_temp.tStay = (Path_time % N) <= Trsp.tDeparture ? Trsp.tDeparture - (Path_time % N) : N + Trsp.tDeparture - (Path_time % N);
			for (auto j : via) {
				if (j.city == _temp.city&&_temp.tStay < j.tStay) {
					Path_time += j.tStay;
					_temp.tStay = j.tStay + ((Path_time % N) <= Trsp.tDeparture ? Trsp.tDeparture - (Path_time % N) : N + Trsp.tDeparture - (Path_time % N));
					Path_time -= j.tStay;
					break;
				}
			}
			path.push_back(_temp);
			unsigned int _tStay = 0;
			Path_time += (_temp.tStay + Trsp.tArrival);
			Passenger::RouteInfo NextCity{ (*path_i),_tStay,Trsp.tArrival ,Trsp.type };
			path.emplace_back(NextCity);
		}
	}
	cost = _cost;
	t_time = Path_time - time;
	return Path_time - time;
}

bool Table::GenerateMinPricePath_ViaCity(std::string from, std::string to, const Passenger::RouteInfoSet &via, const Plan &ref_plan,
	unsigned int time, std::list<Passenger::RouteInfo> &path, int &cost, int &t_time) {
	std::list<std::list<std::string>>	All_path;
	auto citylist = ref_plan.GetCitys();
	int Size_City = citylist.size();//The number of cities in the map
	std::string * ArrCity = new std::string[Size_City];
	int j = 0;
	for (auto i : citylist) {
		ArrCity[j] = i;
		++j;
	}
	j = 0;
	for (int k = 0;k < Size_City;++k) {
		if (ArrCity[k] == from)
			j = k;
	}
	Swap(ArrCity[0], ArrCity[j]);//让起点在ArrCity的首部,只排列后面的城市
	FindPath(ref_plan, ArrCity, 1, citylist.size(), All_path, from, to);
	if (All_path.size() == 0)//没有可行的路径
		return false;
	std::list<std::string> Routetemp /*= All_path.front()*/;
	int min_cost = 65536;
	All_path.unique();
	for (auto i : All_path) {//比较所有生成的路径，找到花费最少的路径
		int cost = 0;
		auto front = i.begin();
		for (auto last = front; front != i.end();) {//两个城市之间
			last = front;
			front++;
			if (front == i.end())
				break;
			auto Trsp = ref_plan.GetTrsp((*last), (*front));
			auto temp_Trsp = Trsp.front();
			int _cost = 0;
			for (auto k : Trsp) {//找到a,b之间费用最少的交通方式temp_Trsp其中包含	
				if (k.cPrice < temp_Trsp.cPrice)
					temp_Trsp = k;
			}
			cost += temp_Trsp.cPrice;
		}
		int Via_Count = 0;
		for (auto k : i) {
			for (auto j : via)
				if (k == j.city)
					Via_Count++;
		}
		if (Via_Count == via.size()) {//All the via cities are in the path
			if (cost < min_cost) {
				min_cost = cost;
				Routetemp = i;
			}
		}
	}
	if (Routetemp.size() == 0)//没有满足要求的路径
		return false;
	auto path_i = Routetemp.begin();
	int Path_time = time;//路径上花的时间
	int _cost = 0;//所花费用
	for (auto path_j = path_i;path_i != Routetemp.end();) {
		path_j = path_i;
		path_i++;
		if (path_i == Routetemp.end())
			break;
		auto Trsp = ref_plan.GetTrsp((*path_j), (*path_i));
		auto temp_Trsp = Trsp.front();

		for (auto k : Trsp) {//找到a,b之间费用最少的交通方式temp_Trsp其中包含	
			if (k.cPrice < temp_Trsp.cPrice)
				temp_Trsp = k;
		}
		_cost += temp_Trsp.cPrice;
		if (path_j == Routetemp.begin()) {//若path_j为出发城市
			unsigned int _tStay;
			_tStay = (Path_time % N) <= temp_Trsp.tDeparture ? temp_Trsp.tDeparture - (Path_time % N) : N + temp_Trsp.tDeparture - Path_time % N;
			for (auto j : via) {
				if (j.city == (*path_j))
					_tStay = _tStay > j.tStay ? _tStay : j.tStay;
			}
			Path_time += _tStay;
			Passenger::RouteInfo NextCity{ (*path_j),_tStay,0 };
			path.emplace_back(NextCity);
			_tStay = 0;
			Path_time += (_tStay + temp_Trsp.tArrival);
			NextCity = { (*path_i),_tStay,static_cast<unsigned>(temp_Trsp.tArrival),temp_Trsp.type };
			path.emplace_back(NextCity);
		}
		else {
			auto _temp = path.back();
			path.pop_back();
			_temp.tStay = (Path_time % N) <= temp_Trsp.tDeparture ? temp_Trsp.tDeparture - (Path_time % N) : N + temp_Trsp.tDeparture - (Path_time % N);
			for (auto j : via) {
				if (j.city == _temp.city&&_temp.tStay < j.tStay) {
					Path_time += j.tStay;
					_temp.tStay = j.tStay + ((Path_time % N) <= temp_Trsp.tDeparture ? temp_Trsp.tDeparture - (Path_time % N) : N + temp_Trsp.tDeparture - (Path_time % N));
					Path_time -= j.tStay;
					break;
				}
			}
			path.push_back(_temp);
			unsigned int _tStay = 0;
			Path_time += (_temp.tStay + temp_Trsp.tArrival);
			Passenger::RouteInfo NextCity{ (*path_i),_tStay,temp_Trsp.tArrival,temp_Trsp.type };
			path.emplace_back(NextCity);
		}
	}
	t_time = Path_time - time;
	cost = _cost;
	//Bug:如果没有路径,打印乘客信息时会出问题。GetPosition
	return true;

}

//void FindPath(const Plan &g_plan, std::string  ArrCity[], int k, int m, std::list<std::list<std::string>> & All_path,
//	std::string from, std::string to
//	) {
//	 do{
//				if (JudgeConnect(g_plan, ArrCity, m, from, to))//Judge whether the path in ArrCity can
//															   //get through from source to destination
//				{
//					int j = 0;
//					std::list<std::string> temp_path;
//					for (j = 0;j < m && ArrCity[j] != to;++j) {
//						temp_path.push_back(ArrCity[j]);
//					}
//					temp_path.push_back(ArrCity[j]);
//					All_path.emplace_back(temp_path);
//					All_path.unique();
//					if(All_path.size() > 100)//找到100种不同路径就直接退出
//						break;
//				}
//	 } while (PathPermutaion(g_plan, ArrCity, k, m, All_path, from, to));
//}
//bool PathPermutaion(const Plan &g_plan, std::string  ArrCity[], int k, int m, std::list<std::list<std::string>> & All_path,
//	std::string from, std::string to
//	) {
//	{
//		int i;
//		for (i = m - 1;i > 1;i--)
//		{//从数组后面往前找到第一个比后面小的地方     
//			if (ArrCity[i - 1]<ArrCity[i])
//				break;
//		}
//		if (i == 1)//整个数组都是逆序，说明是已是排列最后一个     
//			return false;//没有下一个     
//		else
//		{
//			std::string t = ArrCity[i - 1];
//			int pos = i;
//			for (int j = i;j<m;j++)
//			{
//				if (ArrCity[j] > t && ArrCity[j] <= ArrCity[pos])//再往后找比v[i-1]大的数中最小的一个,把<改成<=,否则输入有重复时会出现bug  
//					pos = j;
//			}
//			ArrCity[i - 1] = ArrCity[pos];
//			ArrCity[pos] = t;//交换     
//					   //sort(v.begin()+i,v.end());//从小到达排序
//			for (int j = i, k = m - 1;j <= k;++j, --k) {
//				std::string temp = ArrCity[j];
//				ArrCity[j] = ArrCity[k];
//				ArrCity[k] = temp;
//			}//此时正好逆序，只需反转即可从小到达排序
//			
//			return true;//还有下一个     
//		}
//	}
//}     



void FindPath(const Plan &g_plan, std::string  ArrCity[], int k, int m, std::list<std::list<std::string>> & All_path,
	std::string from, std::string to
	)
{//找到from到to之间的所有路径并存入链表All_path中
	if (k == m)
	{
		if (JudgeConnect(g_plan, ArrCity, m, from, to))//Judge whether the path in ArrCity can
													   //get through from source to destination

		{
			int j = 0;
			std::list<std::string> temp_path;
			for (j = 0;j < m && ArrCity[j] != to;++j) {
				temp_path.push_back(ArrCity[j]);
			}
			temp_path.push_back(ArrCity[j]);
			All_path.emplace_back(temp_path);
		}
	}
	else
	{
		for (int i = k; i < m; i++) //Recursively swap two cities
		{
			{
				if (!JudgeConnect(g_plan, ArrCity, k, from, to))
					continue;
				Swap(ArrCity[k], ArrCity[i]);
				FindPath(g_plan, ArrCity, k + 1, m, All_path, from, to);
				Swap(ArrCity[k], ArrCity[i]);
			}
		}
	}
}
//}     

void Swap(std::string & a, std::string &b) {
	//swap the two cities in the array
	std::string temp;
	temp = a;
	a = b;
	b = temp;
}

bool JudgeConnect(const Plan & g_plan, std::string ArrCity[], int m, std::string from,
	std::string to
	)
{
	int flag = 1;
	for (int i = 0;i < m - 1 && flag != 0 && ArrCity[i] != to;++i) {
		if (!(ExistArc(g_plan, ArrCity, i, i + 1)))
			flag = 0;
	}
	if (flag == 0)
		return false;
	return true;
}
bool NeedSwap(const Plan &g_plan, std::string  ArrCity[], int i, int j, int m) {
	if (i == j) {
		if (i == m) {
			return ExistArc(g_plan, ArrCity, i - 1, i);
		}
		else if (i != 0) {
			return ExistArc(g_plan, ArrCity, i - 1, i) && ExistArc(g_plan, ArrCity, i, i + 1);
		}
		else
			return true;//ExistArc(g_plan, ArrCity, i, i+1);
	}
	else {
		if (i == 0 && j == m)
			return ExistArc(g_plan, ArrCity, j - 1, i) && ExistArc(g_plan, ArrCity, j, i + 1);
		else if (i != 0 && j == m)
			return ExistArc(g_plan, ArrCity, i - 1, j) && ExistArc(g_plan, ArrCity, j, i + 1) && ExistArc(g_plan, ArrCity, j - 1, i);
		else if (i == 0 && j != m)
			return ExistArc(g_plan, ArrCity, j, i + 1) && ExistArc(g_plan, ArrCity, j - 1, i) && ExistArc(g_plan, ArrCity, i, j + 1);
		else
			return ExistArc(g_plan, ArrCity, i - 1, j) && ExistArc(g_plan, ArrCity, i, j + 1) && ExistArc(g_plan, ArrCity, j - 1, i) && ExistArc(g_plan, ArrCity, j, i + 1);
	}
}
bool ExistArc(const Plan &g_plan, std::string  ArrCity[], int i, int j) {
	auto Conj = g_plan.GetConj(ArrCity[i]);
	for (auto k : Conj) {
		if (k == ArrCity[j])
			return true;
	}
	return false;
}
