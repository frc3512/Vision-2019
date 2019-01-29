#include <iostream>
#include <algorithm>

#include "GripPipeline.h"

/**
* Runs an iteration of the pipeline and updates outputs.
*/
void GripPipeline::Process(cv::Mat& source0){
	//Step HSV_Threshold0:
	//input
	cv::Mat hsvThresholdInput = source0;
	double hsvThresholdHue[] = {76.07913669064747, 112.76740237691};
	double hsvThresholdSaturation[] = {91.72661870503596, 255.0};
	double hsvThresholdValue[] = {121.53776978417267, 246.34125636672326};
	hsvThreshold(hsvThresholdInput, hsvThresholdHue, hsvThresholdSaturation, hsvThresholdValue, this->hsvThresholdOutput);
	std::cout << "hsvThreshold" << std::endl;

	cv::Mat findContoursInput = hsvThresholdOutput;
	bool findContoursExternalOnly = false;  // default Boolean
	findContours(findContoursInput, findContoursExternalOnly, this->findContoursOutput);
	std::cout << "findContours" << std::endl;

	std::vector<std::vector<cv::Point> > filterContoursContours = findContoursOutput;
	double filterContoursMinArea = 50.0;  // default Double
	double filterContoursMinPerimeter = 0;  // default Double
	double filterContoursMinWidth = 0;  // default Double
	double filterContoursMaxWidth = 1000;  // default Double
	double filterContoursMinHeight = 50.0;  // default Double
	double filterContoursMaxHeight = 1000;  // default Double
	double filterContoursSolidity[] = {0, 100};
	double filterContoursMaxVertices = 1000000;  // default Double
	double filterContoursMinVertices = 0;  // default Double
	double filterContoursMinRatio = 0;  // default Double
	double filterContoursMaxRatio = 1000;  // default Double
	filterContours(filterContoursContours, filterContoursMinArea, filterContoursMinPerimeter, filterContoursMinWidth, filterContoursMaxWidth, filterContoursMinHeight, filterContoursMaxHeight, filterContoursSolidity, filterContoursMaxVertices, filterContoursMinVertices, filterContoursMinRatio, filterContoursMaxRatio, this->filterContoursOutput);
	std::cout << "filterContours" << std::endl;
	
	std::vector<std::vector<cv::Point> > convexHullsContours = filterContoursOutput;
	convexHulls(convexHullsContours, this->convexHullsOutput);
	std::cout << "convexHulls" << std::endl;
	
	std::vector<std::vector<cv::Point> > findPolyDPInput = convexHullsOutput;
	double polyDPEpsilon = 10.0;
	bool polyDPClosed = true;
	polyDP(findPolyDPInput, polyDPEpsilon, polyDPClosed, this->polyDPOutput);
	std::cout << "polyDP" << std::endl;
}

/**
 * This method is a generated getter for the output of a HSV_Threshold.
 * @return Mat output from HSV_Threshold.
 */
cv::Mat* GripPipeline::GetHsvThresholdOutput(){
	return &(this->hsvThresholdOutput);
}

std::vector<std::vector<cv::Point> >* GripPipeline::GetFindContoursOutput(){
	return &(this->findContoursOutput);
}

std::vector<std::vector<cv::Point> >* GripPipeline::GetFilterContoursOutput(){
	return &(this->filterContoursOutput);
}

/**
 * This method is a generated getter for the output of a Convex_Hulls.
 * @return ContoursReport output from Convex_Hulls.
 */
std::vector<std::vector<cv::Point> >* GripPipeline::GetConvexHullsOutput(){
	return &(this->convexHullsOutput);
}

std::vector<std::vector<cv::Point> >* GripPipeline::GetPolyDPOutput(){
	return &(this->polyDPOutput);
}
	/**
	 * Segment an image based on hue, saturation, and value ranges.
	 *
	 * @param input The image on which to perform the HSL threshold.
	 * @param hue The min and max hue.
	 * @param sat The min and max saturation.
	 * @param val The min and max value.
	 * @param output The image in which to store the output.
	 */
	void GripPipeline::hsvThreshold(cv::Mat &input, double hue[], double sat[], double val[], cv::Mat &out) {
		cv::cvtColor(input, out, cv::COLOR_BGR2HSV);
		cv::inRange(out,cv::Scalar(hue[0], sat[0], val[0]), cv::Scalar(hue[1], sat[1], val[1]), out);
	}

	void GripPipeline::findContours(cv::Mat &input, bool externalOnly, std::vector<std::vector<cv::Point> > &contours) {
		std::vector<cv::Vec4i> hierarchy;
		contours.clear();
		int mode = externalOnly ? cv::RETR_EXTERNAL : cv::RETR_LIST;
		int method = cv::CHAIN_APPROX_SIMPLE;
		cv::findContours(input, contours, hierarchy, mode, method);
	}

	void GripPipeline::filterContours(std::vector<std::vector<cv::Point> > &inputContours, double minArea, double minPerimeter, double minWidth, double maxWidth, double minHeight, double maxHeight, double solidity[], double maxVertexCount, double minVertexCount, double minRatio, double maxRatio, std::vector<std::vector<cv::Point> > &output) {
		std::vector<cv::Point> hull;
		output.clear();
		for (std::vector<cv::Point> contour: inputContours) {
			cv::Rect bb = boundingRect(contour);
			if (bb.width < minWidth || bb.width > maxWidth) continue;
			if (bb.height < minHeight || bb.height > maxHeight) continue;
			double area = cv::contourArea(contour);
			if (area < minArea) continue;
			if (arcLength(contour, true) < minPerimeter) continue;
			cv::convexHull(cv::Mat(contour, true), hull);
			double solid = 100 * area / cv::contourArea(hull);
			if (solid < solidity[0] || solid > solidity[1]) continue;
			if (contour.size() < minVertexCount || contour.size() > maxVertexCount)	continue;
			double ratio = (double) bb.width / (double) bb.height;
			if (ratio < minRatio || ratio > maxRatio) continue;
			output.push_back(contour);
		}
	}	

	/**
	 * Compute the convex hulls of contours.
	 *
	 * @param inputContours The contours on which to perform the operation.
	 * @param outputContours The contours where the output will be stored.
	 */
	void GripPipeline::convexHulls(std::vector<std::vector<cv::Point> > &inputContours, std::vector<std::vector<cv::Point>> &outputContours) {
		std::vector<std::vector<cv::Point> > hull (inputContours.size());
		outputContours.clear();
		for (size_t i = 0; i < inputContours.size(); i++ ) {
			cv::convexHull(cv::Mat(inputContours[i]), hull[i], false);
		}
		outputContours = hull;
	}

	void GripPipeline::polyDP(std::vector<std::vector<cv::Point> > &input, double epsilon, bool closed, std::vector<std::vector<cv::Point> > &out){
		std::vector<std::vector<cv::Point> > hull (input.size());
		out.clear();
			for (size_t i = 0; i < input.size(); i++ ) {
			cv::approxPolyDP(cv::Mat(input[i]), hull[i], epsilon, closed);
		}
		out = hull;
	}	

 // end grip namespace

