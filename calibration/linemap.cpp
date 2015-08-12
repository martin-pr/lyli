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

#include "linemap.h"

namespace Lyli {
namespace Calibration {

LineMap::LineMap() {

}

LineMap::LineMap(InputMap::iterator start, InputMap::iterator end) {
		for (InputMap::iterator it = start; it != end; ++it) {
			LinePtr line = std::make_shared<PtrLine>(it->second);
			map.insert(std::make_pair(it->first, line));
			vector.push_back(line);
		}
	}

LineMap::~LineMap() {

}

std::size_t LineMap::size() const {
	return vector.size();
}

LineMap::LinePtr LineMap::find(float position) {
	// TODO
}

const LineMap::LinePtr LineMap::find(float position) const {
	// TODO
}

LineMap::LinePtr LineMap::at(std::size_t index) {
	return vector[index];
}

const LineMap::LinePtr LineMap::at(std::size_t index) const {
	return vector[index];
}

void LineMap::erase(std::size_t index) {
	// TODO
}

LineMap::ConstIterator LineMap::begin() const {
	return vector.begin();
}

LineMap::ConstIterator LineMap::end() const {
	return vector.end();
}

}
}
