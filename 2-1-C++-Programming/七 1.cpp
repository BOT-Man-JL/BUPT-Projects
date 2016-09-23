#include <unordered_map>
#include <string>
#include <fstream>
#include <algorithm>
//#include <iostream>

namespace My_Phone_Book {  //namespace My_Phone_Book ->

class Phone_Book
{
	std::unordered_map<std::string, int> _map;
public:
	int Search (const std::string &s) const;
	void Modify (const std::string &s, int num) { _map[s] = num; }
	bool Delete (const std::string &s) { return !! (_map.erase (s)); }
	bool Save (const std::string &sFilename) const;
	bool Load (const std::string &sFilename);
};

int Phone_Book::Search (const std::string &s) const
{
	auto p = _map.find (s);
	if (p == _map.cend ()) return -1;
	else return (*p).second;
}

bool Phone_Book::Save (const std::string &sFilename) const
{
	std::ofstream _of (sFilename);
	if (!_of.is_open ()) return false;
	std::for_each (_map.cbegin (), _map.cend (), [&] (const std::pair<std::string, int> &pa) { _of << pa.first << " " << pa.second << std::endl; });
	return true;
}

bool Phone_Book::Load (const std::string &sFilename)
{
	std::ifstream _if (sFilename);
	std::string s;
	int num;
	if (!_if.is_open ()) return false;
	while (_if >> s >> num) Modify (s, num);
	if (_if.good ()) return true;
	else return false;
}

}  //<- namespace My_Phone_Book

//int main ()
//{
//	using namespace My_Phone_Book;
//	Phone_Book pb, pbb;
//
//	pbb.Modify ("ljn", 123);
//	pbb.Modify ("zyj", 456);
//	pbb.Modify ("zl", 78);
//	pbb.Save ("Phone Book.txt");
//
//	pb.Load ("Phone Book.txt");
//	std::cout << pb.Search ("ljn") << std::endl << pb.Search ("haha") << std::endl;
//	pb.Delete ("ljn");
//	std::cout << pb.Search ("ljn") << std::endl << pb.Search ("zyj") << std::endl;
//
//	return 0;
//}
