// htw-kui-ar-gestures.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <iterator>

#include <string>
#include <sstream>
#include <time.h>

#include <opencv2\highgui\highgui.hpp>

#include "FrameProcessor.h"

// construct a filename based on the current time and supplied extension (with a ".")
std::string generateFileName(const char* ext) {
	std::ostringstream ost;

	char timeString[12];
	time_t now = time(0);
	strftime(timeString, sizeof(timeString), "%H-%M-%S", localtime(&now));

	ost << timeString;

	if (ext[0] != '.')
		ost << ".";

	ost << ext;

	return ost.str();
}

int main() {	
	cv::VideoCapture videoCapture;

	if (!videoCapture.open(1)) {
		std::cerr << "Failed when opening video capture device!" << std::endl;
		return -1;
	}

	cv::Mat frame;

	FrameProcessor frameProcessor;

	if (!frameProcessor.initialize()) {
		std::cout << "Failed to initialize Frame Processor!" << std::endl;
		return -2;
	}

	std::vector<Hand> hands;

	cv::Size S = cv::Size((int)videoCapture.get(CV_CAP_PROP_FRAME_WIDTH), 
						  (int)videoCapture.get(CV_CAP_PROP_FRAME_HEIGHT));

	// open video recorder
	cv::VideoWriter outputVideo;

	int ex = static_cast<int>(videoCapture.get(CV_CAP_PROP_FOURCC));

	// construct a video file name
	//outputVideo.open(generateFileName(".avi"), CV_FOURCC('M', 'P', '4', 'V'), 20, S, true);

	if (!outputVideo.isOpened()) {
		std::cerr << "Could not open the output video for write. " << std::endl;
	}
	
	while (true) {

		videoCapture >> frame;

		if (!frame.empty()) {			

			try {
				frameProcessor.processFrame(frame);
			}
			catch (std::exception& ex) { 
				std::cerr << "std exception caught in FrameProcessor::processFrame()!\n" << ex.what() << std::endl;
			}
		
			cv::imshow("video", frame);

			// write the frame into video
			if (outputVideo.isOpened()) {
				outputVideo << frame;
			}
		}

		char c = cv::waitKey(10);

		if (char(c) == 27)
			break;		
	}

	outputVideo.release();

    return 0;
}