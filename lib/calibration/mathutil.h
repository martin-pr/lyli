/*
 * This file is part of Lyli, an application to control Lytro camera
 * Copyright (C) 2016  Lukas Jirkovsky <l.jirkovsky @at@ gmail.com>
 *
 * Lyli is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, version 3 of the License
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LYLI_CALIBRATION_MATHUTIL_H_
#define LYLI_CALIBRATION_MATHUTIL_H_

#include <cmath>
#include <memory>

namespace Lyli {
namespace Calibration {

/**
 * Average value in container with filtering.
 *
 * Before the average of values in container is computed, all values whose
 * distance from mean is larger than sigmaLimit*sigma are removed.
 *
 * @param container container for which the average is computed
 * @param sigmaLimit limit used for filtering values
 */
template < typename T, template <typename, typename> class Container, template <typename> class Alloc >
T filteredAverage(const Container<T, Alloc<T>>& container, double sigmaLimit) {
	double EX2sum = 0;
	double EXsum = 0;
	for (auto val : container) {
		EX2sum += val*val;
		EXsum += val;
	}
	const double EX = EXsum / container.size();
	const double EX2 = EX2sum / container.size();
	const double sigma = std::sqrt(EX2 - EX*EX);

	const double limit = sigmaLimit*sigma;
	double sum = 0;
	std::size_t count = 0;
	for (auto val : container) {
		if (std::abs(val-EX) <= limit) {
			sum += val;
			++count;
		}
	}

	return sum/count;
}

/**
 * Signum function.
 */
template <typename T> int sgn(T val) {
	return (T(0) < val) - (val < T(0));
}

}
}

#endif
