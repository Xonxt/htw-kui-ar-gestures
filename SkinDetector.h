#pragma once

#include <opencv2/contrib/contrib.hpp>

//#include <iostream>

#include "Hand.h"

class SkinDetector {
private:
	// perform Ycbcr skin segmentation
	void getSkinYcbcr(const cv::Mat& inputFrame, cv::Mat& outputMask, const Hand& hand);

	// perform HSV skin segmentation
	void getSkinHsv(const cv::Mat& inputFrame, cv::Mat& outputMask, const Hand& hand);

	// use tha OpenCV sdaptive skin detector
	void getSkinAdaptiveDetector(const cv::Mat& inputFrame, cv::Mat& outputMask);

	// the openCV adaptive skin detector
	CvAdaptiveSkinDetector filter;
public:
	SkinDetector();
	~SkinDetector();

	// extract the skin mask
	void extrackskinMask(const cv::Mat& inputFrame, cv::Mat& outputMask, const Hand& hand, SkinSegmMethod method);
};

