#include "Finger.h"

Finger::~Finger()
{
}

cv::Point Finger::getCoordinates()
{
	return coordinates;
}

void Finger::setCoordicates(cv::Point point)
{
	coordinates = point;
}

void Finger::setCoordinates(int x, int y)
{
	coordinates = cv::Point(x, y);
}

double Finger::getLength()
{
	return length;
}

void Finger::setLength(double len)
{
	length = len;
}

void Finger::setLength(cv::Point center)
{
	length = std::sqrt(std::pow(coordinates.x - center.x, 2) + std::pow(coordinates.y - center.y, 2));
}
