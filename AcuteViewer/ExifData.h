#ifndef SV_EXIFDATA
#define SV_EXIFDATA

#include <thread>
#include <atomic>
#include <iostream>
#include <mutex>
#include <condition_variable>

//Qt
#include <QtCore>

//OpenCV
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

//Exiv2
#include <exiv2.hpp>

#include "utility.h"

namespace sv {

	class ExifData : public QObject {
		Q_OBJECT
	public:
		ExifData(QString const& filepath, bool launchDeferred = false);
		ExifData(std::shared_ptr<std::vector<char>> buffer);
		ExifData(ExifData const& other) = delete;
		ExifData& operator=(ExifData const& other) = delete;
		~ExifData();
		void startLoading();
		bool hasValue(QString const& key) const;
		Exiv2::Value::AutoPtr value(QString const& key) const;
		QString cameraModel() const;
		QString lensModel() const;
		QString exposureTime() const;
		QString fNumber() const;
		QString iso() const;
		QString exposureBias() const;
		QString focalLength() const;
		QString focalLength35mmEquivalent() const;
		QString captureDate() const;
		QString resolution() const;
		cv::Mat largestReadablePreviewImage();
		bool hasExif() const;
		bool hasPreviewImage() const;
		bool isReady() const;
		bool isDeferred() const;
		void join();
	private:
		//functions
		void launchThreadFromPath(QString const& filepath);
		void load(QString filepath);
		void loadFromBuffer(std::shared_ptr<std::vector<char>> buffer);
		void readExifFromImage(Exiv2::Image::AutoPtr const image);

		//variables
		Exiv2::ExifData exifData;
		cv::Mat preview;
		bool previewAvailable = false;
		std::thread thread;
		QString cachedFilepath;
		std::atomic<bool> ready{ false };
		std::atomic<bool> deferred{ true };
	signals:
		void loadingFinished(ExifData* sender);
	};

}

#endif