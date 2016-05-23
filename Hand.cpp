#include "Hand.h"

void Hand::rectToRotatedRect(const cv::Rect& inputRect, cv::RotatedRect& outputRect) {
	// Get Rectangle Size
	cv::Size2f size;
	size.width = inputRect.width;
	size.height = inputRect.height;

	// Get Center Point
	cv::Point2f center;
	center.x = inputRect.x + (int)(inputRect.width / 2);
	center.y = inputRect.y + (int)(inputRect.height / 2);

	// Create RotatedRect with angle 0.0
	outputRect = cv::RotatedRect(center, size, 0.0f);
}

Hand::Hand() {
	Parameters.moveAngle = NAN;
	Parameters.moveSpeed = -1;
	prevPosition = cv::Point(-1, -1);

	// YCrCb thresholds
	YCbCr.Y_MIN = 0;
	YCbCr.Y_MAX = 255;
	YCbCr.Cr_MIN = 133;
	YCbCr.Cr_MAX = 173;
	YCbCr.Cb_MIN = 77;
	YCbCr.Cb_MAX = 127;

	// HSV thresholds
	HSV.H_MIN = 0;
	HSV.H_MAX = 15;
	HSV.S_MIN = 76;
	HSV.S_MAX = 118;
	HSV.V_MIN = 0;
	HSV.V_MAX = 255;
}

Hand::Hand(const cv::Rect & rect) {
	Hand();

	// create a RotatedRect bounding box
	cv::RotatedRect bbox;

	// convert Rect to RotatedRect
	rectToRotatedRect(rect, bbox);

	// set the hand bounding box
	handBox = bbox;

	// detection box
	detectionBox = cv::Rect(rect);

	Tracker.trackWindow = handBox.boundingRect();

	Tracker.isTracked = false;
}

Hand::Hand(const cv::RotatedRect& rect) {
	Hand();

	handBox = cv::RotatedRect(rect);
}

Hand::~Hand() {
}

void Hand::setHandBox(const cv::RotatedRect & box) {
	handBox = cv::RotatedRect(box);
}

void Hand::setHandBox(const cv::Rect & box) {
	// create a RotatedRect bounding box
	cv::RotatedRect bbox;

	// convert Rect to RotatedRect
	rectToRotatedRect(box, bbox);

	// set the hand bounding box
	handBox = cv::RotatedRect(bbox);

	// reset detection box
	setDetectionBox(box);
}

cv::RotatedRect Hand::getHandBox() {
	return handBox;
}

void Hand::setDetectionBox(const cv::Rect & box) {
	detectionBox = cv::Rect(box);
}

cv::Rect Hand::getDetectionBox() {
	return detectionBox;
}

cv::Point Hand::getPrevPosition() {
	return prevPosition;
}

void Hand::setPrevPosition(const cv::Point & pt) {
	prevPosition = cv::Point(pt.x, pt.y);
}

cv::Point Hand::getPosition() {
	return (cv::Point) handBox.center;
}

void Hand::setPosition(const cv::Point & pt) {
	handBox.center = cv::Point2f((float) pt.x, (float) pt.y);
}

Hand::HandInformation Hand::getParameters() {
	return Parameters;
}

void Hand::setParameters(const HandInformation & params) {
	setMoveSpeed(params.moveSpeed);
	setAngle(params.moveAngle);
	setContour(params.handContour);
	setFingers(params.fingers);
}

void Hand::setMoveSpeed(const double speed) {
	Parameters.moveSpeed = speed;
}

void Hand::setAngle(const double angle) {
	Parameters.moveAngle = angle;
}

void Hand::setContour(const std::vector<cv::Point>& contour) {
	Parameters.handContour.clear();
	Parameters.handContour = std::vector<cv::Point>(contour);
}

void Hand::setFingers(const std::vector<Finger>& fingers) {
	Parameters.fingers.clear();
	Parameters.fingers = std::vector<Finger>(fingers);
}

Hand::TrackingInformation Hand::getTracker() {
	return Tracker;
}

void Hand::setTrackWindow(const cv::Rect & wnd) {
	Tracker.trackWindow = cv::Rect(wnd);
}

cv::Rect Hand::getTrackWindow() {
	return Tracker.trackWindow;
}

void Hand::setTrackedState(const bool state) {
	Tracker.isTracked = state;
}

bool Hand::getTrackedState() {
	return Tracker.isTracked;
}

void Hand::setHistogram(const cv::Mat & hist) {
	Tracker.hist = hist.clone();
}

cv::Mat Hand::getHistogram() {
	return Tracker.hist;
}

void Hand::addTrackPoint(const cv::Point & pt) {
	Tracker.camsTrack.push_back(cv::Point(pt.x, pt.y));
}

void Hand::clearTrackLine() {
	Tracker.camsTrack.clear();
}

std::vector<cv::Point> Hand::getTrackLine() {
	return Tracker.camsTrack;
}

// recalculate the hand's thresholding ranges:
void Hand::recalculateRange(const cv::Mat& frame, SkinSegmMethod method, const bool useHistogram) {
	if (method != SKIN_SEGMENT_ADAPTIVE) {
		cv::Rect rect = handBox.boundingRect();

		// reduce the rect to half of its size
		rect.y += (rect.height * 0.5);
		//if (handGesture.getGestureType() != GESTURE_POSTURE_FIST)
		rect.height *= 0.5;
		rect.x += (rect.width * 0.2);
		rect.width *= 0.6;

		cv::Mat crop;

		// Method of color-picker
		if (!useHistogram) {
			cv::Rect picker(rect.x + rect.width / 2, rect.y + rect.height / 2, 4, 4);

			crop = cv::Mat(frame, picker);

			cv::pyrDown(crop, crop);
			cv::pyrDown(crop, crop);
		}
		else { // histogram
			crop = cv::Mat(frame.clone(), rect);
		}

		// convert it to HSV or YCrCb, depending on chosen segmentation method
		if (method == SKIN_SEGMENT_HSV)
			cv::cvtColor(crop, crop, cv::COLOR_BGR2HSV);
		else
			cv::cvtColor(crop, crop, cv::COLOR_BGR2YCrCb);

		if (!useHistogram) {

			cv::Vec3b vec = crop.at<cv::Vec3b>(0, 0);

			if (method == SKIN_SEGMENT_HSV) {
				HSV.H_MIN = vec[0] - 10;
				HSV.H_MAX = vec[0] + 10;
				HSV.S_MIN = vec[1] - 35;
				HSV.S_MAX = vec[1] + 35;
				HSV.V_MIN = 0;//vec[2] - 35;
				HSV.V_MAX = 255;//vec[2] + 35;
			}
			else { // if YCrCb
				YCbCr.Y_MIN = 0; // vec[0] - 50;
				YCbCr.Y_MAX = 255;// vec[0] + 50;
				YCbCr.Cr_MIN = vec[1] - 20;
				YCbCr.Cr_MAX = vec[1] + 20;
				YCbCr.Cb_MIN = vec[2] - 25;
				YCbCr.Cb_MAX = vec[2] + 25;
			}
		}
		else { // histogram method:
			std::vector<cv::Mat> bgr_planes;
			cv::split(crop, bgr_planes);

			int histSize = 256;
			float range[] = { 0, 256 };
			const float* histRange = { range };

			std::vector<cv::Mat> histograms(3);

			/// Compute the histograms:
			cv::calcHist(&bgr_planes[0], 1, 0, cv::Mat(), histograms[0], 1, &histSize, &histRange, true, false);
			cv::calcHist(&bgr_planes[1], 1, 0, cv::Mat(), histograms[1], 1, &histSize, &histRange, true, false);
			cv::calcHist(&bgr_planes[2], 1, 0, cv::Mat(), histograms[2], 1, &histSize, &histRange, true, false);

			int color_ranges[3][2] = { 0, 0, 0, 0, 0, 0 };

			for (int idx = 0; idx < 3; idx++) {
				double minVal, maxVal;
				cv::Point maxloc;
				cv::minMaxLoc(histograms[idx], &minVal, &maxVal, 0, &maxloc);

				int numMaxElement = (int)maxloc.y;

				// here we need to do te gaussian filtering and then differentiate;
				// Generate the Gaussian:
				int sigma = 5, size = 30, i = -size / 2;
				std::vector<int> x;
				std::generate_n(std::back_inserter(x), (size), [&]() {return i++; });
				std::vector<double> gaussFilter;

				double filterSum = 0;

				i = 0;
				std::generate_n(std::back_inserter(gaussFilter), x.size(), [&]() {
					double val = exp(-pow(x[i++], 2) / (2 * sigma * sigma));
					filterSum += val;
					return val;
				});

				std::for_each(gaussFilter.begin(), gaussFilter.end(), [&](double& V) { V /= filterSum; });

				// filter the histogram with the gaussian:
				int paddedLength = histograms[idx].rows + size - 1;
				std::vector<double> convolved(paddedLength); //zeros
				reverse(gaussFilter.begin(), gaussFilter.end());
				for (int outputIdx = 0; outputIdx < paddedLength; outputIdx++) //index into 'convolved' vector
				{
					int vecIdx = outputIdx - size + 1; //aligns with leftmost element of kernel
					int lowerBound = std::max(0, -vecIdx);
					int upperBound = std::min(size, histograms[idx].rows - vecIdx);

					for (int kernelIdx = lowerBound; kernelIdx < upperBound; kernelIdx++) {
						convolved[outputIdx] += gaussFilter[kernelIdx] * histograms[idx].at<float>(vecIdx + kernelIdx);
					}
				}

				std::vector<double> diffVector;

				// find new max value:
				auto newMaxValIt = std::max_element(convolved.begin() + size / 2, convolved.end() - size / 2);
				numMaxElement = std::distance(convolved.begin() + size / 2, newMaxValIt);

				// i = size/2; // chek this!!!

				for (i = size / 2; i < paddedLength - size / 2; i++) {
					diffVector.push_back(convolved[i + 1] - convolved[i]);
				}

				// find lower bound:
				auto itLeft = std::max_element(diffVector.begin(), diffVector.begin() + numMaxElement);
				int left = std::distance(diffVector.begin(), itLeft);

				// find upper bound
				auto itRight = std::max_element(diffVector.begin() + numMaxElement, diffVector.end());
				int right = std::distance(diffVector.begin(), itRight);

				left = left - (numMaxElement - left) * 0.9;
				right = right + (right - numMaxElement) * 0.9;

				//	std::cout << "max: " << numMaxElement << ", " << left << "; " << right << std::endl;

				color_ranges[idx][0] = left;
				color_ranges[idx][1] = right;
			}

			if (method == SKIN_SEGMENT_HSV) {
				HSV.H_MIN = color_ranges[0][0];
				HSV.H_MAX = color_ranges[0][1];
				HSV.S_MIN = color_ranges[1][0];
				HSV.S_MAX = color_ranges[1][1];
				HSV.V_MIN = color_ranges[2][0];
				HSV.V_MAX = color_ranges[2][1];
			}
			else { // if YCrCb
				YCbCr.Y_MIN = color_ranges[0][0];
				YCbCr.Y_MAX = color_ranges[0][1];
				YCbCr.Cr_MIN = color_ranges[1][0];
				YCbCr.Cr_MAX = color_ranges[1][1];
				YCbCr.Cb_MIN = color_ranges[2][0];
				YCbCr.Cb_MAX = color_ranges[2][1];
			}
		}

		//std::cout << "--Range: " << (int)vec[0] << "-" << (int)vec[1] << "-" << (int)vec[2] << "--" << std::endl;

		// assign the ranges
	}
}