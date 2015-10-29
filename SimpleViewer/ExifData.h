#ifndef SV_EXIFDATA
#define SV_EXIFDATA

#include <thread>
#include <atomic>
#include <iostream>
#include <mutex>
#include <condition_variable>

//Qt
#include <QtCore>

//Exiv2
#include <exiv2.hpp>

#include "utility.h"

namespace sv {

	class ExifData : public QObject {
		Q_OBJECT
	public:
		ExifData(QString const& filepath);
		ExifData(std::vector<char> const& buffer);
		ExifData(ExifData const& other) = delete;
		ExifData& operator=(ExifData const& other) = delete;
		~ExifData();
		bool hasValue(QString const& key) const;
		Exiv2::Value::AutoPtr value(QString const& key) const;
		bool hasExif() const;
		bool isReady() const;
		void join();
	private:
		//functions
		void load(QString filepath);
		void readExifFromImage(Exiv2::Image::AutoPtr const image);

		//variables
		Exiv2::ExifData exifData;
		std::thread thread;
		std::atomic<bool> ready = true;
	signals:
		void loadingFinished(ExifData* sender);
	};

}

#endif