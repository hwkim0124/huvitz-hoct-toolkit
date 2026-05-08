#include "stdafx.h"
#include "BmoSmoother.h"


using namespace SemtRetina;
using namespace std;
using namespace cv;


const double BmoSmoother::PI = 3.1415926535897932384626433832795;
const double BmoSmoother::TWO_PI = 2.0 * BmoSmoother::PI;

std::vector<BmoPoint> SemtRetina::BmoSmoother::smoothRobustPolarFourier(const std::vector<BmoPoint>& inputPoints, const Options& options)
{
    const int MIN_POINTS = 6;

    if (inputPoints.size() < MIN_POINTS)
        return inputPoints;

    /*
        Collect finite points for center estimation.
    */
    std::vector<double> validX;
    std::vector<double> validY;

    validX.reserve(inputPoints.size());
    validY.reserve(inputPoints.size());

    for (std::size_t i = 0; i < inputPoints.size(); ++i)
    {
        if (isFinitePoint(inputPoints[i]))
        {
            validX.push_back(inputPoints[i].x);
            validY.push_back(inputPoints[i].y);
        }
    }

    if (validX.size() < MIN_POINTS) {
        return inputPoints;
    }

    /*
        Center choice.

        Best:
            provide the OCT scan center / disc center.

        Default:
            median x/y center, which is more robust than arithmetic mean.
    */
    double cx = 0.0;
    double cy = 0.0;

    if (options.useProvidedCenter) {
        cx = options.centerX;
        cy = options.centerY;

        if (!std::isfinite(cx) || !std::isfinite(cy)) {
            // throw std::invalid_argument("Provided BMO center is not finite.");
            return inputPoints;
        }
    }
    else {
        cx = median(validX);
        cy = median(validY);
    }

    /*
        Convert each valid input point to polar coordinates.
        The original index is preserved so the output can correspond to input.
    */
    std::vector<Sample> samples;
    samples.reserve(inputPoints.size());

    for (std::size_t i = 0; i < inputPoints.size(); ++i)
    {
        if (!isFinitePoint(inputPoints[i])) {
            continue;
        }

        const double dx = inputPoints[i].x - cx;
        const double dy = inputPoints[i].y - cy;

        const double r = std::sqrt(dx * dx + dy * dy);
        if (!std::isfinite(r) || r < 1e-9) {
            continue;
        }

        Sample s;
        s.theta = wrapTo2Pi(std::atan2(dy, dx));
        s.r = r;
        s.originalIndex = i;
        s.baseWeight = 1.0;
        samples.push_back(s);
    }

    if (samples.size() < MIN_POINTS) {
        return inputPoints;
    }

    /*
        Harmonic count guard.

        Need enough points to fit:
            1 + 2 * harmonics coefficients.
    */
    int K = options.harmonics;
    if (K < 1) {
        K = 1;
    }

    const int maxK = static_cast<int>((samples.size() - 1) / 2);
    if (K > maxK) {
        K = maxK;
    }

    /*
        Angular-density compensation.

        This is important because your BMO points are not guaranteed to have
        regular polar spacing.
    */
    if (options.useAngularDensityWeights)
    {
        computeAngularDensityWeights(
            samples,
            options.minAngularWeight,
            options.maxAngularWeight
        );
    }
    else
    {
        for (std::size_t i = 0; i < samples.size(); ++i) {
            samples[i].baseWeight = 1.0;
        }
    }

    /*
        Robust iterative weighted fitting.
    */
    std::vector<double> robustWeights(samples.size(), 1.0);
    std::vector<double> coeffs;

    const int iterations = options.robustIterations > 0 ? options.robustIterations : 1;
    const double lambda = options.lambda >= 0.0 ? options.lambda : 0.0;
    const double huberK = options.huberK > 0.0 ? options.huberK : 1.345;

    for (int iter = 0; iter < iterations; ++iter)
    {
        coeffs = fitWeightedRegularizedFourier(
            samples,
            robustWeights,
            K,
            lambda
        );

        std::vector<double> residuals;
        residuals.reserve(samples.size());

        for (std::size_t i = 0; i < samples.size(); ++i) {
            const double fittedR = evaluateRadius(coeffs, samples[i].theta);
            residuals.push_back(samples[i].r - fittedR);
        }

        const double sigma = robustMADScale(residuals);
        const double cutoff = huberK * sigma;

        for (std::size_t i = 0; i < samples.size(); ++i) {
            const double e = std::fabs(residuals[i]);

            if (e <= cutoff || e < 1e-12) {
                robustWeights[i] = 1.0;
            }
            else {
                robustWeights[i] = cutoff / e;
            }
        }
    }

    /*
        Return same-size, same-order smoothed points.

        Each input BMO point keeps its own angular direction theta.
        Only its radius is replaced by the fitted smooth radius.
    */
    std::vector<BmoPoint> output = inputPoints;

    for (std::size_t i = 0; i < samples.size(); ++i)
    {
        const Sample& s = samples[i];
        double smoothedR = evaluateRadius(coeffs, s.theta);

        if (options.clampNegativeRadius && smoothedR < 0.0) {
            smoothedR = 0.0;
        }

        BmoPoint p;
        p.x = cx + smoothedR * std::cos(s.theta);
        p.y = cy + smoothedR * std::sin(s.theta);
        output[s.originalIndex] = p;
    }

    return output;
}

bool SemtRetina::BmoSmoother::smoothBmoPoints(std::vector<int>& xs, std::vector<int>& ys)
{
    if (xs.size() != ys.size()) {
        return false;
    }

    std::vector<BmoPoint> input;
    input.reserve(xs.size());

    for (auto i = 0; i < xs.size(); i++) {
        BmoPoint p; 
        p.x = xs[i];
        p.y = ys[i];
        input.push_back(p);
    }

    try {
        auto output = smoothRobustPolarFourier(input);
        if (output.size() != xs.size()) {
            return false;
        }

        auto xs2 = xs;
        auto ys2 = ys;
        for (auto i = 0; i < output.size(); i++) {
            xs2[i] = (int)(output[i].x + 0.5f);
            ys2[i] = (int)(output[i].y + 0.5f);
        }
        xs = xs2;
        ys = ys2;
    }
    catch (...) {
        return false;
    }
    return true;
}

bool SemtRetina::BmoSmoother::smoothBmoPoints(std::vector<BmoPoint> input, std::vector<BmoPoint>& output)
{
    try {
        auto points = smoothRobustPolarFourier(input);
        if (points.size() != input.size()) {
            return false;
        }

        output = points;
    }
    catch (...) {
        return false;
    }
    return true;
}
