
#pragma once

#include "DataStructure.h"
#include <iostream>

bool FileStreamInput (const char* file_name,
					  Plan &plan,
					  std::unordered_map<std::string, Passenger> &g_client,
					  unsigned int &g_time, std::list<std::string> &g_city);

void StreamOutput (std::ostream &ostr,
				   const Plan &plan,
				   const std::unordered_map<std::string, Passenger> &g_client,
				   unsigned int g_time);