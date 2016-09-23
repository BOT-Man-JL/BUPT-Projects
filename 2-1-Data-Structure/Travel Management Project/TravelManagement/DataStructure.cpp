
#include "DataStructure.h"
#include "Algorithm.h"
#include "Driver.h"
#include <string>
#include <list>
#include <utility>

#include <iostream>


// Class Plan

std::list<std::string> Plan::GetCitys() const
{
	std::list<std::string> result;

	for (auto i : map)
	{
		result.push_back(i.first);
	}

	return std::move(result);
}

std::list<std::string> Plan::GetConj(const std::string &from) const
{
	std::list<std::string> result;

	// Throw exception when no city
	for (auto i : map.at(from))
	{
		result.push_back(i.first);
	}

	return std::move(result);
}

const std::list<Trsp>& Plan::GetTrsp(const std::string &from, const std::string &to) const
{
	// Throw exception when no city
	return this->map.at(from).at(to);
}

bool Plan::SetTrsp(const std::string &from, const std::string &to, const Trsp & trsp)
{
	this->map[from][to].push_back(trsp);
	return true;
}

// End of Class Plan


// Class Passenger

Passenger::Passenger(Strategy strategy,
	std::string curr,
	std::string dest,
	RouteInfoSet && via,
	unsigned int limit,
	const Plan & plan, unsigned int &time)
	: _strategy(strategy), ref_plan(plan), ref_time(time),
	via_city{ via }, start_city(curr), dest_city(dest), limit_time(limit),
	is_stay(true), count_down(0), path_state(false), is_to_update(false)
{
	// Normalize via_city
	for (auto p = via_city.begin(); p != via_city.end();)
	{
		if (p->city == dest || p->city == curr)
		{
			if ((p = via_city.erase(p)) != via_city.end())
				++p;
		}
		else
			++p;
	}

	// Implicit Init path & curr_pos
	_UpdatePath(curr);
}

Passenger::Strategy Passenger::GetStrategy() const
{
	return _strategy;
}

void Passenger::SetStrategy(Strategy strategy)
{
	_strategy = strategy;
	is_to_update = true;				// delegated to MoveForward
}

const Passenger::RouteInfoSet& Passenger::GetViaCity() const
{
	return via_city;
}

bool Passenger::SetViaCity(std::string city, unsigned int tStay)
{
	// Check is city the curr / dest
	if (city == path.front().city || city == dest_city)
		return false;

	// Implicit unique the set by name
	via_city.emplace(RouteInfo{ city, tStay, 0 });

	is_to_update = true;				// delegated to MoveForward
	return true;
}

std::string Passenger::GetDest() const
{
	return dest_city;
}

bool Passenger::SetDest(std::string city)
{
	// Check if dest is a via_city
	for (auto &i : via_city)
		if (i.city == city)
		{
			via_city.erase(i);
			break;
		}

	dest_city = city;

	is_to_update = true;				// delegated to MoveForward
	return true;
}

void Passenger::SetLimit(unsigned int limit)
{
	limit_time = limit;
}

std::pair<const std::string, const std::string> Passenger::GetPostion() const
{
	if (is_stay) {
		if (path.size() == 0)
			return std::make_pair(start_city, std::string());
		else if (path.size() == 1)
			return std::make_pair(dest_city, std::string());
		else
			return std::make_pair(path.front().city, std::string());	// second = ""
	}

	else
	{
		auto p = path.begin();
		if (path.size() > 1)
			++p;

		return std::make_pair(path.front().city, (*p).city);
	}

}

std::pair<const std::string, const std::string> Passenger::GetPostion(unsigned int after_time) const
{
	// Todo: there are maybe some bugs.

	auto i = path.begin(), j = path.begin();								// (i, j) for test
	after_time -= count_down;

	for (++j; after_time > 0 && j != path.end(); ++i, ++j)
	{
		if (j != path.end() && (*i).tArr + (*i).tStay < after_time && after_time < (*j).tArr)
			return std::make_pair((*i).city, (*j).city);
		else if ((*i).tArr <= after_time && after_time <= (*i).tArr + (*i).tStay)
			return std::make_pair((*i).city, std::string());		// second = ""

		after_time -= (*i).tArr + (*i).tStay;
	}

	return std::make_pair((*i).city, std::string());				// second = ""
}

bool Passenger::Is_Stay() const
{
	return is_stay;
}

int Passenger::GetCountdown() const
{
	return count_down;
}

bool Passenger::MoveForward()
{
	// Count down before the judgement
	--count_down;
	if (path_state == true) {


		if (count_down > 0)				// just skip this hour
		{
			if (is_stay && is_to_update)
			{
				_UpdatePath(path.front().city);			// update at waiting
				is_stay = true;
			}
		}
		else
		{
			if (!is_stay)					// on arrival
			{
				for (auto &i : via_city)
					if (path.front().city == i.city)
					{
						via_city.erase(i);
						break;
					}
				if (path.size() != 1)
					path.pop_front();

				if (is_to_update)
					_UpdatePath(path.front().city);		// update at arrival

				else if (path.size() == 1) {
					// meet the end
					is_stay = true;
					path_state = false;
					start_city = path.front().city;
					return false;
				}
				else if (path.size() == 0) {
					is_stay = true;
					path_state = false;
					return false;
				}
				count_down = path.front().tStay;
				is_stay = true;

				if (count_down == 0)		// on departure as soon as arrival (no stay)
					MoveForward();
			}
			else							// on departure
			{
				auto p = path.begin();
				if (path.size() > 1) {
					++p;
				}
				//path.pop_front();
				count_down = (*p).tArr;
				is_stay = false;

				//if (count_down == 0)		// not possible to happen
				//	MoveForward ();
			}
		}
	}
	return true;
}

int Passenger::GetTotalTime()
{
	auto i = path.begin();
	if (path.size() > 1)
		++i;
	return (*i).tArr;
}

void Passenger::_UpdatePath(std::string from)
{
	// Todo: Algorithm - Update the path and set curr_pos
	// According to the from_city, via_city, dest_city, ref_plan and ref_time

	//todo: to adjust the output
	path.clear();
	int cost, t_time;
	switch (_strategy)
	{
	case Passenger::Strategy::MinPrice:
		GenerateMinPricePath(from, dest_city, via_city, ref_plan, ref_time, path,cost,t_time);
		break;
	case Passenger::Strategy::MinTime:
		GenerateMinTimePath(from, dest_city, via_city, ref_plan, ref_time, path,cost,t_time);
		break;
	case Passenger::Strategy::MinPriceInTime:
		Generate3rdstrategy(from, dest_city, via_city, ref_plan, ref_time, limit_time, path,cost,t_time);
		break;
	default:
		break;
	}
	//todo : 检查path是否为空，若为空，则没有找到适合的路径，并输出信息（输出放在哪里？）
	if (!path.size()) {
		std::cout << "There are no path satisfying the constraints.\n";
		path_state = false;
		return;
	}

	count_down = path.front().tStay;
	path_state = true;
	auto i = path.begin();
	std::cout << "Path: ";
	std::cout << (*i).city ;
	++i;
	for (;i != path.end();){
		std::cout << "--->" << (*i).city ;
		++i;
	}
	std::cout << std::endl;
	std::cout << "Total money cost:" << cost << "  " << "Total time cost:" << t_time << std::endl;

	is_to_update = false;
}

// End of Class Passenger
