#pragma once


#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>

struct Trsp				// Transportation
{
	enum class _type
	{
		Car,
		Train,
		Plane
	};

	_type	type;
	int		cPrice;
	int		tDeparture;			// when to departure
	int		tArrival;			// how long to arrive at
};

class Plan				// Map info.
{
	std::unordered_map<std::string,
		std::unordered_map<std::string, std::list<Trsp>>> map;
public:
	std::list<std::string> GetCitys() const;

	// Throw Exception if the city is not set
	std::list<std::string> GetConj(const std::string &from) const;
	const std::list<Trsp> &GetTrsp(const std::string &from, const std::string &to) const;

	bool SetTrsp(const std::string &from, const std::string &to, const Trsp &trsp);
	int Size() const { return map.size(); }
};

class Passenger			// Passenger
{
public:
	enum class Strategy
	{
		MinPrice,
		MinTime,
		MinPriceInTime
	};

	struct RouteInfo
	{
		std::string city;		// city name
		unsigned int tStay;		// how long to stay at
		unsigned int tArr;		// how long to arrive at
		Trsp::_type TrspType;   // the transporation type
	};

	struct RouteInfoHf
	{
		size_t operator() (const RouteInfo &i) const
		{
			return std::hash<std::string>()(i.city);
		}
	};
	struct RouteInfoEq
	{
		bool operator() (const RouteInfo &i, const RouteInfo &j) const
		{
			return i.city == j.city;
		}
	};
	using RouteInfoSet = std::unordered_set<RouteInfo, RouteInfoHf, RouteInfoEq>;

	Passenger(Strategy strategy,
		std::string curr_city,
		std::string dest_city,
		RouteInfoSet && via_city,
		unsigned int limit,
		const Plan &plan, unsigned int &time);

	std::pair<const std::string, const std::string> GetPostion() const;
	std::pair<const std::string, const std::string> GetPostion(unsigned int after_time) const;

	Strategy GetStrategy() const;
	void SetStrategy(Strategy strategy);

	const RouteInfoSet& GetViaCity() const;
	bool SetViaCity(std::string city, unsigned int tStay);

	std::string GetDest() const;
	bool SetDest(std::string city);
	void SetLimit(unsigned int limit);

	bool Is_Stay()const;
	int GetCountdown() const;
	bool MoveForward();
	int GetTotalTime();
	bool GetPathState() const {
		return path_state;
	}
	Trsp::_type GetFrontTrsp() const {
		auto p = path.begin();
		++p;
		return (*p).TrspType;
	}
private:
	Strategy _strategy;//the strategy that he choose

	RouteInfoSet via_city;//the cities between the starting city and the destination
	std::string start_city;//The start city
	std::string dest_city;//the destination

	void _UpdatePath(std::string from);
	const Plan& ref_plan;
	const unsigned int &ref_time;
	bool is_to_update;
	std::list<RouteInfo> path;
	bool path_state;//whether have a legal path
	bool is_stay;//whether the passenger is on the way or stop at somewhere
	int count_down;//the count-down time from one city to another or the staying time
	unsigned int limit_time;
};

