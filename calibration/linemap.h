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

#ifndef LYLI_CALIBRATION_LINEMAP_H_
#define LYLI_CALIBRATION_LINEMAP_H_

#include "line.h"

#include <map>
#include <memory>
#include <opencv2/core/core.hpp>
#include <utility>
#include <vector>

namespace Lyli {
namespace Calibration {

/**
 * A storage class that provides both quick random access and fast search.
 */
class LineMap {
public:
	using InputMap = std::map<float, PtrLine>;
	using LinePtr = std::shared_ptr<PtrLine>;
	using ConstIterator = std::vector<LinePtr>::const_iterator;

	/**
	 * Default constructor.
	 */
	LineMap();
	/**
	 * Construct LineMap from iterator range
	 */
	LineMap(InputMap::iterator start, InputMap::iterator end);
	/**
	 * A destructor.
	 */
	~LineMap();

	/**
	 * Return the number of stored elements.
	 */
	std::size_t size() const;

	/**
	 * Return the closest line to a given position
	 */
	LinePtr find(float position);
	/**
	 * Return the closest line to a given position
	 */
	const LinePtr find(float position) const;

	/**
	 * Return line at a given index
	 */
	LinePtr at(std::size_t index);
	/**
	 * Return line at a given index
	 */
	const LinePtr at(std::size_t index) const;

	/**
	 * Erase line at a given index.
	 */
	void erase(std::size_t index);

	/**
	 * Get iterator to first element.
	 *
	 * Only const iterator is allowed to ensure nobody can modify the internal
	 * storage vector.
	 */
	ConstIterator begin() const;
	/**
	 * Get iterator to  past-the-end element.
	 */
	ConstIterator end() const;

private:
	using StorageMap = std::map<float, LinePtr>;
	using StorageVector = std::vector<LinePtr>;

	StorageMap map;
	StorageVector vector;
};

}
}

#endif
