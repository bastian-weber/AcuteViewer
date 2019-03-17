#include "ImageView.h"

namespace hb {

	//========================================================================= Public =========================================================================\\

	ImageView::ImageView(QWidget *parent)
		: QWidget(parent),
		interfaceOutline(true),
		useHighQualityDownscaling(true),
		rightClickForHundredPercentView(true),
		usePanZooming(true),
		imageAssigned(false),
		isMat(false),
		zoomBasis(1.5),
		zoomExponent(0),
		preventMagnificationInDefaultZoom(false),
		hundredPercentZoomMode(false),
		panOffset(0, 0),
		viewRotation(0),
		dragging(false),
		pointEditingActive(false),
		pointManipulationActive(false),
		renderPoints(false),
		moved(false),
		panZooming(false),
		paintingActive(false),
		maskInitialized(false),
		brushRadius(5),
		brushPosition(0, 0),
		painting(false),
		visualizeBrushSize(false),
		pointGrabTolerance(10),
		pointGrabbed(false),
		showPointDeletionWarning(false),
		overlayMaskSet(false),
		renderOverlayMask(false),
		renderRectangle(false),
		polylineAssigned(false),
		renderPolyline(false),
		useSmoothTransform(true),
		enablePostResizeSharpening(false),
		postResizeSharpeningStrength(0.5),
		postResizeSharpeningRadius(1),
		polylineManipulationActive(false),
		polylinePointGrabbed(false),
		polylineSelected(false),
		polylinePointGrabTolerance(10),
		polylineLastAddedPoint(0),
		spanningSelectionRectangle(false),
		polylineColor(60, 60, 60),
		externalPostPaintFunctionAssigned(false) {

		cv::ocl::setUseOpenCL(true);
		setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
		setFocusPolicy(Qt::FocusPolicy::StrongFocus);
		setMouseTracking(true);
		QPalette palette = qApp->palette();
		backgroundColor = palette.base().color();
	}

	QSize ImageView::sizeHint() const {
		return QSize(360, 360);
	}

	///Defines wether an outline should be drawn around the widget (indicating also if it has the focus or not)
	void ImageView::setShowInterfaceOutline(bool value) {
		interfaceOutline = value;
		update();
	}

	bool ImageView::getShowInterfaceOutline() {
		return interfaceOutline;
	}

	///Sets the background colour or the widget.
	void ImageView::setInterfaceBackgroundColor(QColor const& color) {
		backgroundColor = color;
		update();
	}

	QColor const& ImageView::getInterfaceBackgroundColor() {
		return backgroundColor;
	}

	///If set to true, a right click zooms the image to 100% magnification.
	void ImageView::setRightClickForHundredPercentView(bool value) {
		rightClickForHundredPercentView = value;
	}

	///Returns \c true if the right click for 100% view feature is enabled.
	bool ImageView::getRightClickForHundredPercentView() {
		return rightClickForHundredPercentView;
	}

	///If set to true, panning while holding the middle mouse button will change the zoom.
	void ImageView::setUsePanZooming(bool value) {
		usePanZooming = value;
	}

	///Returns true if pan-zooming is enabled.
	bool ImageView::getUsesPanZooming() {
		return usePanZooming;
	}

	void ImageView::setUseGpu(bool value) {
		//free the uMat
		if (!value) uMat = cv::UMat();
		//upload the uMat
		if (value && !useGpu && hasMat) {
			mat.copyTo(uMat);
		}
		useGpu = value;
	}

	bool ImageView::getUseGpu() {
		return useGpu;
	}

	bool ImageView::OpenClAvailable() {
		return cv::ocl::haveOpenCL();
	}

	///Rotates the viewport 90° in anticlockwise direction.
	void ImageView::rotateLeft() {
		viewRotation -= 90;
		if (viewRotation < 0) viewRotation += 360;
		enforcePanConstraints();
		updateResizedImage();
		if (isVisible()) update();
	}

	///Rotates the viewport 90° in clockwise direction.
	void ImageView::rotateRight() {
		viewRotation += 90;
		if (viewRotation >= 360) viewRotation -= 360;
		enforcePanConstraints();
		updateResizedImage();
		if (isVisible()) update();
	}

	///Sets the rotation of the view to \p degrees degrees.
	void ImageView::setRotation(double degrees) {
		viewRotation = degrees;
		if (viewRotation >= 360) viewRotation = viewRotation - (360 * std::floor(degrees / 360.0));
		if (viewRotation < 0) viewRotation = viewRotation + (360 * std::ceil(std::abs(degrees / 360.0)));
		enforcePanConstraints();
		updateResizedImage();
		if (isVisible()) update();
	}

	void ImageView::rotateBy(double degrees) {
		setRotation(viewRotation + degrees);
	}

	double ImageView::getRotation() const {
		return viewRotation;
	}

	///Moves the viewport to the point \p point.
	void ImageView::centerViewportOn(QPointF point) {
		QPointF transformedPoint = getTransform().map(point);
		panOffset += (QPointF((double)width() / 2.0, (double)height() / 2.0) - transformedPoint) / (pow(zoomBasis, zoomExponent)*getWindowScalingFactor());
		enforcePanConstraints();
		update();
	}

	///If set to true, images won't be enlarged at the default magnification (fit view).
	void ImageView::setPreventMagnificationInDefaultZoom(bool value) {
		preventMagnificationInDefaultZoom = value;
		update();
	}

	///Makes the \c ImageView display the image \p image, shallow copy assignment.
	void ImageView::setImage(const QImage& image) {
		QSize oldSize = this->image.size();
		this->image = image;
		//free the mat
		isMat = false;
		mat = cv::Mat();
		if (isConvertible(image.format())) {
			shallowCopyImageToMat(image, mat);
			hasMat = true;
		} else {
			hasMat = false;
		}
		if (hasMat && useGpu) {
			mat.copyTo(uMat);
			hasUmat = true;
		}
		if (this->image.size() != oldSize) {
			resetMask();
			hundredPercentZoomMode = false;
		}

		imageAssigned = true;
		updateResizedImage();
		enforcePanConstraints();
		update();
	}

	///Makes the \c ImageView display the image \p image, move assignment.
	void ImageView::setImage(QImage&& image) {
		QSize oldSize = this->image.size();
		this->image = std::move(image);
		//free the mat
		isMat = false;
		mat = cv::Mat();
		if (isConvertible(image.format())) {
			shallowCopyImageToMat(image, mat);
			hasMat = true;
		} else {
			hasMat = false;
		}
		if (hasMat && useGpu) {
			mat.copyTo(uMat);
			hasUmat = true;
		}
		if (image.size() != oldSize) {
			resetMask();
			hundredPercentZoomMode = false;
		}

		imageAssigned = true;
		updateResizedImage();
		enforcePanConstraints();
		update();
	}

	///Makes the \c ImageView display the image \p image, shallow copy assignment.
	void ImageView::setImage(const cv::Mat& image) {
		if (image.type() == CV_8UC4 || image.type() == CV_8UC3 || image.type() == CV_8UC1) {
			QSize oldSize = this->image.size();
			mat = image;
			ImageView::shallowCopyMatToImage(mat, this->image);
			if (this->image.size() != oldSize) {
				resetMask();
				hundredPercentZoomMode = false;
			}
			isMat = true;
			hasMat = true;
			if (hasMat && useGpu) {
				mat.copyTo(uMat);
				hasUmat = true;
			}

			imageAssigned = true;
			updateResizedImage();
			enforcePanConstraints();
			update();
		} else {
			std::cerr << "Image View: cannot assign image because of unsupported type " << image.type() << "." << std::endl;
		}
	}

	///Identical to setImage(const cv::Mat& image) but with a precalculated resized version.
	/**
	 * This funciton can be used to speed up the display process. In \p downscaledImage an
	 * image can be passed that has the dimensions needed for the current zoom level. Thus
	 * the \c ImageView will not have to compute this itself but will just use the provided one.
	 * This offers the possibility to calculate this externally where it can potentially be
	 * done faster (e.g. on the GPU) and save some computation time.
	 */
	void ImageView::setImageWithPrecomputedPreview(const cv::Mat& image, const cv::Mat& downscaledImage) {
		if ((image.type() == CV_8UC4 || image.type() == CV_8UC3 || image.type() == CV_8UC1) && (downscaledImage.type() == CV_8UC4 || downscaledImage.type() == CV_8UC3 || downscaledImage.type() == CV_8UC1)) {
			QSize oldSize = this->image.size();
			mat = image;
			ImageView::shallowCopyMatToImage(mat, this->image);
			if (this->image.size() != oldSize) {
				resetMask();
				hundredPercentZoomMode = false;
			}
			downsampledMat = downscaledImage;
			ImageView::shallowCopyMatToImage(downsampledMat, downsampledImage);
			isMat = true;
			hasMat = true;
			if (hasMat && useGpu) {
				mat.copyTo(uMat);
				hasUmat = true;
			}

			imageAssigned = true;
			update();
		} else {
			std::cerr << "Image View: cannot assign image and downsampled preview because at least one is of unsupported type (" << image.type() << " and " << downscaledImage.type() << ")." << std::endl;
		}
	}

	///Removes the image.
	void ImageView::resetImage() {
		image = QImage();
		imageAssigned = false;
		update();
	}

	///Returns \c true if an image is assigned, false otherwise.
	bool ImageView::getImageAssigned() const {
		return imageAssigned;
	}

	///Maps a point in widget coordinates to image coordinates of the currently assigned image
	QPointF ImageView::mapToImageCoordinates(QPointF pointInWidgetCoordinates) const {
		if (imageAssigned) {
			QPointF result = getTransform().inverted().map(pointInWidgetCoordinates);
			if (result.x() >= 0 && result.y() >= 0 && result.x() < double(image.width()) && result.y() < double(image.height())) {
				return result;
			}
			return QPointF();
		}
		return QPointF();
	}

	///Returns the magnification factor at which the image is dispayed, 1 means the image is at a 100% view and one image pixel corresponds to one pixel of the display.
	double ImageView::getCurrentPreviewScalingFactor() const {
		if (imageAssigned) {
			return std::pow(zoomBasis, zoomExponent) * getWindowScalingFactor();
		} else {
			return -1;
		}
	}

	///Specifies if the image will be resampled with a high quality algorithm when it's displayed with a magnificaiton smaller than 1.
	void ImageView::setUseHighQualityDownscaling(bool value) {
		useHighQualityDownscaling = value;
		updateResizedImage();
		update();
	}

	///Returns \c true if high quality downscaling is enabled, \c false otherwise.
	bool ImageView::getUseHighQualityDownscaling() {
		return useHighQualityDownscaling;
	}

	///Specifies if the sampling will be done bilinear or nearest neighbour when the iamge is displayed at a magnification greater than 1.
	void ImageView::setUseSmoothTransform(bool value) {
		useSmoothTransform = value;
		update();
	}

	///Returns \c true if bilinear sampling is enabled, \c false otherwise.
	bool ImageView::getUseSmoothTransform() const {
		return useSmoothTransform;
	}

	///If enabled the image will be unsharped masked after it has been downsampled to the current zoom level.
	/**
	* When resizing images to lower resolutions their sharpness impression might suffer.
	* By enabling this feature images will be sharpened after they have resampled to a
	* smaller size. The strenght and radius of this sharpening filter can be set via
	* \c ImageView::setPostResizeSharpeningStrength(double value) and
	* \c ImageView::setPostResizeSharpeningRadius(double value). If the zoom level is
	* at a level at which the image does not have to be downsampled, no sharpening
	* filter will be applied.
	*/
	void ImageView::setEnablePostResizeSharpening(bool value) {
		enablePostResizeSharpening = value;
		updateResizedImage();
		update();
	}

	///Returns \c true if the post resize sharpening is enabled, \c false otherwise.
	bool ImageView::getEnablePostResizeSharpening() {
		return enablePostResizeSharpening;
	}

	///Sets the strength value of the post-resize unsharp masking filter to \p value.
	void ImageView::setPostResizeSharpeningStrength(double value) {
		postResizeSharpeningStrength = value;
		updateResizedImage();
		update();
	}

	///Returns the strength value of the post-resize unsharp masking filter.
	double ImageView::getPostResizeSharpeningStrength() {
		return postResizeSharpeningStrength;
	}

	///Sets the radius value of the post-resize unsharp masking filter to \p value.
	void ImageView::setPostResizeSharpeningRadius(double value) {
		postResizeSharpeningRadius = value;
		updateResizedImage();
		update();
	}

	///Returns the radius value of the post-resize unsharp masking filter.
	double ImageView::getPostResizeSharpeningRadius() {
		return postResizeSharpeningRadius;
	}

	///Sets all parameters for the post resize sharpening at once.
	/**
	* The advantage of using this function instead of setting the three
	* parameters separately is that the imageView will only have to update once,
	* resulting in better performance.
	*/
	void ImageView::setPostResizeSharpening(bool enable, double strength, double radius) {
		enablePostResizeSharpening = enable;
		postResizeSharpeningStrength = strength;
		postResizeSharpeningRadius = radius;
		updateResizedImage();
		update();
	}

	///Enables or disables the ability to set new points and the ability to move already set ones; if adding points is enabled, manipulation of the polyline will be disabled.
	void ImageView::setPointEditing(bool enablePointAdding, bool enablePointManipulation) {
		pointEditingActive = enablePointAdding;
		pointManipulationActive = enablePointManipulation;
		if (enablePointAdding || enablePointManipulation)renderPoints = true;
		if (enablePointAdding && polylineManipulationActive) {
			std::cout << "Point adding was enabled, thus polyline manipulation will be disabled." << std::endl;
			polylineManipulationActive = false;
		}
	}

	///Specpfies whether points are rendered or not.
	void ImageView::setRenderPoints(bool value) {
		renderPoints = value;
		update();
	}

	///Returns the currently set points.
	const std::vector<QPointF>& ImageView::getPoints() const {
		return points;
	}

	///Sets the points to \p points.
	void ImageView::setPoints(const std::vector<QPointF>& points) {
		this->points = points;
		update();
	}

	///Sets the points to \p points.
	void ImageView::setPoints(std::vector<QPointF>&& points) {
		this->points = std::move(points);
		update();
	}

	///Adds the point \p point.
	void ImageView::addPoint(const QPointF& point) {
		points.push_back(point);
		update();
	}

	///Deletes all points which are outside the image, might happen when new image is assigned.
	void ImageView::deleteOutsidePoints() {
		for (std::vector<QPointF>::iterator point = points.begin(); point != points.end();) {
			if (point->x() < 0 || point->x() >= image.width() || point->y() < 0 || point->y() >= image.height()) {
				emit(userDeletedPoint(point - points.begin()));
				point = points.erase(point);
			} else {
				++point;
			}
		}
		update();
	}

	///Specifies whether it's possible to do overlay painting or not.
	/**
	 * Painting allows the user, for example, to mask certain areas.
	 */
	void ImageView::setPaintingActive(bool value) {
		if (value == true && !maskInitialized && imageAssigned) {
			mask = QBitmap(image.size());
			mask.fill(Qt::color0);
			maskInitialized = true;
		}
		if (value == false || imageAssigned) {
			paintingActive = value;
		}
	}

	///Specifies whether brush size visualization is enabled or not.
	/**
	 * If this is enabled a brush of the currently assigned brush size will be displayed in the
	 * center of the \c ImageView. Use \c setBrushRadius(int value) to set the brush size. This can
	 * be used to provide feedback to the user as to how large the brush currently is e.g. when
	 * its size is being changed.
	 */
	void ImageView::setVisualizeBrushSize(bool value) {
		visualizeBrushSize = value;
		update();
	}

	///Returns the mask that has been painted by the user.
	const QBitmap& ImageView::getMask() const {
		return mask;
	}

	///Sets a mask that will be displayed as a half-transparent overlay.
	/**
	 * This mask is not related to the panting the user does; instead it is an additional layer.
	 * This overload does a shallow copy assignment.
	 */
	void ImageView::setOverlayMask(const QBitmap& mask) {
		overlayMask = mask;
		overlayMaskSet = true;
		update();
	}

	///Sets a mask that will be displayed as a half-transparent overlay.
	/**
	* This mask is not related to the panting the user does; instead it is an additional layer.
	* This overload does a move assignment.
	*/
	void ImageView::setOverlayMask(QBitmap&& mask) {
		overlayMask = std::move(mask);
		overlayMaskSet = true;
		update();
	}

	///Specifies whether the assigned overlay mask is rendered or not.
	void ImageView::setRenderOverlayMask(bool value) {
		renderOverlayMask = value;
		update();
	}

	void ImageView::setRenderRectangle(bool value) {
		renderRectangle = value;
		update();
	}

	void ImageView::setRectangle(QRectF rectangle) {
		this->rectangle = rectangle;
		update();
	}

	///Specifies whether the assigned polyline is rendered or not.
	void ImageView::setRenderPolyline(bool value) {
		renderPolyline = value;
		update();
	}

	///Assigns a polyline that can be overlayed.
	void ImageView::setPolyline(std::vector<QPointF> border) {
		polyline = border;
		polylineAssigned = true;
		update();
	}

	///Enables or disables the ability to edit the polyline, will disable the ability to add points.
	void ImageView::setPolylineEditingActive(bool value) {
		polylineManipulationActive = value;
		if (value && pointEditingActive) {
			std::cout << "Polyline editing was enabled, thus point adding will be disabled." << std::endl;
			pointEditingActive = false;
		}
	}

	///Returns the polyline as it currently is.
	const std::vector<QPointF>& ImageView::getPolyline() const {
		return polyline;
	}

	///Sets the colour that the polyline is rendered in.
	void ImageView::setPolylineColor(QColor color) {
		polylineColor = color;
		update();
	}

	///Registers any callable target so it will be called at the end of the \c paintEvent method.
	/**
	* This method can be used to register any \c std::function as post-paint function.
	* Also function pointers or lambdas can be passed. They then will be implicitly converted.
	* The corresponding function will be called at the end of the \c paintEvent method.
	* To that function the current widget is passed as a \c QPainter object which enables custom
	* drawing on top of the widget, e.g. to display additional information.
	*/
	void ImageView::setExternalPostPaintFunction(std::function<void(QPainter&)> const& function) {
		externalPostPaint = function;
		externalPostPaintFunctionAssigned = true;
	}

	//Removes any assigned post-paint function, which then is no longer invoked.
	void ImageView::removeExternalPostPaintFunction() {
		externalPostPaintFunctionAssigned = false;
	}

	//========================================================================= Public Slots =========================================================================\\

	///Zooms the viewport in one step.
	void ImageView::zoomInKey() {
		QPointF center = QPointF(double(width()) / 2.0, double(height()) / 2.0);
		if (underMouse()) center = mapFromGlobal(QCursor::pos());
		zoomBy(1, center);
	}

	///Zooms the viewport out one step.
	void ImageView::zoomOutKey() {
		QPointF center = QPointF(double(width()) / 2.0, double(height()) / 2.0);
		if (underMouse()) center = mapFromGlobal(QCursor::pos());
		zoomBy(-1, center);
	}

	///Resets the mask the user is painting, does not affect the overlay mask.
	void ImageView::resetMask() {
		if (maskInitialized) {
			mask = QBitmap(image.size());
			mask.fill(Qt::color0);
			update();
		}
	}

	///Sets the radius of the brush to \p value.
	void ImageView::setBrushRadius(int value) {
		brushRadius = value;
		if (visualizeBrushSize) {
			update();
		}
	}

	///Displays the image at 100% magnification; the point \p center (in widget screen coordinates) will be centered.
	void ImageView::zoomToHundredPercent(QPointF center) {
		if (imageAssigned) {
			if (center == QPointF()) {
				center = QPointF(width() / 2, height() / 2);
			}
			QPointF mousePositionCoordinateBefore = getTransform().inverted().map(center);
			double desiredZoomFactor = 1 / getWindowScalingFactor();
			zoomExponent = log(desiredZoomFactor) / log(zoomBasis);
			QPointF mousePositionCoordinateAfter = getTransform().inverted().map(center);
			//remove the rotation from the delta
			QPointF mouseDelta = getTransformRotateOnly().map(mousePositionCoordinateAfter - mousePositionCoordinateBefore);
			panOffset += mouseDelta;
			hundredPercentZoomMode = true;
			enforcePanConstraints();
			updateResizedImage();
			update();
		}
	}

	void ImageView::resetZoom() {
		zoomExponent = 0;
		hundredPercentZoomMode = false;
		enforcePanConstraints();
		updateResizedImage();
		update();
	}

	///Deletes the point at index \p index.
	void ImageView::deletePoint(int index) {
		if (index >= 0 && index < points.size()) {
			points.erase(points.begin() + index);
			update();
		}
	}

	///Removes all the set points.
	void ImageView::resetPoints() {
		points.clear();
		update();
	}

	///Inverts the colour that the assigned polyline is rendered in.
	void ImageView::invertPolylineColor() {
		polylineColor = QColor(255 - polylineColor.red(), 255 - polylineColor.green(), 255 - polylineColor.blue());
		update();
	}

	//========================================================================= Protected =========================================================================\\

	void ImageView::showEvent(QShowEvent * e) {
		enforcePanConstraints();
	}

	void ImageView::mousePressEvent(QMouseEvent *e) {
		lastMousePosition = e->pos();
		screenId = qApp->desktop()->screenNumber(QCursor::pos());
		initialMousePosition = e->pos();
		infinitePanLastInitialMousePosition = initialMousePosition;

		if (e->modifiers() & Qt::AltModifier && polylineManipulationActive && polylineAssigned) {
			//span a selection rectangle
			polylineSelected = true;
			selectionRectangle = QRectF(e->pos(), e->pos());
			if (!(e->modifiers() & Qt::ControlModifier)) {
				polylineSelectedPoints.clear();
			}
			spanningSelectionRectangle = true;
		} else {
			//check for close points to grab
			IndexWithDistance closestPoint = closestGrabbablePoint(e->pos());
			IndexWithDistance closestPolylinePoint = closestGrabbablePolylinePoint(e->pos());
			double polylineSelectionDistance = smallestDistanceToPolylineSelection(e->pos());
			if (closestPoint.index >= 0 && (closestPolylinePoint.index <= 0 || closestPoint.distance < closestPolylinePoint.distance || !polylineSelected) && (closestPoint.distance < polylineSelectionDistance || polylineSelectedPoints.size() == 0 || !polylineSelected) && pointManipulationActive) {
				//grab a point
				grabbedPointIndex = closestPoint.index;
				qApp->setOverrideCursor(QCursor(Qt::BlankCursor));
				pointGrabbed = true;
			} else if ((closestPolylinePoint.index >= 0 || (polylineSelectionDistance <= polylinePointGrabTolerance && polylineSelectedPoints.size() > 0)) && polylineManipulationActive && polylineSelected) {
				//polyline editing
				if (((polylineSelectionDistance <= closestPolylinePoint.distance || polylineSelectedPoints.find(closestPolylinePoint.index) != polylineSelectedPoints.end()) && polylineSelectedPoints.size() > 0) || closestPolylinePoint.index < 0 && !(e->modifiers() & Qt::ControlModifier) && !(e->modifiers() & Qt::ShiftModifier)) {
					//start moving the selection
					polylinePointGrabbed = true;
				} else {
					if (closestPolylinePoint.index >= 0) {
						if (e->modifiers() & Qt::ShiftModifier) {
							if (!(e->modifiers() & Qt::ControlModifier)) polylineSelectedPoints.clear();
							//add all points inbetween the current point and the last point
							int largerIndex = std::max(closestPolylinePoint.index, polylineLastAddedPoint);
							int smallerIndex = std::min(closestPolylinePoint.index, polylineLastAddedPoint);
							for (int index = smallerIndex; index <= largerIndex; ++index) {
								polylineSelectedPoints.insert(index);
							}
						} else {
							if (e->modifiers() & Qt::ControlModifier) {
								//add point to selected points or remove it
								std::set<int>::iterator point = polylineSelectedPoints.find(closestPolylinePoint.index);
								if (point == polylineSelectedPoints.end()) {
									polylineSelectedPoints.insert(closestPolylinePoint.index);
								} else {
									polylineSelectedPoints.erase(point);
								}
								polylineLastAddedPoint = closestPolylinePoint.index;
							} else {
								polylineSelectedPoints.clear();
								//grab polyline point
								polylineSelectedPoints.insert(closestPolylinePoint.index);
								polylineLastAddedPoint = closestPolylinePoint.index;
								polylinePointGrabbed = true;
							}
						}
					}
				}
			} else if ((!paintingActive && e->button() != Qt::MiddleButton) || (paintingActive && e->button() == Qt::MiddleButton)) {
				//dragging
				dragging = true;		
			} else if (e->button() == Qt::MiddleButton) {
				//pan-zooming
				panZooming = true;
				panZoomingInitialPanOffset = panOffset;
				panZoomingInitialZoomExponent = zoomExponent;
				qApp->setOverrideCursor(QCursor(Qt::SizeVerCursor));
			} else if (imageAssigned) {
				//painting
				painting = true;

				//paint a circle
				QPainter canvas(&mask);
				canvas.setPen(Qt::NoPen);
				if (e->button() == Qt::LeftButton) {
					canvas.setBrush(QBrush(Qt::color1));
				} else {
					canvas.setBrush(QBrush(Qt::color0));
				}
				QTransform transform = getTransform().inverted();
				canvas.drawEllipse(transform.map(QPointF(e->pos())), brushRadius, brushRadius);
				update();
			}
		}

		moved = false;
	}

	void ImageView::mouseMoveEvent(QMouseEvent *e) {
		moved = true;
		bool dontUpdateLastMousePosition = false;

		if (dragging || pointGrabbed || polylinePointGrabbed) {
			QPointF deltaRotated = getTransformScaleRotateOnly().inverted().map((e->pos() - lastMousePosition));
			QPointF deltaScaled = getTransformScaleOnly().inverted().map((e->pos() - lastMousePosition));
			if (dragging) {
				//dragging
				qApp->setOverrideCursor(QCursor(Qt::ClosedHandCursor));
				panOffset += deltaScaled;
				enforcePanConstraints();
				//for infinite panning
				QPoint globalPos = QCursor::pos();
				QRect screen = QApplication::desktop()->screen(screenId)->geometry();
				QPoint newPos;
				if (globalPos.y() >= screen.bottom()) {
					newPos = QPoint(globalPos.x(), screen.top() + 1);
				} else if (globalPos.y() <= screen.top()) {
					newPos = QPoint(globalPos.x(), screen.bottom() - 1);
				} else if (globalPos.x() >= screen.right()) {
					newPos = QPoint(screen.left() + 1, globalPos.y());
				} else if (globalPos.x() <= screen.left()) {
					newPos = QPoint(screen.right() - 1, globalPos.y());
				}
				if (newPos != QPoint()) {
					lastMousePosition = mapFromGlobal(newPos);
					dontUpdateLastMousePosition = true;
					QCursor::setPos(newPos);
				}
			} else if (pointGrabbed) {
				//editing points
				points[grabbedPointIndex] += deltaRotated;
				if (e->pos().x() < 0 || e->pos().y() < 0 || e->pos().x() > width() || e->pos().y() > height() || points[grabbedPointIndex].x() < 0 || points[grabbedPointIndex].y() < 0 || points[grabbedPointIndex].x() >= image.width() || points[grabbedPointIndex].y() >= image.height()) {
					showPointDeletionWarning = true;
					qApp->setOverrideCursor(QCursor(Qt::ArrowCursor));
				} else {
					showPointDeletionWarning = false;
					qApp->setOverrideCursor(QCursor(Qt::BlankCursor));
				}
				emit pointModified();
			} else {
				//editing polyline points
				for (int index : polylineSelectedPoints) {
					polyline[index] += deltaRotated;
					if (polyline[index].x() < 0)polyline[index].setX(0);
					if (polyline[index].x() > image.width())polyline[index].setX(image.width());
					if (polyline[index].y() < 0)polyline[index].setY(0);
					if (polyline[index].y() > image.height())polyline[index].setY(image.height());
				}
			}
			update();
		} else if (spanningSelectionRectangle) {
			selectionRectangle.setBottomLeft(e->pos());
			QTransform transform = getTransform();
			selectionRectanglePoints.clear();
			for (int point = 0; point < polyline.size(); ++point) {
				QPointF transformedPoint = transform.map(polyline[point]);
				if (selectionRectangle.contains(transformedPoint)) {
					selectionRectanglePoints.insert(point);
					polylineLastAddedPoint = point;
				}
			}
			update();
		}

		if (paintingActive) {
			brushPosition = e->pos();
			if (painting) {
				//draw a line from last mouse position to the current
				QPainter canvas(&mask);
				QPen pen;
				if (e->buttons() == Qt::LeftButton) {
					pen.setColor(Qt::color1);
				} else {
					pen.setColor(Qt::color0);
				}
				pen.setWidth(2 * brushRadius);
				pen.setCapStyle(Qt::RoundCap);
				canvas.setPen(pen);
				QTransform transform = getTransform().inverted();
				canvas.drawLine(transform.map(lastMousePosition), transform.map(e->pos()));
			}
			update();
		}

		if (panZooming) {
			zoomExponent = panZoomingInitialZoomExponent;
			panOffset = panZoomingInitialPanOffset;
			double delta = (infinitePanLastInitialMousePosition - e->pos()).y() * (-0.025);
			zoomBy(delta, initialMousePosition);
			//for infinite pan zooming
			QPoint globalPos = QCursor::pos();
			QRect screen = QApplication::desktop()->screen(screenId)->geometry();
			QPoint newPos;
			if (globalPos.y() >= screen.bottom()) {
				newPos = QPoint(globalPos.x(), screen.top() + 1);
			} else if (globalPos.y() <= screen.top()) {
				newPos = QPoint(globalPos.x(), screen.bottom() - 1);
			} else if (globalPos.x() >= screen.right()) {
				newPos = QPoint(screen.left() + 1, globalPos.y());
			} else if (globalPos.x() <= screen.left()) {
				newPos = QPoint(screen.right() - 1, globalPos.y());
			}
			if (newPos != QPoint()) {
				infinitePanLastInitialMousePosition = mapFromGlobal(newPos);
				panZoomingInitialPanOffset = panOffset;
				panZoomingInitialZoomExponent = zoomExponent;
				QCursor::setPos(newPos);
			}
			//doesn't work as expected
			//QCursor::setPos(mapToGlobal(lastMousePosition.toPoint()));
		}

		if (!dragging && !painting && !pointGrabbed && !spanningSelectionRectangle && !panZooming) {
			//check for close points to grab
			if (pointManipulationActive) {
				if (closestGrabbablePoint(e->pos()).index >= 0) {
					qApp->setOverrideCursor(QCursor(Qt::OpenHandCursor));
				} else {
					qApp->setOverrideCursor(QCursor(Qt::ArrowCursor));
				}
			}
		}

		if ((dragging || painting || pointGrabbed || polylinePointGrabbed) && !dontUpdateLastMousePosition) {
			lastMousePosition = e->pos();
		}
	}

	void ImageView::mouseReleaseEvent(QMouseEvent *e) {

		//clicking points
		if (pointEditingActive && imageAssigned && !moved) {
			//this was a click, add a point
			QTransform transform = getTransform();
			QPointF clickedPoint = e->pos();
			QPointF worldPoint = transform.inverted().map(clickedPoint);
			if (worldPoint.x() >= 0 && worldPoint.x() <= image.width() && worldPoint.y() >= 0 && worldPoint.y() <= image.height()) {
				points.push_back(worldPoint);
				std::cout << "Point added: " << worldPoint.x() << "  " << worldPoint.y() << std::endl;
				emit pointModified();
			}
		} else if (!pointEditingActive && !moved && imageAssigned) {
			if (polylineManipulationActive && e->button() != Qt::RightButton) {
				//this was a click, select or unselect polyline
				if (smallestDistanceToPolyline(e->pos()) <= polylinePointGrabTolerance) {
					//clicked close enough to a point, select line
					polylineSelected = true;
				} else {
					//clicked somewehere else, deselect it
					polylineSelected = false;
					polylineSelectedPoints.clear();
				}
			}

			if (e->button() == Qt::RightButton && rightClickForHundredPercentView) {
				//zoom to 100%
				if (hundredPercentZoomMode) {
					resetZoom();
				} else {
					zoomToHundredPercent(e->pos());
				}
			}

			//emit pixel click signal
			QTransform transform = getTransform();
			QPointF clickedPoint = e->pos();
			QPointF worldPoint = transform.inverted().map(clickedPoint);
			emit(pixelClicked(QPoint(std::floor(worldPoint.x()), std::floor(worldPoint.y()))));
		}

		if (pointGrabbed) {
			if (e->pos().x() < 0 || e->pos().y() < 0 || e->pos().x() > width() || e->pos().y() > height() || points[grabbedPointIndex].x() < 0 || points[grabbedPointIndex].y() < 0 || points[grabbedPointIndex].x() >= image.width() || points[grabbedPointIndex].y() >= image.height()) {
				deletePoint(grabbedPointIndex);
				emit(userDeletedPoint(grabbedPointIndex));
				showPointDeletionWarning = false;
				qApp->setOverrideCursor(QCursor(Qt::ArrowCursor));
			} else {
				qApp->setOverrideCursor(QCursor(Qt::OpenHandCursor));
			}
			pointGrabbed = false;
			emit pointModified();
		}

		if (polylinePointGrabbed) {
			polylinePointGrabbed = false;
			if (moved) emit polylineModified();
		}

		if (spanningSelectionRectangle) {
			spanningSelectionRectangle = false;
			polylineSelectedPoints.insert(selectionRectanglePoints.begin(), selectionRectanglePoints.end());
			selectionRectanglePoints.clear();
		}

		if (dragging) {
			if (paintingActive) {
				qApp->setOverrideCursor(QCursor(Qt::BlankCursor));
			} else {
				qApp->setOverrideCursor(QCursor(Qt::ArrowCursor));
			}
			dragging = false;
		}
		painting = false;

		if (panZooming) {
			qApp->setOverrideCursor(QCursor(Qt::ArrowCursor));
			panZooming = false;
		}

		update();
	}

	void ImageView::mouseDoubleClickEvent(QMouseEvent* e) {
		e->ignore();
	}

	void ImageView::wheelEvent(QWheelEvent* e) {
		if (!panZooming) {
			double divisor = 1;
			if (e->modifiers() & Qt::ControlModifier) {
				divisor = 600;
			} else if (!e->modifiers()) {
				divisor = 120;
			} else {
				e->ignore();
				return;
			}
			zoomBy(e->delta() / divisor, e->pos());
		}
		e->accept();
	}

	void ImageView::resizeEvent(QResizeEvent* e) {
		//maintain 100% view if in 100% view
		if (hundredPercentZoomMode) {
			QPointF center(width() / 2.0, height() / 2.0);
			zoomToHundredPercent(center);
		}
		updateResizedImage();
	}

	void ImageView::enterEvent(QEvent* e) {
		if (paintingActive) {
			qApp->setOverrideCursor(QCursor(Qt::BlankCursor));
		}
	}

	void ImageView::leaveEvent(QEvent* e) {
		qApp->setOverrideCursor(QCursor(Qt::ArrowCursor));
		if (paintingActive) {
			update();
		}
	}

	void ImageView::paintEvent(QPaintEvent* e) {
		QPainter canvas(this);
		canvas.setRenderHint(QPainter::Antialiasing, true);
		canvas.setRenderHint(QPainter::SmoothPixmapTransform, useSmoothTransform);
		QSize canvasSize = size();
		QTransform transform = getTransform();
		QPalette palette = qApp->palette();
		canvas.fillRect(0, 0, width(), height(), backgroundColor);

		//drawing of the image
		if (imageAssigned) {
			if (std::pow(zoomBasis, zoomExponent) * getWindowScalingFactor() >= 1 || !useHighQualityDownscaling) {
				canvas.setTransform(transform);
				canvas.drawImage(QPoint(0, 0), image);
			} else {
				canvas.setTransform(getTransformDownsampledImage());
				canvas.drawImage(QPoint(0, 0), downsampledImage);
			}
		}

		//drawing of the overlay mask
		if (overlayMaskSet && renderOverlayMask) {
			canvas.setTransform(transform);
			QImage image = overlayMask.toImage();
			image.setColor(Qt::color0, QColor(Qt::white).rgb());
			image.setColor(Qt::color1, Qt::transparent);
			canvas.setOpacity(0.9);
			canvas.setRenderHint(QPainter::SmoothPixmapTransform, false);
			canvas.drawImage(QPoint(0, 0), image);
			canvas.setRenderHint(QPainter::SmoothPixmapTransform, true);
			canvas.setOpacity(1);
		}

		//drawing of bounds (rectangle) overlay
		if (imageAssigned && renderRectangle) {
			QPixmap rect = QPixmap(canvasSize);
			rect.fill(Qt::transparent);
			QRectF imageArea(QPointF(0, 0), image.size());
			imageArea = transform.mapRect(imageArea);
			QPainter p(&rect);
			p.setRenderHint(QPainter::Antialiasing, true);
			p.setPen(Qt::NoPen);
			p.setBrush(QColor(0, 0, 0, 100));
			p.drawRect(imageArea);
			p.setBrush(QBrush(Qt::transparent));
			p.setCompositionMode(QPainter::CompositionMode_SourceOut);
			QRectF eraseRect = transform.mapRect(rectangle);
			p.drawRect(eraseRect);
			canvas.resetTransform();
			canvas.drawPixmap(0, 0, rect);
		}

		//drawing of the mask that is currently painted
		if (paintingActive) {
			canvas.setTransform(transform);
			QImage image = mask.toImage();
			image.setColor(Qt::color0, Qt::transparent);
			image.setColor(Qt::color1, QColor(Qt::red).rgb());
			canvas.setOpacity(0.5);
			canvas.setRenderHint(QPainter::SmoothPixmapTransform, false);
			canvas.drawImage(QPoint(0, 0), image);
			canvas.setRenderHint(QPainter::SmoothPixmapTransform, true);
			canvas.setOpacity(1);
		}

		//drawing of the polyline if assigned
		if (polylineAssigned && renderPolyline && polyline.size() > 0) {
			canvas.setRenderHint(QPainter::Antialiasing, true);
			canvas.setTransform(transform);
			QPen linePen = QPen(polylineColor);
			linePen.setJoinStyle(Qt::MiterJoin);
			if (!polylineSelected || !polylineManipulationActive) linePen.setWidth(3);
			QBrush brush = QBrush(polylineColor);
			linePen.setCosmetic(true);
			canvas.setPen(linePen);
			if (polyline.size() > 1) {
				canvas.drawPolyline(polyline.data(), polyline.size());
				if (polylineManipulationActive && polylineSelected) {
					canvas.resetTransform();
					const int squareSize = 4;
					const int squareOffset = squareSize / 2;
					for (int point = 0; point < polyline.size(); ++point) {
						if (selectionRectanglePoints.find(point) != selectionRectanglePoints.end() || polylineSelectedPoints.find(point) != polylineSelectedPoints.end()) {
							canvas.setBrush(brush);
						} else {
							canvas.setBrush(Qt::NoBrush);
						}
						QPointF transformedPoint = transform.map(polyline[point]);
						canvas.drawRect(transformedPoint.x() - squareOffset, transformedPoint.y() - squareOffset, squareSize, squareSize);
					}
				}
			} else {
				canvas.drawPoint(polyline[0]);
			}
		}

		//draw selection rectangle when selecting
		if (spanningSelectionRectangle) {
			canvas.resetTransform();
			canvas.setPen(QPen(Qt::darkGray, 1, Qt::DashDotLine));
			canvas.setBrush(Qt::NoBrush);
			canvas.drawRect(selectionRectangle);
		}

		//drawing of the points
		if (renderPoints) {
			canvas.resetTransform();
			QPen pen(Qt::black, 2);
			pen.setCosmetic(true);
			QPen textPen(palette.buttonText().color());
			QBrush brush(Qt::white);
			canvas.setBrush(brush);
			QFont font;
			font.setPointSize(10);
			canvas.setFont(font);
			QColor base = palette.base().color();
			base.setAlpha(200);
			canvas.setBackground(base);
			canvas.setBackgroundMode(Qt::OpaqueMode);
			QPointF transformedPoint;
			for (int point = 0; point < points.size(); ++point) {
				transformedPoint = transform.map(points[point]);
				canvas.setPen(pen);
				canvas.drawEllipse(transformedPoint, 5, 5);
				canvas.setPen(textPen);
				canvas.drawText(transformedPoint + QPointF(7.0, 14.0), QString::number(point + 1));
			}
			if (pointEditingActive) {
				canvas.setPen(textPen);
				QString statusMessage = ((points.size() != 1) ? QString(tr("There are ")) : QString(tr("There is "))) + QString::number(points.size()) + ((points.size() != 1) ? QString(tr(" points set.")) : QString(tr(" point set.")));
				canvas.drawText(QPoint(20, height() - 15), statusMessage);
			}
		}

		//if painting active draw brush outline
		if (paintingActive && underMouse() && !dragging) {
			canvas.resetTransform();
			double scalingFactor = pow(zoomBasis, zoomExponent) * getWindowScalingFactor();
			canvas.setBrush(Qt::NoBrush);
			canvas.setPen(QPen(Qt::darkGray, 1));
			canvas.drawEllipse(brushPosition, brushRadius*scalingFactor, brushRadius*scalingFactor);
		}

		//visualization of the brush size (e.g. when changing it)
		if (visualizeBrushSize) {
			canvas.resetTransform();
			canvas.setPen(QPen(Qt::darkGray));
			canvas.setBrush(Qt::NoBrush);
			double scalingFactor = pow(zoomBasis, zoomExponent) * getWindowScalingFactor();
			canvas.drawEllipse(QPointF((double)width() / 2.0, (double)height() / 2.0), brushRadius*scalingFactor, brushRadius*scalingFactor);
		}

		//the point deletion warning
		if (showPointDeletionWarning) {
			canvas.resetTransform();
			QFont font;
			font.setPointSize(20);
			canvas.setFont(font);
			QColor base = palette.base().color();
			base.setAlpha(200);
			canvas.setBackground(base);
			canvas.setBackgroundMode(Qt::OpaqueMode);
			QPen textPen(palette.buttonText().color());
			canvas.setPen(textPen);
			canvas.drawText(QRect(0, 0, width(), height()), Qt::AlignCenter, QString(tr("Release to delete point")));
		}

		//add a contour
		if (interfaceOutline) {
			canvas.resetTransform();
			canvas.setRenderHint(QPainter::Antialiasing, 0);
			QColor strokeColour;
			if (hasFocus()) {
				strokeColour = palette.highlight().color();
			} else {
				strokeColour = palette.base().color();
				strokeColour.setRed(strokeColour.red() / 2);
				strokeColour.setGreen(strokeColour.green() / 2);
				strokeColour.setBlue(strokeColour.blue() / 2);
			}
			canvas.setPen(QPen(strokeColour, 1));
			canvas.setBrush(Qt::NoBrush);
			canvas.drawRect(0, 0, width() - 1, height() - 1);
		}

		//call external post paint function
		if (externalPostPaintFunctionAssigned) {
			canvas.resetTransform();
			externalPostPaint(canvas);
		}
	}

	void ImageView::keyPressEvent(QKeyEvent * e) {
		if ((isVisible() && (underMouse() || e->key() == Qt::Key_X) && imageAssigned) || e->key() == Qt::Key_S) {
			if (e->key() == Qt::Key_Plus && !panZooming) {
				zoomInKey();
			} else if (e->key() == Qt::Key_Minus && !panZooming) {
				zoomOutKey();
			} else if (e->key() == Qt::Key_S) {
				setUseSmoothTransform(!useSmoothTransform);
			} else if (e->key() == Qt::Key_X && polylineAssigned && renderPolyline) {
				invertPolylineColor();
			} else {
				e->ignore();
			}
		} else {
			e->ignore();
		}
	}

	bool ImageView::eventFilter(QObject *object, QEvent *e) {
		if (e->type() == QEvent::KeyPress) {
			QKeyEvent* keyEvent = (QKeyEvent*)e;
			if ((keyEvent->key() == Qt::Key_Plus || keyEvent->key() == Qt::Key_Minus) && isVisible() && underMouse() && imageAssigned) {
				keyPressEvent(keyEvent);
				return true;
			} else if (keyEvent->key() == Qt::Key_S) {
				keyPressEvent(keyEvent);
			} else if (keyEvent->key() == Qt::Key_X && isVisible() && imageAssigned && polylineAssigned && renderPolyline) {
				keyPressEvent(keyEvent);
				return true;
			}
		}
		return false;
	}

	//========================================================================= Private =========================================================================\\

	double ImageView::getEffectiveImageWidth() const {
		return std::abs(std::cos(viewRotation * M_PI / 180)) * (double)image.width() + std::abs(std::sin(viewRotation * M_PI / 180)) * (double)image.height();
	}

	double ImageView::getEffectiveImageHeight() const {
		return std::abs(std::cos(viewRotation * M_PI / 180)) * (double)image.height() + std::abs(std::sin(viewRotation * M_PI / 180)) * (double)image.width();
	}

	double ImageView::getWindowScalingFactor() const {
		if (imageAssigned && image.width() != 0 && image.height() != 0) {
			double imageWidth = getEffectiveImageWidth();
			double imageHeight = getEffectiveImageHeight();
			double scalingFactor = std::min((double)size().width() / imageWidth, (double)size().height() / imageHeight);
			if (preventMagnificationInDefaultZoom && scalingFactor > 1) {
				return 1;
			} else {
				return scalingFactor;
			}
		} else {
			return 1;
		}
	}

	QTransform ImageView::getTransform() const {
		//makes the map always fill the whole interface element
		double factor = getWindowScalingFactor();
		double zoomFactor = pow(zoomBasis, zoomExponent);
		double centeringOffsetX = (double)image.width() / 2;
		double centeringOffsetY = (double)image.height() / 2;
		double transX = ((width() / factor) - image.width()) / 2;
		double transY = ((height() / factor) - image.height()) / 2;
		//those transforms are performed in inverse order, so read bottom - up
		QTransform transform;
		//apply the window scaling factor
		transform.scale(factor, factor);
		//translation that ensures that the image is always centered in the view
		transform.translate(transX, transY);
		//move left upper corner back to 0, 0
		transform.translate(centeringOffsetX, centeringOffsetY);
		//apply users zoom
		transform.scale(zoomFactor, zoomFactor);
		//apple users pan
		transform.translate(panOffset.x(), panOffset.y());
		//rotate the view
		transform.rotate(viewRotation);
		//move image center to 0, 0
		transform.translate((-1)*centeringOffsetX, (-1)*centeringOffsetY);

		return transform;
	}

	QTransform ImageView::getTransformDownsampledImage() const {
		//makes the map always fill the whole interface element
		double factor = getWindowScalingFactor();
		double zoomFactor = pow(zoomBasis, zoomExponent);
		/*Here we can do integer division for the centering offset because this function is only called
		when the image is displayed with negative magnificaiton. The Error of ca. 0.5 pixels can be
		accepted in this case because it will not be visible very much. Floating point numbers in
		contrast whould result in a slightly blurred image when the image is rotated and one ofset
		is integer while the other one is a fraction (because of the difference when moving the image to
		the origin and moving the image back would be < 1px due to the intermediate roation)*/
		double centeringOffsetX = downsampledImage.width() / 2;
		double centeringOffsetY = downsampledImage.height() / 2;
		double transX = ((width()) - downsampledImage.width()) / 2;
		double transY = ((height()) - downsampledImage.height()) / 2;
		//those transforms are performed in inverse order, so read bottom - up
		QTransform transform;
		//apply the window scaling factor
		//transform.scale(factor, factor);
		//translation that ensures that the image is always centered in the view
		transform.translate(transX, transY);
		//move left upper corner back to 0, 0
		transform.translate(centeringOffsetX, centeringOffsetY);
		//apply users zoom
		//transform.scale(zoomFactor, zoomFactor);
		//apple users pan
		transform.translate(panOffset.x() * zoomFactor * factor, panOffset.y() * zoomFactor * factor);
		//rotate the view
		transform.rotate(viewRotation);
		//move image center to 0, 0
		transform.translate((-1)*centeringOffsetX, (-1)*centeringOffsetY);

		return transform;
	}

	QTransform ImageView::getTransformScaleRotateOnly() const {
		double factor = getWindowScalingFactor();
		double zoomFactor = pow(zoomBasis, zoomExponent);
		//those transforms are performed in inverse order, so read bottom - up
		QTransform transform;
		//apply the window scaling factor
		transform.scale(factor, factor);
		//apply users zoom
		transform.scale(zoomFactor, zoomFactor);
		//rotate the view
		transform.rotate(viewRotation);
		return transform;
	}

	QTransform ImageView::getTransformScaleOnly() const {
		double factor = getWindowScalingFactor();
		double zoomFactor = pow(zoomBasis, zoomExponent);
		//those transforms are performed in inverse order, so read bottom - up
		QTransform transform;
		//apply the window scaling factor
		transform.scale(factor, factor);
		//apply users zoom
		transform.scale(zoomFactor, zoomFactor);
		return transform;
	}

	QTransform ImageView::getTransformRotateOnly() const {
		//those transforms are performed in inverse order, so read bottom - up
		QTransform transform;
		//rotate the view
		transform.rotate(viewRotation);
		return transform;
	}

	void ImageView::zoomBy(double delta, QPointF const& center) {
		if (imageAssigned) {
			QPointF mousePositionCoordinateBefore = getTransform().inverted().map(center);
			zoomExponent += delta;
			if (zoomExponent < 0)zoomExponent = 0;
			QPointF mousePositionCoordinateAfter = getTransform().inverted().map(center);
			//remove the rotation from the delta
			QPointF mouseDelta = getTransformRotateOnly().map(mousePositionCoordinateAfter - mousePositionCoordinateBefore);
			panOffset += mouseDelta;
			hundredPercentZoomMode = false;
			enforcePanConstraints();
			updateResizedImage();
			update();
		}
	}

	void ImageView::enforcePanConstraints() {
		double imageWidth = getEffectiveImageWidth();
		double imageHeight = getEffectiveImageHeight();
		double factor = getWindowScalingFactor();
		double zoomFactor = pow(zoomBasis, zoomExponent);
		double maxXOffset = (-1)*(((width() / factor / zoomFactor) - imageWidth) / 2);
		double maxYOffset = (-1)*(((height() / factor / zoomFactor) - imageHeight) / 2);
		maxXOffset = std::max(0.0, maxXOffset);
		maxYOffset = std::max(0.0, maxYOffset);
		if (panOffset.x() > maxXOffset)panOffset.setX(maxXOffset);
		if (panOffset.x() < (-1) * maxXOffset)panOffset.setX((-1) * maxXOffset);
		if (panOffset.y() > maxYOffset)panOffset.setY(maxYOffset);
		if (panOffset.y() < (-1) * maxYOffset)panOffset.setY((-1) * maxYOffset);
	}

	void ImageView::updateResizedImage() {
		if (useHighQualityDownscaling && imageAssigned) {
			double scalingFactor = std::pow(zoomBasis, zoomExponent) * getWindowScalingFactor();
			if (scalingFactor < 1) {
				if (!hasMat) {
					//alternative for QImages that could not be converted to a mat
					downsampledImage = image.scaledToWidth(image.width() * scalingFactor, Qt::SmoothTransformation);
				} else {

					bool fallBackToCpu = false;
					if (useGpu && OpenClAvailable()) {
						try {
							cv::resize(uMat, downsampledUmat, cv::Size(), scalingFactor, scalingFactor, cv::INTER_AREA);
							if (enablePostResizeSharpening) {
								ImageView::sharpen(downsampledUmat, postResizeSharpeningStrength, postResizeSharpeningRadius);
							}
							downsampledUmat.copyTo(downsampledMat);
						} catch (...) {
							//something went wrong, fall back to CPU
							fallBackToCpu = true;
						}
					}
					if (!useGpu || !OpenClAvailable() || fallBackToCpu) {
						cv::resize(mat, downsampledMat, cv::Size(), scalingFactor, scalingFactor, cv::INTER_AREA);
						if (enablePostResizeSharpening) {
							ImageView::sharpen(downsampledMat, postResizeSharpeningStrength, postResizeSharpeningRadius);
						}
					}
					ImageView::shallowCopyMatToImage(downsampledMat, downsampledImage);

				}
			}
		}
	}

	double ImageView::distance(const QPointF& point1, const QPointF& point2) {
		return std::sqrt(std::pow(point2.x() - point1.x(), 2) + std::pow(point2.y() - point1.y(), 2));
	}

	ImageView::IndexWithDistance ImageView::closestGrabbablePoint(QPointF const& mousePosition) const {
		if (points.size() > 0) {
			QTransform transform = getTransform();
			double smallestDistance = distance(transform.map(points[0]), mousePosition);
			double index = 0;
			for (int point = 1; point < points.size(); ++point) {
				double tmpDistance = distance(transform.map(points[point]), mousePosition);
				if (tmpDistance < smallestDistance) {
					smallestDistance = tmpDistance;
					index = point;
				}
			}
			if (smallestDistance < pointGrabTolerance) {
				return IndexWithDistance(index, smallestDistance);
			}
		}
		return IndexWithDistance(-1, 0);
	}

	ImageView::IndexWithDistance ImageView::closestGrabbablePolylinePoint(QPointF const& mousePosition) const {
		if (polyline.size() > 0) {
			QTransform transform = getTransform();
			double smallestDistance = distance(transform.map(polyline[0]), mousePosition);
			double index = 0;
			for (int point = 1; point < polyline.size(); ++point) {
				double tmpDistance = distance(transform.map(polyline[point]), mousePosition);
				if (tmpDistance < smallestDistance) {
					smallestDistance = tmpDistance;
					index = point;
				}
			}
			if (smallestDistance < pointGrabTolerance) {
				return IndexWithDistance(index, smallestDistance);
			}
		}
		return IndexWithDistance(-1, 0);
	}

	double ImageView::smallestDistanceToPolyline(QPointF const& mousePosition) const {
		if (polyline.size() > 0) {
			QTransform transform = getTransform();
			double smallestDistance = distance(polyline[0], mousePosition);
			if (polyline.size() > 1) {
				for (int point = 0; point < polyline.size() - 1; ++point) {
					QPointF point1 = transform.map(polyline[point]);
					QPointF point2 = transform.map(polyline[point + 1]);
					double d = ImageView::distanceOfPointToLineSegment(point1, point2, mousePosition);
					if (d < smallestDistance) smallestDistance = d;
				}
			}
			return smallestDistance;
		}
		return 0;
	}

	double ImageView::smallestDistanceToPolylineSelection(QPointF const& mousePosition) const {
		if (polyline.size() > 0) {
			QTransform transform = getTransform();
			double smallestDistance = -1;
			for (int index : polylineSelectedPoints) {
				QPointF point1 = transform.map(polyline[index]);
				double d;
				if (polylineSelectedPoints.find(index + 1) != polylineSelectedPoints.end()) {
					//check distance to line segment	
					QPointF point2 = transform.map(polyline[index + 1]);
					d = ImageView::distanceOfPointToLineSegment(point1, point2, mousePosition);
				} else {
					//check distance to point
					d = distance(point1, mousePosition);
				}
				if (d < smallestDistance || smallestDistance == -1) smallestDistance = d;
			}
			return smallestDistance;
		}
		return 0;
	}

	double ImageView::distanceOfPointToLineSegment(QPointF const& lineStart, QPointF const& lineEnd, QPointF const& point) {
		QVector2D pointConnection(lineEnd - lineStart);
		QVector2D lineNormal(pointConnection);
		double tmp = lineNormal.x();
		lineNormal.setX(lineNormal.y());
		lineNormal.setY(tmp);
		if (lineNormal.x() != 0) {
			lineNormal.setX(lineNormal.x() * (-1));
		} else {
			lineNormal.setX(lineNormal.y() * (-1));
		}
		lineNormal.normalize();
		QVector2D point1ToMouse(point - lineStart);
		QVector2D point2ToMouse(point - lineEnd);
		double smallestDistance;
		if (point1ToMouse.length() * std::abs(QVector2D::dotProduct(pointConnection, point1ToMouse) / (pointConnection.length() * point1ToMouse.length())) > pointConnection.length()) {
			//perpendicular is not on line segment
			smallestDistance = ImageView::distance(lineEnd, point);
		} else if (point2ToMouse.length() * std::abs(QVector2D::dotProduct((-1)*pointConnection, point2ToMouse) / (pointConnection.length() * point2ToMouse.length())) > pointConnection.length()) {
			//perpendicular is also not on line segment
			smallestDistance = ImageView::distance(lineStart, point);
		} else {
			smallestDistance = std::abs(QVector2D::dotProduct(lineNormal, point1ToMouse));
		}
		return smallestDistance;
	}

	void ImageView::sharpen(cv::Mat& image, double strength, double radius) {
		cv::Mat tmp;
		cv::GaussianBlur(image, tmp, cv::Size(0, 0), radius);
		cv::addWeighted(image, 1 + strength, tmp, -strength, 0, image);
	}

	void ImageView::sharpen(cv::UMat& image, double strength, double radius) {
		cv::UMat tmp;
		cv::GaussianBlur(image, tmp, cv::Size(0, 0), radius);
		cv::addWeighted(image, 1 + strength, tmp, -strength, 0, image);
	}

	bool ImageView::isConvertible(QImage::Format) {
		return (image.format() == QImage::Format_RGB888 ||
				image.format() == QImage::Format_Indexed8 ||
				image.format() == QImage::Format_Grayscale8 ||
				image.format() == QImage::Format_ARGB32 ||
				image.format() == QImage::Format_RGB32);
	}

	void ImageView::shallowCopyMatToImage(const cv::Mat& mat, QImage& destImage) {
		ImageView::matToImage(mat, destImage, false);
	}

	void ImageView::deepCopyMatToImage(const cv::Mat& mat, QImage& destImage) {
		ImageView::matToImage(mat, destImage, true);
	}

	void ImageView::shallowCopyImageToMat(const QImage& image, cv::Mat& destMat) {
		ImageView::imageToMat(image, destMat, false);
	}

	void ImageView::deepCopyImageToMat(const QImage& image, cv::Mat& destMat) {
		ImageView::imageToMat(image, destMat, true);
	}

	void ImageView::matToImage(const cv::Mat& mat, QImage& destImage, bool deepCopy) {
		if (mat.type() == CV_8UC4) {
			if (deepCopy) {
				destImage = QImage((const uchar*)mat.data, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32).copy();
			} else {
				destImage = QImage(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
			}
		} else if (mat.type() == CV_8UC3) {
			//this only works for opencv images that are RGB instead of GBR. Use cvtColor to change channel order.
			if (deepCopy) {
				destImage = QImage((const uchar*)mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888).copy();
			} else {
				destImage = QImage(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
			}
		} else if (mat.type() == CV_8UC1) {
			if (deepCopy) {
				destImage = QImage((const uchar*)mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Grayscale8).copy();
			} else {
				destImage = QImage(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Grayscale8);
			}
		} else {
			std::cerr << "ERROR: Conversion from cv::Mat to QImage unsuccessfull because type is unknown." << std::endl;
		}
	}

	void ImageView::imageToMat(const QImage& image, cv::Mat& destMat, bool deepCopy) {
		if (image.format() == QImage::Format_ARGB32 || image.format() == QImage::Format_ARGB32_Premultiplied || image.format() == QImage::Format_RGB32) {
			if (deepCopy) {
				destMat = cv::Mat(image.height(), image.width(), CV_8UC4, const_cast<uchar*>(image.bits()), image.bytesPerLine()).clone();
			} else {
				destMat = cv::Mat(image.height(), image.width(), CV_8UC4, const_cast<uchar*>(image.bits()), image.bytesPerLine());
			}
		} else if (image.format() == QImage::Format_RGB888) {
			if (deepCopy) {
				destMat = cv::Mat(image.height(), image.width(), CV_8UC3, const_cast<uchar*>(image.bits()), image.bytesPerLine()).clone();
			} else {
				destMat = cv::Mat(image.height(), image.width(), CV_8UC3, const_cast<uchar*>(image.bits()), image.bytesPerLine());
			}
		} else if (image.format() == QImage::Format_Indexed8 || image.format() == QImage::Format_Grayscale8) {
			if (deepCopy) {
				destMat = cv::Mat(image.height(), image.width(), CV_8UC1, const_cast<uchar*>(image.bits()), image.bytesPerLine()).clone();
			} else {
				destMat = cv::Mat(image.height(), image.width(), CV_8UC1, const_cast<uchar*>(image.bits()), image.bytesPerLine());
			}
		} else {
			std::cerr << "ERROR: Conversion from QImage to cv::Mat unsuccessfull because type is unknown." << std::endl;
		}
	}

}