#include "ExifData.h"

namespace sv {

	ExifData::ExifData(QString const& filepath, bool launchDeferred) {
		if (launchDeferred) {
			cachedFilepath = filepath;
		} else {
			launchThreadFromPath(filepath);
		}
	}

	ExifData::ExifData(std::shared_ptr<std::vector<char>> buffer) {
		try {
			deferred = false;
			thread = std::thread(&ExifData::loadFromBuffer, this, buffer);
		} catch (...) {
			ready = true;
			emit(loadingFinished(this));
		}
	}

	ExifData::~ExifData() {
		join();
	}

	void ExifData::startLoading() {
		if(deferred) {
			launchThreadFromPath(cachedFilepath);
		}
	}

	bool ExifData::hasValue(QString const& key) const {
		try {
			if (exifData.empty()) return false;
			return (exifData.findKey(Exiv2::ExifKey(key.toStdString())) != exifData.end());
		} catch (...) {
			return false;
		}
	}

	Exiv2::Value::AutoPtr ExifData::value(QString const& key) const {
		if (!exifData.empty()) {
			Exiv2::ExifData::const_iterator it = exifData.findKey(Exiv2::ExifKey(key.toStdString()));
			if (it != exifData.end()) {
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
		if (hasValue("Exif.Image.Model")) {
			cameraModel = QString::fromStdString(value("Exif.Image.Model")->toString()).trimmed();
		}
		return cameraModel;
	}

	QString ExifData::lensModel() const {
		QString lensModel = "";
		if (hasValue("Exif.Photo.LensModel")) {
			lensModel = QString::fromStdString(value("Exif.Photo.LensModel")->toString()).trimmed();
		}
		return lensModel;
	}

	QString ExifData::exposureTime() const {
		QString speed = "";
		if (hasValue("Exif.Photo.ExposureTime")) {
			Exiv2::Rational speedValue = value("Exif.Photo.ExposureTime")->toRational();
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
		if (hasValue("Exif.Photo.FNumber")) {
			Exiv2::Rational apertureValue = value("Exif.Photo.FNumber")->toRational();
			aperture = QString::number(double(apertureValue.first) / double(apertureValue.second));
		}
		return aperture;
	}

	QString ExifData::iso() const {
		QString iso = "";
		if (hasValue("Exif.Photo.ISOSpeedRatings")) {
			long isoValue = value("Exif.Photo.ISOSpeedRatings")->toLong();
			iso = QString::number(isoValue);
		}
		return iso;
	}

	QString ExifData::exposureBias() const {
		QString exposureBias = "";
		if (hasValue("Exif.Photo.ExposureBiasValue")) {
			Exiv2::Rational exposureBiasValue = value("Exif.Photo.ExposureBiasValue")->toRational();
			double decimal = double(exposureBiasValue.first) / double(exposureBiasValue.second);
			exposureBias = QString::number(decimal);
			if (decimal == 0) {
				exposureBias = QString::fromWCharArray(L"\u00B1%1").arg(exposureBias);
			} else if (decimal > 0) {
				exposureBias = QString("+%1").arg(exposureBias);
			}
		}
		return exposureBias;
	}

	QString ExifData::focalLength() const {
		QString focalLength = "";
		if (hasValue("Exif.Photo.FocalLength")) {
			Exiv2::Rational focalLengthValue = value("Exif.Photo.FocalLength")->toRational();
			focalLength = QString::number(double(focalLengthValue.first) / double(focalLengthValue.second));
		}
		return focalLength;
	}



	QString ExifData::focalLength35mmEquivalent() const {
		QString focalLength = "";
		if (hasValue("Exif.Photo.FocalLengthIn35mmFilm")) {
			Exiv2::Rational focalLengthValue = value("Exif.Photo.FocalLengthIn35mmFilm")->toRational();
			focalLength = QString::number(double(focalLengthValue.first) / double(focalLengthValue.second));
		}
		return focalLength;
	}

	QString ExifData::captureDate() const {
		QString captureDate = "";
		if (hasValue("Exif.Photo.DateTimeOriginal")) {
			captureDate = QString::fromStdString(value("Exif.Photo.DateTimeOriginal")->toString());
			QString date = captureDate.section(' ', 0, 0);
			QString time = captureDate.section(' ', 1, 1);
			date.replace(':', '-');
			captureDate = QString("%1 %2").arg(date).arg(time);
		}
		return captureDate;
	}

	QString ExifData::resolution() const {
		QString resolution = "";
		if (hasValue("Exif.Photo.PixelXDimension") && hasValue("Exif.Photo.PixelYDimension")) {
			long xRes = value("Exif.Photo.PixelXDimension")->toLong();
			long yRes = value("Exif.Photo.PixelYDimension")->toLong();
			resolution = QString::fromWCharArray(L"%1\u2006x\u2006%2").arg(xRes).arg(yRes);
		}else if (hasValue("Exif.Image.ImageWidth") && hasValue("Exif.Image.ImageLength")) {
			long xRes = value("Exif.Image.ImageWidth")->toLong();
			long yRes = value("Exif.Image.ImageLength")->toLong();
			resolution = QString::fromWCharArray(L"%1\u2006x\u2006%2").arg(xRes).arg(yRes);
		}
		return resolution;
	}

	int ExifData::orientation() const {
		if (hasValue("Exif.Image.Orientation")) {
			return value("Exif.Image.Orientation")->toLong();
		}
		return -1;
	}

	cv::Mat ExifData::largestReadablePreviewImage() {
		return preview;
	}

	bool ExifData::hasExif() const {
		return ready && !exifData.empty();
	}

	bool ExifData::hasPreviewImage() const {
		return previewAvailable;
	}

	bool ExifData::isReady() const {
		return ready;
	}

	bool ExifData::isDeferred() const {
		return deferred;
	}

	void ExifData::join() {
		if (deferred) {
			startLoading();
		}
		if (thread.joinable()) thread.join();
	}

	//=============================================================================== PRIVATE ===============================================================================\\

	void ExifData::launchThreadFromPath(QString const& filepath) {
		try {
			deferred = false;
			thread = std::thread(&ExifData::load, this, filepath);
		} catch (...) {
			ready = true;
			emit(loadingFinished(this));
		}
	}

	void ExifData::load(QString filepath) {
		try {
			if (utility::isCharCompatible(filepath)) {
				Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(filepath.toStdString());
				readExifFromImage(image);
			} else {
				std::shared_ptr<std::vector<char>> buffer = utility::readFileIntoBuffer(filepath);
				loadFromBuffer(buffer);
			}
		} catch (...) {
			ready = true;
			emit(loadingFinished(this));
		}
	}

	void ExifData::loadFromBuffer(std::shared_ptr<std::vector<char>> buffer) {
		try {
			Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(reinterpret_cast<Exiv2::byte const*>(buffer->data()), buffer->size());
			readExifFromImage(image);
		} catch (...) {
			ready = true;
			emit(loadingFinished(this));
		}
	}

	void ExifData::readExifFromImage(Exiv2::Image::AutoPtr const image) {
		try {
			if (image.get() != 0) {
				image->readMetadata();
				exifData = image->exifData();
				Exiv2::PreviewManager previews(*image);
				Exiv2::PreviewPropertiesList list = previews.getPreviewProperties();
				if (list.size() > 0) {
					for (int i = list.size() - 1; i > -1; --i) {
						Exiv2::PreviewImage preview = previews.getPreviewImage(list[i]);
						//use a mat instead of a vector as buffer to avoid having to copy the data; const cast should be ok here because we only use the mat as buffer and do not modify it
						cv::Mat buffer(1, preview.size(), CV_8U, const_cast<Exiv2::byte*>(preview.pData()));
						this->preview = cv::imdecode(buffer, cv::IMREAD_UNCHANGED);
						if (this->preview.data) {
							previewAvailable = true;
							break;
						}
					}
				}
			}
			ready = true;
			emit(loadingFinished(this));
		} catch (...) {
			ready = true;
			emit(loadingFinished(this));
		}
	}

}