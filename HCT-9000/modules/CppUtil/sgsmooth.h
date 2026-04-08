#pragma once

/* common definitions for the sgsmooth plugin */
#ifndef _SGSMOOTH_PLUGIN_H
#define _SGSMOOTH_PLUGIN_H

//! default convergence
static const double TINY_FLOAT = 1.0e-300;

//! comfortable array of doubles
typedef std::vector<double> float_vect;
//! comfortable array of ints;
typedef std::vector<int>    int_vect;

// savitzky golay smoothing.
float_vect sg_smooth(const float_vect &v, const int w, const int deg);
//! numerical derivative based on savitzky golay smoothing.
float_vect sg_derivative(const float_vect &v, const int w,
	const int deg, const double h = 1.0);

#ifdef __cplusplus
extern "C"
{
#endif

	extern double *calc_sgsmooth(const int ndat, double *input,
		const int window, const int order);

	extern double *calc_sgsderiv(const int ndat, double *input,
		const int window, const int order,
		const double delta);

	extern void sgs_error(const char *errmsg);


#ifdef __cplusplus
}
#endif

#endif