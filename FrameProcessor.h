#pragma once

#include "Hand.h"
#include "HandDetector.h"
#include "HandTracker.h"

#define DEFAULT_FACE_DETECTOR "Detectors/lbpcascade_frontalface.xml"

class FrameProcessor {
private:
	// current frame from the camera
	cv::Mat currentFrame;

	// grayscale version of the frame
	cv::Mat grayscaleFrame;

	// Tool for detecting every hand (open palm!) in the image
	HandDetector handDetector;

	// the list of detected hands
	std::vector<Hand> hands;

	// the list of detected faces
	std::vector<cv::Rect> faces;

	// face cascade detector
	cv::CascadeClassifier faceCascade;

	// Tool fot tracking of hands
	HandTracker handTracker;

	// frame number
	int frameNumber = 0;

	// a set of camera parameters:
	struct CameraSettings {
		// show backprojection mask or not?
		bool showMask = false;

		// show the contour?
		bool showContour = false;

		// show the bounding box (or bounding ellipse)?
		bool showBoundingBox = true;

		// bool show fingertips?
		bool showFingers = false;

		// show hand-text?
		bool showHandText = false;

		// show text information
		bool showInformation = false;

		// show the glow effect
		bool showGlowEffect = true;
	} cameraParameters;

	// mutexes for the parallel threads
	//std::mutex mutexHands, mutexFaces;

	// generate a random number in range	
	int randomNumber(int limi, int lims);

	// perform morphological operation
	void bwMorph(cv::Mat& inputImage, const int operation, const int mShape, const int mSize);

	void detectHands();

public:
	FrameProcessor();
	FrameProcessor(bool init);
	~FrameProcessor();

	// initizlize the processor
	bool initialize();

	// process frame, find hands, detect gestures
	void processFrame(cv::Mat& frame);

	// toggle show boolean skin-mask
	void toggleShowMask();

	// toggle showing the hand contours
	void toggleShowContour();

	// toggle showing the bounding box
	void toggleShowBoundingBox();

	// toggle showing fingertips as small circles
	void toggleShowFingers();

	// toggle showing information about the hand
	void toggleShowHandText();

	// toggle showing on-screen display information
	void toggleShowInformation();

	// toggle the glow effect around the hand and the glowing afterimage 
	void toggleGlowEffect();

	// get the camera parameters
	FrameProcessor::CameraSettings getCameraSettings();
};

