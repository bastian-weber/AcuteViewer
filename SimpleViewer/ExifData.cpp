#include "ExifData.h"

namespace sv {

	ExifData::ExifData(QString const& filepath) : thread(&ExifData::load, this, filepath) { }

	ExifData::ExifData(std::vector<char> const& buffer) {
		Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(reinterpret_cast<Exiv2::byte const*>(buffer.data()), buffer.size());
		this->readExifFromImage(image);
	}

	ExifData::~ExifData() {
		this->join();
	}

	std::string ExifData::value(QString const& key) const {
		if (!this->exifData.empty()) {
			Exiv2::ExifData::const_iterator it = this->exifData.findKey(Exiv2::ExifKey(key.toStdString()));
			if (it != this->exifData.end()) {
				return it->value().toString();
			} else {
				return "";
			}
		} else {
			return "";
		}
	}

	bool ExifData::hasExif() const {
		return this->ready && !this->exifData.empty();
	}

	bool ExifData::isReady() const {
		return this->ready;
	}

	void ExifData::join() {
		if(this->thread.joinable()) this->thread.join();
	}

	void ExifData::load(QString filepath) {
#ifdef Q_OS_WIN
		if (utility::isASCII(filepath)) {
#endif
			Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(filepath.toStdString());
			this->readExifFromImage(image);
#ifdef Q_OS_WIN
		} else {
			std::vector<char> buffer = utility::readFileIntoBuffer(filepath);
			Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(reinterpret_cast<Exiv2::byte const*>(buffer.data()), buffer.size());
			this->readExifFromImage(image);
		}
#endif
	}

	void ExifData::readExifFromImage(Exiv2::Image::AutoPtr const image) {
		if (image.get() != 0) {
			image->readMetadata();
			this->exifData = image->exifData();
		}
		this->ready = true;
		emit(loadingFinished(this));
	}

}