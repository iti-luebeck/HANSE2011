#ifndef _SVMCLASSIFIER_
#define _SVMCLASSIFIER_

#include <opencv/cv.h>
#include <opencv/ml.h>

#define ITERATIONS 100000

class SVMClassifier
{
public:
	SVMClassifier(void);
	~SVMClassifier(void);

private:
	CvSVM* svm;
        bool loaded;

public:
        int svmClassification(CvMat* sample);
	void train(CvMat* data, CvMat *classes);
	CvMat* classify(CvMat* data);
	void saveClassifier(const char* fileName);
        bool loadClassifier(const char* fileName);
        bool isSVM();
};

#endif
