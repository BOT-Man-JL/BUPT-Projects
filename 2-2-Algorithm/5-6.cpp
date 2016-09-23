
// Chapter 5-6 Assignment Project
// By BOT Man JL.

#include <iostream>
#include <fstream>
#include <strstream>
#include <vector>
#include <queue>
#include <string>
#include <ctime>

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

	inline T& operator () (size_t a, size_t b)
	{
		if (a >= m || b >= n)
			throw std::out_of_range ("Matrix index out of range");
		return buf[a * n + b];
	}

	inline const T& operator () (size_t a, size_t b) const
	{
		if (a >= m || b >= n)
			throw std::out_of_range ("Matrix index out of range");
		return buf[a * n + b];
	}
};

namespace Coloring_Impl
{
	class Color
	{
		const Matrix<bool>& arr;	// Graph
		unsigned n;		// number of vertices
		unsigned m;		// number of colors

		std::vector<unsigned> sln;		// Solution
		unsigned long long count;		// number of slns
		bool is_feasible_4_m;

		bool IsFeasible (unsigned k) const
		{
			for (unsigned j = 0; j < k; ++j)
				if (arr (k, j) && sln[j] == sln[k])
					return false;
			return true;
		}

	public:
		Color (unsigned _n, unsigned _m, const Matrix<bool>& _arr)
			: n (_n), m (_m), arr (_arr), sln (_n, 0), count (0), is_feasible_4_m (false)
		{}

		unsigned long long GetCount () const
		{
			return count;
		}

		bool IsFeasible () const
		{
			return is_feasible_4_m;
		}

		std::vector<unsigned> GetSln () const
		{
			return sln;
		}

		void SetColor (unsigned _m)
		{
			m = _m;
			is_feasible_4_m = false;
		}

		void BackTrack (unsigned t)
		{
			if (t >= n)
			{
				is_feasible_4_m = true;
				return;
			}

			++count;
			for (unsigned i = 1; i <= m; ++i)
			{
				sln[t] = i;

				if (IsFeasible (t))
					BackTrack (t + 1);

				if (is_feasible_4_m)
					return;

				//sln[t] = 0;
			}
		}
	};
}

void coloring_scheme (const std::string &str_prefix)
{
	using namespace Coloring_Impl;

	std::ifstream ifs (str_prefix + " basestations.txt");
	std::ofstream ofs ("Coloring " + str_prefix + ".txt");

	if (!ifs.is_open () || !ofs.is_open ())
	{
		std::cerr << str_prefix + " open files failed.\n";
		return;
	}

	// #0 Read Graph

	unsigned _n = 0;
	int tmp;

	// Get Counts
	{
		char line[1024];

		ifs.getline (line, 1024);

		std::istrstream istrs (line);
		while (istrs >> tmp)
			++_n;

		// Dummy Row
		ifs.getline (line, 1024);
	}

	double tmp2;
	Matrix<bool> _arr (_n, _n);

	for (unsigned i = 0; i < _n; ++i)
	{
		// Two Dummy Rows
		ifs >> tmp;
		ifs >> tmp;

		for (unsigned j = 0; j < _n; ++j)
		{
			ifs >> tmp2;
			_arr (i, j) = (tmp2 != 99999);
		}
	}

	// #1 Find Solution

	unsigned m = _n;
	std::vector<unsigned> sln;
	unsigned long long time = 0, count = 0;

	while (true)
	{
		std::cout << m << '\n';
		Color color (_n, m, _arr);

#ifdef QUICK_M
		if (m <= 4)
			break;
#endif

		auto tt = clock ();
		color.BackTrack (0);
		tt = clock () - tt;

		if (color.IsFeasible ())
		{
			time += tt;
			count += color.GetCount ();
			sln = color.GetSln ();

			--m;
		}
		else
			break;
	}

	// #2 Output

	ofs << "m: " << m + 1 << " L: " << count << " T: " << time / 1000.0 << "\nSolution:\n";

	for (const auto &x : sln)
		ofs << x << ' ';
	ofs << std::endl;
}

namespace TSP_Impl
{
	// Remark: Use < or > to walkaround cmp double issue;
	const double iNoEdge = 99999;

	struct Node
	{
		double lower_bound;
		double curr_cost;
		double residual_cost;

		unsigned index;
		std::vector<unsigned> sln;

		Node (double lb, double cc, double rc,
			  unsigned ci, std::vector<unsigned> &&_sln)
			: lower_bound (lb), curr_cost (cc), residual_cost (rc),
			index (ci), sln (_sln)
		{}
	};

	bool operator < (const Node &a, const Node &b)
	{
		return a.lower_bound < b.lower_bound;
	}

	class Tsp
	{
		const Matrix<double>& arr;
		unsigned n;

		std::vector<unsigned> curr_sln, best_sln;
		double curr_cost, best_cost;
		unsigned long long count;

	public:
		Tsp (unsigned _n, const Matrix<double>& _arr)
			: n (_n), arr (_arr), count (0),
			curr_cost (0), best_cost (DBL_MAX)
		{
			for (auto i = 0; i <= n; ++i)
			{
				curr_sln.push_back (i);
				best_sln.push_back (i);
			}
		}

		std::vector<unsigned> GetSln () const
		{
			return best_sln;
		}

		std::pair<double, unsigned long long>
			GetBCnC () const
		{
			return std::make_pair (best_cost, count);
		}


		void BranchNBound ()
		{
			auto lower = 0;
			auto upper = 0;

			unsigned index = 1;
			std::vector<bool> marked (n + 1, false);
			for (auto i = 1; i <= n; ++i)
			{
				auto next_index = index;
				auto cost = DBL_MAX;
				for (auto j = 1; j <= n; ++j)
					if (arr (index, j) < iNoEdge &&
						arr (index, j) < cost &&
						!marked[j])
						{
							next_index = j;
							cost = arr (index, j);
						}
				index = next_index;
				marked[next_index] = true;
				upper += cost;
			}

			for (auto i = 1; i <= n; ++i)
			{
				double min_cost = DBL_MAX;
				double sub_min_cost = DBL_MAX;

				//for (auto j = 1; j <= n; ++j)
				//	//Calc Min and SubMin;

				lower += min_cost;
				lower += sub_min_cost;
			}
			lower /= 2;

			std::vector<double> min_out (n + 1);
			double min_sum = 0;

			for (auto i = 1; i <= n; ++i)
			{
				double min_cost = DBL_MAX;

				for (auto j = 1; j <= n; ++j)
					if (arr (i, j) < iNoEdge &&
						arr (i, j) < min_cost)
						min_cost = arr (i, j);

				min_out[i] = min_cost;
				min_sum += min_cost;
			}

			std::priority_queue<Node> heap;
			auto ssln = std::vector<unsigned> (n + 1);
			for (auto i = 1; i <= n; ++i)
				ssln[i] = i;
			Node e = Node (0, 0, min_sum, 1, std::move (ssln));

			// Not a leaf
			while (e.index <= n - 1)
			{
				// Father of a leaf
				if (e.index == n - 1)
				{
					auto e1 = arr (e.sln[n - 1], e.sln[n]);
					auto e2 = arr (e.sln[n], e.sln[1]);
					auto alt = e.curr_cost + e1 + e2;
					if (e1 < iNoEdge && e2 < iNoEdge && alt < best_cost)
					{
						best_sln = e.sln;
						best_cost = alt;
					}
				}
				else
				{
					// Expand e
					for (auto i = e.index + 1; i <= n; ++i)
						if (arr (e.sln[e.index], e.sln[i]) < iNoEdge)
						{
							auto cost = e.curr_cost + ii;
							auto residual_cost = e.residual_cost - min_out[e.sln[e.index]];
							auto lower_bound = cost + residual_cost;
							if (lower_bound >= best_cost)
								continue;

							auto sln = e.sln;
							std::swap (sln[e.index + 1], sln[i]);

							heap.emplace (Node (lower_bound, cost, residual_cost,
												e.index + 1, std::move (sln)));
						}
				}
				// Get new e
				e = heap.top ();
				heap.pop ();
			}
		}

		// Call BackTrack (2)
		void BackTrack (unsigned i)
		{
			if (i == n)
			{
				auto e1 = arr (curr_sln[n - 1], curr_sln[n]);
				auto e2 = arr (curr_sln[n], curr_sln[1]);
				auto alt = curr_cost + e1 + e2;
				if (e1 < iNoEdge && e2 < iNoEdge && alt < best_cost)
				{
					best_sln = curr_sln;
					best_cost = alt;
				}
				return;
			}

			++count;
			for (auto j = i; j <= n; ++j)
			{
				auto ii = arr (curr_sln[i - 1], curr_sln[j]);
				if (ii < iNoEdge &&
					curr_cost + ii < best_cost)
				{
					std::swap (curr_sln[i], curr_sln[j]);
					curr_cost += ii;
					BackTrack (i + 1);
					curr_cost -= ii;
					std::swap (curr_sln[i], curr_sln[j]);
				}
			}
		}
	};
}

void tsp_scheme (std::string str_prefix, unsigned from_index, bool is_backtrack)
{
	using namespace TSP_Impl;

	std::ifstream ifs (str_prefix + " basestations.txt");
	std::ofstream ofs ("Tsp " + ((is_backtrack ? "BT " : "BB ") + str_prefix + ".txt"));

	if (!ifs.is_open () || !ofs.is_open ())
	{
		std::cerr << str_prefix + " open files failed.\n";
		return;
	}

	// #0 Read Graph

	unsigned _n = 0;
	int tmp;

	// Get Counts
	{
		char line[1024];

		ifs.getline (line, 1024);

		std::istrstream istrs (line);
		while (istrs >> tmp)
			++_n;

		// Dummy Row
		ifs.getline (line, 1024);
	}

	double tmp2;
	Matrix<double> _arr (_n + 1, _n + 1);

	for (unsigned i = 1; i <= _n; ++i)
	{
		// Two Dummy Rows
		ifs >> tmp;
		ifs >> tmp;

		for (unsigned j = 1; j <= _n; ++j)
		{
			ifs >> tmp2;
			_arr (i, j) = tmp2;
		}
	}

	//for (int i = 1; i <= _n; ++i)
	//{
	//	for (int j = 1; j <= _n; ++j)
	//		std::cout << _arr (i, j) << "\t";
	//	std::cout << std::endl;
	//}

	// #1 Calc

	Tsp tsp (_n, _arr);
	if (is_backtrack)
		tsp.BackTrack (2);
	else
		tsp.BranchNBound ();

	// #2 Output

	auto result = tsp.GetBCnC ();
	ofs << "Best Cost: " << result.first
		<< " Calc Count: " << result.second
		<< "\nPath:\n";
	auto path = tsp.GetSln ();
	for (auto i = 0; i <= _n; ++i)
	{
		if (path[i] == from_index)
		{
			for (auto offset = 0; offset <= _n + 1; ++offset)
			{
				if ((i + offset) % (_n + 1))
					ofs << path[(i + offset) % (_n + 1)] << ' ';
			}
			break;
		}
	}
}

int main ()
{
	coloring_scheme ("22");
	coloring_scheme ("42");

	tsp_scheme ("15", 1, true);
	tsp_scheme ("20", 1, true);

	tsp_scheme ("15", 1, false);
	tsp_scheme ("20", 1, false);

	return 0;
}
