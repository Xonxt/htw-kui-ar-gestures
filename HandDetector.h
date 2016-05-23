#pragma once

#include "Hand.h"
#include <iostream>
#include "opencv2\objdetect\objdetect.hpp"
#include "opencv2\imgproc\imgproc.hpp"

#define RESIZE_HEIGHT 240
#define RESIZE_WIDTH  320

#define DEFAULT_DETECTOR "Detectors/hogcascade_righthand_v3.0.xml"

class HandDetector {
private:
	// my cascade hand detector
	cv::CascadeClassifier handCascade;

	// path to the cascade detector XML
	char* cascadePath;

	bool isInitialized = false;

public:
	HandDetector();
	HandDetector(const char* path);
	~HandDetector();

	bool initialize();

	void detectHands(const cv::Mat& frame, std::vector<Hand>& hands);

	bool checkInitializedState();
};

