#include "stdafx.h"
#include "Stat.h"

namespace rms {
	Stat::Stat() {
		reset();
	}

	void Stat::reset()
	{
		last = 0;
		top = 0;
		nb = 0;
		min = 1e12;
		max = -1e12;
		total = 0;
		total2 = 0;
	}

	void Stat::add(double val) {
		nb++;
		if (val < min)
			min = val;
		if (val > max)
			max = val;
		total += val;
		total2 += val*val;
	}
}