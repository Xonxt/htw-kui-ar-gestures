#include "FrameProcessor.h"

int FrameProcessor::randomNumber(int limi, int lims) {
	return (std::rand() % (lims - limi)) + limi;
}

void FrameProcessor::bwMorph(cv::Mat & inputImage, const int operation, const int mShape, const int mSize) {
	cv::Mat element = cv::getStructuringElement(mShape, cv::Size(2 * mSize + 1, 2 * mSize + 1),
		cv::Point(mSize, mSize));
	cv::morphologyEx(inputImage, inputImage, operation, element);
}

void FrameProcessor::detectHands() {

	// first track the present hands in the frame
	std::cout << "Hands before tracking: " << hands.size() << std::endl;
	//try {
		handTracker.trackHands(currentFrame, hands);
	/*}
	catch (cv::Exception& ex) {
		std::cerr << "Exception caught while tracking: " << std::endl << ex.what() << std::endl;
	}
	catch (...) {
		std::cerr << "Some other kind of exception caught" << std::endl;
	}*/
	std::cout << "Hands after tracking: " << hands.size() << std::endl;
	// create the hands vector
	std::vector<Hand> detectedHands;
	detectedHands.clear();

	// now detect (new) hands in the frame	
	handDetector.detectHands(grayscaleFrame, detectedHands);
	std::cout << "Hands detected: " << detectedHands.size() << std::endl;

	// recalculate the color ranges for each hand:
	for (int i = 0; i < detectedHands.size(); i++) {
		detectedHands[i].recalculateRange(currentFrame, handTracker.getSkinMethod(), true);
	}

	// were any new hands added?
	bool newHandsAdded = false;

	if (detectedHands.size() > 0) {		
		// go through the detected hands vector
		for (int i = 0; i < detectedHands.size(); i++) {
			// is it one of the already tracked hands?
			bool oldHand = false;

			Hand hand = detectedHands[i];

			// go through the old vector of already tracked hands
			std::for_each(hands.begin(), hands.end(), [&hand, &oldHand] (Hand& trackedHand) {
				cv::Rect intersection = hand.getDetectionBox() & trackedHand.getHandBox().boundingRect();

				// if the area of intersection is 75% of the new hand's size or more
				if (intersection.area() >= (hand.getDetectionBox().area() * 0.75)) { 
					// if true, then this hand is already in the list and is already being tracked
					// just update it's position, because detection position is more reliable
					trackedHand.setHandBox(hand.getDetectionBox());
					oldHand = true;
					std::cout << "Intersection of old and new hands found! " << std::endl;
				}
			});

			// if oldHand == false, i.e. the hand is new and wasn't in the list
			if (!oldHand) { 
				hands.push_back(detectedHands[i]);
				newHandsAdded = true;
				std::cout << "New hand added!" << std::endl;
			}
		}
	}

	// for every five frames check if any hands have overlapped the face region
	if (frameNumber == 5) { 
		if (hands.size() > 0) { 
			faces.clear();
			
			faceCascade.detectMultiScale(grayscaleFrame, faces, 1.1, 5, 0, cv::Size(40, 60));
			
			std::cout << "Faces detected: " << faces.size() << std::endl;
			// iterate trough all faces
			if (faces.size() > 0) { 
				for (cv::Rect face : faces) { 
					// remove all hands from the vector if they intersect with the face region
					hands.erase(
						std::remove_if(hands.begin(), hands.end(), [&face](Hand& hand) { 							
							cv::Rect intersection = face & hand.getHandBox().boundingRect();
							std::cout << "Intersection area of face and hand: " << intersection.area() << std::endl;
							return (intersection.area() >= (face.area() * 0.5));
						})
					);
					std::cout << "Hands maybe erased" << std::endl;
				}
			}
		}
		// set the frame counter to zero again
		frameNumber = 0;
	}
}

FrameProcessor::FrameProcessor() {	
}

FrameProcessor::FrameProcessor(bool init) {
	std::cout << "Created FrameProcessor object" << std::endl;
}

FrameProcessor::~FrameProcessor() {
}

bool FrameProcessor::initialize() {
	bool result = true;

	// clear the hands list
	hands.clear();

	frameNumber = 0;

	if (!(result &= handDetector.initialize())) {
		std::cerr << "Error initializing hand detector" << std::endl;
	}

	if (!(result &= faceCascade.load(DEFAULT_FACE_DETECTOR))) {
		std::cerr << "\tError initializing face detector cascade!" << std::endl;
	}

	if (!(result &= handTracker.initialize())) {
		std::cerr << "Error initializing the Hand Tracker!" << std::endl;
	}

	// init the RNG
	//std::srand(time(0));

	return result;
}

void FrameProcessor::processFrame(cv::Mat & frame) {

	if (!handDetector.checkInitializedState()) { 
		std::cerr << "Hand detector uninitialized!" << std::endl;
		return;
		//throw std::exception("Hand detector uninitialized!");
	}

	frameNumber++;
	
	// copy the frame
	currentFrame = frame;	

	// convert the frame into grayscale
	switch (frame.channels()) { 
	case 1:
		grayscaleFrame = frame.clone();
		break;
	case 4:
		cv::cvtColor(frame, grayscaleFrame, cv::COLOR_BGRA2GRAY);
		break;
	default:
		cv::cvtColor(frame, grayscaleFrame, cv::COLOR_BGR2GRAY);
		break;
	}

	// detect and track the hands in the frame
	
	detectHands();
	

	// draw the hands on the image
	for (Hand hand : hands) { 
		// draw ellipse
		cv::ellipse(frame, hand.getHandBox(), CV_RGB(255, 0, 0), 2);

		// draw contour
		if (hand.getParameters().handContour.size() > 0) { 
			std::vector<std::vector<cv::Point> > contour;
			contour.push_back(hand.getParameters().handContour);
			cv::drawContours(frame, contour, 0, CV_RGB(0, 255, 0), 1);
		}
	}
}

void FrameProcessor::toggleShowMask() {
	cameraParameters.showMask = !cameraParameters.showMask;
}

void FrameProcessor::toggleShowContour() {
	cameraParameters.showContour = !cameraParameters.showContour;
}

void FrameProcessor::toggleShowBoundingBox() {
	cameraParameters.showBoundingBox = !cameraParameters.showBoundingBox;
}

void FrameProcessor::toggleShowFingers() {
	cameraParameters.showFingers = !cameraParameters.showFingers;
}

void FrameProcessor::toggleShowHandText() {
	cameraParameters.showHandText = !cameraParameters.showHandText;
}

void FrameProcessor::toggleShowInformation() {
	cameraParameters.showInformation = !cameraParameters.showInformation;
}

void FrameProcessor::toggleGlowEffect() {
	cameraParameters.showGlowEffect = !cameraParameters.showGlowEffect;
}

FrameProcessor::CameraSettings FrameProcessor::getCameraSettings() {
	return cameraParameters;
}
