#include "HandTracker.h"

int HandTracker::extractContour(Hand & hand) {
	if (hand.getHandBox().size.width <= 0 || hand.getHandBox().size.height <= 0) {
		return -1;
	}

	std::cout << "\t\tTracker: extract contour: begin" << std::endl;

	// a vector of vectors of points, containing all the contours in the image
	std::vector< std::vector<cv::Point> > contours;

	// a vector of contour hierarchy
	std::vector<cv::Vec4i> hierarchy;

	// attempt to extract the contours in the image
	//try {
		cv::RotatedRect handBox = hand.getHandBox();
		cv::Rect handBoxRect = handBox.boundingRect();

		cv::Mat crop;
		cropRoi(mask.clone(), crop, handBoxRect);

		std::cout << "\t\tTracker: extract contour: roi cropped" << std::endl;

		// check if there's enough color inside this region
		float whiteRatio = (float) cv::countNonZero(crop) / (float) handBoxRect.area();

		if (whiteRatio < 0.05) {
			std::cout << "\t\tTracker: extract contour: ration < 0.05" << std::endl;
			return -1;
		}

		handBox.size.width *= 1.5;
		handBox.size.height *= 1.5;
		handBoxRect = handBox.boundingRect();

		cropRoi(mask.clone(), crop, handBoxRect);

		findContours(crop, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE, handBoxRect.tl());

		std::cout << "\t\tTracker: extract contour: contours found" << std::endl;

		if (!contours.empty()) {
			for (int i = 0; i < contours.size(); i++) {
				if (cv::pointPolygonTest(contours[i], hand.getHandBox().center, false) >= 0) {					
					hand.setContour(contours[i]);
					std::cout << "\t\tTracker: extract contour: contour set" << std::endl;
					break;
				}
			}
		}
		else {
			return -1;
		}
	//} // end-try
	//catch (cv::Exception& ex) {
	//	std::cout << "Exception caught while extracting a contour: " << std::endl << ex.what() << std::endl;		
	//	return -2;
	//}

	std::cout << "\t\tTracker: extract contour: end" << std::endl;
	std::cout << "\t\tTracker: extract contour: size of contour: " << hand.getParameters().handContour.size() << std::endl;
	return 1;
}

void HandTracker::removeSmallBlobs(cv::Mat& inputImage, const double blobSize) {
	// Only accept single channel images
	if (inputImage.channels() != 1 || inputImage.type() != CV_8U) {
		std::cerr << "chan != 1 or !cv_u8" << std::endl;
		return;
	}

	// Find all contours
	std::vector< std::vector<cv::Point> > contours;
	cv::findContours(inputImage, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
	for (int i = 0; i < contours.size(); i++) {
		double area = cv::contourArea(contours[i]);
		if (area >= 0) {
			if (area <= blobSize)
				cv::drawContours(inputImage, contours, i, CV_RGB(0, 0, 0), -1);
		}
	}
}

void HandTracker::bwMorph(cv::Mat & inputImage, const int operation, const int mShape, const int mSize) {
	cv::Mat element = cv::getStructuringElement(mShape, cv::Size(2 * mSize + 1, 2 * mSize + 1),
		cv::Point(mSize, mSize));
	cv::morphologyEx(inputImage, inputImage, operation, element);
}

void HandTracker::cropRoi(const cv::Mat& inputImage, cv::Mat& outputCrop, cv::Rect& roiRectangle) {
	cv::Rect roiRect;

	roiRect.x = (roiRectangle.x < 0) ? 0 : roiRectangle.x;
	roiRect.y = (roiRectangle.y < 0) ? 0 : roiRectangle.y;

	roiRect.width = (roiRect.x - 1 + roiRectangle.width) >= inputImage.cols ? (inputImage.cols - 1 - roiRect.x) : roiRectangle.width;
	roiRect.height = (roiRect.y - 1 + roiRectangle.height) >= inputImage.rows ? (inputImage.rows - 1 - roiRect.y) : roiRectangle.height;

	roiRect.width -= 1;
	roiRect.height -= 1;

	roiRect.x = abs(roiRect.x);
	roiRect.y = abs(roiRect.y);
	roiRect.width = abs(roiRect.width);
	roiRect.height = abs(roiRect.height);

	outputCrop = cv::Mat(inputImage, roiRect);

	roiRectangle = roiRect;
}

HandTracker::HandTracker() {
}

HandTracker::~HandTracker() {
}

bool HandTracker::initialize() {
	bool result = false;

	somethingIsTracked = false;

	skinSegmMethod = SKIN_SEGMENT_HSV;

	result = true;

	isInitialized = result;

	return result;
}

bool HandTracker::getNewPosition(Hand & hand) {

	// prepare the ROI
	cv::Rect selection = hand.getHandBox().boundingRect();
	cv::Rect trackWindow = hand.getTrackWindow();

	// prepare the histogram
	int h_bins = 30; int s_bins = 32;
	int histSize[] = { h_bins, s_bins };

	float h_range[] = { 0, 179 };
	float s_range[] = { 0, 255 };
	//	float v_range[] = { 0, 255 };
	const float* ranges[] = { h_range, s_range };

	int channels[] = { 0, 1 };

	// if the hand wasn't tracked yet, calculate histograms
	if (!hand.getTrackedState()) {
		cv::Mat roi(hsv, selection), maskroi(mask, selection);

		// Get the Histogram and normalize it
		cv::Mat hist;// = hand.getHistogram();
		calcHist(&roi, 1, channels, maskroi, hist, 2, histSize, ranges, true, false);
		cv::normalize(hist, hist, 0, 255, cv::NORM_MINMAX, -1, cv::Mat());
		hand.setHistogram(hist);
		// set the hand to 'being tracked'
		hand.setTrackedState(true);
	}

	// calculate the back projection
	cv::Mat backproj;
	cv::calcBackProject(&hsv, 1, channels, hand.getHistogram(), backproj, ranges, 1, true);

	// apply the binary mask
	backproj &= mask;

	// perform the CamShift algorithm
	cv::RotatedRect trackBox = CamShift(backproj, trackWindow,
		cv::TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1));

	if (trackBox.boundingRect().area() <= 1) { 
		return false;
	}

	if (trackWindow.area() <= 1) {
		int cols = backproj.cols, rows = backproj.rows, r = (MIN(cols, rows) + 5) / 6;
		trackWindow = cv::Rect(trackWindow.x - r, trackWindow.y - r,
							   trackWindow.x + r, trackWindow.y + r) &
					  cv::Rect(0, 0, cols, rows);
	}

	// append the backprojection mask
	backprojection |= backproj;

	// assign new position and track window
	hand.setTrackWindow(trackWindow);
	hand.setHandBox(trackBox);

	somethingIsTracked = true;

	return true;
}

void HandTracker::trackHands(const cv::Mat& inputFrame, std::vector<Hand>& hands) {
	std::cout << "\tTracker: begin" << std::endl;
	currentFrame = inputFrame;

	// get frame's initial size
	int frameHeight = inputFrame.rows;
	int frameWidth = inputFrame.cols;

	// convert the image to HSV
	cv::cvtColor(inputFrame, hsv, cv::COLOR_BGR2HSV);
	std::cout << "\tTracker: bgr->hsv done" << std::endl;

	// clear the backproj mask
	backprojection = cv::Mat::zeros(hsv.rows, hsv.cols, CV_8U);
	backprojection = cv::Scalar::all(0);

	// iterate through all hands
	for (int i = 0; i < hands.size(); i++) {
		std::cout << "\tTracker: hand[" << i << "], begin" << std::endl;
		// extract the boolean mask
		cv::Mat smallMask;
		mask = cv::Mat(frameHeight, frameWidth, CV_8U);
		mask = cv::Mat::zeros(frameHeight, frameWidth, CV_8U);
		skinDetector.extrackskinMask(inputFrame, smallMask, hands[i], skinSegmMethod);
		std::cout << "\tTracker: hand[" << i << "] mask extracted" << std::endl;

		// filter out the small blobs
	//	removeSmallBlobs(smallMask, 100);

		cv::Mat tempMask = smallMask.clone();
		std::ostringstream ofs;
		ofs << "mask_hand_" << i << ".jpg";
		cv::imwrite(ofs.str(), tempMask);
		ofs.flush();
		ofs.str("");

		smallMask.copyTo(mask);

		// locate the new position for the hand:
		bool result = getNewPosition(hands[i]);
		std::cout << "\tTracker: hand[" << i << "] position extracted" << std::endl;

		// if the tracking was unsuccessful, remove the hand
		if (!result) {
			std::cout << "\tTracker: hand[" << i << "], getPosition unsuccessful, erasing" << std::endl;
			hands.erase(hands.begin() + i--);
		} 
		else { 		
			//extract hand contour
			//std::cout << "\tTracker: hand[" << i << "], extracting contour..." << std::endl;
			//int reslt = extractContour(hands[i]);
			//std::cout << "\tTracker: hand[" << i << "] contour extracted" << std::endl;
			//if (reslt == -1) {
			//	std::cout << "\tTracker: hand[" << i << "] contour empty, erasing" << std::endl;
			//	// if hand empty, remove it
			//	hands.erase(hands.begin() + i--);				
			//}						
		}
		std::cout << "\tTracker: hand[" << i << "], end" << std::endl;
	}
	std::cout << "\tTracker: end" << std::endl;
}

bool HandTracker::checkInitializedState() {
	return isInitialized;
}

void HandTracker::changeSkinMethod(SkinSegmMethod method) {
	skinSegmMethod = method;
}

SkinSegmMethod HandTracker::getSkinMethod() {
	return skinSegmMethod;
}

void HandTracker::getSkinMask(cv::Mat& outputSkinMask) {
	if (somethingIsTracked) {
		cv::cvtColor(mask, outputSkinMask, cv::COLOR_GRAY2BGR);
	}
	else
		currentFrame.copyTo(outputSkinMask);
}
