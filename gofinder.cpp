#include <opencv2/opencv.hpp>
#include<iostream>
using namespace std;

cv::Point intersect(float ftheata, float frho, float stheata, float srho){
    float topleft = cosf(ftheata);
    float topright = sinf(ftheata);
    float bottomleft = cosf(stheata);
    float bottomright = sinf(stheata);
    float determinant = (topleft * bottomright) - (topright - bottomleft);
    if(determinant == 0.0f){
        //possibly not the best way, but this shouldnt really happen
        return cv::Point(-1, -1);
    }
}


int main(){
    cv::Mat sourceImg;
    int rightmost, leftmost, topmost, bottommost;
    sourceImg = cv::imread("gogo.jpg", cv::IMREAD_GRAYSCALE);
    vector<cv::Vec3f> circles;
    cv::HoughCircles(sourceImg, circles, cv::HOUGH_GRADIENT, 1,
                 sourceImg.rows/20,  // change this value to detect circles with different distances to each other
                 100, 23, 10, 25 // change the last two parameters
            // (min_radius & max_radius) to detect larger circles
    );
    cv::Mat m = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5,5));
    cv::Mat rected;
    cv::morphologyEx(sourceImg, rected, cv::MORPH_BLACKHAT, m);
    cv::Mat cannied;
    cv::Canny(rected, cannied, 50, 200, 3);
    vector<cv::Vec2f> lines;
    vector<int> horizlines, verticlines;
    //horizontal lines have theta of 0 or 180 ~
    //vertical lines have theta of 90 ~
    //lets give an error of 10 deg
    cv::HoughLines(cannied, lines, 1, (CV_PI)/180, 250, 0, 0 );
    for( size_t i = 0; i < lines.size(); i++ )
    {
        float rho = lines[i][0], theta = lines[i][1];
        cv::Point pt1, pt2;
        if(abs(0-theta) <= 10 || abs(180-theta) <= 10){
            horizlines.push_back(i);
        } else if(abs(90 - theta) <= 10){
            verticlines.push_back(i);
        }
        double a = cos(theta), b = sin(theta);
        double x0 = a*rho, y0 = b*rho;
        pt1.x = cvRound(x0 + 1000*(-b));
        pt1.y = cvRound(y0 + 1000*(a));
        pt2.x = cvRound(x0 - 1000*(-b));
        pt2.y = cvRound(y0 - 1000*(a));
        cv::line( sourceImg ,pt1, pt2, cv::Scalar(50,0,255), 3, cv::LINE_AA);
    }
    for( size_t i = 0; i < circles.size(); i++ )
    {
        cv::Vec3i c = circles[i];
        cv::Point center = cv::Point(c[0], c[1]);
        // circle center
        cv::circle( sourceImg, center, 1, cv::Scalar(0,100,100), 3, cv::LINE_AA);
        // circle outline
        int radius = c[2];
        circle( sourceImg, center, radius, cv::Scalar(0,100,100), 3, cv::LINE_AA);
    }

    


    cv::imshow("Go boardUnmodified", sourceImg);
    
    
    cv::waitKey();
    cv::destroyAllWindows();
    return 0;
}