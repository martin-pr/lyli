/*
 * This file is part of Lyli, an application to control Lytro camera
 * Copyright (C) 2015  Lukas Jirkovsky <l.jirkovsky @at@ gmail.com>
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
#ifndef LYLI_CALIBRATION_SUBGRID_H_
#define LYLI_CALIBRATION_SUBGRID_H_

#include <cstddef>
#include <vector>

namespace Lyli {
namespace Calibration {

/**
 * Subgrid that specifies indices if thae main grid.
 */
class SubGrid {
public:
	using LineIndices = std::vector<std::size_t>;

	SubGrid() = default;
	~SubGrid() = default;
	SubGrid(const SubGrid &other) = default;
	SubGrid(SubGrid &&other) = default;
	SubGrid& operator=(const SubGrid &other) = default;
	SubGrid& operator=(SubGrid &&other) = default;

	const LineIndices& getHorizontalIndices() const;
	const LineIndices& getVerticalIndices() const;

	void addHorizontalIndex(std::size_t index);
	void addVerticalIndex(std::size_t index);

private:
	LineIndices horizontal;
	LineIndices vertical;
};

}
}

#endif
