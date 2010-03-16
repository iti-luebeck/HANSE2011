#include "helpers.h"

Helpers::Helpers()
{
}

void Helpers::convertRGB2Gray(Mat rgb, Mat &gray)
{
    gray.create(rgb.size(), CV_8UC1);
    gray.setTo(Scalar(0));
    for (int i = 0; i < rgb.rows; i++)
    {
        for (int j = 0; j < rgb.cols; j++)
        {
            Vec<unsigned char, 3> v = rgb.at<Vec<unsigned char, 3> >(i, j);
            gray.at<unsigned char>(i, j) = 0.299 * v[0] + 0.587 * v[1] + 0.114 * v[2];
        }
    }
}

void Helpers::convertBGR2Gray(Mat bgr, Mat &gray)
{
    gray.create(bgr.size(), CV_8UC1);
    gray.setTo(Scalar(0));
    for (int i = 0; i < bgr.rows; i++)
    {
        for (int j = 0; j < bgr.cols; j++)
        {
            Vec<unsigned char, 3> v = bgr.at<Vec<unsigned char, 3> >(i, j);
            gray.at<unsigned char>(i, j) = 0.299 * v[2] + 0.587 * v[1] + 0.114 * v[0];
        }
    }
}
