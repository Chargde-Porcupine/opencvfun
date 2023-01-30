#include <opencv2/opencv.hpp>
#include <iostream>
using namespace std;

cv::Point intersect(float ftheata, float frho, float stheata, float srho)
{
    float determinant = (cosf(ftheata) * sinf(stheata)) - (sinf(ftheata) - cosf(stheata));
    if (determinant == 0.0f)
    {
        // possibly not the best way, but this shouldnt really happen
        return cv::Point(-1, -1);
    }
    return cv::Point((sinf(stheata) * frho - sinf(ftheata) * srho) / determinant, (-1 * cosf(stheata) * frho + cosf(ftheata) * srho) / determinant);
}


int main()
{
    cv::Mat sourceImg;
    int rightmost, leftmost, topmost, bottommost;
    sourceImg = cv::imread("gogo.jpg", cv::IMREAD_GRAYSCALE);
    vector<cv::Vec3f> circles;
    cv::HoughCircles(sourceImg, circles, cv::HOUGH_GRADIENT, 1,
                     sourceImg.rows / 20, // change this value to detect circles with different distances to each other
                     100, 23, 10, 25      // change the last two parameters
                                          // (min_radius & max_radius) to detect larger circles
    );
    cv::Mat m = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
    cv::Mat rected;
    cv::morphologyEx(sourceImg, rected, cv::MORPH_BLACKHAT, m);
    cv::Mat cannied;
    cv::Canny(rected, cannied, 50, 200, 3);
    vector<cv::Vec2f> lines;
    vector<int> horizlines, verticlines, ybounds, xbounds;
    // horizontal lines have theta of 0 or 180 ~
    // vertical lines have theta of 90 ~
    // lets give an error of 10 deg
    cv::HoughLines(cannied, lines, 1, (CV_PI) / 180, 250, 0, 0);
    for( size_t i = 0; i < lines.size(); i++ )
    {
        float rho = lines[i][0], theta = lines[i][1];
        cv::Point pt1, pt2;
        if(abs(0-theta) <= 0.1){
            horizlines.push_back(i);
            ybounds.push_back((rho-500*cosf(theta))/sinf(theta));
            
        } else if(abs(CV_PI/2 - theta) <= 0.1){
            verticlines.push_back(i);
            xbounds.push_back((rho-500*sinf(theta))/cosf(theta));
        }
    }
    for (size_t i = 0; i < horizlines.size(); i++)
    {

        float frho = lines[horizlines[i]][0], ftheta = lines[horizlines[i]][1];
        for (size_t j = 0; j < verticlines.size(); j++)
        {
            float srho = lines[verticlines[j]][0], stheta = lines[verticlines[j]][1];
            cv::Point isect = intersect(ftheta, frho, stheta, srho);
            if (isect.x == -1)
            {
                continue;
            }
            cout << isect.x << isect.y << endl;
            cv::circle(sourceImg, isect, 1, cv::Scalar(0, 100, 100), 3, cv::LINE_AA);
        }
    }
    for (size_t i = 0; i < circles.size(); i++)
    {
        cv::Vec3i c = circles[i];
        cv::Point center = cv::Point(c[0], c[1]);
        // circle center
        cv::circle(sourceImg, center, 1, cv::Scalar(0, 100, 100), 3, cv::LINE_AA);
        // circle outline
        int radius = c[2];
        circle(sourceImg, center, radius, cv::Scalar(0, 100, 100), 3, cv::LINE_AA);
    }

    //do something with x and y bounds idk

    cv::imshow("Go boardUnmodified", sourceImg);

    cv::waitKey();
    cv::destroyAllWindows();
    return 0;
}
