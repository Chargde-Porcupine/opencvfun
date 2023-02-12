#include <opencv2/opencv.hpp>
#include <iostream>
#include <numeric>
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

vector<cv::Point> corners(vector<cv::Point> intersection)
{
    // return in the clockwise order: tl, tr, br, bl
    vector<cv::Point> ret = {cv::Point(1000, 1000), cv::Point(0, 1000), cv::Point(0, 0), cv::Point(1000, 0)};
    for (size_t i = 0; i < intersection.size(); i++)
    {
        // topleft
        if (intersection[i].x <= ret[0].x && intersection[i].y <= ret[0].y)
        {
            ret[0].x = intersection[i].x;
            ret[0].y = intersection[i].y;
        }
        // topright
        if (intersection[i].x >= ret[1].x && intersection[i].y <= ret[1].y)
        {
            ret[1].x = intersection[i].x;
            ret[1].y = intersection[i].y;
        }
        // bottomright
        if (intersection[i].x >= ret[2].x && intersection[i].y >= ret[2].y)
        {
            ret[2].x = intersection[i].x;
            ret[2].y = intersection[i].y;
        }
        // bottomleft
        if (intersection[i].x <= ret[3].x && intersection[i].y >= ret[3].y)
        {
            ret[3].x = intersection[i].x;
            ret[3].y = intersection[i].y;
        }
    }
    return ret;
}

int main()
{
    cv::Mat sourceImg;
    int rightmost, leftmost, topmost, bottommost;
    sourceImg = cv::imread("gogo.jpg", cv::IMREAD_GRAYSCALE);
    vector<cv::Vec3f> circles;
    cv::HoughCircles(sourceImg, circles, cv::HOUGH_GRADIENT, 1,
                     sourceImg.rows / 20, // change this value to detect circles with different distances to each other
                     100, 23, 10, 27      // change the last two parameters
                                          // (min_radius & max_radius) to detect larger circles
    );
    cv::Mat m = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
    cv::Mat rected;
    cv::morphologyEx(sourceImg, rected, cv::MORPH_BLACKHAT, m);
    cv::Mat cannied;
    cv::Canny(rected, cannied, 50, 200, 3);
    vector<cv::Vec2f> lines;
    vector<int> horizlines, verticlines, hbounds, vbounds;
    cv::Point isect;
    vector<cv::Point> isects, cornr;
    // vertical lines have theta of 0 or 180 ~
    // horizontal lines have theta of 90 ~
    // lets give an error of 10 deg
    cv::HoughLines(cannied, lines, 1, (CV_PI) / 180, 250, 0, 0);
    for (size_t i = 0; i < lines.size(); i++)
    {
        float rho = lines[i][0], theta = lines[i][1];

        if (abs(0 - theta) <= 0.1)
        {
            verticlines.push_back(i);
            vbounds.push_back(rho * cos(theta));
        }
        else if (abs(CV_PI / 2 - theta) <= 0.1)
        {
            horizlines.push_back(i);
            hbounds.push_back(rho * cos(theta));
        }
    }
    for (size_t i = 0; i < horizlines.size(); i++)
    {

        float frho = lines[horizlines[i]][0], ftheta = lines[horizlines[i]][1];
        for (size_t j = 0; j < verticlines.size(); j++)
        {
            float srho = lines[verticlines[j]][0], stheta = lines[verticlines[j]][1];
            isect = intersect(ftheta, frho, stheta, srho);
            if (isect.x == -1)
            {
                continue;
            }
            isects.push_back(isect);
        }
    }
    cornr = corners(isects);
    for (size_t i = 0; i < cornr.size(); i++)
    {
        circle(sourceImg, cornr[i], 1, cv::Scalar(0, 100, 100), 6, cv::LINE_AA);
    }
    int modediff, meanrad;
    int diffs[vbounds.size()];
    map<int, int> amountsofdiffs;

    //!Need to run sanity check on values

    for (size_t i = 0; i < circles.size(); i++)
    {
        meanrad += (2 * circles[i][2]);
    }
    meanrad /= circles.size();
    cout << meanrad << endl;
    // sort vbounds here
    sort(vbounds.begin(), vbounds.end());
    adjacent_difference(vbounds.begin(), vbounds.end(), diffs);
    for (size_t i = 0; i < vbounds.size() - 1; i++)
    {
        //cout << diffs[i] << endl;
        if(abs(meanrad - diffs[i]) > 10){
            continue;
        }
        if(amountsofdiffs[diffs[i]]){
            amountsofdiffs[diffs[i]]++;
        } else{
            amountsofdiffs[diffs[i]] = 1;
        }

        // rewrite
    }
    modediff = max_element(amountsofdiffs.begin(), amountsofdiffs.end(),
    [](const pair<int, int>& p1, const pair<int, int>& p2) {
        return p1.second < p2.second; }) -> first;

    cout << modediff << endl;
    for (size_t i = 0; i < circles.size(); i++)
    {
        cv::Vec3i c = circles[i];
        cv::Point center = cv::Point(c[0], c[1]);
        // circle center
        cv::circle(sourceImg, center, 1, cv::Scalar(0, 100, 100), 1, cv::LINE_AA);
        // circle outline
        int radius = c[2];
        circle(sourceImg, center, radius, cv::Scalar(0, 100, 100), 1, cv::LINE_AA);
        // take distance of radius from left and top divided by radius

        cv::putText(sourceImg, to_string(int(round((float(center.x) - cornr[3].x) / (2 * modediff)))), cv::Point(center.x + 2, center.y), cv::FONT_HERSHEY_COMPLEX_SMALL, 0.7, cv::Scalar(0, 100, 100));
    }

    cv::imshow("Go boardUnmodified", sourceImg);

    cv::waitKey();
    cv::destroyAllWindows();
    return 0;
}
