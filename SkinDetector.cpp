#include "SkinDetector.h"

void SkinDetector::getSkinYcbcr(const cv::Mat & inputFrame, cv::Mat & outputMask, const Hand & hand) {
	// first convert our RGB image to YCrCb
	cv::cvtColor(inputFrame, outputMask, cv::COLOR_BGR2YCrCb);

	// filter the image in YCrCb color space
	cv::inRange(outputMask,
		cv::Scalar(MIN(hand.YCbCr.Y_MIN, hand.YCbCr.Y_MAX), MIN(hand.YCbCr.Cr_MIN, hand.YCbCr.Cr_MAX), MIN(hand.YCbCr.Cb_MIN, hand.YCbCr.Cb_MAX)),
		cv::Scalar(MAX(hand.YCbCr.Y_MIN, hand.YCbCr.Y_MAX), MAX(hand.YCbCr.Cr_MIN, hand.YCbCr.Cr_MAX), MAX(hand.YCbCr.Cb_MIN, hand.YCbCr.Cb_MAX)),
		outputMask);
}

void SkinDetector::getSkinHsv(const cv::Mat & inputFrame, cv::Mat & outputMask, const Hand & hand) {
	// convert input image to HSV
	cv::cvtColor(inputFrame, outputMask, cv::COLOR_BGR2HSV);

	// filter the image in HSV color space
	cv::inRange(outputMask,
		cv::Scalar(MIN(hand.HSV.H_MIN, hand.HSV.H_MAX), MIN(hand.HSV.S_MIN, hand.HSV.S_MAX), MIN(hand.HSV.V_MIN, hand.HSV.V_MAX)),
		cv::Scalar(MAX(hand.HSV.H_MIN, hand.HSV.H_MAX), MAX(hand.HSV.S_MIN, hand.HSV.S_MAX), MAX(hand.HSV.V_MIN, hand.HSV.V_MAX)),
		outputMask);
}

void SkinDetector::getSkinAdaptiveDetector(const cv::Mat & inputFrame, cv::Mat & outputMask) {
	// Convert input Mat-type to old-school IplImage-type
	IplImage* _frame = new IplImage(inputFrame);

	// Create IplImage-type mask
	IplImage* _mask = cvCreateImage(cvSize(inputFrame.cols, inputFrame.rows), IPL_DEPTH_8U, 1);

	// perform filtering
	filter.process(_frame, _mask);

	// convert mask to Mat-type and make it black&white
	outputMask = cv::Mat(_mask);
	inRange(outputMask, cv::Scalar(1), cv::Scalar(180), outputMask);
}

SkinDetector::SkinDetector() {
	
}


SkinDetector::~SkinDetector() {
}

void SkinDetector::extrackskinMask(const cv::Mat & inputFrame, cv::Mat & outputMask, const Hand & hand, SkinSegmMethod method) {
	// get the skin detection method and extract mask
	switch (method) {
	case SKIN_SEGMENT_ADAPTIVE:
		getSkinAdaptiveDetector(inputFrame, outputMask);
		break;
	case SKIN_SEGMENT_HSV:
		getSkinHsv(inputFrame, outputMask, hand);
		break;
	case SKIN_SEGMENT_YCRCB:
		getSkinYcbcr(inputFrame, outputMask, hand);
		break;
	default:
		//throw std::exception("Unknown skin segmentation method!");
		//std::cerr << "Unknown skin segmentation method!" << std::endl;
		break;
	}
}
