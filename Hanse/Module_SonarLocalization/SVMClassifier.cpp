#include "SVMClassifier.h"

#include <QtCore>

SVMClassifier::SVMClassifier(void)
{
        svm = new CvSVM();
}

SVMClassifier::~SVMClassifier(void)
{
	delete(svm);
}

void SVMClassifier::train(CvMat *data, CvMat *classes)
{
	// Scale training data to an interval from 0 to 1 for all components.
        //calcScale(data);
        //scale(data);


    qDebug("SVM TRAIN");
    for (int i = 0; i < classes->rows; i++) {

        qDebug("%d", (int)cvGet2D(classes, i, 0).val[0]);
        qDebug("%d", (int)cvGet2D(data,0,0).val[0]);
    }


	// Set up class weights for two-class classifier (should be equal).
	CvMat* class_weights = cvCreateMat(1, 2, CV_32FC1);
	cvmSet(class_weights, 0, 0, 1);
	cvmSet(class_weights, 0, 1, 1);

	// Set up termination criteria. Will terminate after some given number of iterations.
	CvTermCriteria term_crit = {CV_TERMCRIT_ITER, ITERATIONS, 0};

	// Set up the SVM parameters. The SVM will be a nu-SVM with a Radial Base Function-Kernel.
	CvSVMParams params;
	params.svm_type = CvSVM::NU_SVC;
	params.kernel_type = CvSVM::RBF;
	// params.degree = 0;
	params.gamma = 0.1;
	// params.coef0 = 0;
	// params.C = 0;
	params.nu = 0.1;
	params.class_weights = class_weights;
	params.term_crit = term_crit;

	// We will use the "train_auto" method, which means that some parameters (nu for the
	// svm, gamma for the kernel) will be optimized via k-fold cross-validation.
	CvParamGrid gamma_grid = CvSVM::get_default_grid(CvSVM::GAMMA);
	CvParamGrid nu_grid = CvSVM::get_default_grid(CvSVM::NU);

//        svm->train(data,classes,NULL,NULL,params);
        svm->train_auto(data, classes,	// Train data, classes
                NULL, NULL,					// Var index, sample index
                params, 20,					// Params, K
                CvParamGrid(),				// C
                gamma_grid,					// GAMMA
                CvParamGrid(),				// P
                nu_grid,					// NU
                CvParamGrid(),				// COEF
                CvParamGrid());				// DEGREE

         params = svm->get_params();
}

int SVMClassifier::svmClassification(CvMat* sample)
{
	float prediction = svm->predict(sample);
	return (int) prediction;
	/*if (prediction > 0) {
		return 1;
	} else {
		return -1;
	}*/
}

CvMat* SVMClassifier::classify(CvMat *data)
{
	// Create return vector.
	CvMat* classes = cvCreateMat(1, data->rows, CV_32FC1);

	// Scale data.
        //scale(data);

	// Classify each feature row-vector in the matrix data.
	CvMat* temp = cvCreateMat(1, data->cols, CV_32FC1);
	int num_samples = data->rows;
	for (int i = 0; i < num_samples; i++) {
		// Copy next row of the matrix to a temporary vector.
		for (int j = 0; j < data->cols; j++) {
			cvmSet(temp, 0, j, cvmGet(data, i, j));
		}

		// Classify single vector.
		cvmSet(classes, 0, i, svmClassification(temp));
	}

	cvReleaseMat(&temp);
	return classes;
}

void SVMClassifier::saveClassifier(const char *fileName)
{
	svm->save(fileName);
}

void SVMClassifier::loadClassifier(const char *fileName)
{
	svm->load(fileName);
}
