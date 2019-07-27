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
#include "gridmapper.h"

#include <cassert>

namespace Lyli {
namespace Calibration {

GridMapper::GridMapper(std::size_t horizontalSize, std::size_t verticalSize) :
	horizontalMapping(horizontalSize),
	verticalMapping(verticalSize) {

}

std::size_t GridMapper::mapHorizontal(std::size_t from) const {
	assert(from < horizontalMapping.size());
	return horizontalMapping[from];
}

std::size_t GridMapper::mapVertical(std::size_t from) const {
	assert(from < verticalMapping.size());
	return verticalMapping[from];
}

void GridMapper::mapHorizontal(std::size_t from, std::size_t to) {
	assert(from < horizontalMapping.size());
	horizontalMapping[from] = to;
}

void GridMapper::mapVertical(std::size_t from, std::size_t to) {
	assert(from < verticalMapping.size());
	verticalMapping[from] = to;
}

}
}
