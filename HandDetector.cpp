#include "HandDetector.h"

HandDetector::HandDetector() {
	std::strcpy(cascadePath, DEFAULT_DETECTOR);
}

HandDetector::HandDetector(const char * path) {
	std::strcpy(cascadePath, path);
}

HandDetector::~HandDetector() {
}

bool HandDetector::initialize() {
	bool result = true;

	if (!(result &= handCascade.load(cascadePath))) {
		std::cerr << "Error initializing hand detector cascade!" << std::endl;
	}

	isInitialized = result;

	return result;
}

void HandDetector::detectHands(const cv::Mat & frame, std::vector<Hand>& hands) {

	if (!isInitialized) { 
		std::cerr << "Hand tracker is not initialized!" << std::endl;
		return;
		//throw std::exception("tracker uninitialized");
	}
	
	// clear the vector
	hands.clear();

	// get frame's initial size
	int frameHeight = frame.rows;
	int frameWidth = frame.cols;

	// downsize the frame
	cv::Mat frameResized;
	if (frameHeight > RESIZE_HEIGHT && frameWidth > RESIZE_WIDTH)
		cv::resize(frame, frameResized, cv::Size(RESIZE_WIDTH, RESIZE_HEIGHT));
	else
		frameResized = frame;

	// prepare a temporary hand-Rects vector
	std::vector<cv::Rect> handRects;

	// look for hand objects in the frame
	handCascade.detectMultiScale(frame, handRects, 1.1, 5);

	if (handRects.size() < 1)
		return;

	if (handRects.size() > 10) { 
		handRects.clear();
		return;
	}

	// iterate through located rectangles and create Hand objects
	for (cv::Rect rect : handRects) { 
		Hand hand = Hand(rect);
		hands.push_back(hand);
	}
}

bool HandDetector::checkInitializedState() {
	return isInitialized;
}
