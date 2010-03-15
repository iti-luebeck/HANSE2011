#ifndef _SVMCLASSIFIER_
#define _SVMCLASSIFIER_

#include "cv.h"
#include "ml.h"

#define ITERATIONS 100000

class SVMClassifier
{
public:
	SVMClassifier(void);
	~SVMClassifier(void);

private:
	CvSVM* svm;

private:
	int svmClassification(CvMat* sample);
public:
	void train(CvMat* data, CvMat *classes);
	CvMat* classify(CvMat* data);
	void saveClassifier(const char* fileName);
	void loadClassifier(const char* fileName);
};

#endif
