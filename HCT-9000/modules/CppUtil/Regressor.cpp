#include "stdafx.h"
#include "Regressor.h"
#include "PolyFit.h"

using namespace CppUtil;
using namespace std;
using namespace cv;

Regressor::Regressor()
{
}


Regressor::~Regressor()
{
}

bool CppUtil::Regressor::chevyPolyCoeffs(std::vector<double> dataX, std::vector<double> dataY, int degree, std::vector<double>& coeffs)
{
	int		N, M;
	int		i, j, k;
	int		size;
	double	*Zx;
	double	*Tz;
	double	*A, *b;
	double	sum;
	CvMat	*Amat, *bmat, *cmat;
	double	*Aptr;
	bool	ret;

	if (dataX.size() <= 0 || dataY.size() <= 0) {
		return false;
	}

	size = (int) dataX.size();
	N = degree + 1;
	M = size;
	Tz = new double[N*M];
	Zx = new double[M];
	A = new double[N*N];
	b = new double[N];

	//	Normalize x. 
	for (k = 0; k < M; k++) {
		Zx[k] = dataX[k];
	}

	//	Chebyshev polynomials.
	for (k = 0; k < M; k++) {
		Tz[0 * M + k] = 1.0;
		Tz[1 * M + k] = Zx[k];
		for (j = 2; j < N; j++) {
			Tz[j*M + k] = 2.0*Zx[k] * Tz[(j - 1)*M + k] - Tz[(j - 2)*M + k];
		}
	}

	//	Normal equations with coefficient matrix.
	for (i = 0; i < N; i++) {
		sum = 0.0;
		for (k = 0; k < M; k++) {
			sum += (dataY[k] * Tz[i*M + k]);
		}
		b[i] = sum;

		for (j = i; j < N; j++) {
			sum = 0.0;
			for (k = 0; k < M; k++) {
				sum += (Tz[i*M + k] * Tz[j*M + k]);
			}
			A[i*N + j] = sum;
			A[j*N + i] = sum;
		}
	}

	//	Solve equations. 
	Amat = cvCreateMat(N, N, CV_64FC1);
	bmat = cvCreateMat(N, 1, CV_64FC1);
	cmat = cvCreateMat(N, 1, CV_64FC1);

	for (i = 0; i < N; i++) {
		Aptr = (double*)(Amat->data.ptr + i*Amat->step);
		for (j = 0; j < N; j++) {
			*(Aptr + j) = A[i*N + j];
		}
		*((double*)(bmat->data.ptr + i*bmat->step)) = b[i];
	}

	ret = (cvSolve(Amat, bmat, cmat, CV_SVD_SYM) ? true : false);

	// TRACE(_T("ret= %d\n"), ret) ;
	for (i = 0; i < N; i++) {
		coeffs[i] = *((double*)(cmat->data.ptr + i*cmat->step));
		// TRACE(_T("%d => %f\n"), i, coe[i]) ;
	}

	delete[] Tz;
	delete[] Zx;
	delete[] A;
	delete[] b;

	cvReleaseMat(&Amat);
	cvReleaseMat(&bmat);
	cvReleaseMat(&cmat);
	return ret;
}


bool CppUtil::Regressor::chevyPolyCurve(std::vector<int> dataX, std::vector<int> dataY, int degree, bool nonZero, std::vector<int>& result, std::vector<double>& coeffs)
{
	int		k, j;
	int		N, M;
	double	dat;
	double	fsx, fex;
	bool	ret = true;

	if (dataX.size() <= 0 || dataY.size() <= 0 || degree <= 0) {
		return false;
	}

	// Initial input data size. 
	int size = (int)dataX.size();
	if (size < (degree + 1)) {
		return false;
	}

	auto xs = std::vector<double>();
	auto ys = std::vector<double>();

	// If nonzero flag is set, only the data with positive value are included. 
	if (nonZero) {
		for (j = 0, k = 0; j < size; j++) {
			if (dataY[j] > 0.0) {
				xs.push_back(dataX[j]);
				ys.push_back(dataY[j]);
				k++;
			}
		}
	}
	else {
		for (j = 0, k = 0; j < size; j++) {
			xs.push_back(dataX[j]);
			ys.push_back(dataY[j]);
			k++;
		}
	}

	N = degree + 1;
	if (k <= N) {
		return false;
	}

	// Reset the data size, and x range for normalization. 
	M = (int)xs.size();
	// fsx = xs[0];
	// fex = xs[M-1];
	fsx = dataX[0];
	fex = dataX[dataX.size() - 1];

	auto coe = std::vector<double>(N);
	auto wtm = std::vector<double>(N + 2);
	auto zx = std::vector<double>(M);

	//	Normalize x. 
	for (k = 0; k < M; k++) {
		zx[k] = (2.0*xs[k] - fsx - fex) / (fex - fsx);
	}

	//	Chebyshev coefficients.
	if (chevyPolyCoeffs(zx, ys, degree, coe) == false) {
		return false;
	}
	else {
		// Denormalize x range with input data. 
		fsx = dataX[0];
		fex = dataX[dataX.size() - 1];
	}
	coeffs = coe;

	//	Generate a fitting curve by newly assiging y corresponding to x.  
	//	Note that the y values with less than zero are fitted together. 
	for (k = 0; k < dataX.size(); k++) {
		dat = (2.0*dataX[k] - fsx - fex) / (fex - fsx);

		wtm[N + 1] = 0;
		wtm[N] = 0;
		for (j = (N - 1); j >= 0; j--) {
			wtm[j] = coe[j] + 2.0*dat*wtm[j + 1] - wtm[j + 2];
		}

		result[k] = (int)(wtm[0] - dat*wtm[1]);
	}
	return true;
}



bool CppUtil::Regressor::chevyPolyCurve(std::vector<double> dataX, std::vector<double> dataY, int degree, bool nonZero, std::vector<double>& result, std::vector<double>& coeffs)
{
	int		k, j;
	int		N, M;
	double	dat;
	double	fsx, fex;
	bool	ret = true;

	if (dataX.size() <= 0 || dataY.size() <= 0 || degree <= 0) {
		return false;
	}

	// Initial input data size. 
	int size = (int)dataX.size();
	if (size < (degree + 1)) {
		return false;
	}

	auto xs = std::vector<double>();
	auto ys = std::vector<double>();

	// If nonzero flag is set, only the data with positive value are included. 
	if (nonZero) {
		for (j = 0, k = 0; j < size; j++) {
			if (dataY[j] > 0.0) {
				xs.push_back(dataX[j]);
				ys.push_back(dataY[j]);
				k++;
			}
		}
	}
	else {
		for (j = 0, k = 0; j < size; j++) {
			xs.push_back(dataX[j]);
			ys.push_back(dataY[j]);
			k++;
		}
	}

	N = degree + 1;
	if (k <= N) {
		return false;
	}

	// Reset the data size, and x range for normalization. 
	M = (int)xs.size();
	fsx = xs[0];
	fex = xs[M - 1];

	auto coe = std::vector<double>(N);
	auto wtm = std::vector<double>(N + 2);
	auto zx = std::vector<double>(M);

	//	Normalize x. 
	for (k = 0; k < M; k++) {
		zx[k] = (2.0*xs[k] - fsx - fex) / (fex - fsx);
	}

	//	Chebyshev coefficients.
	if (chevyPolyCoeffs(zx, ys, degree, coe) == false) {
		return false;
	}
	else {
		// Denormalize x range with input data. 
		fsx = dataX[0];
		fex = dataX[dataX.size() - 1];
	}
	coeffs = coe;

	//	Generate a fitting curve by newly assiging y corresponding to x.  
	//	Note that the y values with less than zero are fitted together. 
	for (k = 0; k < dataX.size(); k++) {
		dat = (2.0*dataX[k] - fsx - fex) / (fex - fsx);

		wtm[N + 1] = 0;
		wtm[N] = 0;
		for (j = (N - 1); j >= 0; j--) {
			wtm[j] = coe[j] + 2.0*dat*wtm[j + 1] - wtm[j + 2];
		}

		result[k] = (wtm[0] - dat*wtm[1]);
	}
	return true;
}


bool CppUtil::Regressor::calcRmsError(std::vector<double> dataY, std::vector<double> fittY, int degree, double * rms)
{
	
	if (dataY.size() != fittY.size()) {
		return false;
	}

	int size = (int)dataY.size();
	double dsum = 0.0, diff;
	for (int i = 0; i < size; i++) {
		diff = (fittY[i] - dataY[i]);
		dsum += (diff * diff);
	}

	*rms = sqrt(dsum / (size - degree));
	return true;
}



bool CppUtil::Regressor::polyFit(const std::vector<double>& dataX, const std::vector<double>& dataY, int degree, std::vector<double>& coeffs)
{
	coeffs = polyfit<double>(dataX, dataY, degree);
	return true;
}

