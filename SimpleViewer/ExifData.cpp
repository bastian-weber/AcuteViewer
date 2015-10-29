#include "ExifData.h"

namespace sv {

	ExifData::ExifData(QString const& filepath) {
		try {
			ready = false;
			this->thread = std::thread(&ExifData::load, this, filepath);
		} catch (...) {
			ready = true;
			emit(loadingFinished(this));
		}
	}

	ExifData::ExifData(std::shared_ptr<std::vector<char>> buffer) {
		try {
			ready = false;
			this->thread = std::thread(&ExifData::loadFromBuffer, this, buffer);
		} catch (...) {
			ready = true;
			emit(loadingFinished(this));
		}
	}

	ExifData::~ExifData() {
		this->join();
	}

	bool ExifData::hasValue(QString const & key) const {
		return (this->exifData.findKey(Exiv2::ExifKey(key.toStdString())) != this->exifData.end());
	}

	Exiv2::Value::AutoPtr ExifData::value(QString const& key) const {
		if (!this->exifData.empty()) {
			Exiv2::ExifData::const_iterator it = this->exifData.findKey(Exiv2::ExifKey(key.toStdString()));
			if (it != this->exifData.end()) {
				return it->getValue();
			} else {
				return Exiv2::Value::AutoPtr(Exiv2::Value::create(Exiv2::asciiString));
			}
		} else {
			return Exiv2::Value::AutoPtr(Exiv2::Value::create(Exiv2::asciiString));
		}
	}

	QString ExifData::cameraModel() const {
		QString cameraModel = "";
		if (this->hasValue("Exif.Image.Model")) {
			QString cameraModel = QString::fromStdString(this->value("Exif.Image.Model")->toString()).trimmed();
		}
		return cameraModel;
	}

	QString ExifData::exposureTime() const {
		QString speed = "";
		if (this->hasValue("Exif.Photo.ExposureTime")) {
			Exiv2::Rational speedValue = this->value("Exif.Photo.ExposureTime")->toRational();
			if (speedValue.first < speedValue.second) {
				speed = QString("%1/%2").arg(speedValue.first / speedValue.first).arg(speedValue.second / speedValue.first);
			} else {
				speed = QString::number(double(speedValue.first) / double(speedValue.second));
			}
		}
		return speed;
	}

	QString ExifData::fNumber() const {
		QString aperture = "";
		if (this->hasValue("Exif.Photo.FNumber")) {
			Exiv2::Rational apertureValue = this->value("Exif.Photo.FNumber")->toRational();
			aperture = QString::number(double(apertureValue.first) / double(apertureValue.second));
		}
		return aperture;
	}

	QString ExifData::iso() const {
		QString iso = "";
		if (this->hasValue("Exif.Photo.ISOSpeedRatings")) {
			long isoValue = this->value("Exif.Photo.ISOSpeedRatings")->toLong();
			iso = QString::number(isoValue);
		}
		return iso;
	}

	QString ExifData::captureDate() const {
		QString captureDate = "";
		if (this->hasValue("Exif.Photo.DateTimeOriginal")) {
			captureDate = QString::fromStdString(this->value("Exif.Photo.DateTimeOriginal")->toString());
			QString date = captureDate.section(' ', 0, 0);
			QString time = captureDate.section(' ', 1, 1);
			date.replace(':', '-');
			captureDate = QString("%1 %2").arg(date).arg(time);
		}
		return captureDate;
	}

	bool ExifData::hasExif() const {
		return this->ready && !this->exifData.empty();
	}

	bool ExifData::isReady() const {
		return this->ready;
	}

	void ExifData::join() {
		if (this->thread.joinable()) this->thread.join();
	}

	//=============================================================================== PRIVATE ===============================================================================\\

	void ExifData::load(QString filepath) {
		try {
#ifdef Q_OS_WIN
			if (utility::isASCII(filepath)) {
#endif
				Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(filepath.toStdString());
				this->readExifFromImage(image);
#ifdef Q_OS_WIN
			} else {
				std::shared_ptr<std::vector<char>> buffer = utility::readFileIntoBuffer(filepath);
				this->loadFromBuffer(buffer);
			}
#endif
		} catch (...) {
			this->ready = true;
			emit(loadingFinished(this));
		}
	}

	void ExifData::loadFromBuffer(std::shared_ptr<std::vector<char>> buffer) {
		try {
			Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(reinterpret_cast<Exiv2::byte const*>(buffer->data()), buffer->size());
			this->readExifFromImage(image);
		} catch (...) {
			ready = true;
			emit(loadingFinished(this));
		}
	}

	void ExifData::readExifFromImage(Exiv2::Image::AutoPtr const image) {
		try {
			if (image.get() != 0) {
				image->readMetadata();
				this->exifData = image->exifData();
			}
			this->ready = true;
			emit(loadingFinished(this));
		} catch (...) {
			this->ready = true;
			emit(loadingFinished(this));
		}
	}

}