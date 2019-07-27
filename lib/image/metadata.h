

#pragma once

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
class Metadata {
public:
	typedef std::shared_ptr<::Json::Value> ValuePtr;

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

	// generated accessors

	class Image {
	public:
		Image() = default;
		Image(const Image& other) = default;
		Image(const ValuePtr &root);

		int getWidth() const;
		int getHeight() const;
		int getOrientation() const;
		std::string getRepresentation() const;

		class Rawdetails {
		public:
			Rawdetails() = default;
			Rawdetails(const Rawdetails& other) = default;
			Rawdetails(const ValuePtr &root);


			class Pixelformat {
			public:
				Pixelformat() = default;
				Pixelformat(const Pixelformat& other) = default;
				Pixelformat(const ValuePtr &root);

				int getRightshift() const;

				class Black {
				public:
					Black() = default;
					Black(const Black& other) = default;
					Black(const ValuePtr &root);

					int getR() const;
					int getGr() const;
					int getGb() const;
					int getB() const;
				private:
					ValuePtr m_root;
				};
				Black getBlack() const;

				class White {
				public:
					White() = default;
					White(const White& other) = default;
					White(const ValuePtr &root);

					int getR() const;
					int getGr() const;
					int getGb() const;
					int getB() const;
				private:
					ValuePtr m_root;
				};
				White getWhite() const;
			private:
				ValuePtr m_root;
			};
			Pixelformat getPixelformat() const;

			class Pixelpacking {
			public:
				Pixelpacking() = default;
				Pixelpacking(const Pixelpacking& other) = default;
				Pixelpacking(const ValuePtr &root);

				std::string getEndianness() const;
				int getBitsperpixel() const;
			private:
				ValuePtr m_root;
			};
			Pixelpacking getPixelpacking() const;

			class Mosaic {
			public:
				Mosaic() = default;
				Mosaic(const Mosaic& other) = default;
				Mosaic(const ValuePtr &root);

				std::string getTile() const;
				std::string getUpperleftpixel() const;
			private:
				ValuePtr m_root;
			};
			Mosaic getMosaic() const;
		private:
			ValuePtr m_root;
		};
		Rawdetails getRawdetails() const;

		class Color {
		public:
			Color() = default;
			Color(const Color& other) = default;
			Color(const ValuePtr &root);

			float* getCcmrgbtosrgbarray() const;
			float getGamma() const;

			class Whitebalancegain {
			public:
				Whitebalancegain() = default;
				Whitebalancegain(const Whitebalancegain& other) = default;
				Whitebalancegain(const ValuePtr &root);

				float getR() const;
				float getGr() const;
				float getGb() const;
				float getB() const;
			private:
				ValuePtr m_root;
			};
			Whitebalancegain getWhitebalancegain() const;
		private:
			ValuePtr m_root;
		};
		Color getColor() const;
		float getModulationexposurebias() const;
		float getLimitexposurebias() const;
	private:
		ValuePtr m_root;
	};
	Image getImage() const;

	class Devices {
	public:
		Devices() = default;
		Devices(const Devices& other) = default;
		Devices(const ValuePtr &root);


		class Clock {
		public:
			Clock() = default;
			Clock(const Clock& other) = default;
			Clock(const ValuePtr &root);

			std::string getZulutime() const;
		private:
			ValuePtr m_root;
		};
		Clock getClock() const;

		class Sensor {
		public:
			Sensor() = default;
			Sensor(const Sensor& other) = default;
			Sensor(const ValuePtr &root);

			int getBitsperpixel() const;

			class Mosaic {
			public:
				Mosaic() = default;
				Mosaic(const Mosaic& other) = default;
				Mosaic(const ValuePtr &root);

				std::string getTile() const;
				std::string getUpperleftpixel() const;
			private:
				ValuePtr m_root;
			};
			Mosaic getMosaic() const;
			int getIso() const;

			class Analoggain {
			public:
				Analoggain() = default;
				Analoggain(const Analoggain& other) = default;
				Analoggain(const ValuePtr &root);

				float getR() const;
				float getGr() const;
				float getGb() const;
				float getB() const;
			private:
				ValuePtr m_root;
			};
			Analoggain getAnaloggain() const;
			float getPixelpitch() const;
		private:
			ValuePtr m_root;
		};
		Sensor getSensor() const;

		class Lens {
		public:
			Lens() = default;
			Lens(const Lens& other) = default;
			Lens(const ValuePtr &root);

			float getInfinitylambda() const;
			float getFocallength() const;
			int getZoomstep() const;
			int getFocusstep() const;
			float getFnumber() const;
			float getTemperature() const;
			int getTemperatureadc() const;
			int getZoomstepperoffset() const;
			int getFocusstepperoffset() const;

			class Exitpupiloffset {
			public:
				Exitpupiloffset() = default;
				Exitpupiloffset(const Exitpupiloffset& other) = default;
				Exitpupiloffset(const ValuePtr &root);

				float getZ() const;
			private:
				ValuePtr m_root;
			};
			Exitpupiloffset getExitpupiloffset() const;
		private:
			ValuePtr m_root;
		};
		Lens getLens() const;

		class Ndfilter {
		public:
			Ndfilter() = default;
			Ndfilter(const Ndfilter& other) = default;
			Ndfilter(const ValuePtr &root);

			float getExposurebias() const;
		private:
			ValuePtr m_root;
		};
		Ndfilter getNdfilter() const;

		class Shutter {
		public:
			Shutter() = default;
			Shutter(const Shutter& other) = default;
			Shutter(const ValuePtr &root);

			std::string getMechanism() const;
			float getFrameexposureduration() const;
			float getPixelexposureduration() const;
		private:
			ValuePtr m_root;
		};
		Shutter getShutter() const;

		class Soc {
		public:
			Soc() = default;
			Soc(const Soc& other) = default;
			Soc(const ValuePtr &root);

			float getTemperature() const;
			int getTemperatureadc() const;
		private:
			ValuePtr m_root;
		};
		Soc getSoc() const;

		class Accelerometer {
		public:
			Accelerometer() = default;
			Accelerometer(const Accelerometer& other) = default;
			Accelerometer(const ValuePtr &root);


			class Samplearray {
			public:
				Samplearray() = default;
				Samplearray(const Samplearray& other) = default;
				Samplearray(const ValuePtr &root);

				float getX() const;
				float getY() const;
				float getZ() const;
				float getTime() const;
			private:
				ValuePtr m_root;
			};
			Samplearray getSamplearray() const;
		private:
			ValuePtr m_root;
		};
		Accelerometer getAccelerometer() const;

		class Mla {
		public:
			Mla() = default;
			Mla(const Mla& other) = default;
			Mla(const ValuePtr &root);

			std::string getTiling() const;
			float getLenspitch() const;
			float getRotation() const;
			std::string getConfig() const;

			class Scalefactor {
			public:
				Scalefactor() = default;
				Scalefactor(const Scalefactor& other) = default;
				Scalefactor(const ValuePtr &root);

				float getX() const;
				float getY() const;
			private:
				ValuePtr m_root;
			};
			Scalefactor getScalefactor() const;

			class Sensoroffset {
			public:
				Sensoroffset() = default;
				Sensoroffset(const Sensoroffset& other) = default;
				Sensoroffset(const ValuePtr &root);

				float getX() const;
				float getY() const;
				float getZ() const;
			private:
				ValuePtr m_root;
			};
			Sensoroffset getSensoroffset() const;
		private:
			ValuePtr m_root;
		};
		Mla getMla() const;
	private:
		ValuePtr m_root;
	};
	Devices getDevices() const;

	class Modes {
	public:
		Modes() = default;
		Modes(const Modes& other) = default;
		Modes(const ValuePtr &root);

		std::string getCreative() const;
		bool getManualcontrols() const;
		std::string getExposuredurationmode() const;
		float getExposuredurationspec() const;
		std::string getIsomode() const;
		float getIsospec() const;
		std::string getNdfiltermode() const;
		bool getExposurelock() const;
		float getOverscan() const;
	private:
		ValuePtr m_root;
	};
	Modes getModes() const;

	class Camera {
	public:
		Camera() = default;
		Camera(const Camera& other) = default;
		Camera(const ValuePtr &root);

		std::string getMake() const;
		std::string getModel() const;
		std::string getFirmware() const;
	private:
		ValuePtr m_root;
	};
	Camera getCamera() const;

	class Privatemetadata {
	public:
		Privatemetadata() = default;
		Privatemetadata(const Privatemetadata& other) = default;
		Privatemetadata(const ValuePtr &root);


		class Devices {
		public:
			Devices() = default;
			Devices(const Devices& other) = default;
			Devices(const ValuePtr &root);


			class Sensor {
			public:
				Sensor() = default;
				Sensor(const Sensor& other) = default;
				Sensor(const ValuePtr &root);

				std::string getSensorserial() const;
			private:
				ValuePtr m_root;
			};
			Sensor getSensor() const;
		private:
			ValuePtr m_root;
		};
		Devices getDevices() const;

		class Camera {
		public:
			Camera() = default;
			Camera(const Camera& other) = default;
			Camera(const ValuePtr &root);

			std::string getSerialnumber() const;
		private:
			ValuePtr m_root;
		};
		Camera getCamera() const;
	private:
		ValuePtr m_root;
	};
	Privatemetadata getPrivatemetadata() const;


private:
	ValuePtr m_root;
};

}
}

