
// Chapter 2 Assignment Project
// By BOT Man JL.

#include <fstream>
#include <vector>
#include <tuple>
#include <algorithm>
#include <cmath>

struct BS
{
	int id;					// ENODEBID
	long double lot, lat;	// LONGITUDE, LATITUDE
	long double kd;			// K_DIST
};
using Lbs = std::vector<BS>;

namespace Impl_KDist
{
	bool operator < (const BS &a, const BS &b)
	{
		return a.kd < b.kd;
	}

	void _Merge (Lbs &lbs, Lbs &tmp, int lPos, int rPos, int rEnd)
	{
		int lEnd = rPos - 1;
		int tPos = lPos;
		int c = rEnd - lPos + 1;

		while (lPos <= lEnd && rPos <= rEnd)
			if (lbs[lPos] < lbs[rPos])
				tmp[tPos++] = std::move (lbs[lPos++]);
			else
				tmp[tPos++] = std::move (lbs[rPos++]);

		while (lPos <= lEnd)
			tmp[tPos++] = std::move (lbs[lPos++]);

		while (rPos <= rEnd)
			tmp[tPos++] = std::move (lbs[rPos++]);

		for (int i = 0; i < c; --rEnd, ++i)
			lbs[rEnd] = std::move (tmp[rEnd]);
	}

	void _MergeSort (Lbs &lbs, Lbs &tmp, int l, int r)
	{
		if (l < r)
		{
			int m = (l + r) / 2;
			_MergeSort (lbs, tmp, l, m);
			_MergeSort (lbs, tmp, m + 1, r);
			_Merge (lbs, tmp, l, m + 1, r);
		}
	}

	void _QuickSort (Lbs &lbs, int l, int r)
	{
		if (l >= r) return;

		// Median3 Scheme
		int m = (l + r) / 2;
		if (lbs[m] < lbs[l])
			std::swap (lbs[m], lbs[l]);
		if (lbs[r] < lbs[m])
			std::swap (lbs[r], lbs[m]);
		if (lbs[m] < lbs[l])
			std::swap (lbs[m], lbs[l]);

		// Place pivot
		std::swap (lbs[m], lbs[r - 1]);
		BS &pivot = lbs[r - 1];

		int i = l, j = r - 1;
		while (true)
		{
			while (lbs[++i] < pivot);
			while (pivot < lbs[--j]);
			if (i < j)
				std::swap (lbs[i], lbs[j]);
			else
				break;
		}
		// Restore pivot
		std::swap (lbs[i], lbs[r - 1]);

		_QuickSort (lbs, l, i - 1);
		_QuickSort (lbs, i + 1, r);
	}

	void _QuickSelect (Lbs &lbs, int l, int r, int k)
	{
		if (l >= r) return;

		// Median3 Scheme
		int m = (l + r) / 2;
		if (lbs[m] < lbs[l])
			std::swap (lbs[m], lbs[l]);
		if (lbs[r] < lbs[m])
			std::swap (lbs[r], lbs[m]);
		if (lbs[m] < lbs[l])
			std::swap (lbs[m], lbs[l]);

		// Place pivot
		std::swap (lbs[m], lbs[r - 1]);
		BS &pivot = lbs[r - 1];

		int i = l, j = r - 1;
		while (true)
		{
			while (lbs[++i] < pivot);
			while (pivot < lbs[--j]);
			if (i < j)
				std::swap (lbs[i], lbs[j]);
			else
				break;
		}
		// Restore pivot
		std::swap (lbs[i], lbs[r - 1]);

		// Only Diff
		if (k < i)
			_QuickSelect (lbs, l, i - 1, k);
		else if (k > i)
			_QuickSelect (lbs, i + 1, r, k - i - 1);
		// k == i => return;
	}
}

namespace Impl_Dist
{
	const double PI = 3.14159;
	const double EARTH_RADIUS = 6378.137;

	inline long double rad (long double LatOrLon)
	{
		return LatOrLon * PI / 180.0;
	}

	inline long double GetDistance2 (long double lng1, long double lat1, long double lng2, long double lat2)
	{
		long double  radLat1 = rad (lat1);
		long double  radLat2 = rad (lat2);
		long double  radlng1 = rad (lng1);
		long double  radlng2 = rad (lng2);

		long double  s = acos (cos (radLat1) * cos (radLat2) * cos (radlng1 - radlng2)
							   + sin (radLat1) * sin (radLat2));
		return s * EARTH_RADIUS;
	}

	long double Dist (const BS &bs1, const BS &bs2)
	{
		return GetDistance2 (bs1.lot, bs1.lat, bs2.lot, bs2.lat);
	}

	std::tuple<long double, std::pair<BS, BS>, long double, std::pair<BS, BS>> _ClosestPair
	(const Lbs &tmp_x, int l, int r)
	{
		std::pair<BS, BS> p1, p2;
		long double d1 = LDBL_MAX, d2 = LDBL_MAX;

		if (r - l <= 2)
		{
			for (auto i = l; i <= r; ++i)
				for (auto j = i + 1; j <= r; ++j)
				{
					auto dist = Dist (tmp_x[i], tmp_x[j]);
					if (dist < d1)
					{
						d2 = d1;
						p2 = std::move (p1);
						d1 = dist;
						p1 = std::make_pair (tmp_x[i], tmp_x[j]);
					}
				}
			return std::make_tuple (d1, p1, d2, p2);
		}

		// Find min in Left / Right Region
		auto mid = (l + r) / 2;

		auto tl = _ClosestPair (tmp_x, l, mid);
		auto dl1 = std::get<0> (tl);
		auto dl2 = std::get<2> (tl);

		auto tr = _ClosestPair (tmp_x, mid + 1, r);
		auto dr1 = std::get<0> (tr);
		auto dr2 = std::get<0> (tr);

		// Merge Left / Right Region
		if (dl1 < dr1)
		{
			d1 = dl1;
			p1 = std::move (std::get<1> (tl));

			if (dl2 < dr1)
			{
				d2 = dl2;
				p2 = std::move (std::get<3> (tl));
			}
			else
			{
				d2 = dr1;
				p2 = std::move (std::get<1> (tr));
			}
		}
		else
		{
			d1 = dr1;
			p1 = std::move (std::get<1> (tr));

			if (dr2 < dl1)
			{
				d2 = dr2;
				p2 = std::move (std::get<3> (tr));
			}
			else
			{
				d2 = dl1;
				p2 = std::move (std::get<1> (tl));
			}
		}

		// Find pairs in Strip
		Lbs tmp_s;
		for (auto i = l; i <= r; ++i)
		{
			if (abs (tmp_x[i].lot - tmp_x[mid].lot) < d1)
				tmp_s.push_back (tmp_x[i]);
		}

		// Sort points with Latitude
		std::sort (tmp_s.begin (), tmp_s.end (), [] (const BS &a, const BS &b) { return a.lat < b.lat; });

		// Compare pairs in Strip with in LR Region
		for (auto p = tmp_s.cbegin (); p != tmp_s.cend (); ++p)
			for (auto q = p + 1;
				 q != tmp_s.cend () && ((*q).lat - (*p).lat) < d1;
				 ++q)
		{
			auto dist = Dist ((*p), (*q));
			if (dist < d1)
			{
				d2 = d1;
				p2 = std::move (p1);
				d1 = dist;
				p1 = std::make_pair ((*p), (*q));
			}
		}

		return std::make_tuple (d1, p1, d2, p2);
	}
}

void MergeSort (Lbs &lbs)
{
	Lbs tmp (lbs.size ());
	Impl_KDist::_MergeSort (lbs, tmp, 0, lbs.size () - 1);
}

void QuickSort (Lbs &lbs)
{
	Impl_KDist::_QuickSort (lbs, 0, lbs.size () - 1);
}

BS QuickSelect (const Lbs &lbs, int k)
{
	Lbs tmp (lbs);
	Impl_KDist::_QuickSelect (tmp, 0, lbs.size () - 1, k - 1);
	return tmp[k - 1];
}

std::tuple<long double, std::pair<BS, BS>, long double, std::pair<BS, BS>> ClosestPair (const Lbs& lbs)
{
	Lbs tmp_x (lbs);

	// Sort tmp_x up with Longitude, Latitude respectively
	std::sort (tmp_x.begin (), tmp_x.end (), [] (const BS &a, const BS &b) { return a.lot < b.lot; });

	// Use Divide and Conquer
	// Require an assert: lbs.size() >= 2
	return Impl_Dist::_ClosestPair (tmp_x, 0, lbs.size () - 1);
}

std::ostream &operator << (std::ostream &os, const BS& bs)
{
	return os << bs.id << "\t" << bs.lot << "\t" << bs.lat << "\t" << bs.kd << "\n";
}

int main ()
{
	std::ifstream ifs ("base_station_data.txt");
	std::ofstream ofs ("base_station_result.txt");
	Lbs lbs;

	// Input
	int id;
	long double lot, lat, kdist;
	while (ifs >> id >> lot >> lat >> kdist)
		lbs.push_back (BS {id, lot, lat, kdist});

	// Cloest Pair
	auto t = ClosestPair (lbs);
	ofs << "Closest Pair has dist of: " << std::get<0> (t) << "\n"
		<< std::get<1> (t).first << std::get<1> (t).second << "\n"
		<< "Sub-closest Pair has dist of: " << std::get<2> (t) << "\n"
		<< std::get<3> (t).first << std::get<3> (t).second << std::endl;

	// Quick Select
	ofs << "Quick Select min, 5, 50, max:\n"
		<< QuickSelect (lbs, 1)
		<< QuickSelect (lbs, 5)
		<< QuickSelect (lbs, 50)
		<< QuickSelect (lbs, lbs.size ()) << std::endl;

	// Quick Sort
	QuickSort (lbs);

	// Merge Sort
	//MergeSort (lbs);

	// Output lbs
	ofs << "Sort Result:\n";
	for (auto i : lbs)
		ofs << i;

	return 0;
}