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
#ifndef LYLI_CALIBRATION_GRIDMAPPER_H_
#define LYLI_CALIBRATION_GRIDMAPPER_H_

#include <cstddef>
#include <vector>

namespace Lyli {
namespace Calibration {

/**
 * Maps grid line indices from one grid to another.
 */
class GridMapper {
public:
	using MappingLUT = std::vector<std::size_t>;

	GridMapper() = default;
	virtual ~GridMapper() = default;

	GridMapper(const GridMapper &other) = default;
	GridMapper(GridMapper &&other) = default;
	GridMapper& operator=(const GridMapper &other) = default;
	GridMapper& operator=(GridMapper &&other) = default;

	/**
	 * Map horizontal index from source grid to destination grid index
	 * @param from index in source grid
	 * @return index in target
	 */
	std::size_t mapHorizontal(std::size_t from);

	/**
	 * Map source grid vertical index for odd lines to index in destination grid
	 * @param from index in source grid
	 * @return index in target
	 */
	std::size_t mapVerticalOdd(std::size_t from);

	/**
	 * Map source grid vertical index for even lines to index in destination grid
	 * @param from index in source grid
	 * @return index in target
	 */
	std::size_t mapVerticalEven(std::size_t from);

	void setHorizontalMapping(const MappingLUT &mapping);
	void setVerticalOddMapping(const MappingLUT &mapping);
	void setVerticalEvenMapping(const MappingLUT &mapping);

private:
	MappingLUT horizontalMapping;
	MappingLUT verticalOddMapping;
	MappingLUT verticalEvenMapping;
};

}
}

#endif
