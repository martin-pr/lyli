
#include "metadata.h"

#include <memory>

#include <json/reader.h>
#include <json/value.h>

namespace Lyli {
namespace Image {

Metadata::Metadata() {

}

Metadata::Metadata(std::istream& is) {
	read(is);
}

Metadata::~Metadata() {

}

void Metadata::read(std::istream &is) {
	::Json::CharReaderBuilder builder;
	::Json::Value root;
	std::string errs;
	::Json::parseFromStream(builder, is, &root, &errs);
	m_root = std::make_shared< ::Json::Value >(root);
}

// generated accessors
Metadata::Image::Image(const ValuePtr &root) : m_root(root) {

}

int Metadata::Image::getWidth() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["image"]["width"].asInt();
}

int Metadata::Image::getHeight() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["image"]["height"].asInt();
}

int Metadata::Image::getOrientation() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["image"]["orientation"].asInt();
}

std::string Metadata::Image::getRepresentation() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["image"]["representation"].asString();
}

Metadata::Image::Rawdetails::Rawdetails(const ValuePtr &root) : m_root(root) {

}

Metadata::Image::Rawdetails::Pixelformat::Pixelformat(const ValuePtr &root) : m_root(root) {

}

int Metadata::Image::Rawdetails::Pixelformat::getRightshift() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["image"]["rawDetails"]["pixelFormat"]["rightShift"].asInt();
}

Metadata::Image::Rawdetails::Pixelformat::Black::Black(const ValuePtr &root) : m_root(root) {

}

int Metadata::Image::Rawdetails::Pixelformat::Black::getR() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["image"]["rawDetails"]["pixelFormat"]["black"]["r"].asInt();
}

int Metadata::Image::Rawdetails::Pixelformat::Black::getGr() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["image"]["rawDetails"]["pixelFormat"]["black"]["gr"].asInt();
}

int Metadata::Image::Rawdetails::Pixelformat::Black::getGb() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["image"]["rawDetails"]["pixelFormat"]["black"]["gb"].asInt();
}

int Metadata::Image::Rawdetails::Pixelformat::Black::getB() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["image"]["rawDetails"]["pixelFormat"]["black"]["b"].asInt();
}

Metadata::Image::Rawdetails::Pixelformat::Black Metadata::Image::Rawdetails::Pixelformat::getBlack() const {
	return Black(m_root);
}

Metadata::Image::Rawdetails::Pixelformat::White::White(const ValuePtr &root) : m_root(root) {

}

int Metadata::Image::Rawdetails::Pixelformat::White::getR() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["image"]["rawDetails"]["pixelFormat"]["white"]["r"].asInt();
}

int Metadata::Image::Rawdetails::Pixelformat::White::getGr() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["image"]["rawDetails"]["pixelFormat"]["white"]["gr"].asInt();
}

int Metadata::Image::Rawdetails::Pixelformat::White::getGb() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["image"]["rawDetails"]["pixelFormat"]["white"]["gb"].asInt();
}

int Metadata::Image::Rawdetails::Pixelformat::White::getB() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["image"]["rawDetails"]["pixelFormat"]["white"]["b"].asInt();
}

Metadata::Image::Rawdetails::Pixelformat::White Metadata::Image::Rawdetails::Pixelformat::getWhite() const {
	return White(m_root);
}

Metadata::Image::Rawdetails::Pixelformat Metadata::Image::Rawdetails::getPixelformat() const {
	return Pixelformat(m_root);
}

Metadata::Image::Rawdetails::Pixelpacking::Pixelpacking(const ValuePtr &root) : m_root(root) {

}

std::string Metadata::Image::Rawdetails::Pixelpacking::getEndianness() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["image"]["rawDetails"]["pixelPacking"]["endianness"].asString();
}

int Metadata::Image::Rawdetails::Pixelpacking::getBitsperpixel() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["image"]["rawDetails"]["pixelPacking"]["bitsPerPixel"].asInt();
}

Metadata::Image::Rawdetails::Pixelpacking Metadata::Image::Rawdetails::getPixelpacking() const {
	return Pixelpacking(m_root);
}

Metadata::Image::Rawdetails::Mosaic::Mosaic(const ValuePtr &root) : m_root(root) {

}

std::string Metadata::Image::Rawdetails::Mosaic::getTile() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["image"]["rawDetails"]["mosaic"]["tile"].asString();
}

std::string Metadata::Image::Rawdetails::Mosaic::getUpperleftpixel() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["image"]["rawDetails"]["mosaic"]["upperLeftPixel"].asString();
}

Metadata::Image::Rawdetails::Mosaic Metadata::Image::Rawdetails::getMosaic() const {
	return Mosaic(m_root);
}

Metadata::Image::Rawdetails Metadata::Image::getRawdetails() const {
	return Rawdetails(m_root);
}

Metadata::Image::Color::Color(const ValuePtr &root) : m_root(root) {

}

float Metadata::Image::Color::getGamma() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["image"]["color"]["gamma"].asFloat();
}

Metadata::Image::Color::Whitebalancegain::Whitebalancegain(const ValuePtr &root) : m_root(root) {

}

float Metadata::Image::Color::Whitebalancegain::getR() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["image"]["color"]["whiteBalanceGain"]["r"].asFloat();
}

float Metadata::Image::Color::Whitebalancegain::getGr() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["image"]["color"]["whiteBalanceGain"]["gr"].asFloat();
}

float Metadata::Image::Color::Whitebalancegain::getGb() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["image"]["color"]["whiteBalanceGain"]["gb"].asFloat();
}

float Metadata::Image::Color::Whitebalancegain::getB() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["image"]["color"]["whiteBalanceGain"]["b"].asFloat();
}

Metadata::Image::Color::Whitebalancegain Metadata::Image::Color::getWhitebalancegain() const {
	return Whitebalancegain(m_root);
}

Metadata::Image::Color Metadata::Image::getColor() const {
	return Color(m_root);
}

float Metadata::Image::getModulationexposurebias() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["image"]["modulationExposureBias"].asFloat();
}

float Metadata::Image::getLimitexposurebias() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["image"]["limitExposureBias"].asFloat();
}

Metadata::Image Metadata::getImage() const {
	return Image(m_root);
}

Metadata::Devices::Devices(const ValuePtr &root) : m_root(root) {

}

Metadata::Devices::Clock::Clock(const ValuePtr &root) : m_root(root) {

}

std::string Metadata::Devices::Clock::getZulutime() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["devices"]["clock"]["zuluTime"].asString();
}

Metadata::Devices::Clock Metadata::Devices::getClock() const {
	return Clock(m_root);
}

Metadata::Devices::Sensor::Sensor(const ValuePtr &root) : m_root(root) {

}

int Metadata::Devices::Sensor::getBitsperpixel() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["devices"]["sensor"]["bitsPerPixel"].asInt();
}

Metadata::Devices::Sensor::Mosaic::Mosaic(const ValuePtr &root) : m_root(root) {

}

std::string Metadata::Devices::Sensor::Mosaic::getTile() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["devices"]["sensor"]["mosaic"]["tile"].asString();
}

std::string Metadata::Devices::Sensor::Mosaic::getUpperleftpixel() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["devices"]["sensor"]["mosaic"]["upperLeftPixel"].asString();
}

Metadata::Devices::Sensor::Mosaic Metadata::Devices::Sensor::getMosaic() const {
	return Mosaic(m_root);
}

int Metadata::Devices::Sensor::getIso() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["devices"]["sensor"]["iso"].asInt();
}

Metadata::Devices::Sensor::Analoggain::Analoggain(const ValuePtr &root) : m_root(root) {

}

float Metadata::Devices::Sensor::Analoggain::getR() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["devices"]["sensor"]["analogGain"]["r"].asFloat();
}

float Metadata::Devices::Sensor::Analoggain::getGr() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["devices"]["sensor"]["analogGain"]["gr"].asFloat();
}

float Metadata::Devices::Sensor::Analoggain::getGb() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["devices"]["sensor"]["analogGain"]["gb"].asFloat();
}

float Metadata::Devices::Sensor::Analoggain::getB() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["devices"]["sensor"]["analogGain"]["b"].asFloat();
}

Metadata::Devices::Sensor::Analoggain Metadata::Devices::Sensor::getAnaloggain() const {
	return Analoggain(m_root);
}

float Metadata::Devices::Sensor::getPixelpitch() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["devices"]["sensor"]["pixelPitch"].asFloat();
}

Metadata::Devices::Sensor Metadata::Devices::getSensor() const {
	return Sensor(m_root);
}

Metadata::Devices::Lens::Lens(const ValuePtr &root) : m_root(root) {

}

float Metadata::Devices::Lens::getInfinitylambda() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["devices"]["lens"]["infinityLambda"].asFloat();
}

float Metadata::Devices::Lens::getFocallength() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["devices"]["lens"]["focalLength"].asFloat();
}

int Metadata::Devices::Lens::getZoomstep() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["devices"]["lens"]["zoomStep"].asInt();
}

int Metadata::Devices::Lens::getFocusstep() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["devices"]["lens"]["focusStep"].asInt();
}

float Metadata::Devices::Lens::getFnumber() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["devices"]["lens"]["fNumber"].asFloat();
}

float Metadata::Devices::Lens::getTemperature() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["devices"]["lens"]["temperature"].asFloat();
}

int Metadata::Devices::Lens::getTemperatureadc() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["devices"]["lens"]["temperatureAdc"].asInt();
}

int Metadata::Devices::Lens::getZoomstepperoffset() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["devices"]["lens"]["zoomStepperOffset"].asInt();
}

int Metadata::Devices::Lens::getFocusstepperoffset() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["devices"]["lens"]["focusStepperOffset"].asInt();
}

Metadata::Devices::Lens::Exitpupiloffset::Exitpupiloffset(const ValuePtr &root) : m_root(root) {

}

float Metadata::Devices::Lens::Exitpupiloffset::getZ() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["devices"]["lens"]["exitPupilOffset"]["z"].asFloat();
}

Metadata::Devices::Lens::Exitpupiloffset Metadata::Devices::Lens::getExitpupiloffset() const {
	return Exitpupiloffset(m_root);
}

Metadata::Devices::Lens Metadata::Devices::getLens() const {
	return Lens(m_root);
}

Metadata::Devices::Ndfilter::Ndfilter(const ValuePtr &root) : m_root(root) {

}

float Metadata::Devices::Ndfilter::getExposurebias() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["devices"]["ndfilter"]["exposureBias"].asFloat();
}

Metadata::Devices::Ndfilter Metadata::Devices::getNdfilter() const {
	return Ndfilter(m_root);
}

Metadata::Devices::Shutter::Shutter(const ValuePtr &root) : m_root(root) {

}

std::string Metadata::Devices::Shutter::getMechanism() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["devices"]["shutter"]["mechanism"].asString();
}

float Metadata::Devices::Shutter::getFrameexposureduration() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["devices"]["shutter"]["frameExposureDuration"].asFloat();
}

float Metadata::Devices::Shutter::getPixelexposureduration() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["devices"]["shutter"]["pixelExposureDuration"].asFloat();
}

Metadata::Devices::Shutter Metadata::Devices::getShutter() const {
	return Shutter(m_root);
}

Metadata::Devices::Soc::Soc(const ValuePtr &root) : m_root(root) {

}

float Metadata::Devices::Soc::getTemperature() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["devices"]["soc"]["temperature"].asFloat();
}

int Metadata::Devices::Soc::getTemperatureadc() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["devices"]["soc"]["temperatureAdc"].asInt();
}

Metadata::Devices::Soc Metadata::Devices::getSoc() const {
	return Soc(m_root);
}

Metadata::Devices::Accelerometer::Accelerometer(const ValuePtr &root) : m_root(root) {

}

Metadata::Devices::Accelerometer::Samplearray::Samplearray(const ValuePtr &root) : m_root(root) {

}

float Metadata::Devices::Accelerometer::Samplearray::getX() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["devices"]["accelerometer"]["sampleArray"][0]["x"].asFloat();
}

float Metadata::Devices::Accelerometer::Samplearray::getY() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["devices"]["accelerometer"]["sampleArray"][0]["y"].asFloat();
}

float Metadata::Devices::Accelerometer::Samplearray::getZ() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["devices"]["accelerometer"]["sampleArray"][0]["z"].asFloat();
}

float Metadata::Devices::Accelerometer::Samplearray::getTime() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["devices"]["accelerometer"]["sampleArray"][0]["time"].asFloat();
}

Metadata::Devices::Accelerometer::Samplearray Metadata::Devices::Accelerometer::getSamplearray() const {
	return Samplearray(m_root);
}

Metadata::Devices::Accelerometer Metadata::Devices::getAccelerometer() const {
	return Accelerometer(m_root);
}

Metadata::Devices::Mla::Mla(const ValuePtr &root) : m_root(root) {

}

std::string Metadata::Devices::Mla::getTiling() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["devices"]["mla"]["tiling"].asString();
}

float Metadata::Devices::Mla::getLenspitch() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["devices"]["mla"]["lensPitch"].asFloat();
}

float Metadata::Devices::Mla::getRotation() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["devices"]["mla"]["rotation"].asFloat();
}

std::string Metadata::Devices::Mla::getConfig() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["devices"]["mla"]["config"].asString();
}

Metadata::Devices::Mla::Scalefactor::Scalefactor(const ValuePtr &root) : m_root(root) {

}

float Metadata::Devices::Mla::Scalefactor::getX() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["devices"]["mla"]["scaleFactor"]["x"].asFloat();
}

float Metadata::Devices::Mla::Scalefactor::getY() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["devices"]["mla"]["scaleFactor"]["y"].asFloat();
}

Metadata::Devices::Mla::Scalefactor Metadata::Devices::Mla::getScalefactor() const {
	return Scalefactor(m_root);
}

Metadata::Devices::Mla::Sensoroffset::Sensoroffset(const ValuePtr &root) : m_root(root) {

}

float Metadata::Devices::Mla::Sensoroffset::getX() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["devices"]["mla"]["sensorOffset"]["x"].asFloat();
}

float Metadata::Devices::Mla::Sensoroffset::getY() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["devices"]["mla"]["sensorOffset"]["y"].asFloat();
}

float Metadata::Devices::Mla::Sensoroffset::getZ() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["devices"]["mla"]["sensorOffset"]["z"].asFloat();
}

Metadata::Devices::Mla::Sensoroffset Metadata::Devices::Mla::getSensoroffset() const {
	return Sensoroffset(m_root);
}

Metadata::Devices::Mla Metadata::Devices::getMla() const {
	return Mla(m_root);
}

Metadata::Devices Metadata::getDevices() const {
	return Devices(m_root);
}

Metadata::Modes::Modes(const ValuePtr &root) : m_root(root) {

}

std::string Metadata::Modes::getCreative() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["modes"]["creative"].asString();
}

bool Metadata::Modes::getManualcontrols() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["modes"]["manualControls"].asBool();
}

std::string Metadata::Modes::getExposuredurationmode() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["modes"]["exposureDurationMode"].asString();
}

float Metadata::Modes::getExposuredurationspec() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["modes"]["exposureDurationSpec"].asFloat();
}

std::string Metadata::Modes::getIsomode() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["modes"]["isoMode"].asString();
}

float Metadata::Modes::getIsospec() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["modes"]["isoSpec"].asFloat();
}

std::string Metadata::Modes::getNdfiltermode() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["modes"]["ndFilterMode"].asString();
}

bool Metadata::Modes::getExposurelock() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["modes"]["exposureLock"].asBool();
}

float Metadata::Modes::getOverscan() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["modes"]["overscan"].asFloat();
}

Metadata::Modes Metadata::getModes() const {
	return Modes(m_root);
}

Metadata::Camera::Camera(const ValuePtr &root) : m_root(root) {

}

std::string Metadata::Camera::getMake() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["camera"]["make"].asString();
}

std::string Metadata::Camera::getModel() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["camera"]["model"].asString();
}

std::string Metadata::Camera::getFirmware() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["metadata"]["camera"]["firmware"].asString();
}

Metadata::Camera Metadata::getCamera() const {
	return Camera(m_root);
}

Metadata::Privatemetadata::Privatemetadata(const ValuePtr &root) : m_root(root) {

}

Metadata::Privatemetadata::Devices::Devices(const ValuePtr &root) : m_root(root) {

}

Metadata::Privatemetadata::Devices::Sensor::Sensor(const ValuePtr &root) : m_root(root) {

}

std::string Metadata::Privatemetadata::Devices::Sensor::getSensorserial() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["privateMetadata"]["devices"]["sensor"]["sensorSerial"].asString();
}

Metadata::Privatemetadata::Devices::Sensor Metadata::Privatemetadata::Devices::getSensor() const {
	return Sensor(m_root);
}

Metadata::Privatemetadata::Devices Metadata::Privatemetadata::getDevices() const {
	return Devices(m_root);
}

Metadata::Privatemetadata::Camera::Camera(const ValuePtr &root) : m_root(root) {

}

std::string Metadata::Privatemetadata::Camera::getSerialnumber() const {
	return (*m_root)["master"]["picture"]["frameArray"][0]["frame"]["privateMetadata"]["camera"]["serialNumber"].asString();
}

Metadata::Privatemetadata::Camera Metadata::Privatemetadata::getCamera() const {
	return Camera(m_root);
}

Metadata::Privatemetadata Metadata::getPrivatemetadata() const {
	return Privatemetadata(m_root);
}



}
}
