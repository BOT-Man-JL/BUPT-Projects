
// Chapter 3 Assignment Project
// By BOT Man JL.

#include <string>
#include <vector>
#include <tuple>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <strstream>
#include <exception>

// Open-source EggAche Graphical Library is available on GitHub
// https://github.com/BOT-Man-JL/EggAche-GL
#include "EggAche.h"

template<typename T> class Matrix
{
private:
	T* buf;

public:
	size_t m, n;

	Matrix (size_t _m, size_t _n)
		: m (_m), n (_n), buf (new T[_m * _n])
	{}
	Matrix (const Matrix &) = delete;

	~Matrix ()
	{
		delete[] buf;
	}

	T& operator () (size_t a, size_t b)
	{
		if (a >= m || b >= n)
			throw std::out_of_range ("Matrix index out of range");
		return buf[a * n + b];
	}
};

namespace LCS_impl
{
	class LCS
	{
	private:
		enum dir
		{
			up, left, upleft
		};

		Matrix<unsigned int> c;
		Matrix<dir> d;
		std::string x, y;

		void _print (std::ostream &os, size_t i, size_t j)
		{
			if (i == 0 || j == 0)
				return;
			switch (d (i, j))
			{
			case upleft:
				_print (os, i - 1, j - 1);
				os << x[i];
				break;

			case up:
				_print (os, i - 1, j);
				break;

			case left:
				_print (os, i, j - 1);
				break;

			default:
				throw std::runtime_error ("unknown error");
			}
		}

	public:
		LCS (const std::string &a, const std::string &b)
			: x (a), y (b),
			c (a.size (), b.size ()),
			d (a.size (), b.size ())
		{
			for (auto i = 0; i < a.size (); ++i)
				c (i, 0) = 0;
			for (auto j = 0; j < b.size (); ++j)
				c (0, j) = 0;

			for (auto i = 1; i < a.size (); ++i)
				for (auto j = 1; j < b.size (); ++j)
					if (a[i] == b[j])
					{
						c (i, j) = c (i - 1, j - 1) + 1;
						d (i, j) = upleft;
					}
					else if (c (i, j - 1) >= c (i - 1, j))
					{
						c (i, j) = c (i, j - 1);
						d (i, j) = left;
					}
					else // c (i - 1, j) > c (i, j - 1)
					{
						c (i, j) = c (i - 1, j);
						d (i, j) = up;
					}
		}

		void print (std::ostream &os)
		{
			os << "The longest common subsequence, of length "
				<< (int) c (x.size () - 1, y.size () - 1) << ", is\n";
			_print (os, x.size () - 1, y.size () - 1);
			os << std::endl;
		}
	};

	std::ostream & operator << (std::ostream &os, LCS &lcs)
	{
		lcs.print (os);
		return os;
	}
}

void lcs_scheme ()
{
	using namespace LCS_impl;

	std::ifstream ifs ("longest_common_subsequence_input.txt");
	std::ofstream ofs ("longest_common_subsequence_output.txt");

	if (!ifs.is_open () || !ofs.is_open ())
		std::cerr << "open files failed.\n";

	std::string str[4];
	int index = -1;
	char buf[1000];

	while (ifs.getline (buf, 1000))
	{
		if (buf[0] >= 'A' && buf[0] <= 'D')
			index = buf[0] - 'A';
		else
			str[index] += buf;
	}

	ofs << "LCS of a, b:\n" << LCS (str[0], str[1]) << std::endl
		<< "LCS of c, d:\n" << LCS (str[2], str[3]) << std::endl
		<< "LCS of a, d:\n" << LCS (str[0], str[3]) << std::endl
		<< "LCS of c, b:\n" << LCS (str[2], str[1]) << std::endl;
}

namespace MSS_impl
{
	std::tuple<int, int, int> maxsubsum (const std::vector<int> &a)
	{
		int maxSum = 0, thisSum = 0;
		size_t thisFrom = 0, thisTo = 0;
		size_t maxFrom = 0, maxTo = 0;

		for (int j = 0; j < a.size (); ++j)
		{
			thisSum += a[j];
			thisTo = j;

			if (thisSum > maxSum)
			{
				maxSum = thisSum;
				maxFrom = thisFrom;
				maxTo = thisTo;
			}
			else if (thisSum < 0)
			{
				thisSum = 0;
				thisFrom = j + 1;
			}
		}

		return std::make_tuple (maxSum, maxFrom, maxTo);
	}
}

void mss_scheme ()
{
	using namespace MSS_impl;

	std::ofstream ofs ("maximum_subsequence_sum_output.txt");

	// file 1
	{
		std::ifstream ifs ("maximum_subsequence_sum_1.txt");

		if (!ifs.is_open () || !ofs.is_open ())
			std::cerr << "open files failed.\n";

		int t;
		std::vector<int> a;
		while (ifs >> t)
			a.push_back (t);

		auto result = maxsubsum (a);
		ofs << "maximum_subsequence_sum_1 is " << std::get<0> (result)
			<< "\n0-based indexs start from " << std::get<1> (result)
			<< " to " << std::get<2> (result) << std::endl;
	}

	// file 2
	{
		std::ifstream ifs ("maximum_subsequence_sum_2.txt");

		if (!ifs.is_open () || !ofs.is_open ())
			std::cerr << "open files failed.\n";

		int t;
		std::vector<int> a;
		while (ifs >> t)
			a.push_back (t);

		auto result = maxsubsum (a);
		ofs << "maximum_subsequence_sum_2 is " << std::get<0> (result)
			<< "\n0-based indexs start from " << std::get<1> (result)
			<< " to " << std::get<2> (result) << std::endl;
	}
}

namespace MWT_impl
{
	struct BS
	{
		int id;					// ENODEBID
		long double lot, lat;	// LONGITUDE, LATITUDE
	};
	using Lbs = std::vector<BS>;

	const double PI = 3.14159;
	const double EARTH_RADIUS = 6378.137;

	inline long double rad (long double LatOrLon)
	{
		return LatOrLon * PI / 180.0;
	}

	inline long double GetDistance2 (long double lng1, long double lat1,
									 long double lng2, long double lat2)
	{
		long double  radLat1 = rad (lat1);
		long double  radLat2 = rad (lat2);
		long double  radlng1 = rad (lng1);
		long double  radlng2 = rad (lng2);

		long double  s = acos (cos (radLat1) * cos (radLat2)
							   * cos (radlng1 - radlng2)
							   + sin (radLat1) * sin (radLat2));
		return s * EARTH_RADIUS * 1000;
	}

	inline long double Weight (const BS &bs1, const BS &bs2, const BS &bs3)
	{
		return
			GetDistance2 (bs1.lot, bs1.lat, bs2.lot, bs2.lat) +
			GetDistance2 (bs2.lot, bs2.lat, bs3.lot, bs3.lat) +
			GetDistance2 (bs3.lot, bs3.lat, bs1.lot, bs1.lat);
	}

	class MWT
	{
	private:
		Matrix<long double> weight;
		Matrix<unsigned> source_point;
		Lbs _lbs;

		void printee (std::ostream &os, size_t i, size_t j,
					  std::string prefix, bool isTail)
		{
			auto k = source_point (i, j);

			// Node
			os << prefix + (isTail ? "└── " : "├── ")
				<< '(' << i << " - " << j << ')' << std::endl;

			// Left Child
			if (i + 1 != k)
				printee (os, i, k, prefix + (isTail ? "    " : "│   "), false);
			else
				os << prefix + (isTail ? "    " : "│   ") + "├── "
				<< '(' << i << " - " << k << ')' << std::endl;

			// Right Child
			if (k + 1 != j)
				printee (os, k, j, prefix + (isTail ? "    " : "│   "), true);
			else
				os << prefix + (isTail ? "    " : "│   ") + "└── "
				<< '(' << k << " - " << j << ')' << std::endl;
		}

		void drawey (EggAche::Egg &egg, size_t i, size_t j,
					 long double maxlot, long double maxlat,
					 long double minlot, long double minlat)
		{
			egg.DrawLine ((_lbs[i].lot - minlot) * 800 / (maxlot - minlot) + 50,
				(_lbs[i].lat - minlat) * 800 / (maxlat - minlat) + 50,
						  (_lbs[j].lot - minlot) * 800 / (maxlot - minlot) + 50,
						  (_lbs[j].lat - minlat) * 800 / (maxlat - minlat) + 50);

			char ch[5];

			sprintf (ch, "%d", i);
			egg.DrawTxt ((_lbs[i].lot - minlot) * 800 / (maxlot - minlot) + 50,
				(_lbs[i].lat - minlat) * 800 / (maxlat - minlat) + 50,
						 ch);
			sprintf (ch, "%d", j);
			egg.DrawTxt ((_lbs[j].lot - minlot) * 800 / (maxlot - minlot) + 50,
				(_lbs[j].lat - minlat) * 800 / (maxlat - minlat) + 50,
						 ch);
		}

		void drawee (EggAche::Egg &egg, size_t i, size_t j,
					 long double maxlot, long double maxlat,
					 long double minlot, long double minlat)
		{
			auto k = source_point (i, j);

			// Node
			drawey (egg, i, j, maxlot, maxlat, minlot, minlat);

			// Left Child
			if (i + 1 != k)
				drawee (egg, i, k, maxlot, maxlat, minlot, minlat);
			else
				drawey (egg, i, k, maxlot, maxlat, minlot, minlat);

			// Right Child
			if (k + 1 != j)
				drawee (egg, k, j, maxlot, maxlat, minlot, minlat);
			else
				drawey (egg, k, j, maxlot, maxlat, minlot, minlat);
		}

	public:
		MWT (const Lbs &lbs, bool is_optimal)
			: _lbs (lbs),
			weight (lbs.size (), lbs.size ()),
			source_point (lbs.size (), lbs.size ())
		{
			// r = 0 to (size - 1)
			for (int r = 1; r <= lbs.size () - 1; ++r)
				// There are (size - r) elem to calc
				// i = left of the interval
				for (int i = 0; i < lbs.size () - r; ++i)
				{
					// j = right of the interval
					int j = i + r;

					// r = 0 or 1
					if (r == 0 || r == 1)
					{
						weight (i, j) = 0;
						continue;
					}

					// r >= 2
					// k = (i + 1) to (j - 1)
					weight (i, j) = LDBL_MAX;
					if (is_optimal)
					{
						// #1 Optimal
						// Traverse to find the optimal solution
						for (int k = i + 1; k <= j - 1; k++)
						{
							auto u = weight (i, k) + weight (k, j) +
								Weight (lbs[i], lbs[j], lbs[k]);
							if (u < weight (i, j))
							{
								weight (i, j) = u;
								source_point (i, j) = k;
							}
						}
					}
					else
					{
						// #2 Heuristic
						// Add an edge to two adjacent edges
						// to make a triangle, iteratively
						auto k = (i + j) / 2;
						{
							auto u = weight (i, k) + weight (k, j) +
								Weight (lbs[i], lbs[j], lbs[k]);
							if (u < weight (i, j))
							{
								weight (i, j) = u;
								source_point (i, j) = k;
							}
						}
					}
				}
		}

		void print (std::ostream &os)
		{
			os << "Total Weight is: "
				<< weight (0, _lbs.size () - 1);

			os << "\nCorresponding Triangulation is:\n";
			//_print (os, 0, _lbs.size () - 1);
			printee (os, 0, _lbs.size () - 1, "", true);

			os << std::endl;
		}

		void draw ()
		{
			long double minlot = LDBL_MAX, minlat = LDBL_MAX,
				maxlot = 0, maxlat = 0;
			for (auto &i : _lbs)
			{
				maxlot = max (maxlot, i.lot);
				minlot = min (minlot, i.lot);
				maxlat = max (maxlat, i.lat);
				minlat = min (minlat, i.lat);
			}

			EggAche::Window wnd {900, 900};
			EggAche::Egg egg {900, 900};
			wnd.AddEgg (egg);

			drawee (egg, 0, _lbs.size () - 1,
					maxlot, maxlat, minlot, minlat);

			wnd.Refresh ();
		}
	};

	std::ostream & operator << (std::ostream &os, MWT &mwt)
	{
		mwt.print (os);
		mwt.draw ();
		return os;
	}
}

void mwt_scheme ()
{
	using namespace MWT_impl;

	std::ofstream ofs ("minimum_weight_triangulation_output.txt");

	// file 1
	{
		std::ifstream ifs ("21_basestations.txt");

		if (!ifs.is_open () || !ofs.is_open ())
			std::cerr << "open files failed.\n";

		char dummy_line[1000];
		ifs.getline (dummy_line, 1000);

		int id, dummy;
		long double lot, lat;
		Lbs lbs;
		while (ifs >> id >> lot >> lat >> dummy)
			lbs.push_back (BS {id, lot, lat});

		// Leave out the duplicated one
		lbs.pop_back ();

		ofs << "Optimal triangulation of 21_basestations is "
			<< MWT (lbs, true) << std::endl;
		ofs << "Heuristic triangulation of 21_basestations is "
			<< MWT (lbs, false) << std::endl;
	}

	// file 2
	{
		std::ifstream ifs ("29_basestations.txt");

		if (!ifs.is_open () || !ofs.is_open ())
			std::cerr << "open files failed.\n";

		char dummy_line[1000];
		ifs.getline (dummy_line, 1000);

		int id, dummy;
		long double lot, lat;
		Lbs lbs;
		while (ifs >> id >> lot >> lat >> dummy)
			lbs.push_back (BS {id, lot, lat});

		// Leave out the duplicated one
		lbs.pop_back ();

		ofs << "Optimal triangulation of 29_basestations is "
			<< MWT (lbs, true) << std::endl;
		ofs << "Heuristic triangulation of 29_basestations is "
			<< MWT (lbs, false) << std::endl;
	}
}

namespace Knapsack_impl
{
	struct Item
	{
		unsigned value, weight;
	};
	using Items = std::vector<Item>;

	class Knapsack
	{
	private:
		Matrix<unsigned> m;
		Matrix<unsigned> t;
		Items items;
		unsigned capacity;

	public:
		Knapsack (const Items &_items, unsigned _capacity)
			: m (_items.size () + 1, _capacity + 1),
			t (_items.size () + 1, _capacity + 1),
			items (_items), capacity (_capacity)
		{
			for (auto j = 0; j <= capacity; ++j)
				m (0, j) = 0;

			for (auto i = 0; i < items.size (); ++i)
				for (auto j = 0; j <= capacity; ++j)
					if (items[i].weight > j ||
						m (i, j) > m (i, j - items[i].weight) + items[i].value)
					{
						m (i + 1, j) = m (i, j);
						t (i + 1, j) = j;
					}
					else
					{
						m (i + 1, j) = m (i, j - items[i].weight) + items[i].value;
						t (i + 1, j) = j - items[i].weight;
					}
		}

		void print (std::ostream &os)
		{
			os << "The optimal total value is: "
				<< m (items.size (), capacity) << std::endl;

			auto j = capacity;
			for (auto i = items.size (); i > 0; --i)
				if (t (i, j) != j)
				{
					os << '(' << items[i - 1].value << ','
						<< items[i - 1].weight << ")\t";
					j = t (i, j);
				}
			os << std::endl;
		}
	};

	std::ostream & operator << (std::ostream &os, Knapsack &knapsack)
	{
		knapsack.print (os);
		return os;
	}
}

void knapsack_scheme ()
{
	using namespace Knapsack_impl;

	// Wide Chars and Enpty Lines have been removed from Input
	std::ifstream ifs ("Knapsack_input.txt");
	std::ofstream ofs ("Knapsack_output.txt");

	if (!ifs.is_open () || !ofs.is_open ())
		std::cerr << "open files failed.\n";

	Items items;
	char buffer[1000];
	unsigned capacity;
	unsigned tmp;

	for (auto i = 0; i < 2; ++i)
	{
		// Clear Items
		items.clear ();

		// Get Capacity
		ifs >> capacity;
		ifs.getline (buffer, 1000);	// Eat '\n'

									// Get Weights
		ifs.getline (buffer, 1000);
		std::strstream istrs1 (buffer, 1000);

		std::vector<unsigned> tmp_vector;
		while (istrs1 >> tmp)
			tmp_vector.push_back (tmp);

		// Get Values
		ifs.getline (buffer, 1000);
		std::strstream istrs2 (buffer, 1000);

		unsigned index = 0;
		while (istrs2 >> tmp)
			items.push_back (Item {tmp, tmp_vector[index++]});

		ofs << "Solution of Group - " << i << std::endl
			<< Knapsack (items, capacity) << std::endl;
	}
}

int main ()
{
	lcs_scheme ();
	mss_scheme ();
	mwt_scheme ();
	knapsack_scheme ();

	return 0;
}
