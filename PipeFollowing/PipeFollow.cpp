
#include "PipeFollow.h"
#include <highgui.h>
#include <QDebug>
//#include <math.h>
//#include <iostream>



    PipeFollow::PipeFollow(int threshSegmentation, int debug, int deltaRohr)
{
    PipeFollow::threshSegmentation = threshSegmentation;
    PipeFollow::debug = debug;
    PipeFollow::deltaRohr = deltaRohr;
    PipeFollow::speed = 10.0;
    PipeFollow::kp = 5.0;
    PipeFollow::robCenter = Point(320.0,176.0);
//    PipeFollow::robCenter = Point(176,320);
}

    /** Regelt Hanse's Rohrverfolgung
        int pVec - Potentialvektor
        * positiv: Schnittpunkt der Ideallinie und des Rohrs liegen vor dem Robotermittelpunkt (Point robCenter)
        * negativ: Schnittpunkt der Ideallinie und des Rohrs liegen hinter dem Robotermittelpunkt
        float istWinkel - Der Winkel zwischen Rohr und Ideallinie
        Point robCenter - Mittelpunkt des Roboters
        Point schnittP - Schnittpunkt zwischen Rohr und Ideallinie
        vector<Vec2f> pipe - Das erkennte Rohr
        float kp - Regelparameter Kp des P-Reglers
        float speed - Geschwindigkeit des Roboters (konstant)
        int deltaRohr - Tolleranzbereich indem der Sollwinkel nicht verändert wird

    */

    void PipeFollow::pRegler()
    {
        float sollWinkel;
        /*Fälle: Winkel ist nahezu 0 Grad und es gibt kein Schnittpunkt Roboter parallel zum Rohr */
        if(istWinkel < 2.5)
        {
            return;
        }
        /*Fälle: Großes Potential (negativ oder positiv)
         * bei kleinem Winkel (bis 6 Grad)
         */

    }





/** Findet das Rohr und berechnet Schnittpunkte usw. */

void PipeFollow::findPipe(QString videoFile)
{


    VideoCapture vc(videoFile.toStdString());
    qDebug() << vc.isOpened();
    vc.open(videoFile.toStdString().c_str());
    qDebug() << vc.isOpened();
    if (vc.isOpened())
    {
        qDebug() << "los jetzt";
        namedWindow("Image", 1);
        namedWindow("Canny",1);
       for(;;)
        {

            Mat frame;
            Mat frameHSV;
            vc >> frame;
            Mat tframe = frame;
//            transpose(tframe,frame);
            if (!frame.empty())
            {
                cvtColor(frame,frameHSV,CV_BGR2HSV);
//                  imshow("Image",frameHSV);

            }
            int u;
            Mat cannyFrame;
            Mat grau;
            cvtColor(frame, grau, CV_BGR2GRAY);

            /*****convert HSV Image to 3 Binary */
            int rows = frameHSV.rows;
            int cols = frameHSV.cols;
            Mat h, s, v;
            h.create(rows,cols,CV_8UC1);
            s.create(rows,cols,CV_8UC1);
            v.create(rows,cols,CV_8UC1);

            for (int i = 0; i < rows; i++)
            {
                for (int j = 0; j < cols; j++)
                {
                    Vec<unsigned char, 3> hsvV = frameHSV.at<Vec<unsigned char, 3> >(i, j);
                    h.at<unsigned char>(i, j) = hsvV[0];
                    s.at<unsigned char>(i, j) = hsvV[1];
                    v.at<unsigned char>(i, j) = hsvV[2];

                }
            }

            /**** Segmentation */
            Mat thresh;
            threshold(s,thresh,threshSegmentation,255,THRESH_BINARY);
            Canny(thresh, cannyFrame, 50, 200, 3 );

            /*debug */
            if(debug)
            {
              imshow("Canny",frameHSV);
              u = waitKey();
              imshow("Canny",h);
              u = waitKey();
              imshow("Canny",s);
              u = waitKey();
              imshow("Canny",v);
              u = waitKey();
              imshow("Canny",grau);
              u = waitKey();
              imshow("Canny",thresh);
              u = waitKey();
              imshow("Canny",cannyFrame);
              u = waitKey();

             }

            /* hough transformation */
            vector<Vec2f> lines;
            HoughLines(cannyFrame, lines, 1, CV_PI/180, 100 );

            /* DEBUG durchschnitt fuer alle erkannten linien */
            float avRho = 0.0;
            float avTheta = 0.0;

            for( size_t i = 0; i < lines.size(); i++ )
            {
                if(debug)
                {
                  PipeFollow::drawLineHough(frame,lines[i][0],lines[i][1],Scalar(0,0,255));
                  qDebug() << "rho theta" << lines[i][0] << lines[i][1];
                }
                 avRho += lines[i][0];
                 avTheta += lines[i][1];
             }
            avRho /= lines.size();
            avTheta /= lines.size();
            if(debug)
            {
              qDebug() << "****************";
              qDebug() << "= " << avRho << avTheta;
              PipeFollow::drawLineHough(frame,avRho,avTheta,Scalar(255,0,255));
            }

            /* jeweils Durchschnitt fuer linke und rechte Seite des Rohrs berechnen */
            float counterClass1 = 0;
            float counterClass2 = 0;
            float avRhoClass1 = 0.0;
            float avRhoClass2 = 0.0;
            float avThetaClass1 = 0.0;
            float avThetaClass2 = 0.0;

            for (size_t i = 0; i < lines.size(); i++)
            {

                if(lines[i][0] < avRho)
                { //Class1
                    counterClass1 += 1.0;
                    avRhoClass1 += lines[i][0];
                    avThetaClass1 += lines[i][1];
                }
                else
                { //Class2
                    counterClass2 += 1.0;
                    avRhoClass2 += lines[i][0];
                    avThetaClass2 += lines[i][1];
                }

            }
            avRhoClass1 /= counterClass1;
            avThetaClass1 /= counterClass1;
            avRhoClass2 /= counterClass2;
            avThetaClass2 /= counterClass2;

            /***** DEBUG linkes und recht berechnen und Zeichnen */
            if(debug)
            {
              qDebug() << "Class 1" << avRhoClass1 << avThetaClass1;
              qDebug() << "Class 2" << avRhoClass2 << avThetaClass2;
              PipeFollow::drawLineHough(frame,avRhoClass1,avThetaClass1,Scalar(255,255,255));
              PipeFollow::drawLineHough(frame,avRhoClass2,avThetaClass2,Scalar(255,255,255));

            }

            /* Parameter fuers Rohr bestimmen */
             avRhoClass1 = (avRhoClass1 + avRhoClass2) / 2.0;
             avThetaClass1 = (avThetaClass1 + avThetaClass2) / 2.0;

            /* Istwinkel */
           istWinkel = ((avThetaClass1 * 180.0) / CV_PI);
            qDebug() << "Winkel " << istWinkel;


            /* Rohrlinie berechnen und Zeichnen */
            PipeFollow::drawLineHough(frame,avRhoClass1,avThetaClass1,Scalar(255,255,255));
            if(debug)
            {
               qDebug() << "rohr " << avRhoClass1 << avThetaClass1;
            }

            /* Schnittpunkt des erkannten Rohrs mit der Ideallinie */
            PipeFollow::compIntersect(avRhoClass1,avThetaClass1);
            if(debug)
            {
                qDebug() << "Schnittpunkt " << schnittP.x << " " << schnittP.y;
                circle(frame,schnittP,3,Scalar(255,0,255),3,8);
                qDebug() << "deltaA " << deltaA.x << deltaA.y;
                circle(frame,deltaA,3,Scalar(0,0,255),3,8);

                circle(frame,robCenter,3,Scalar(255,0,255),3,8);

            }

            /* Potentialvektor berechnen */
            pVec = -schnittP.y + robCenter.y;
            qDebug() << "potential " << pVec;

            /* Ideallinie zeichnen */
            line( frame,Point(frame.cols/2,0.0) , Point(frame.cols/2,frame.rows), Scalar(0,255,0), 3, 8 );


            imshow("Image",frame);

            if(debug)
            {
                waitKey();
            }
            u = waitKey(100);
            if (u == 'q')
                break;



        }
    vc.release();
    cvDestroyWindow("Image");
    cvDestroyWindow("Canny");
}
}

/** Schwellwerte anpassen */
void PipeFollow::setThresh(int thresh)
{
    threshSegmentation = thresh;
}

/** Debug Ausgaben, sowie Unterbrechungen zur Kontrolle  */

void PipeFollow::setDebug(int debug)
{
    PipeFollow::debug = debug;
}

/** Zeichnet eine Linie aus gegebenen Hough Parametern in ein Bild
    Mat frame - Das Bild
    double rho - Parameter der Gerade
    double theta - Parameter der Gerade
    Scalar color - Farbe der Linie
 */
void PipeFollow::drawLineHough(Mat &frame, double rho, double theta, Scalar color)
{
    double a = cos(theta), b = sin(theta);
    double x1 = a*rho, y1 = b*rho;
    Point pt1(cvRound(x1 + 1000*(-b)),
              cvRound(y1 + 1000*(a)));
    Point pt2(cvRound(x1 - 1000*(-b)),
              cvRound(y1 - 1000*(a)));
    line( frame, pt1, pt2, color, 3, 8 );
}

/** Schnittpunkt der erkannten Gerade und der Ideallinie bestimmen
    double rho - Parameter der Gerade
    double theta - Parameter der Gerade
 */
void PipeFollow::compIntersect(double rho, double theta)
{
    double a = cos(theta), b = sin(theta);
    double x1 = a*rho, y1 = b*rho;
    Point pt1(cvRound(x1 + 1000*(-b)),
              cvRound(y1 + 1000*(a)));
    Point pt2(cvRound(x1 - 1000*(-b)),
              cvRound(y1 - 1000*(a)));
    Point richtungsv((pt2.x - pt1.x) , (pt2.y - pt1.y));
    double rv[2] = {(pt2.x - pt1.x) , (pt2.y - pt1.y)};
    float r = ((robCenter.x)/1.0 - pt1.x)/rv[0];
    Point gmp(pt1.x + (((r) * rv[0])) ,
              pt1.y + (((r) * rv[1])));
    schnittP = gmp;
    /* y Schnittpunkt */
    r = ((robCenter.y)/1.0 - pt1.y)/rv[1];
    Point gmp2(pt1.x + (((r) * rv[0])) ,
              pt1.y + (((r) * rv[1])));
    deltaA = gmp2;


    /*Abstand zwischen ermittelter Gerade und robCenter */
    /*Richtungsvekter double[] rv und Stuetzvektor Point pt1 */
    double tmp = (((robCenter.x - pt1.x) * rv[0]) + ((robCenter.y - pt1.y)* rv[1]));
    double spRV = (rv[0] * rv[0]) + (rv[1] * rv[1]);
    double c[2] = {0,0};
    c[0] = spRV * rv[0] + pt1.x;
    c[1] = spRV * rv[1] + pt1.y;
    double d[2] ={0, 0};
    d[0] = robCenter.x - c[0];
    d[1] = robCenter.y - c[1];

    double norm = sqrt((d[0] * d[0]) + (d[1] * d[1]));
    qDebug() << "Norm" << norm;

    /* abstand zwischen gmp2 und ideallinie */
    /* ideal zwischen robCenter.x, robCenter.y und robCenter.x und robCenter.y *2 */
//    double idealRV[2] = {0.0 , robCenter.y};
//    double normRV = sqrt(idealRV[1] * idealRV[1]);
//    double ap[2] = {gmp2.x - robCenter.x , gmp2.y - robCenter.y};
//    double normAP = sqrt((ap[0] * ap[0]) + (ap[1] * ap[1]));
//    double cosphi = ((((ap[0]) * idealRV[0]) + ((ap[1]) * idealRV[1])) / (normRV * normAP ));
//    cosphi = acos(cosphi);
//    cosphi = sin(cosphi);
//   deltaRohr = normAP * cosphi;
//    qDebug() << "Abstand " << deltaRohr;
}

void PipeFollow::findPipeInPic(QString picture)
{

    namedWindow("Image", 1);
    namedWindow("Canny",1);

    String str = picture.toStdString();
     int u;
    Mat frame;
    Mat frameHSV;
    frame = cvLoadImageM(str.c_str(), CV_LOAD_IMAGE_COLOR);
    imshow("Image",frame);
     u = waitKey();


    Mat grau;
    cvtColor(frame,grau,CV_BGR2GRAY);
   cvtColor(frame,frameHSV,CV_BGR2HSV);

   imshow("Image",frameHSV);

    /*****convert HSV Image to 3 Binary */
    int rows = frameHSV.rows;
    int cols = frameHSV.cols;
    Mat cannyFrame, h, s, v;
    h.create(rows,cols,CV_8UC1);
    s.create(rows,cols,CV_8UC1);
    v.create(rows,cols,CV_8UC1);

    Mat r,g,b;
    r.create(rows,cols,CV_8UC1);
    g.create(rows,cols,CV_8UC1);
    b.create(rows,cols,CV_8UC1);

    int count[360];
    for(int i = 0; i <360; i++)
    {
        count[i] = 0;
    }
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            Vec<unsigned char, 3> hsvV = frameHSV.at<Vec<unsigned char, 3> >(i, j);
            h.at<unsigned char>(i, j) = hsvV[0];
            s.at<unsigned char>(i, j) = hsvV[1];
            v.at<unsigned char>(i, j) = hsvV[2];

            Vec<unsigned char, 3> hsvV2;



           count[hsvV[0]]++;

            if((h.at<unsigned char>(i, j)) < 42)
            {
                hsvV2[0] = 0;
                hsvV2[1] = 10;
                hsvV2[2] = 0;
            } else
            {
            hsvV2[0] = h.at<unsigned char>(i, j);
            hsvV2[1] = s.at<unsigned char>(i, j);
            hsvV2[2] = v.at<unsigned char>(i, j);
            }


            frameHSV.at<Vec<unsigned char, 3> >(i, j) = hsvV2;

        }
    }
    Mat thresh, tmp;
//    for(int i = 0; i<180;i++)
//    {
//        qDebug() << i << " - " << count[i];
//    }




//    imshow("Canny",frameHSV);
//    u = waitKey();
//    imshow("Canny",h);
//    u = waitKey();
//    imshow("Canny",s);
//    u = waitKey();
//    imshow("Canny",v);
//    u = waitKey();

//    threshold(h,thresh,42,255,THRESH_TOZERO_INV);
    threshold(s,thresh,170,255,THRESH_TRUNC);
    imshow("Canny", thresh);
    u = waitKey();

    equalizeHist(thresh,thresh);
    imshow("Canny", thresh);
    u = waitKey();

    threshold(thresh,thresh,142,255,THRESH_TRUNC);
    imshow("Canny", thresh);
    u = waitKey();

    equalizeHist(thresh,thresh);
    imshow("Canny", thresh);
    u = waitKey();

    threshold(thresh,thresh,142,255,THRESH_TOZERO);
    imshow("Canny", thresh);
    u = waitKey();

//        threshold(h,thresh,42,255,THRESH_BINARY_INV);
    dilate(thresh,thresh,Mat(),Point(-1,-1),5);
    imshow("Canny", thresh);
    u = waitKey();
    erode(thresh,thresh,Mat(),Point(-1,-1),10);
    imshow("Canny", thresh);
    u = waitKey();


    Moments M;
    M = moments(thresh,1);


    double theta = 0.5 * atan2( 2 * (M.mu11 / M.m00) , ((M.mu20 / M.m00) - (M.mu02 /M.m00) ) );

    line( thresh, Point( (M.m10 / M.m00), (M.m01 /M.m00) ),
            Point( (M.m10 /M.m00) + cos(theta)*200, (M.m01 /M.m00) + sin(theta)*200 ),
            Scalar( 255, 0, 255 ), 3, 8);

    imshow("Canny", thresh);
    u = waitKey();


    equalizeHist(h,thresh);
    imshow("Canny", thresh);
    u = waitKey();
    threshold(thresh,tmp,threshSegmentation,255,THRESH_TOZERO_INV);
    imshow("Canny",tmp);
    u = waitKey();
    equalizeHist(tmp,thresh);
    imshow("Canny",thresh);
    u = waitKey();
    threshold(thresh,tmp,threshSegmentation,255,THRESH_TOZERO_INV);
    imshow("Canny",tmp);
    u = waitKey();


    /**** Segmentation */

    threshold(v,thresh,threshSegmentation,255,THRESH_BINARY);

    Canny(tmp, cannyFrame, 50, 100, 3 );


    equalizeHist(grau,thresh);

    /*debug */
    if(debug)
    {
      imshow("Canny",frameHSV);
      u = waitKey();
      imshow("Canny",h);
      u = waitKey();
      imshow("Canny",s);
      u = waitKey();
      imshow("Canny",v);
      u = waitKey();
      imshow("Canny",grau);
      u = waitKey();
      imshow("Canny",thresh);
      u = waitKey();
      imshow("Canny",cannyFrame);
      u = waitKey();

     }

    /* hough transformation */
    vector<Vec2f> lines;
    HoughLines(cannyFrame, lines, 1, CV_PI/180, 100 );


cvDestroyWindow("Image");
cvDestroyWindow("Canny");


}

