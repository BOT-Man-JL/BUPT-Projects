
// Probability Experiment
// Simulation of Gambling

#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include <array>
#include <Windows.h>	// Sleep
#include <time.h>		// clock

// Open-source EggAche Graphical Library is available on GitHub
// https://github.com/BOT-Man-JL/EggAche-GL
#include "EggAche.h"

template <unsigned N> class Simulator
{
	std::array<unsigned, 2 * N + 1> arr {0};
	unsigned count = 0;

public:
	void run (unsigned cMoney)
	{
		std::random_device generator;
		std::bernoulli_distribution distribution;

		while (cMoney != 0 && cMoney != 2 * N)
		{
			if (distribution (generator))
				++cMoney;
			else
				--cMoney;
		}

		++arr[cMoney];
		++count;
	}

	void clear ()
	{
		arr.fill (0);
		count = 0;
	}

	double prob_A_win () const
	{
		return (double) arr[2 * N] / count;
	}
};

template <unsigned N>
void draw_scheme (EggAche::Egg &egg,
				  std::array<double, 2 * N + 1>& probability)
{
	const unsigned width = 1000 - 50;
	const unsigned height = 750 - 50;

	const double wGrid = (double) width / (2 * N);

	// X-axis
	egg.DrawLine (50 / 2, 750 - 50 / 2,
				  1000 - 50 / 2, 750 - 50 / 2);
	for (auto i = 0; i <= 2 * N; ++i)
	{
		char str[20];
		itoa (i, str, 10);
		egg.DrawTxt (50 / 2 + i * wGrid - 5, 750 - 20, str);
	}

	// Y-axis
	egg.DrawLine (50 / 2, 50 / 2,
				  50 / 2, 750 - 50 / 2);
	for (auto i = 0; i <= 10; ++i)
	{
		std::string str;
		if (i == 0)
			str = "1.";
		else
		{
			str += '.';
			str += 10 - i + '0';
		}
		egg.DrawTxt (5, 50 / 2 + i * height / 10 - 5, str.c_str ());
	}

	// Graph
	auto last_point = std::make_pair (0, probability[0]);
	for (auto i = 1; i <= 2 * N; ++i)
	{
		auto curr_point = std::make_pair (i, probability[i]);
		egg.DrawLine (50 / 2 + last_point.first * wGrid,
					  50 / 2 + (1 - last_point.second) * height,
					  50 / 2 + curr_point.first * wGrid,
					  50 / 2 + (1 - curr_point.second) * height);
		last_point = curr_point;
	}
}

int main ()
{
	constexpr unsigned N = 10;
	const unsigned round = 1e4;
	Simulator<N> smt;
	std::array<double, 2 * N + 1> probability {0};

	for (auto cMoney = 0; cMoney <= 2 * N; ++cMoney)
	{
		auto tBeg = clock ();
		for (auto i = 1; i <= round; ++i)
			smt.run (cMoney);
		probability[cMoney] = smt.prob_A_win ();
		std::cout << cMoney << " of " << 2 * N << " done.\tElapse Time: "
			<< (double) (clock () - tBeg) / 1000 << std::endl;
		smt.clear ();
	}

	std::ofstream ofs {"result.txt"};
	for (auto i : probability)
		ofs << i << '\n';

	EggAche::Window wnd;
	EggAche::Egg egg {1000, 750};
	wnd.AddEgg (egg);

	draw_scheme<N> (egg, probability);
	wnd.Refresh ();

	while (!wnd.IsClosed ())
		Sleep (500);

	return 0;
}
