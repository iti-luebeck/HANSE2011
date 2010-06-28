#include "surfclassifier.h"
#include "helpers.h"

SURFClassifier::SURFClassifier(double thresh, QList<Mat> objects)
{
    SURFClassifier::thresh = thresh;
    SURFClassifier::objects = objects;
}

void SURFClassifier::classifyOne(int type, vector<KeyPoint> points, Mat features, FoundObject &match)
{
    match.left = INT_MAX;
    match.right = -1;
    match.top = INT_MAX;
    match.bottom = -1;
    match.numMatches = 0;
    match.percentMatches = 0.0;

    if (type < 0 || type >= objects.size())
    {
        return;
    }

    Mat objectFeatures = objects.at(type);

    for (int i = 0; i < features.cols; i++)
    {
        int bestk = -1;
        double best = 1e20;
        double secondBest = 1e20;
        Mat feature = Mat(64, 1, CV_32F, Scalar(0));
        feature = feature + features.col(i);
        for (int j = 0; j < objectFeatures.cols; j++)
        {
            Mat objectFeature = Mat(64, 1, CV_32F, Scalar(0));
            objectFeature = objectFeature + objectFeatures.col(j);
            double acc = norm(feature - objectFeature, NORM_L2);

            if (acc < best)
            {
                secondBest = best;
                best = acc;
                bestk = j;
            }
            else if (acc < secondBest)
            {
                secondBest = acc;
            }
        }

        if (best < thresh * secondBest && bestk >= 0)
        {
            if (points[i].pt.x < match.left)
            {
                match.left = points[i].pt.x;
            }
            if (points[i].pt.x > match.right)
            {
                match.right = points[i].pt.x;
            }
            if (points[i].pt.y < match.top)
            {
                match.top = points[i].pt.y;
            }
            if (points[i].pt.y > match.bottom)
            {
                match.bottom = points[i].pt.y;
            }
            match.numMatches++;
        }
    }

    match.percentMatches = 100.0 * ((double)match.numMatches) / ((double)features.cols);
}

void SURFClassifier::classify(Mat &image, QList<FoundObject> &matches)
{
    Mat imageGray;
    Mat features;
    vector<KeyPoint> keyPoints;
    vector<float> descriptors;

    Helpers::convertRGB2Gray(image, imageGray);

    SURF surf(1500);
    surf(imageGray, Mat::ones(imageGray.size(), CV_8UC1), keyPoints, descriptors);

    features.create(64, keyPoints.size(), CV_32F);
    features.setTo(Scalar(0));
    for (int i = 0; i < keyPoints.size(); i++)
    {
        for (int k = i*64; k < (i+1)*64; k++)
        {
            features.at<float>(k % 64, i) = descriptors[k];
        }
    }

    matches.clear();
    for (int i = 0; i < objects.size(); i++)
    {
        FoundObject fo;
        classifyOne(i, keyPoints, features, fo);
        matches.append(fo);
    }
}

void SURFClassifier::setObjects(QList<Mat>objects)
{
    SURFClassifier::objects = objects;
}

void SURFClassifier::setThresh(double thresh)
{
    SURFClassifier::thresh = thresh;
}

double SURFClassifier::getThresh()
{
    return thresh;
}
