
#include "Filestream.h"
#include "Algorithm.h"
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <tuple>
bool FileStreamInput(const char* file_name,
	Plan &plan,
	std::unordered_map<std::string, Passenger> &g_client,
	unsigned int &g_time,
	std::list<std::string> &g_city)
{
	std::ifstream ifstrm(file_name, std::ifstream::in);
	if (!ifstrm.is_open())
		throw std::runtime_error("Fail to open input file.");

	bool GraphFinish = false;
	std::string command, mode;
	std::unordered_map<std::string, bool> city;
	std::unordered_map<std::string, std::tuple<Passenger::Strategy,
		std::string, std::string, Passenger::RouteInfoSet, unsigned int >> tPassenger;

	while (ifstrm >> command)
	{
		if (command == "touch")
		{
			ifstrm >> mode;
			if (mode == "-c")
			{
				std::string city_name;
				ifstrm >> city_name;
				while (city_name != "-n")
				{
					city[city_name] = true;
					g_city.push_back(city_name);
					//plan.SetTrsp(city_name, std::string(), Trsp());
					ifstrm >> city_name;					
				}
			}
			else
				return false;
		}
		else if (command == "trp")
		{
			ifstrm >> mode;
			if (mode == "-c" || mode == "-t" || mode == "-p")
			{
				int cPrice, tDeparture, tArrival;
				std::string cityA, cityB;
				ifstrm >> cPrice;
				if (!ifstrm.fail())
				{
					ifstrm >> cityA >> cityB;
					try
					{
						if (city.at(cityA) && city.at(cityB))
						{
							ifstrm >> tDeparture >> tArrival;
							if (!ifstrm.fail())
							{
								Trsp obj;
								obj.cPrice = cPrice;
								obj.tDeparture = tDeparture;
								obj.tArrival = tArrival;
								if (mode == "-c") obj.type = Trsp::_type::Car;
								else if (mode == "-t") obj.type = Trsp::_type::Train;
								else if (mode == "-p") obj.type = Trsp::_type::Plane;
								plan.SetTrsp(cityA, cityB, obj);
							}
							else return false;
						}
					}
					catch (std::out_of_range &err)
					{
						std::cout << err.what() << std::endl;
					}
				}
				else return false;
			}
			else return false;
		}
		else if (command == "client")
		{
			if (!GraphFinish) {
				GraphFinish = true;
			}
			std::string name, strategy, curr_city, dest_city;
			unsigned int limit = 0;
			ifstrm >> mode;

			if (!ifstrm.fail() && mode == "-p")
			{
				ifstrm >> name >> strategy >> curr_city >> dest_city;

				Passenger::Strategy stg;
				if (strategy == "-p") stg = Passenger::Strategy::MinPrice;
				else if (strategy == "-t") stg = Passenger::Strategy::MinTime;
				else if (strategy == "-pt") {
					stg = Passenger::Strategy::MinPriceInTime;
					ifstrm >> limit;
				}
				else return false;

				try
				{
					auto i = city.at(curr_city);
					auto j = city.at(dest_city);


					tPassenger.emplace(std::make_pair(std::move(name),
						std::make_tuple(std::move(stg),
							std::move(curr_city),
							std::move(dest_city),
							Passenger::RouteInfoSet{},
							std::move(limit))));
				}
				catch (const std::out_of_range &err)
				{
					std::cout << err.what() << std::endl;
				}
			}
			else if (mode == "-v")
			{
				std::string via_city;
				unsigned int stay_time;
				ifstrm >> name >> via_city >> stay_time;
				if (!ifstrm.fail())
				{
					try
					{
						std::get<3>(tPassenger.at(name))
							.emplace(Passenger::RouteInfo{ via_city, stay_time, 0 });
					}
					catch (const std::out_of_range &err)
					{
						std::cout << err.what() << std::endl;
					}
				}
			}
			else return false;
		}
		else
			return false;
	}

	// Copy temp Passengers
	for (auto &i : tPassenger)
	{
		g_client.emplace(std::make_pair(std::move(i.first),
			Passenger{ std::move(std::get<0>(i.second)),
			std::move(std::get<1>(i.second)),
			std::move(std::get<2>(i.second)),
			std::move(std::get<3>(i.second)),
			std::move(std::get<4>(i.second)),
			plan, g_time }));
	}

	InitTable(plan, g_city);

	return true;
}

void StreamOutput(std::ostream &ostr,
	const Plan &plan,
	const std::unordered_map<std::string, Passenger> &g_client,
	unsigned int g_time)
{
	std::list<std::string> ConjCities;
	std::list<Trsp> Trspor;

	ostr << "=============================Log=============================\n";
	ostr << "Current Time: " <<"Day:"<<(g_time/24)<< " HOUR:"<<(g_time % 24) << "\n\n";

	//std::list<std::string> Cities = plan.GetCitys ();

	//if (!Cities.empty ())
	//	ostr << "Time Table:" << std::endl;
	//for (auto i : Cities)
	//{
	//	ConjCities = plan.GetConj (i);
	//	for (auto j : ConjCities)
	//	{
	//		Trspor = plan.GetTrsp (i, j);
	//		for (auto k : Trspor)
	//		{
	//			ostr << "Departure City: " << i << "\tArrival City: " << j << std::endl
	//				<< "\tTransportation: ";

	//			switch (k.type)
	//			{
	//			case Trsp::_type::Car: ostr << "Car";
	//				break;
	//			case Trsp::_type::Plane: ostr << "Plane";
	//				break;
	//			case Trsp::_type::Train: ostr << "Train";
	//				break;
	//			}

	//			ostr << "\nPrice: " << k.cPrice << std::endl
	//				<< "Departure Time: " << k.tDeparture 
	//				<< " Travelling Time: " << k.tArrival << std::endl;
	//		}
	//		ostr << std::endl;
	//	}
	//}

	for (auto i : g_client)
	{
		ostr << "Passenger:\t" << i.first << "\n";
		ostr << "Strategy:\t";
		switch (i.second.GetStrategy())
		{
		case Passenger::Strategy::MinPrice:ostr << "Min Price\n";
			break;
		case Passenger::Strategy::MinTime:ostr << "Min Time\n";
			break;
		case Passenger::Strategy::MinPriceInTime: ostr << "Min Price In Limited Time\n";
			break;
		}
		ostr << "State:\t";
		if (i.second.GetPathState() == false) {
			if (i.second.Is_Stay())
				ostr << "Staying in " << i.second.GetPostion().first << ".\n";
		}
		else {
			if (i.second.Is_Stay())
				ostr << "Staying in " << i.second.GetPostion().first << ", leaving in "
				<< i.second.GetCountdown() << " hours.\n";
			else
			{
				auto temp = i.second.GetPostion();
				ostr << "On the way from " << temp.first << " to "
					<< temp.second << " by ";
				switch (i.second.GetFrontTrsp()) {
				case Trsp::_type::Car:
					ostr << "Car";
					break;
				case Trsp::_type::Plane:
					ostr << "Plane";
					break;
				case Trsp::_type::Train:
					ostr << "Train";
					break;
				}

				ostr << ", arriving in " << i.second.GetCountdown() << " hours.\n";

			}

		}

		ostr << std::endl;
	}
	//ostr << "=============================Log=============================" << std::endl;
}