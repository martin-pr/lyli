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
 * 
 */

#ifndef LYLI_CONTEXT_H
#define LYLI_CONTEXT_H

#include <functional>
#include <memory>
#include <vector>

#include "camera.h"

namespace Usbpp {
class Context;
}

namespace Lyli {

/**
 * List of cameras
 */
using CameraList = std::vector<Camera>;

class Context {
public:
	/**
	 * A constructor.
	 *
	 * Creates new USB context.
	 */
	Context();
	/**
	 * A destructor.
	 */
	~Context();

	/**
	* Get list of cameras.
	* \return list of cameras available at the time of call
	*/
	CameraList getCameras();
private:
	class Impl;
	Impl* pimpl;
};

}

#endif // LYLI_CONTEXT_H
