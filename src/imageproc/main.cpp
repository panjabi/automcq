#include <stdio.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include "opencv2/nonfree/nonfree.hpp"

using namespace cv;

// Returns aligned image
Mat alignImage( Mat image) {

  Mat org = imread( "org.png", CV_LOAD_IMAGE_GRAYSCALE );

  //-- Step 1: Detect the keypoints using SURF Detector
  int minHessian = 400;

  SurfFeatureDetector detector( minHessian );

  std::vector<KeyPoint> keypointsOrg, keypointsImage;

  detector.detect( org, keypointsOrg );
  detector.detect( image, keypointsImage );

  //-- Step 2: Calculate descriptors (feature vectors)
  SurfDescriptorExtractor extractor;

  Mat descriptorsOrg, descriptorsImage;

  extractor.compute( org, keypointsOrg, descriptorsOrg );
  extractor.compute( image, keypointsImage, descriptorsImage );

  //-- Step 3: Matching descriptor vectors using FLANN matcher
  FlannBasedMatcher matcher;
  std::vector< DMatch > matches;
  matcher.match( descriptorsOrg, descriptorsImage, matches );

  double max_dist = 0;
  double min_dist = 100;

  //-- Quick calculation of max and min distances between keypoints
  for( int i = 0; i < descriptorsOrg.rows; i++ )
  { double dist = matches[i].distance;
    if( dist < min_dist ) min_dist = dist;
    if( dist > max_dist ) max_dist = dist;
  }

  printf("-- Max dist : %f \n", max_dist );
  printf("-- Min dist : %f \n", min_dist );

  //-- Use "good" matches (i.e. whose distance is less than 10*min_dist )
  std::vector< DMatch > good_matches;

  for( int i = 0; i < descriptorsOrg.rows; i++ )
  { if( matches[i].distance < 10*min_dist )
     { good_matches.push_back( matches[i]); }
  }

  //-- Localize the object
  std::vector<Point2f> kptsOrg;
  std::vector<Point2f> kptsImage;

  for( int i = 0; i < good_matches.size(); i++ )
  {
    //-- Get the keypoints from the good matches
    kptsOrg.push_back( keypointsOrg[ good_matches[i].queryIdx ].pt );
    kptsImage.push_back( keypointsImage[ good_matches[i].trainIdx ].pt );
  }

  Mat H = findHomography( kptsImage, kptsOrg, CV_RANSAC );

  // Mat dst(img_object);
  warpPerspective( image, org, H, org.size() );
  imshow( "img_object", org);

  waitKey(0);
  return org;
  }

// Main function
int main(int argc, char** argv) {
  
  Mat image = imread( argv[1], CV_LOAD_IMAGE_GRAYSCALE );
  Mat imageAligned= alignImage( image );

}
