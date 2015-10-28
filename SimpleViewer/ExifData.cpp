#include "ExifData.h"

namespace sv {

	ExifData::ExifData(QString const& filepath) {
		std::thread(&ExifData::load, this, filepath).detach();
	}

	ExifData::ExifData(std::vector<char> const& buffer) {
		Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(reinterpret_cast<Exiv2::byte const*>(buffer.data()), buffer.size());
		this->readExifFromImage(image);
	}

	Exiv2::Exifdatum ExifData::value(QString const& key) const {
		if (!exifData.empty()) {
			Exiv2::ExifData::const_iterator it = exifData.findKey(Exiv2::ExifKey(key.toStdString()));
			return *it;
		} else {
			return Exiv2::Exifdatum(Exiv2::ExifKey("NULL"));
		}
	}

	bool ExifData::hasExif() const {
		return !exifData.empty();
	}

	bool ExifData::isReady() const {
		return this->ready;
	}

	void ExifData::get() {
		std::unique_lock<std::mutex> lock(this->mutex);
		if (this->ready) return;
		this->conditionVariable.wait(lock, [this] () { return this->isReady(); });
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
			{
				std::lock_guard<std::mutex> lock(this->mutex);
				ready = true;
			}
		} else {
			std::cout << "Reading image failed" << std::endl;
		}
		this->conditionVariable.notify_all();
		emit(loadingFinished());
	}

}