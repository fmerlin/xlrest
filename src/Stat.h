#pragma once

#include <time.h>
#include <math.h>

namespace rms {
	class __declspec(dllexport) Stat
	{
		clock_t top;
		double last;
		int nb;
		double total;
		double total2;
		double min;
		double max;

	public:
		Stat();
		void reset();
		void add(double val);
		void start() { top = clock(); }
		void end() { if (top > 0) { last = ((double)clock() - top) / CLOCKS_PER_SEC;  add(last); } }
		double mean() { return total / nb; }
		double stddev() { double m = mean();  return sqrt(total2 / nb - m*m); }
		double getLast() { return last;  }
	};
}

