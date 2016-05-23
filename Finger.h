#pragma once

#include <opencv2\core\core.hpp>

class Finger
{
private:
	// the coordinates of this fingertip
	cv::Point coordinates;

	// finger length - the distance from palm-center to fingertip
	double length = 0;
public:
	Finger() : coordinates(0, 0) {}
	Finger(cv::Point pt) : coordinates(pt.x, pt.y) {}
	Finger(int x, int y) : coordinates(x, y) {}
	~Finger();

	/**
		@returns The coordinates of the fingertip as cv::Point
	*/
	cv::Point getCoordinates();

	/**
		Sets new coordinates for the fingertip

		@param point The new coordinates
	*/
	void setCoordicates(cv::Point point);

	/**
		Sets new coordinates for the fingertip

		@param x The x coordinate
		@param y The y coordinate
	*/
	void setCoordinates(int x, int y);

	/**
		@returns The distance from the fingertip to the palm center
	*/
	double getLength();

	/**
		Sets the distance from the fingertip to the palm center

		@param len The length value
	*/
	void setLength(double len);

	/** 
		Automatically calculates and sets the distance from the fingertip 
		to the palm center

		@param center The coordinates of the palm center
	*/
	void setLength(cv::Point center);
};

