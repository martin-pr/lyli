#!/usr/bin/env python3

from string import Template

TEMPLATE_HEADER = Template("""

#ifndef LYLI_IMAGE_${CLASS}_H_
#define LYLI_IMAGE_${CLASS}_H_

#include <iostream>
#include <memory>
#include <string>

namespace Json {
class Value;
}

namespace Lyli {
namespace Image {

/**
 * A class providing native access to JSON data.
 */
class ${CLASS} {
public:
	typedef std::shared_ptr<::Json::Value> ValuePtr;

	/**
	 * A constructor.
	 */
	${CLASS}();

	/**
	 * Construct metadata object from a stream.
	 */
	${CLASS}(std::istream &is);

	/**
	 * A destructor.
	 */
	~${CLASS}();

	/**
	 * Read metadata from a stream.
	 */
	void read(std::istream &is);

	// generated accessors
${GENERATED}

private:
	ValuePtr m_root;
};

}
}

#endif
""")

TEMPLATE_SOURCE = Template("""
#include "${INCLUDE}.h"

#include <memory>

#include <json/reader.h>
#include <json/value.h>

namespace Lyli {
namespace Image {

${CLASS}::${CLASS}() {

}

${CLASS}::${CLASS}(std::istream& is) {
	read(is);
}

${CLASS}::~${CLASS}() {

}

void ${CLASS}::read(std::istream &is) {
	::Json::CharReaderBuilder builder;
	::Json::Value root;
	std::string errs;
	::Json::parseFromStream(builder, is, &root, &errs);
	m_root = std::make_shared< ::Json::Value >(root);
}

// generated accessors
${GENERATED}

}
}
""")
