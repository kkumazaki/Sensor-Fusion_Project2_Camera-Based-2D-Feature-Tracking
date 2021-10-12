/* INCLUDES FOR THIS PROJECT */
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <limits>
#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/xfeatures2d/nonfree.hpp>

#include "dataStructures.h"
#include "matching2D.hpp"

using namespace std;

/* MAIN PROGRAM */
int main(int argc, const char *argv[])
{

    /* INIT VARIABLES AND DATA STRUCTURES */

    // data location
    string dataPath = "../";

    // camera
    string imgBasePath = dataPath + "images/";
    string imgPrefix = "KITTI/2011_09_26/image_00/data/000000"; // left camera, color
    string imgFileType = ".png";
    int imgStartIndex = 0; // first file index to load (assumes Lidar and camera names have identical naming convention)
    int imgEndIndex = 9;   // last file index to load
    int imgFillWidth = 4;  // no. of digits which make up the file index (e.g. img-0001.png)

    // misc
    int dataBufferSize = 2;       // no. of images which are held in memory (ring buffer) at the same time
    vector<DataFrame> dataBuffer; // list of data frames which are held in memory at the same time
    //bool bVis = true;            // visualize results (Debug)
    bool bVis = false;            // visualize results

    /* MAIN LOOP OVER ALL IMAGES */

    for (size_t imgIndex = 0; imgIndex <= imgEndIndex - imgStartIndex; imgIndex++)
    {
        /* LOAD IMAGE INTO BUFFER */

        // assemble filenames for current index
        ostringstream imgNumber;
        imgNumber << setfill('0') << setw(imgFillWidth) << imgStartIndex + imgIndex;
        string imgFullFilename = imgBasePath + imgPrefix + imgNumber.str() + imgFileType;

        // load image from file and convert to grayscale
        cv::Mat img, imgGray;
        img = cv::imread(imgFullFilename);
        cv::cvtColor(img, imgGray, cv::COLOR_BGR2GRAY);

        //// STUDENT ASSIGNMENT
        //// TASK MP.1 -> replace the following code with ring buffer of size dataBufferSize
        cout << "-----imgIndex: " << imgIndex << "-----" << endl;

        // push image into data frame buffer
        DataFrame frame;
        frame.cameraImg = imgGray;
        dataBuffer.push_back(frame);

        int dataSize = dataBuffer.size();

        if (dataSize > dataBufferSize){
            dataBuffer.erase(dataBuffer.begin()); // There's no methos "pop_front"
            dataBuffer.shrink_to_fit(); // Release memory
        }

        cout << "dataBuffer size: " << dataBuffer.size() << endl;
        //cout << "dataBuffer capacity: " << dataBuffer.capacity() << endl;

        //// EOF STUDENT ASSIGNMENT
        cout << "#1 : LOAD IMAGE INTO BUFFER done" << endl;

        /* DETECT IMAGE KEYPOINTS */

        // extract 2D keypoints from current image
        vector<cv::KeyPoint> keypoints; // create empty feature list for current image
        string detectorType = "SIFT";  // Task MP.2 Modern fast methods: FAST, BRISK, ORB, AKAZE, SIFT
        //string detectorType = "HARRIS";  // Task MP.2 Another slow method: HARRIS
        //string detectorType = "SHITOMASI";  // This should be changed in Task MP.2

        //// STUDENT ASSIGNMENT
        //// TASK MP.2 -> add the following keypoint detectors in file matching2D.cpp and enable string-based selection based on detectorType
        //// -> HARRIS, FAST, BRISK, ORB, AKAZE, SIFT

        if (detectorType.compare("SHITOMASI") == 0) {
            detKeypointsShiTomasi(keypoints, imgGray, bVis);
            //detKeypointsShiTomasi(keypoints, imgGray, false);
        }
        else if (detectorType.compare("HARRIS") == 0) {
            detKeypointsHarris(keypoints, imgGray, bVis);
            //detKeypointsHarris(keypoints, imgGray, false);
        }
        else {
            detKeypointsModern(keypoints, imgGray, detectorType, bVis);
            //detKeypointsModern(keypoints, imgGray, detectorType, false);
        }
        //// EOF STUDENT ASSIGNMENT

        //// STUDENT ASSIGNMENT
        //// TASK MP.3 -> only keep keypoints on the preceding vehicle
        // Use it only in this project
        bool bFocusOnVehicle = true;
        vector<float> vehicleRect{535, 180, 180, 150}; // Roughly define rectangle area.
        //cv::Rect vehicleRect(535, 180, 180, 150); // Roughly define rectangle area.
        vector<cv::KeyPoint> keypointsCropped; // create empty feature list for after cropping
        int keypointSize = keypoints.size();

        if (bFocusOnVehicle)
        {
            /*
            // (i) push back
            for (int i = 0; i < keypointSize; i++){
                if ((keypoints[i].pt.x > vehicleRect[0]) && (keypoints[i].pt.x < (vehicleRect[0]+vehicleRect[2])) &&
                    (keypoints[i].pt.y > vehicleRect[1]) && (keypoints[i].pt.y < (vehicleRect[1]+vehicleRect[3]))) {
                        keypointsCropped.push_back(keypoints[i]);
                        cout << "cropped keypoints[" << i << "]: x=" << keypoints[i].pt.x << ", y=" << keypoints[i].pt.y << endl;
                }
            }
            */
           // (ii) erase (be careful about for iteration!)
            for (int i = keypointSize; i > 0; i--){
                if ((keypoints[i].pt.x < vehicleRect[0]) || (keypoints[i].pt.x > (vehicleRect[0]+vehicleRect[2])) ||
                    (keypoints[i].pt.y < vehicleRect[1]) || (keypoints[i].pt.y > (vehicleRect[1]+vehicleRect[3]))) {
                        keypoints.erase(keypoints.begin() + i);
                        //cout << "cropped keypoints[" << i << "]: x=" << keypoints[i].pt.x << ", y=" << keypoints[i].pt.y << endl;
                }
            }

            if (bVis == true) {
                cv::Mat visImage = img.clone();
                //cv::drawKeypoints(img, keypointsCropped, visImage, cv::Scalar::all(-1), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
                cv::drawKeypoints(img, keypoints, visImage, cv::Scalar::all(-1), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
                string windowName = detectorType + " Detector Results after Cropping"; 
                cv::namedWindow(windowName, 6);
                imshow(windowName, visImage);
                cv::waitKey(0);
            }
        }
        //// EOF STUDENT ASSIGNMENT

        // optional : limit number of keypoints (helpful for debugging and learning)
        // dont't set true except for debugging
        bool bLimitKpts = false;
        if (bLimitKpts)
        {
            int maxKeypoints = 50;

            if (detectorType.compare("SHITOMASI") == 0)
            { // there is no response info, so keep the first 50 as they are sorted in descending quality order
                keypoints.erase(keypoints.begin() + maxKeypoints, keypoints.end());
            }
            cv::KeyPointsFilter::retainBest(keypoints, maxKeypoints);
            cout << " NOTE: Keypoints have been limited!" << endl;
        }

        // push keypoints and descriptor for current frame to end of data buffer
        (dataBuffer.end() - 1)->keypoints = keypoints;
        cout << "#2 : DETECT KEYPOINTS done" << endl;

        /* EXTRACT KEYPOINT DESCRIPTORS */

        //// STUDENT ASSIGNMENT
        //// TASK MP.4 -> add the following descriptors in file matching2D.cpp and enable string-based selection based on descriptorType
        //// -> BRIEF, ORB, FREAK, AKAZE, SIFT

        cv::Mat descriptors;
        string descriptorType = "ORB"; // BRIEF, ORB, FREAK, AKAZE, SIFT, BRISK
        //string descriptorType = "BRISK"; // BRIEF, ORB, FREAK, AKAZE, SIFT
        descKeypoints((dataBuffer.end() - 1)->keypoints, (dataBuffer.end() - 1)->cameraImg, descriptors, descriptorType);
        //// EOF STUDENT ASSIGNMENT

        // push descriptors for current frame to end of data buffer
        (dataBuffer.end() - 1)->descriptors = descriptors;

        cout << "#3 : EXTRACT DESCRIPTORS done" << endl;

        if (dataBuffer.size() > 1) // wait until at least two images have been processed
        {

            /* MATCH KEYPOINT DESCRIPTORS */

            vector<cv::DMatch> matches;
            string matcherType = "MAT_BF";        // MAT_BF, MAT_FLANN
            //string matcherType = "MAT_BF";        // MAT_BF, MAT_FLANN
            //string descriptorType = "DES_HOG"; // DES_BINARY, DES_HOG
            string descriptorType = "DES_BINARY"; // DES_BINARY, DES_HOG
            string selectorType = "SEL_KNN";       // SEL_NN, SEL_KNN
            //string selectorType = "SEL_NN";       // SEL_NN, SEL_KNN

            //// STUDENT ASSIGNMENT
            //// TASK MP.5 -> add FLANN matching in file matching2D.cpp
            //// TASK MP.6 -> add KNN match selection and perform descriptor distance ratio filtering with t=0.8 in file matching2D.cpp

            matchDescriptors((dataBuffer.end() - 2)->keypoints, (dataBuffer.end() - 1)->keypoints,
                             (dataBuffer.end() - 2)->descriptors, (dataBuffer.end() - 1)->descriptors,
                             matches, descriptorType, matcherType, selectorType);

            //// EOF STUDENT ASSIGNMENT

            // store matches in current data frame
            (dataBuffer.end() - 1)->kptMatches = matches;

            cout << "#4 : MATCH KEYPOINT DESCRIPTORS done" << endl;

            // visualize matches between current and previous image
            bVis = true;
            if (bVis)
            {
                cv::Mat matchImg = ((dataBuffer.end() - 1)->cameraImg).clone();
                cv::drawMatches((dataBuffer.end() - 2)->cameraImg, (dataBuffer.end() - 2)->keypoints,
                                (dataBuffer.end() - 1)->cameraImg, (dataBuffer.end() - 1)->keypoints,
                                matches, matchImg,
                                cv::Scalar::all(-1), cv::Scalar::all(-1),
                                vector<char>(), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

                string windowName = "Matching keypoints between two camera images";
                cv::namedWindow(windowName, 7);
                cv::imshow(windowName, matchImg);
                cout << "Press key to continue to next image" << endl;
                cv::waitKey(0); // wait for key to be pressed
            }
            bVis = false;
        }

    } // eof loop over all images

    return 0;
}