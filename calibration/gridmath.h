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
#ifndef LYLI_CALIBRATION_GRIDMATH_H_
#define LYLI_CALIBRATION_GRIDMATH_H_

#include <vector>

#include <calibration/gridmapper.h>
#include <calibration/linegrid.h>

namespace Lyli {
namespace Calibration {

/**
 * Construct an average grids
 *
 * @param grids vector of grids to average
 * @return the average grid and a list of mappers that maps line indices of each grid to the average grid.
 *         The indices in the vector of grid mappers correspond to the indices in the grids vector.
 */
std::pair<LineGrid, std::vector<GridMapper>> averageGrids(const std::vector<LineGrid> &grids);

}
}

#endif
