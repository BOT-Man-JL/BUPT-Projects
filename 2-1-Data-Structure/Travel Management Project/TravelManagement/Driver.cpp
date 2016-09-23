
#include "DataStructure.h"
#include "FileStream.h"
#include <iostream>
#include <string>
#include "Driver.h"
#include "EggAche.h"
#include <mutex>
#include <chrono>
#include <tuple>
#include <vector>

#include <strstream>
#include <unordered_map>
#include <fstream>
const int YAxis = 800;
const int XAxis = 1300;
struct ArrInfo {
	std::string name;
	int x;
	int y;
};

struct Passenger_Egg {
	EggAche::Egg Egg;
	int x;//store the x axis position right now
	int y;//store the y axis position right now
};
namespace Driver_impl		// shared data
{
	Plan g_plan;
	std::unordered_map<std::string, Passenger> g_client;
	unsigned int g_time;
	std::list<std::string> g_city;
	bool g_fExit;
	std::mutex g_mRW;
	std::mutex g_mHeat;
	std::timed_mutex g_mTime;

	std::ofstream g_file_out;

	EggAche::Window g_window(XAxis,YAxis, "Travel Management", nullptr, nullptr);//Window
	EggAche::Egg background(XAxis, YAxis);	// bakcground egg
	std::unordered_map<std::string, std::pair<int, int>>CityEgg;
	std::unordered_map<std::string, Passenger_Egg> PassengerEgg;
	std::list<ArrInfo> ArrPs;
}

using namespace Driver_impl;
const double PI = 3.1415926;
void Init(const char* file_name)
{
	try
	{
		// Input data
		FileStreamInput(file_name, g_plan, g_client, g_time, g_city);

		//add for test

		// Init file_out stream
		g_file_out.open("log.txt", std::ios::out);
		if (!g_file_out.is_open())
			throw std::runtime_error("Fail to open the output file!");
		GraphicInit();
		// Output the initial state
		StreamOutput(std::cout, g_plan, g_client, g_time);
		StreamOutput(g_file_out, g_plan, g_client, g_time);
		
	}
	catch (std::exception &e)
	{
		std::cerr << "Init failed: " << e.what() << std::endl;
	}
}

void Output()
{
	// Log file output
	try
	{
		// To Console
		StreamOutput(std::cout, g_plan, g_client, g_time);
		// To File
		StreamOutput(g_file_out, g_plan, g_client, g_time);
		// To Graphic
		if (!g_window.IsClosed())
		GraphicOutPut();
	}
	catch (std::exception &e)
	{
		std::cerr << "Output failed: " << e.what() << std::endl;
	}

	// Todo: Graphics Output using EggAche
}

void Update()
{
	// Push forward time
	++g_time;

	// Move the passenger
	for (auto p = g_client.begin(); p != g_client.end();)
	{
		//p->second.MoveForward();
		if (!p->second.MoveForward())
		{
			std::cout << (*p).first << " has arrived at the destination " << (*p).second.GetDest() << "." << std::endl;
			if ((p = g_client.erase(p)) != g_client.end())
				++p;
		}
		else
			++p;
	}
}

void fnTimeTick()
{
	static const int cDue = 1;

	auto tStart = std::chrono::steady_clock::now();
	std::chrono::steady_clock::duration tDue = std::chrono::seconds{ cDue };

	while (!g_fExit)
	{
		if (g_mTime.try_lock_for(tDue))				//#Case1: Interrupted
		{
			tDue -= std::chrono::steady_clock::now() - tStart;

			{
				std::lock_guard<std::mutex> {g_mHeat};	// Wait for heat-up
			}

			g_mTime.unlock();							// Finish the interruption

			{	// Make sure heat up and retrieve happen before fnTimeTick continue
				std::lock_guard<std::mutex> {g_mRW};
			}
		}
		else											//#Case2: Tick
		{
			std::lock_guard<std::mutex> {g_mRW};

			Update();
			Output();

			tDue = std::chrono::seconds{ cDue };			// Reset the timer
			// Implicit ~g_mRW.unlock ();
		}

		tStart = std::chrono::steady_clock::now();		// Update the start time point
	}
}

void fnInput()
{
	// Todo: make sure this happen before the fnTimeTick
	// A possible solution: begin fnTimeTick after user input ...
	g_mTime.lock();

	char buff[100];
	std::string cmd;
	bool fPause = false;

	static const std::string sPause{ "pause" };
	static const std::string sResume{ "resume" };
	static const std::string sQuery{ "query" };
	static const std::string sMod{ "client" };

	std::unordered_map<std::string, std::tuple<Passenger::Strategy,
		std::string, std::string, Passenger::RouteInfoSet, unsigned int >> tPassenger;

	while (!g_fExit)
	{
		// Todo: Get input from mouse

		std::cin.getline(buff, 100);
		std::istrstream istrs{ buff };
		istrs >> cmd;

		if (!fPause && cmd == sPause)
		{
			g_mHeat.lock();
			g_mTime.unlock();				// Interrupt fnTimeTick
			fPause = true;
			std::cout << "Tips of command lines:" << std::endl;
			std::cout << " query <name> : return the situation of the passenger." << std::endl
				<< " client -p <name> -<p|t|pt> <current city> <destination> : add a passenger." << std::endl
				<< " client -r <name> : delete a passenger." << std::endl
				<< " client -d <name> <destination> : set the destination." << std::endl
				<< " client -s <name> -<p|t|pt> : set the strategy." << std::endl
				<< " client -v <name> <city> <stay time> : set a via city." << std::endl;
			std::cout << std::endl;
		}

		else if (fPause && cmd == sResume)
		{
			// Make sure heat up and retrieve happen before fnTimeTick continue
			std::lock_guard<std::mutex> {g_mRW};

			//add by xuzhu
			for (auto &i : tPassenger)
			{
				g_client.emplace(std::make_pair(std::move(i.first),
					Passenger{ std::move(std::get<0>(i.second)),
					std::move(std::get<1>(i.second)),
					std::move(std::get<2>(i.second)),
					std::move(std::get<3>(i.second)),
					std::move(std::get<4>(i.second)),
					g_plan, g_time }));
			}

			g_mHeat.unlock();				// Heat up fnTimeTick
			g_mTime.lock();				// Retrieve TimedLock
			fPause = false;
		}

		else if (cmd == sQuery)
		{
			std::lock_guard<std::mutex> {g_mRW};

			istrs >> cmd;
			if (istrs.fail())
				goto tag_ErrInput;

			try
			{
				auto obj = g_client.at(cmd);

				std::cout << "At: " << obj.GetPostion().first << " "
					<< obj.GetPostion().second << std::endl;

				std::cout << "Via: " << std::endl;
				if (!obj.GetViaCity().size()) std::cout << "No via city." << std::endl;
				for (auto &i : obj.GetViaCity())
				{
					std::cout << "\t" << i.city << "\t" << i.tStay << "\t" << i.tArr << std::endl;
				}

				std::cout << "Dest: " << obj.GetDest() << std::endl;
			}
			catch (std::exception)
			{
				std::cerr << std::string("No such Passenger.\n");
			}

			// Implicit ~g_mRW.unlock ();
		}

		else if (fPause && cmd == sMod)
		{
			std::lock_guard<std::mutex> {g_mRW};

			istrs >> cmd;
			if (cmd == "-p")					// Add passenger -p name p|t|pt curr_city dest_city
			{
				std::string name, stg, curr_city, dest_city;
				unsigned int limit = 0;
				istrs >> name >> stg >> curr_city >> dest_city;
				if (istrs.fail())
					goto tag_ErrInput;

				try
				{
					g_client.at(name);		// throw if no such guy
					std::cerr << "The passenger already exists.\n";
				}
				catch (std::exception)
				{
					Passenger::Strategy stgg;

					if (stg == "-p")
						stgg = Passenger::Strategy::MinPrice;
					else if (stg == "-t")
						stgg = Passenger::Strategy::MinTime;
					else if (stg == "-pt") {
						stgg = Passenger::Strategy::MinPriceInTime;
						istrs >> limit;
					}
					else
						goto tag_ErrInput;

					/*g_client.emplace(std::make_pair(name,
						Passenger(stgg, curr_city, dest_city,
							Passenger::RouteInfoSet{}, limit, g_plan, g_time)));*/
					//add by xuzhu
					tPassenger.emplace(std::make_pair(std::move(name),
						std::make_tuple(std::move(stgg),
							std::move(curr_city),
							std::move(dest_city),
							Passenger::RouteInfoSet{},
							std::move(limit))));
				}
			}
			else if (cmd == "-r")			// Remove passenger -r name
			{
				std::string name;
				istrs >> name;
				if (istrs.fail())
					goto tag_ErrInput;

				try
				{
					g_client.at(name);
					g_client.erase(name);
					tPassenger.at(name);
					tPassenger.erase(name);
				}
				catch (std::exception)
				{
					std::cerr << "No such Passenger.\n";
				}
			}
			else if (cmd == "-d")			// Set Destination -d name city
			{
				std::string name, city;
				istrs >> name >> city;
				if (istrs.fail())
					goto tag_ErrInput;

				try
				{
					auto &obj = g_client.at(name);
					obj.SetDest(city);
				}
				catch (std::exception)
				{
					std::cerr << "No such Passenger.\n";
				}
			}
			else if (cmd == "-s")			// Set Strategy -s name p|t|pt
			{
				std::string name, stg;
				istrs >> name >> stg;
				if (istrs.fail())
					goto tag_ErrInput;

				try
				{
					auto &obj = g_client.at(name);
					if (stg == "-p")
						obj.SetStrategy(Passenger::Strategy::MinPrice);
					else if (stg == "-t")
						obj.SetStrategy(Passenger::Strategy::MinTime);
					else if (stg == "-pt")
						obj.SetStrategy(Passenger::Strategy::MinPriceInTime);
					else
						goto tag_ErrInput;
				}
				catch (std::exception)
				{
					std::cerr << "No such Passenger.\n";
				}
			}
			else if (cmd == "-v")			// Set ViaCity -v name city tStay
			{
				std::string name, city;
				unsigned int tStay;
				istrs >> name >> city >> tStay;
				if (istrs.fail())
					goto tag_ErrInput;

				try
				{
					std::get<3>(tPassenger.at(name)).emplace(Passenger::RouteInfo{ city, tStay, 0 });
				}
				catch (std::exception)
				{
					std::cerr << "No such Passenger.\n";
				}
			}
			else
			{
				goto tag_ErrInput;
			}

			// Implicit ~g_mRW.unlock ();
		}

		else
		{
		tag_ErrInput:
			std::cerr << "Bad Input\n";
		}
	}
}
void GraphicInit() {
	auto Cities = g_plan.GetCitys();
	std::list<std::string> ConjCities;
	std::list<Trsp> Trspor;
	int NumCities = Cities.size();

	background.DrawBmp("green2.bmp", 0, 0, XAxis, YAxis, 203, 233, 207);
	g_window.AddEgg(background);
	int x = 0, y = 0, i = 0;
	
	for (auto j : Cities) {//Draw the cities;


		x = static_cast<int>(XAxis/2 + 0.4*XAxis * cos(2.0*PI*i / NumCities));
		y = static_cast<int>(YAxis/2 + 0.4*YAxis * sin(2.0*PI*i / NumCities));

		std::string city_name = j + ".bmp";
		int c = background.DrawBmp(city_name.c_str(), x, y, 140, 110, 255, 255, 255);
		background.DrawTxt(x, y, j.c_str());
		if (x == XAxis/2 && y <= YAxis/2)
			y += 0.03*YAxis;
		else if (x == XAxis/2 && y > YAxis/2)
			y -= 0.03*YAxis;
		else if (x < XAxis/2 && y == YAxis/2)
			x += 0.03*XAxis;
		else if (x > XAxis/2 && y == YAxis/2)
			x -= 0.02*XAxis;
		else if ( x < XAxis/2 && y <= YAxis/2) {
			x += 0.1*XAxis;
			y += 0.03*YAxis;
		}
		else if ( x < XAxis/2 && y > YAxis/2) {
			x += 0.1*XAxis;
			//y -= 20;
		}
		else if ( x > XAxis/2 && y <= YAxis/2) {
			x -= 0.03*XAxis;
			y += 0.03*YAxis;
		}
		else {  //( x > XAxis/2 && y > YAxis/2)
			;
		}

		CityEgg[j] = std::make_pair(x, y);
		++i;
	}
	g_window.Refresh();

	for (auto j : Cities) {//Draw the roads between citites
		ConjCities = g_plan.GetConj(j);
		for (auto i : ConjCities) {
			Trspor = g_plan.GetTrsp(j, i);
			for (auto k : Trspor) {
				background.SetPen(5, 0, 0, 255);
				background.DrawLine(CityEgg[j].first, CityEgg[j].second, CityEgg[i].first, CityEgg[i].second);
				g_window.Refresh();
			}

		}

	}
	for (auto k : g_client)
	{	
		EggAche::Egg temp(60, 60, 0, 0);
		temp.DrawTxt(20, 20, k.first.c_str());
		struct Passenger_Egg _PE {temp,0,0};	
		PassengerEgg.emplace(k.first , _PE);
		//PassengerEgg[k.first].DrawTxt(20, 20, k.first.c_str());
		g_window.AddEgg(PassengerEgg.at(k.first).Egg);
	}
}

void GraphicOutPut()
{		
	const int Time_Slot = 20;
	int count = 0;
	while (count < Time_Slot) {//时间片到底怎么整
		auto t_Start = std::chrono::steady_clock::now();
		while (std::chrono::steady_clock::now() < t_Start + std::chrono::milliseconds(static_cast<int>(1000/ (Time_Slot)*(0.8))))
			;
				count++;
		for (auto k : g_client)
		{	
			if (!PassengerEgg.count(k.first)) {
				EggAche::Egg temp(100, 100, 0, 0);
				temp.DrawTxt(20, 20, k.first.c_str());
				struct Passenger_Egg tempEgg {temp,0,0};
				PassengerEgg.emplace(std::make_pair(k.first, tempEgg));
				g_window.AddEgg(PassengerEgg.at(k.first).Egg);
			}
			

			if (k.second.Is_Stay()) {
				std::string temp;
				temp = k.second.GetPostion().first;
				int  _x = CityEgg.at(temp).first;
				int  _y = CityEgg.at(temp).second;
				int _count = 1;//To count the number of passengers in the city
				for (auto j = g_client.begin();(*j).first != k.first;++j) {
					if (((*j).second.GetPostion().first) == temp)
						_count++;
				}
				int _temp;
				if (_x < XAxis/2)
					_temp = -0.06*XAxis;
				else
					_temp = 0.02*XAxis;
				PassengerEgg.at(k.first).Egg.Clear();
				PassengerEgg.at(k.first).Egg.DrawTxt(20, 20, k.first.c_str());
				PassengerEgg.at(k.first).Egg.MoveTo(_x + _temp , _y + _count * 10);
				PassengerEgg.at(k.first).x = _x;
				PassengerEgg.at(k.first).y = _y;
				g_window.Refresh();
			}
			else
			{
				int t_total = k.second.GetTotalTime();
				int t_left = k.second.GetCountdown();
				auto temp = k.second.GetPostion();
				double ratio = 0;//the ratio of the road the passenger has travelled.
				if (t_total != 0)
					ratio = (t_total - t_left)*1.0 / (t_total);
				else
					ratio = 0;
				int _xfrom = CityEgg.at(temp.first).first;
				int _yfrom = CityEgg.at(temp.first).second;
				int _xto = CityEgg.at(temp.second).first;
				int _yto = CityEgg.at(temp.second).second;

				//if (k.second.GetDest() == temp.second) {
				//	ArrInfo temp_Info{ k.first, _xto,_yto };
				//	ArrPs.emplace_back(temp_Info);
				//}
				//else
				//	for (auto i = ArrPs.begin();i != ArrPs.end();++i) {
				//		if ((*i).name == k.first)
				//			ArrPs.erase(i);
				//	}
				////PassengerEgg.at(k.first).DrawTxt(20, 20, k.first.c_str());
				//////////////////////count/16 
				int _x = PassengerEgg.at(k.first).x;
				int _y = PassengerEgg.at(k.first).y;
				ratio = 1 / (t_total*1.0);
				PassengerEgg.at(k.first).Egg.Clear();
				PassengerEgg.at(k.first).Egg.DrawTxt(20, 20, k.first.c_str());
				switch (k.second.GetFrontTrsp()) {
				case Trsp::_type::Car:
					PassengerEgg.at(k.first).Egg.DrawBmp("Car.bmp", 20, 35, 70, 25, -1, -1, -1);
					break;
				case Trsp::_type::Plane:
					PassengerEgg.at(k.first).Egg.DrawBmp("Plane.bmp", 20, 35, 55, 25, -1, -1, -1);
					break;
				case Trsp::_type::Train:
					PassengerEgg.at(k.first).Egg.DrawBmp("Train.bmp", 20, 35, 50, 25, -1, -1, -1);
					break;
				}
				PassengerEgg.at(k.first).Egg.MoveTo(static_cast<int>(_x + (_xto - _xfrom)*ratio*1/ Time_Slot), static_cast<int>(_y + (_yto - _yfrom)*ratio*1/ Time_Slot));
				PassengerEgg.at(k.first).x = static_cast<int>(_x + (_xto - _xfrom)*ratio*1 / Time_Slot);
				PassengerEgg.at(k.first).y = static_cast<int>(_y + (_yto - _yfrom)*ratio*1 / Time_Slot);
 				g_window.Refresh();
			}
		}
		//for (auto i : ArrPs) {
		//	int _count = 1, _x, _y;
		//	_y = i.y;
		//	if (i.x <= XAxis/2 && i.y <= YAxis/2)
		//		_x = i.x - 80;
		//	else if (i.x <= XAxis/2 && i.y > YAxis/2)
		//		_x = i.x - 80;
		//	else if (i.x > XAxis/2 && i.y <= YAxis/2)
		//		_x = i.x + 80;
		//	else
		//		_x = i.x + 80;
		//	PassengerEgg.at(i.name).Egg.Clear();
		//	PassengerEgg.at(i.name).Egg.DrawTxt(20, 20, i.name.c_str());
		//	g_window.AddEgg(PassengerEgg.at(i.name).Egg);
		//	PassengerEgg.at(i.name).Egg.MoveTo(_x, _y + _count * 10);
		//}
	}

	//Todo :另开一个CPP把我写的东西弄进去，然后让Driver 调用
}
