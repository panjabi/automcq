#include <stdio.h>
#include <iostream>
#include <math.h>
#include <opencv2/opencv.hpp>
#include "opencv2/nonfree/nonfree.hpp"

using namespace cv;

// Returns aligned image
Mat alignImage( Mat image ) {

  Mat org = imread( "template-blank.jpg", CV_LOAD_IMAGE_GRAYSCALE );
  if(!org.data) std::cout << "unable to read template image";

  // Detect the keypoints using SURF Detector
  int minHessian = 400;

  SurfFeatureDetector detector( minHessian );

  std::vector<KeyPoint> keypointsOrg, keypointsImage;

  detector.detect( org, keypointsOrg );
  detector.detect( image, keypointsImage );

  // Calculate descriptors (feature vectors)
  SurfDescriptorExtractor extractor;

  Mat descriptorsOrg, descriptorsImage;

  extractor.compute( org, keypointsOrg, descriptorsOrg );
  extractor.compute( image, keypointsImage, descriptorsImage );

  // Matching descriptor vectors using FLANN matcher
  FlannBasedMatcher matcher;
  std::vector< DMatch > matches;
  matcher.match( descriptorsOrg, descriptorsImage, matches );

  double max_dist = 0;
  double min_dist = 100;

  // Quick calculation of max and min distances between keypoints
  for( int i = 0; i < descriptorsOrg.rows; i++ )
  { double dist = matches[i].distance;
    if( dist < min_dist ) min_dist = dist;
    if( dist > max_dist ) max_dist = dist;
  }

  // Use "good" matches (i.e. whose distance is less than ((399*min_dist)+max_dist)/400 )
  std::vector< DMatch > good_matches;

  for( int i = 0; i < descriptorsOrg.rows; i++ )
  { if( matches[i].distance < ((399*min_dist)+max_dist)/400 )
     { good_matches.push_back( matches[i]); }
  }

  // Localize the object
  std::vector<Point2f> kptsOrg;
  std::vector<Point2f> kptsImage;

  for( int i = 0; i < good_matches.size(); i++ )
  {
    // Get the keypoints from the good matches
    kptsOrg.push_back( keypointsOrg[ good_matches[i].queryIdx ].pt );
    kptsImage.push_back( keypointsImage[ good_matches[i].trainIdx ].pt );
  }

  Mat H = findHomography( kptsImage, kptsOrg, CV_RANSAC );

  warpPerspective( image, org, H, org.size() );
  // imshow( "img_object", org);
  // waitKey(0);

  return org;
}

// Define the positions of options/ rollnumber
Mat definePtns ( int numQues, int numOpts, int numQuesRow, int centerX, int centerY, int distRow, int distCol, int distOpts ) {

  Mat optsPtns( numQues, numOpts, CV_16UC2 );

  unsigned int colNum = 0;
  unsigned int rowNum = 0;

  for (int i = 0; i < numQues; ++i)
  {
   for (int j = 0; j < numOpts; ++j)
    {
      colNum = i/numQuesRow;
      rowNum = i%numQuesRow;
      optsPtns.at<Vec2s>(i,j)[0] = centerX + (distCol*colNum) + (distOpts*j);
      optsPtns.at<Vec2s>(i,j)[1] = centerY + (distRow*rowNum);
    }
  }
  return optsPtns;
}

// If a given option is marked
short isOptMarked( Vec2s optCenter, Mat& image, int radius ) {
  
  float X = 0.0;
  float Y = 0.0;
  float effRadius = radius - 0.5;
  unsigned int counter = 0;
  unsigned int sum = 0;
  unsigned int threshold = 80;
  short isMarked = 0;

  for (int j = optCenter[1]-radius+1; j <= optCenter[1]+radius ; ++j)
  {
    for (int i = optCenter[0]-radius+1; i <= optCenter[0]+radius; ++i)
    { 
      X = i - (optCenter[0] + 0.5);
      Y = j - (optCenter[1] + 0.5);

      if ((X*X) + (Y*Y) <= (effRadius*effRadius))
      {
        counter = counter + 1;
        sum = sum + image.at<uchar>(j,i);
      } 
    }
  }

  if (sum/counter < threshold)
  {
    isMarked = 1;
  }

  return isMarked;
}

// Reads the marked answer options
Mat readOpts( Mat& image ) {
  
  // define constants
  int numQues = 60;
  int numQuesRow = 21;
  int numOpts = 5;
  int distRow = 42;
  int distCol = 182;
  int distOpts = 28;

  int centerX = 221;
  int centerY = 176;
  int radius = 10;

  Mat opts( numQues, numOpts, CV_8SC1);
  Mat optsPtns = definePtns( numQues, numOpts, numQuesRow, centerX, centerY, distRow, distCol, distOpts );

  for (int i = 0; i < optsPtns.rows; ++i)
  {
    for (int j = 0; j < optsPtns.cols; ++j)
    {
      Vec2s optCenter = optsPtns.at<Vec2s>(i,j);
      opts.at<short>(i,j) = isOptMarked(optCenter, image, radius);
    }
  }

  return opts;
}

// Read the roll number bubbles
int readRoll( Mat& image ) {

  // define constants
  int numDgts = 10;
  int numUnts = 5;
  int distUnts = 31;
  int distRow = 28;

  int centerX = 19;
  int centerY = 204;
  int radius = 11;

  int rollNo = 0;
  Mat rollPtns = definePtns( numDgts, numUnts, numDgts, centerX, centerY, distRow, 0, distUnts );

  for (int j = 0; j < rollPtns.cols; ++j)
  {
    for (int i = 0; i < rollPtns.rows; ++i)
    {
      Vec2s optCenter = rollPtns.at<Vec2s>(i,j);
      if( isOptMarked(optCenter, image, radius) == 1 )
      {
        rollNo += i * pow( 10, rollPtns.cols-1-j );
        continue;
      }
    }
  }

  return rollNo;
}

// Main function
int main(int argc, char** argv) {
  
  Mat image = imread( argv[1], CV_LOAD_IMAGE_GRAYSCALE );
  if(!image.data) std::cout << "unable to read argument image";
  
  Mat imageAligned = alignImage( image );

  Mat markedOpts = readOpts( imageAligned );
  int rollNo = readRoll( imageAligned );

  std::cout << rollNo << "\n";
}
