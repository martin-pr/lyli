
#include "context.h"
#include "buffer.h"
#include "hiddevice.h"
#include "hidreport.h"

#include <cstdio>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <libusb.h>
#include <map>
#include <string>

namespace {

std::map<uint8_t, const char*> TAGS_GLOBAL = {
	{0x0, "Usage Page"},
	{0x1, "Logical Minimum"},
	{0x2, "Logical Maximum"},
	{0x3, "Physical Minimum"},
	{0x4, "Physical Maximum"},
	{0x5, "Unit Exponent"},
	{0x6, "Unit"},
	{0x7, "Report Size"},
	{0x8, "Report ID"},
	{0x9, "Report Count"},
	{0xA, "Push"},
	{0xB, "Pop"},
};
std::map<uint8_t, const char*> TAGS_LOCAL = {
	{0x0, "Usage"},
	{0x1, "Usage Minimum"},
	{0x2, "Usage Maximum"},
	{0x3, "Designator Index"},
	{0x4, "Designator Minimum"},
	{0x5, "Designator Maximum"},
	{0x7, "String Index"},
	{0x8, "String Minimum"},
	{0x9, "String Maximum"},
	{0xa, "Delimiter"},
};

std::map<uint8_t, const char*> TAGS_MAIN = {
	{0x8, "Input"},
	{0x9, "Output"},
	{0xb, "Feature"},
	{0xa, "Collection"},
	{0xc, "End Collection"},
};

}

std::ostream& operator<<(std::ostream& os, const Usbpp::HID::ReportItem &item)
{
	std::string type;
	std::string tagString;
	switch (item.getType()) {
		case Usbpp::HID::ReportItem::Type::MAIN:
			type = "MAIN";
			try {
				tagString = TAGS_MAIN.at(item.getTag());
			}
			catch (...) {}
			break;
		case Usbpp::HID::ReportItem::Type::GLOBAL:
			type = "GLOBAL";
			try {
				tagString = TAGS_GLOBAL.at(item.getTag());
			}
			catch (...) {}
			break;
		case Usbpp::HID::ReportItem::Type::LOCAL:
			type = "LOCAL";
			try {
				tagString = TAGS_LOCAL.at(item.getTag());
			}
			catch (...) {}
			break;
	}

	os << type << ": ";
	os << "0x" << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(item.getTag());
	os << " (" << tagString << ")";
	os << " [ ";
	for(std::size_t i = 0; i < item.getData().size(); ++i) {
		os << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(item.getData()[i]) << " ";
	}
	os << "]";

	return os;
}

std::ostream& operator<<(std::ostream& os, const Usbpp::HID::ReportNode::Ptr &node)
{
	return os << node->getItem();
}

void printNode(const Usbpp::HID::ReportNode::Ptr &node, int indent) {
	std::cout << std::string(indent, '\t') << node << std::endl;
	std::cout << std::string(indent, '\t') << "  GLOBAL STATE" << std::endl;
	for( auto state : node->getGlobalState() ) {
		std::cout << std::string(indent, '\t') << "  --" << (state.second) << std::endl;
	}
	std::cout << std::string(indent, '\t') << "  LOCAL STATE" << std::endl;
	for( auto state : node->getLocalState() ) {
		std::cout << std::string(indent, '\t') << "  --" << (state.second) << std::endl;
	}

	for(auto child : node->getChildren()) {
		printNode(child, indent+1);
	}
}

int main(int argc, char *argv[]) {
	int vendor;
	int product;
	if (argc != 2 || sscanf(argv[1],"%x:%x",&vendor,&product) != 2) {
		std::cerr << "Wrong arguments!" << std::endl;
		std::cerr << "\tuse vendor:product, where vendor and product are hexadecimal numbers" << std::endl;
		return 1;
	}

	Usbpp::Context context;
	std::vector<Usbpp::Device> devices(context.getDevices());

	Usbpp::HID::HIDDevice device;
	bool found = false;
	for (Usbpp::Device dev : devices) {
		try {
			dev.open(true);
			libusb_device_descriptor descr(dev.getDescriptor());
			if (descr.idVendor == vendor && descr.idProduct == product) {
				device = dev;
				found = true;
				break;
			}
			else {
				dev.close();
			}
		}
		catch (const Usbpp::DeviceException &e) {
			// just silently ignore the exception and try the next device
			continue;
		}
	}

	if (!found) {
		std::cerr << "No device found!" << std::endl;
		return 1;
	}

	device.claimInterface(0);
	try {
		Usbpp::HID::ReportTree report(device.getHidReport(0));
		printNode(report.getRoot(), 0);
	}
	catch (const Usbpp::DeviceException &e) {
		std::cerr << "Failed to obtain HID report descriptor!" << std::endl;
		return 1;
	}

	return 0;
}
