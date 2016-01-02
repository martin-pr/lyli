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
	/**
	 * Construct grid mapper with maps of a given size
	 */
	GridMapper(std::size_t horizontalSize, std::size_t verticalSize);
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
	 * Map source grid vertical index to index in destination grid
	 * @param from index in source grid
	 * @return index in target
	 */
	std::size_t mapVertical(std::size_t from);

	/**
	 * Set horizontal mapping
	 * @param from index in source grid
	 * @param to index in target grid
	 */
	void mapHorizontal(std::size_t from, std::size_t to);
	/**
	 * Set vertical mapping
	 * @param from index in source grid
	 * @param to index in target grid
	 */
	void mapVertical(std::size_t from, std::size_t to);

private:
	MappingLUT horizontalMapping;
	MappingLUT verticalMapping;
};

}
}

#endif
