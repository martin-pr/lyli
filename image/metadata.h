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

#ifndef LYLI_IMAGE_METADATA_H_
#define LYLI_IMAGE_METADATA_H_

#include <iostream>
#include <string>
#include <vector>

namespace Lyli {
namespace Image {

/**
 * A class providing native access to image JSON metadata.
 */
class Metadata {
public:
	/**
	 * A constructor.
	 */
	Metadata();

	/**
	 * Construct metadata object from a stream.
	 */
	Metadata(std::istream &is);

	/**
	 * A destructor.
	 */
	~Metadata();

	/**
	 * Read metadata from a stream.
	 */
	void read(std::istream &is);

	// The JSON structure
	// generate using JSONedit and then hand-edited to provite getter interface.

	// a shared class for color metadata
	struct ColorMeta {
		float getB();
		float getGb();
		float getGr();
		float getR();
	};

	std::string getType();

	struct Camera {
		std::string getFirmware();
		std::string getMake();
		std::string getModel();
	};
	Camera getCamera();

	struct Devices {
		struct Accelerometer {
			struct SampleArray {
				float getTime();
				float getX();
				float getY();
				float getZ();
			};
			std::vector<SampleArray> getSampleArray();
		};
		Accelerometer getAccelerometer();

		struct Clock {
			std::string getZuluTime();
		};
		Clock getClock();

		struct Lens {
			struct ExitPupilOffset {
				float getZ();
			};
			ExitPupilOffset getExitPupilOffset();

			float getFNumber();
			float getFocalLength();
			int getFocusStep();
			int getFocusStepperOffset();
			float getInfinityLambda();
			float getTemperature();
			int getTemperatureAdc();
			int getZoomStep();
			int getZoomStepperOffset();
		};
		Lens getLens();

		struct Mla {
			std::string getConfig();
			std::vector<int> getDefectArray();
			float getLensPitch();
			float getRotation();
			std::string gettiling();

			struct ScaleFactor {
				float getX();
				float getY();
			};
			ScaleFactor getScaleFactor();

			struct SensorOffset {
				float getX();
				float getY();
				float getZ();
			};
			SensorOffset getsensorOffset();
		};
		Mla getMla();

		struct Ndfilter {
			float getExposureBias();
		};
		Ndfilter getNdfilter();

		struct Sensor {
			int getBitsPerPixel();
			int getIso();
			float getPixelPitch();

			ColorMeta getAnalogGain();

			struct Mosaic {
				std::string getTile();
				std::string getUpperLeftPixel();
			};
			Mosaic getMosaic();

		};
		Sensor getSensor();

		struct Shutter {
			float getFrameExposureDuration();
			std::string getMechanism();
			float getPixelExposureDuration();
		};
		Shutter getShutter();

		struct Soc {
			float getTemperature();
			int getTemperatureAdc();
		};
		Soc getSoc();
	};
	Devices getDevices();

	struct Image {
		int getHeight();
		int getWidth();
		float getLimitExposureBias();
		float getModulationExposureBias();
		int getOrientation();
		std::string getRepresentation();

		struct Color {
			std::vector<float> getScmRgbToSrgbArray();
			float getGamma();
			ColorMeta getWhiteBalanceGain();

		};
		Color getColor();

		struct RawDetails {
			struct Mosaic {
				std::string getTile();
				std::string getUpperLeftPixel();
			};
			Mosaic getMosaic();

			struct PixelFormat {
				int getRightShift();
				ColorMeta getBlack();
				ColorMeta getWhite();
			};
			PixelFormat getPixelFormat();

			struct PixelPacking {
				int getBitsPerPixel();
				std::string getEndianness();
			};
			PixelPacking getPixelPacking();
		};
		RawDetails getRawDetails();
	};
	Image getImage();

	struct Modes {
		std::string getCreative();
		std::string getExposureDurationMode();
		float getExposureDurationSpec();
		bool getExposureLock();
		std::string getIsoMode();
		float getIsoSpec();
		bool getManualControls();
		std::string getNdFilterMode();
		float getOverscan();
	};
	Modes getModes();

	struct PrivateMetadata {
		struct Camera {
			std::string getSerialNumber();
		};
		Camera getCamera();

		struct Devices {
			struct Sensor {
				std::string getSensorSerial();
			};
			Sensor getSensor();
		};
		Devices getDevices();
	};
	PrivateMetadata getPrivateMetadata();
};

}
}

#endif
