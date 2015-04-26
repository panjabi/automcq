#include <stdio.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include "opencv2/nonfree/nonfree.hpp"

using namespace cv;

// Returns aligned image
Mat alignImage( Mat image) {

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
      optsPtns.at<Vec2s>(i,j)[0] = 221 + (182*colNum) + (28*j);
      optsPtns.at<Vec2s>(i,j)[1] = 176 + (42*rowNum);
    }
  }
  return optsPtns;
}

Mat readOpts( Mat image ) {
  
  // define constants
  int numQues = 60;
  int numQuesRow = 21;
  int numOpts = 5;
  int distRow = 42;
  int distCol = 182;
  int distOpts = 28;

  int centerX = 221;
  int centerY = 176;
  int radius = 11;

  Mat opts( numQues, numOpts, CV_8UC1, 0 );
  Mat optsPtns = definePtns( numQues, numOpts, numQuesRow, centerX, centerY, distRow, distCol, distOpts );

  std::cout << optsPtns.at<Vec2s>(0,0) << "\n"; 
  std::cout << optsPtns.at<Vec2s>(59,4) << "\n"; 
  
  return optsPtns;
}

// Main function
int main(int argc, char** argv) {
  
  Mat image = imread( argv[1], CV_LOAD_IMAGE_GRAYSCALE );
  if(!image.data) std::cout << "unable to read argument image";
  
  Mat imageAligned = alignImage( image );

  Mat markedOpts = readOpts( imageAligned );

}
