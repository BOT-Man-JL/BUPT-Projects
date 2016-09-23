
// Chapter 4 Assignment Project
// By BOT Man JL.

#include <string>
#include <vector>
#include <list>
#include <map>
#include <queue>
#include <algorithm>
#include <iostream>
#include <strstream>
#include <fstream>
#include <exception>
#include <math.h>		// log ()

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
		std::ifstream ifs ("basestations_21.txt");

		if (!ifs.is_open () || !ofs.is_open ())
		{
			std::cerr << "open files failed. #1\n";
			return;
		}

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
		std::ifstream ifs ("basestations_29.txt");

		if (!ifs.is_open () || !ofs.is_open ())
		{
			std::cerr << "open files failed. #2\n";
			return;
		}

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

namespace Huffman_impl
{
	template <typename T>
	struct Tree
	{
		T val;
		Tree<T> *lc, *rc;
	};
	using HuffmanTree = Tree<std::pair<char, int>>;

	HuffmanTree GetHuffmanTree (const std::map<char, int> &dic)
	{
		struct pair_cmp
		{
			bool operator () (const HuffmanTree &a,
							  const HuffmanTree &b)
			{
				return a.val.second > b.val.second;
			}
		};

		std::priority_queue<
			HuffmanTree,
			std::vector<HuffmanTree>,
			pair_cmp
		> li;

		for (auto i : dic)
			li.emplace (
				HuffmanTree {std::move (i), nullptr, nullptr});

		while (li.size () > 1)
		{
			auto a = li.top ();
			li.pop ();
			auto b = li.top ();
			li.pop ();

			li.emplace (
				HuffmanTree {
				std::make_pair (0, a.val.second + b.val.second),
				new HuffmanTree (std::move (a)),
				new HuffmanTree (std::move (b))});
		}

		return std::move (li.top ());
	}

	void FreeHuffmanTree (HuffmanTree &tree)
	{
		// Remark: the root is in stack :-(
		//         unable to use destructor
		if (tree.lc)
		{
			if (tree.lc->lc)
				FreeHuffmanTree (*tree.lc->lc);
			if (tree.lc->rc)
				FreeHuffmanTree (*tree.lc->rc);
			delete tree.lc;
		}
		if (tree.rc)
		{
			if (tree.rc->lc)
				FreeHuffmanTree (*tree.rc->lc);
			if (tree.rc->rc)
				FreeHuffmanTree (*tree.rc->rc);
			delete tree.rc;
		}
	}

	void _GetHuffmanTable (const HuffmanTree &tree, const std::string &prefix,
						   std::map<char, std::string> &table)
	{
		if (tree.val.first)
			table[tree.val.first] = prefix;
		if (tree.lc)
			_GetHuffmanTable (*tree.lc, prefix + '0', table);
		if (tree.rc)
			_GetHuffmanTable (*tree.rc, prefix + '1', table);
	}

	std::map<char, std::string> GetHuffmanTable (const HuffmanTree &tree)
	{
		std::map<char, std::string> table;
		_GetHuffmanTable (tree, "", table);
		return std::move (table);
	}
}

void huffman_scheme ()
{
	using namespace Huffman_impl;

	std::ifstream ifs ("Huffman_Encoding.txt");

	if (!ifs.is_open ())
	{
		std::cerr << "open files failed. #1\n";
		return;
	}

	std::map<char, int> dic;
	char	ch;
	size_t	count = 0;

	while (ifs >> ch)
	{
		// Ignore Case
		if (ch >= 'A' && ch <= 'Z')
			ch -= 'A' - 'a';

		++dic[ch];
		++count;
	}
	ifs.close ();

	auto tree = GetHuffmanTree (dic);
	auto table = GetHuffmanTable (tree);
	FreeHuffmanTree (tree);

	std::ofstream ofs ("Huffman_Encoding_result.txt");
	ifs.open ("Huffman_Encoding.txt");

	if (!ifs.is_open () || !ofs.is_open ())
	{
		std::cerr << "open files failed. #2\n";
		return;
	}

	// #1 Output Dic
	ofs << "The Char Frequency is:\n";
	for (const auto &i : dic)
		ofs << i.first << '\t' << i.second << '\n';
	ofs << std::endl;

	// #2 Output Table
	ofs << "The Encoding Table is:\n";
	for (const auto &i : table)
		ofs << i.first << '\t' << i.second << '\n';
	ofs << std::endl;

	// #3 Output Result
	size_t count2 = 0;
	ofs << "The Encoded result is:\n";
	while (ifs >> ch)
	{
		// Ignore Case
		if (ch >= 'A' && ch <= 'Z')
			ch -= 'A' - 'a';

		ofs << table[ch];
		count2 += table[ch].length ();
	}
	ofs << "\n" << std::endl;

	// #4 Output Statistic info
	ofs << "Huffman Encoded File length is " << count2 << " bits.\n";

	auto log_result = (size_t) (log ((double) dic.size ()) / log ((double) 2));
	ofs << "Fix-len Encoded File length is "
		<< (log_result + (pow (2, log_result) == dic.size () ? 0 : 1)) * count << " bits.\n";
	ofs << std::endl;
}

namespace Graph_impl
{
	// [index] [] <adja index, dist>
	using Graph = std::vector<std::list<std::pair<unsigned, double>>>;

	// [index] <prev index, dist>
	using Path = std::vector<std::pair<unsigned, double>>;

	// <from, to, dist>
	using Edge = std::tuple<unsigned, unsigned, double>;

	// [index] <id>
	using IDMapping = std::vector<int>;
}

namespace STP_impl
{
	using namespace Graph_impl;

	Path dijkstra (
		const Graph &graph,
		unsigned src_node)
	{
		auto pair_cmp = [] (const std::pair<unsigned, double> &a,
							const std::pair<unsigned, double> &b)
		{
			return a.second > b.second;
		};

		// [] <index, dist>
		std::priority_queue<
			std::pair<unsigned, double>,
			std::vector<std::pair<unsigned, double>>,
			decltype (pair_cmp)> heap (pair_cmp);

		Path path (graph.size (), std::make_pair (INT_MAX, DBL_MAX));
		std::vector<bool> is_checked (graph.size (), false);
		path[src_node].second = 0;

		for (auto i = 0; i < graph.size (); ++i)
			heap.emplace (std::make_pair (i, path[i].second));

		while (!heap.empty ())
		{
			// t.first has min dist
			auto t = heap.top ();
			heap.pop ();

			// Duplicate
			if (is_checked[t.first])
				continue;
			else
				is_checked[t.first] = true;

			// Foreach adjacent
			for (auto i : graph[t.first])
			{
				// Relax dist
				auto alt = t.second + i.second;
				if (alt < path[i.first].second)
				{
					// Update dist
					path[i.first].second = alt;

					// Update prev
					path[i.first].first = t.first;

					// Update priority
					heap.emplace (std::make_pair (i.first, alt));
				}
			}
		}

		return std::move (path);
	}

	void print_path (
		std::ostream &os,
		const Path &path,
		const IDMapping &id_mapping,
		unsigned src_node,
		unsigned snk_node)
	{
		if (snk_node == src_node)
			os << id_mapping[src_node];
		else
		{
			print_path (os, path, id_mapping,
						src_node, path[snk_node].first);
			os << " -> " << id_mapping[snk_node];
		}
	}
}

namespace MST_impl
{
	using namespace Graph_impl;

	class DisjSets
	{
	public:
		explicit DisjSets (int numElements)
			: s (numElements, -1)
		{}

		int find (int x)
		{
			if (s[x] < 0)
				return x;
			else
				return s[x] = find (s[x]);
		}

		void unionSets (int root1, int root2)
		{
			if (s[root2] < s[root1])	// root2 is deeper
				s[root1] = root2;		// Make root2 new root
			else
			{
				if (s[root1] == s[root2])
					--s[root1];			// Update height if same
				s[root2] = root1;		// Make root1 new root
			}
		}

	private:
		std::vector<int> s;
	};

	struct edge_cmp
	{
		bool operator () (const Edge &a,
						  const Edge &b)
		{
			return std::get<2> (a) > std::get<2> (b);
		}
	};
	using EdgePQ = std::priority_queue<
		Edge, std::vector<Edge>, edge_cmp>;

	std::pair<std::vector<Edge>, double> kruskal (
		const Graph &graph)
	{
		EdgePQ heap;

		DisjSets ds (graph.size ());
		std::vector<Edge> mst;
		double cost = 0;

		for (auto i = 0; i < graph.size (); ++i)
			for (auto &j : graph[i])
				heap.emplace (std::make_tuple (i, j.first, j.second));

		while (mst.size () != graph.size () - 1)
		{
			Edge e = heap.top ();
			heap.pop ();

			auto uset = ds.find (std::get<0> (e));
			auto vset = ds.find (std::get<1> (e));

			if (uset != vset)
			{
				cost += std::get<2> (e);
				mst.emplace_back (std::move (e));
				ds.unionSets (uset, vset);
			}
		}

		return std::make_pair (mst, cost);
	}

	std::pair<std::vector<Edge>, double> prim (
		const Graph &graph)
	{
		EdgePQ heap;

		std::vector<bool> is_checked (graph.size (), false);
		std::vector<Edge> mst;
		double cost = 0;

		is_checked[0] = true;
		for (auto &j : graph[0])
			heap.emplace (std::make_tuple (0, j.first, j.second));

		while (mst.size () != graph.size () - 1)
		{
			Edge e = heap.top ();
			heap.pop ();

			if (is_checked[std::get<1> (e)])
				continue;
			else
				is_checked[std::get<1> (e)] = true;

			for (auto &j : graph[std::get<1> (e)])
				heap.emplace (std::make_tuple (std::get<1> (e), j.first, j.second));

			cost += std::get<2> (e);
			mst.emplace_back (std::move (e));
		}

		return std::make_pair (mst, cost);
	}

	struct BS
	{
		int id;					// ENODEBID
		long double lot, lat;	// LONGITUDE, LATITUDE
		long double kd;			// K_DIST
	};
	using Lbs = std::vector<BS>;

	void DrawMST (const Graph &graph,
				  const std::vector<Edge> &edgeset,
				  const Lbs &lbs,
				  const IDMapping &id_mapping)
	{
		Lbs lbs_mapping (id_mapping.size ());
		long double minlot = LDBL_MAX, minlat = LDBL_MAX,
			maxlot = 0, maxlat = 0;

		// Get lbs_mapping & bounds
		{
			std::map<unsigned, unsigned> r_id_mapping;

			for (auto i = 0; i < id_mapping.size (); ++i)
				r_id_mapping[id_mapping[i]] = i;

			for (auto bs : lbs)
			{
				try
				{
					lbs_mapping[r_id_mapping.at (bs.id)] = bs;
					maxlot = max (maxlot, bs.lot);
					minlot = min (minlot, bs.lot);
					maxlat = max (maxlat, bs.lat);
					minlat = min (minlat, bs.lat);
				}
				catch (std::exception &)
				{}
			}
		}

		EggAche::Window wnd {900, 900};
		EggAche::Egg egg {900, 900};
		wnd.AddEgg (egg);

		// Draw Graph
		for (auto i = 0; i < graph.size (); ++i)
			for (auto adj : graph[i])
			{
				auto j = adj.first;

				egg.DrawLine ((lbs_mapping[i].lot - minlot) * 800 / (maxlot - minlot) + 50,
					(lbs_mapping[i].lat - minlat) * 800 / (maxlat - minlat) + 50,
							  (lbs_mapping[j].lot - minlot) * 800 / (maxlot - minlot) + 50,
							  (lbs_mapping[j].lat - minlat) * 800 / (maxlat - minlat) + 50);

				char ch[5];

				sprintf (ch, "%d", i);
				egg.DrawTxt ((lbs_mapping[i].lot - minlot) * 800 / (maxlot - minlot) + 50,
					(lbs_mapping[i].lat - minlat) * 800 / (maxlat - minlat) + 50,
							 ch);
				sprintf (ch, "%d", j);
				egg.DrawTxt ((lbs_mapping[j].lot - minlot) * 800 / (maxlot - minlot) + 50,
					(lbs_mapping[j].lat - minlat) * 800 / (maxlat - minlat) + 50,
							 ch);
			}

		// Draw Edgeset
		egg.SetPen (2, 255, 0, 0);

		for (auto e : edgeset)
		{
			auto i = std::get<0> (e);
			auto j = std::get<1> (e);

			egg.DrawLine ((lbs_mapping[i].lot - minlot) * 800 / (maxlot - minlot) + 50,
				(lbs_mapping[i].lat - minlat) * 800 / (maxlat - minlat) + 50,
						  (lbs_mapping[j].lot - minlot) * 800 / (maxlot - minlot) + 50,
						  (lbs_mapping[j].lat - minlat) * 800 / (maxlat - minlat) + 50);
		}
		wnd.Refresh ();
	}
}

void graph_scheme (std::string postfix, int src_node, int snk_node)
{
	using namespace STP_impl;
	using namespace MST_impl;

	std::ofstream ofs ("basestations_result_" + postfix + ".txt");
	std::ifstream ifs ("basestations_" + postfix + ".txt");
	std::ifstream ifs2 ("basestation_data.txt");

	if (!ifs.is_open () || !ifs2.is_open () || !ofs.is_open ())
	{
		std::cerr << "open files failed.\n";
		return;
	}

	// #0 Read Graph

	int tmp;
	char line[1024];
	IDMapping id_mapping;

	// dummy row
	ifs.getline (line, 1024);

	// id line
	{
		ifs.getline (line, 1024);
		std::istrstream istrs (line);
		while (istrs >> tmp)
			id_mapping.emplace_back (tmp);
	}

	Graph graph (id_mapping.size ());
	// table
	for (auto i = 0; i < id_mapping.size (); ++i)
	{
		ifs.getline (line, 1024);
		std::istrstream istrs (line);

		// dummy col
		istrs >> tmp;

		// id col
		istrs >> tmp;
		if (tmp != id_mapping[i])
		{
			std::cerr << "row / col id not matched.\n";
			return;
		}

		// adjacent matrix
		auto j = 0;
		double tmp2;
		while (istrs >> tmp2)
		{
			if (tmp2 != -1)
				graph[i].emplace_back (std::make_pair (j, tmp2));
			++j;
		}
	}

	// #1 Shortest Path

	// Find corresponding src_node
	auto src_node_id = 0;
	for (; src_node_id < id_mapping.size (); ++src_node_id)
		if (id_mapping[src_node_id] == src_node)
			break;

	if (src_node_id == id_mapping.size ())
	{
		std::cerr << "No such source node\n";
		return;
	}

	// Calc shortest path
	auto path = dijkstra (graph, src_node_id);

	// Output all shortest paths
	ofs << "The shortest path from Node '" << src_node << "' is:\n";
	for (auto i = 0; i < path.size (); ++i)
	{
		if (i == src_node_id)
			continue;

		if (path[i].second != DBL_MAX)
			ofs << "To Node '" << id_mapping[i] << "' is of length "
			<< path[i].second << "\tvia Node '" << id_mapping[path[i].first] << "'\n";
		else
			ofs << "To Node '" << id_mapping[i] << "' is out of reach\n";
	}
	ofs << std::endl;

	// Find corresponding snk_node
	auto snk_node_id = 0;
	for (; snk_node_id < id_mapping.size (); ++snk_node_id)
		if (id_mapping[snk_node_id] == snk_node)
			break;

	if (snk_node_id == id_mapping.size ())
	{
		std::cerr << "No such sink node\n";
		return;
	}

	// Output specific path
	ofs << "The shortest path from Node '" << src_node
		<< "' to Node '" << snk_node << "' is:\n";
	STP_impl::print_path (ofs, path, id_mapping, src_node_id, snk_node_id);
	ofs << "\n" << std::endl;

	// #2 Min Spanning Tree

	auto mst = prim (graph);
	//auto mst = kruskal (graph);

	ofs << "The min spanning tree's cost is:\n"
		<< std::get<1> (mst) << std::endl;

	Lbs lbs;
	int id;
	long double lot, lat, kdist;
	ifs2.getline (line, 1024);
	while (ifs2 >> id >> lot >> lat >> kdist)
		lbs.push_back (BS {id, lot, lat, kdist});

	MST_impl::DrawMST (graph, std::get<0> (mst), lbs, id_mapping);
}

int main ()
{
	mwt_scheme ();
	huffman_scheme ();

	graph_scheme ("22", 567443, 33109);
	graph_scheme ("42", 565845, 565667);

	return 0;
}
