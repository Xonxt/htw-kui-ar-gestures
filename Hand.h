#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/contrib/contrib.hpp>
#include <iterator>

#include "stdafx.h"
#include "Finger.h"

class Hand
{
private:
	// the hand's bounding box (with position and rotation angle)
	cv::RotatedRect handBox;

	// initial rect of the hand from the cv::detectMultiscale
	cv::Rect detectionBox;

	// last stationary position
	cv::Point prevPosition;


	// the information for the gesture
	struct HandInformation {
		// the speed of hand movement (normalized to ROI width)
		double moveSpeed;
		// the angle of hand movenent in degrees (0..359)
		double moveAngle;
		// the hand contour
		std::vector<cv::Point> handContour;
		// list of fingertips
		std::vector<Finger> fingers;
	} Parameters;

	// all the tracking information
	struct TrackingInformation {
		// the tracking window for the CamShift algorithm
		cv::Rect trackWindow;
		// is the hand being tracked at the moment?
		bool isTracked;
		// hand histogram
		cv::Mat hist;
		// camshift track
		std::vector<cv::Point> camsTrack;
	} Tracker;

	void rectToRotatedRect(const cv::Rect& inputRect, cv::RotatedRect& outputRect);

public:
	Hand();
	Hand(const cv::Rect& rect);
	Hand(const cv::RotatedRect& rect);
	~Hand();

	void setHandBox(const cv::RotatedRect& box);
	void setHandBox(const cv::Rect& box);
	cv::RotatedRect getHandBox();

	void setDetectionBox(const cv::Rect& box);
	cv::Rect getDetectionBox();

	cv::Point getPrevPosition();
	void setPrevPosition(const cv::Point& pt);

	cv::Point getPosition();
	void setPosition(const cv::Point& pt);
	
	HandInformation getParameters();
	void setParameters(const HandInformation& params);
	void setMoveSpeed(const double speed);
	void setAngle(const double angle);
	void setContour(const std::vector<cv::Point>& contour);
	void setFingers(const std::vector<Finger>& fingers);

	TrackingInformation getTracker();
	void setTrackWindow(const cv::Rect& wnd);
	cv::Rect getTrackWindow();
	void setTrackedState(const bool state);
	bool getTrackedState();
	void setHistogram(const cv::Mat& hist);
	cv::Mat getHistogram();
	void addTrackPoint(const cv::Point& pt);
	void clearTrackLine();
	std::vector<cv::Point> getTrackLine();

	void recalculateRange(const cv::Mat& frame, SkinSegmMethod method, const bool useHistogram);

	// The hand's color ranges
	YCbCrBounds YCbCr;
	HsvBounds   HSV;
};

