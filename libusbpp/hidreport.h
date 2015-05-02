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

#ifndef LIBUSBPP_HID_REPORT_H_
#define LIBUSBPP_HID_REPORT_H_

#include "buffer.h"

#include <stddef.h>
#include <stdint.h>
#include <memory>
#include <map>
#include <vector>

namespace Usbpp {
namespace HID {

/**
 * A representation of a USB HID Report Item.
 */
class ReportItem {
public:
	enum class Format {
		SHORT,
		LONG
	};

	enum class Type : uint8_t {
		MAIN = 0x0,
		GLOBAL = 0x1,
		LOCAL = 0x2,
	};

	enum class TagsMain : uint8_t {
		INPUT = 0x8,
		OUTPUT = 0x9,
		FEATURE = 0xb,
		COLLECTION = 0xa,
		END_COLLECTION = 0xc
	};

	enum class TagsGlobal : uint8_t {
		USAGE_PAGE = 0x0,
		LOGICAL_MINIMUM = 0x1,
		LOGICAL_MAXIMUM = 0x2,
		PHYSICAL_MINIMUM = 0x3,
		PHYSICAL_MAXIMUM = 0x4,
		UNIT_EXPONENT = 0x5,
		UNIT = 0x6,
		REPORT_SIZE = 0x7,
		REPORT_ID = 0x8,
		REPORT_COUNT = 0x9,
		PUSH = 0xa,
		POP = 0xb
	};

	enum class TagsLocal : uint8_t {
		USAGE = 0x0,
		USAGE_MINIMUM = 0x1,
		USAGE_MAXIMUM = 0x2,
		DESIGNATOR_INDEX = 0x3,
		DESIGNATOR_MINIMUM = 0x4,
		DESIGNATOR_MAXIMUM = 0x5,
		STRING_INDEX = 0x7,
		STRING_MINIMUM = 0x8,
		STRING_MAXIMUM = 0x9,
		DELIMITER = 0xa
	};

	ReportItem();
	ReportItem(const ReportItem &other);
	ReportItem(ReportItem &&other);
	~ReportItem();

	ReportItem &operator=(const ReportItem &other);
	ReportItem &operator=(ReportItem &&other) noexcept;

	/**
	 * short/long
	 */
	Format getFormat() const;
	/**
	 * bSize/bDataSize
	 */
	uint8_t getDataSize() const;
	/**
	 * bType
	 */
	Type getType() const;
	/**
	 * bTag/bLongItemTag
	 */
	uint8_t getTag() const;
	/**
	 * the additional payload.
	 */
	const ByteBuffer &getData() const;
private:
	friend class ReportTree;

	/**
	 * A constructor.
	 *
	 * Constructs the item from the binary data.
	 */
	ReportItem(const uint8_t* data);

	class Impl;
	std::unique_ptr<Impl> pimpl;
};

/**
 * A node in the tree representation of a HID report.
 *
 * Each node corresponds to a main item with a state set by global and local items.
 */
class ReportNode {
public:
	typedef std::map<ReportItem::TagsGlobal, ReportItem> GlobalItemMap;
	typedef std::map<ReportItem::TagsLocal, ReportItem> LocalItemMap;
	typedef std::shared_ptr<ReportNode> Ptr;
	typedef std::vector<Ptr> List;

	ReportNode();
	ReportNode(const ReportNode &other);
	ReportNode(ReportNode &&other) noexcept;
	~ReportNode();

	ReportNode &operator=(const ReportNode &other);
	ReportNode &operator=(ReportNode &&other) noexcept;

	/**
	 * Get the state configured using the global items.
	 *
	 * @return map of global items valid for the current main item.
	 */
	const GlobalItemMap &getGlobalState() const;
	/**
	 * Get the state configured using the local items.
	 *
	 * @return map of local items valid for the current main item.
	 */
	const LocalItemMap &getLocalState() const;
	/**
	 * Get the main item associated with the node.
	 *
	 * @return main item
	 */
	const ReportItem &getItem() const;
	/**
	 * Get the parent node.
	 *
	 * The parent node is part of the tree structure generated when collections
	 * are used.
	 *
	 * @return parent node or empty pointer
	 */
	const Ptr &getParent() const;
	/**
	 * Get list of children nodes
	 *
	 * The list of children nodes is part of the tree structure generated when
	 * collections are used.
	 *
	 * @return list of child nodes
	 */
	const List &getChildren() const;

private:
	friend class ReportTree;
	ReportNode(const Ptr &parent_,
	           const ReportItem &item_,
	           const GlobalItemMap &globalState_,
	           const LocalItemMap &localState_);

	class Impl;
	std::unique_ptr<Impl> pimpl;
};

/**
 * A tree representation of a HID report descriptor.
 *
 * The tree is not copyable for, as the copy should create a deep copy
 * to ensure that modification of the copy doesn't affect the original.
 */
class ReportTree {
public:
	/**
	 * A constructor.
	 *
	 * Constructs the report tree from the binary representation of the descriptor.
	 */
	ReportTree(const ByteBuffer &buffer);
	ReportTree(ReportTree &&other);
	ReportTree &operator=(ReportTree &&other);
	~ReportTree();

	/**
	 * Get root node of the parsed HID report descriptor.
	 */
	ReportNode::Ptr getRoot() const;

	// avoid copy
	ReportTree(const ReportTree &other) = delete;
	ReportTree &operator=(const ReportTree &other) = delete;

private:
	ReportNode::Ptr root;
};

}
}

#endif
