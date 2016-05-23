#pragma once

#include "Hand.h"
#include "stdafx.h"
#include "SkinDetector.h"
#include <iostream>
#include <sstream>
#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\video\tracking.hpp>
#include <opencv2\highgui\highgui.hpp>

class HandTracker {
private:
	// was the tracker properly initialized?
	bool isInitialized = false;

	// the skin detector/segmentation tool
	SkinDetector skinDetector;

	// the current skin segmentation method
	SkinSegmMethod skinSegmMethod;

	// locate new hand position
	bool getNewPosition(Hand& hand);

	/** 
		extract hand contour from the mask
		@returns values:
		 0: everything ok
		-1: false detection, delete hand
		-2: exception caught, return empty contour 
	*/
	int extractContour(Hand& hand);

	// filter out the blobs smaller than a threshold
	void removeSmallBlobs(cv::Mat& inputImage, const double blobSize);

	/**
		perform a morphological operation of type 'operation', with a structural element 'mShape' of size 'mSize'
		where operations are:
		MORPH_OPEN - an opening operation
		MORPH_CLOSE - a closing operation
		MORPH_GRADIENT - a morphological gradient
		MORPH_TOPHAT - “top hat”
		MORPH_BLACKHAT - “black hat”
	*/
	void bwMorph(cv::Mat& inputImage, const int operation, const int mShape, const int mSize);

	// crop roi
	void cropRoi(const cv::Mat& inputImage, cv::Mat& outputCrop, cv::Rect& roiRectangle);

	// current frame image
	cv::Mat currentFrame;

	// the image converted to hsv
	cv::Mat hsv;

	// the current boolean mask
	cv::Mat mask;

	// the calculated back-projection
	cv::Mat backprojection;

	// something is being tracked
	bool somethingIsTracked;

public:
	HandTracker();
	~HandTracker();

	//initialize the tracker
	bool initialize();

	// track the hands in the frame
	void trackHands(const cv::Mat& inputFrame, std::vector<Hand>& hands);

	bool checkInitializedState();

	// change the skin segmentation method
	void changeSkinMethod(SkinSegmMethod method);

	// get current segmentation method
	SkinSegmMethod getSkinMethod();

	// retrieve the skin mask for debugging purposes
	void getSkinMask(cv::Mat& outputSkinMask);
};

