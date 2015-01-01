/*
 * This file is part of Usbpp, a C++ wrapper around libusb
 * Copyright (C) 2015  Lukas Jirkovsky <l.jirkovsky @at@ gmail.com>
 *
 * Usbpp is free software: you can redistribute it and/or modify
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

#ifndef LIBUSBPP_BUFFER_H_
#define LIBUSBPP_BUFFER_H_

#include <algorithm>
#include <cstdlib>
#include <cstring>

namespace Usbpp {

/**
 * A static-sized buffer.
 */
template <typename T>
class Buffer {
public:
	Buffer() : mdata(nullptr), msize(0) {
		
	}
	
	Buffer(const T* data_, std::size_t msize_) {
		mdata = static_cast<T*>(malloc(msize_ * sizeof(T)));
		std::memcpy(mdata, data_, msize_ * sizeof(T));
		msize = msize_;
	}
	
	Buffer(const Buffer<T> &other) {
		mdata = static_cast<T*>(malloc(other.msize * sizeof(T)));
		std::memcpy(mdata, other.mdata, other.msize * sizeof(T));
		msize = other.msize;
	}
	
	Buffer(Buffer<T> &&other) noexcept {
		mdata = other.mdata;
		other.mdata = nullptr;
		msize = other.msize;
		other.msize = 0;
	}
	
	~Buffer() {
		if (mdata) {
			free(mdata);
			mdata = nullptr;
			msize = 0;
		}
	}
	
	Buffer<T> &operator=(const Buffer<T> &other) {
		Buffer tmp(other);
		std::swap(mdata, tmp.mdata);
		std::swap(msize, tmp.msize);
	}
	
	Buffer<T> &operator=(Buffer<T> &&other) noexcept {
		std::swap(mdata, other.mdata);
		std::swap(msize, other.msize);
	}
	
	Buffer<T> &append(const Buffer<T> &other) {
		T *tmp(static_cast<T*>(realloc(mdata, msize + other.msize)));
		if (tmp == nullptr) {
			// TODO: throw
		}
		// copy the contents
		memcpy(tmp + msize, other.mdata, other.msize);
		// update the current buffer
		mdata = tmp;
		msize += other.msize;
	}
	
	std::size_t size() const {
		return msize;
	}
	
	T* data() {
		return mdata;
	}
	
	const T* data() const {
		return mdata;
	}
	
private:
	T* mdata;
	std::size_t msize;
};

typedef Buffer<uint8_t> ByteBuffer;

}

#endif
