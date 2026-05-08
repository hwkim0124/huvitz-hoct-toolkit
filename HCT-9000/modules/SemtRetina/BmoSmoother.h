#pragma once

#include "SemtRetinaDef.h"

namespace SemtRetina
{
    struct BmoPoint {
        double x;
        double y;
    };


    class SEMTRETINA_DLL_API BmoSmoother
    {
    public:
        struct Options {
            /*
                Recommended:
                    3 for about 20 BMO points
                    4 for about 30-50 BMO points
                    5 only if the contour is reliable and truly irregular
            */
            int harmonics = 4;
            int outputResolution = 100;     // output contour point count

            /*
                   Smoothness strength.

                   Larger = smoother.
                   Recommended starting range:
                       0.03 to 0.08
               */
            double lambda = 0.05;

            /*
                Robust Huber iterations.
                5-8 is usually enough.
            */
            int robustIterations = 6;       // Huber reweighting iterations

            /*
                Huber threshold.
                1.345 is a standard robust-regression choice.
            */
            double huberK = 1.345;          // standard Huber value

            bool useProvidedCenter = false;
            double centerX = 0.0;
            double centerY = 0.0;

            /*
                Important for irregularly distributed BMO points.

                true:
                    points in a dense angular cluster receive smaller base weight,
                    points in sparse angular regions receive larger base weight.

                This prevents one clustered sector from dominating the contour.
            */
            bool useAngularDensityWeights = true;

            /*
                Clamp angular-density weights to avoid extreme influence from very
                isolated points or nearly duplicated points.
            */
            double minAngularWeight = 0.25;
            double maxAngularWeight = 4.0;

            /*
                If fitted radius becomes negative in a very noisy sector,
                clamp it to zero.
            */
            bool clampNegativeRadius = true;
        };

        static std::vector<BmoPoint> smoothRobustPolarFourier(const std::vector<BmoPoint>& inputPoints, const Options& options = Options());
        static bool smoothBmoPoints(std::vector<int>& xs, std::vector<int>& ys);
        static bool smoothBmoPoints(std::vector<BmoPoint> input, std::vector<BmoPoint>& output);

    private:
        static const double PI;
        static const double TWO_PI;

        struct Sample {
            double r;
            double theta;
            std::size_t originalIndex;
            double baseWeight;
        };

        static bool isFinite(double v)
        {
            return std::isfinite(v);
        }

        static bool isFinitePoint(const BmoPoint& p)
        {
            return isFinite(p.x) && isFinite(p.y);
        }

        static double wrapTo2Pi(double t)
        {
            t = std::fmod(t, TWO_PI);
            if (t < 0.0) {
                t += TWO_PI;
            }
            return t;
        }

        static double median(std::vector<double> values)
        {
            if (values.empty()) {
                return 0.0;
            }

            const std::size_t n = values.size();
            const std::size_t mid = n / 2;

            std::nth_element(values.begin(), values.begin() + mid, values.end());
            double m = values[mid];

            if ((n % 2) == 0) {
                std::nth_element(values.begin(), values.begin() + mid - 1, values.end());
                m = 0.5 * (m + values[mid - 1]);
            }

            return m;
        }

        static double medianCoordinate(const std::vector<BmoPoint>& points, bool useX)
        {
            std::vector<double> values;
            values.reserve(points.size());

            for (std::size_t i = 0; i < points.size(); ++i) {
                values.push_back(useX ? points[i].x : points[i].y);
            }

            return median(values);
        }

        static double clamp(double v, double lo, double hi)
        {
            if (v < lo) return lo;
            if (v > hi) return hi;
            return v;
        }

        static std::vector<double> solveLinearSystem(
            std::vector< std::vector<double> > A,
            std::vector<double> b)
        {
            const int n = static_cast<int>(b.size());
            const double eps = 1e-12;

            for (int col = 0; col < n; ++col)
            {
                int pivot = col;
                double maxAbs = std::fabs(A[col][col]);

                for (int row = col + 1; row < n; ++row)
                {
                    const double v = std::fabs(A[row][col]);

                    if (v > maxAbs)
                    {
                        maxAbs = v;
                        pivot = row;
                    }
                }

                if (maxAbs < eps)
                {
                    throw std::runtime_error(
                        "Singular linear system in BMO Fourier smoothing."
                    );
                }

                if (pivot != col)
                {
                    std::swap(A[pivot], A[col]);
                    std::swap(b[pivot], b[col]);
                }

                const double diag = A[col][col];

                for (int j = col; j < n; ++j)
                    A[col][j] /= diag;

                b[col] /= diag;

                for (int row = 0; row < n; ++row)
                {
                    if (row == col)
                        continue;

                    const double factor = A[row][col];

                    if (std::fabs(factor) < eps)
                        continue;

                    for (int j = col; j < n; ++j)
                        A[row][j] -= factor * A[col][j];

                    b[row] -= factor * b[col];
                }
            }

            return b;
        }

        static void makeBasis(double theta, int harmonics, std::vector<double>& basis)
        {
            const int numCoeffs = 1 + 2 * harmonics;

            basis.assign(static_cast<std::size_t>(numCoeffs), 0.0);

            basis[0] = 1.0;

            for (int k = 1; k <= harmonics; ++k)
            {
                basis[static_cast<std::size_t>(2 * k - 1)] =
                    std::cos(static_cast<double>(k) * theta);

                basis[static_cast<std::size_t>(2 * k)] =
                    std::sin(static_cast<double>(k) * theta);
            }
        }

        static double evaluateRadius(const std::vector<double>& coeffs, double theta)
        {
            const int harmonics =
                static_cast<int>((coeffs.size() - 1) / 2);

            double r = coeffs[0];

            for (int k = 1; k <= harmonics; ++k)
            {
                r += coeffs[static_cast<std::size_t>(2 * k - 1)] *
                    std::cos(static_cast<double>(k) * theta);

                r += coeffs[static_cast<std::size_t>(2 * k)] *
                    std::sin(static_cast<double>(k) * theta);
            }
            return r;
        }


        static double robustMADScale(const std::vector<double>& residuals)
        {
            if (residuals.empty())
                return 1.0;

            const double med = median(residuals);

            std::vector<double> absDev;
            absDev.reserve(residuals.size());

            for (std::size_t i = 0; i < residuals.size(); ++i)
            {
                absDev.push_back(std::fabs(residuals[i] - med));
            }

            double sigma = 1.4826 * median(absDev);

            if (!std::isfinite(sigma) || sigma < 1e-9)
            {
                double sum2 = 0.0;

                for (std::size_t i = 0; i < residuals.size(); ++i)
                    sum2 += residuals[i] * residuals[i];

                sigma = std::sqrt(
                    sum2 / static_cast<double>(residuals.size())
                );

                if (!std::isfinite(sigma) || sigma < 1e-9)
                    sigma = 1.0;
            }
            return sigma;
        }

        static void computeAngularDensityWeights(std::vector<Sample>& samples, double minWeight, double maxWeight)
        {
            const std::size_t N = samples.size();

            if (N < 3) {
                for (std::size_t i = 0; i < N; ++i) {
                    samples[i].baseWeight = 1.0;
                }
                return;
            }

            std::vector<std::size_t> order(N);
            for (std::size_t i = 0; i < N; ++i) {
                order[i] = i;
            }

            std::sort(
                order.begin(),
                order.end(),
                [&samples](std::size_t a, std::size_t b)
                {
                    return samples[a].theta < samples[b].theta;
                }
            );

            for (std::size_t pos = 0; pos < N; ++pos)
            {
                const std::size_t idx = order[pos];

                const std::size_t prevIdx =
                    order[(pos + N - 1) % N];

                const std::size_t nextIdx =
                    order[(pos + 1) % N];

                double prevGap = samples[idx].theta - samples[prevIdx].theta;
                double nextGap = samples[nextIdx].theta - samples[idx].theta;

                if (prevGap < 0.0)
                    prevGap += TWO_PI;

                if (nextGap < 0.0)
                    nextGap += TWO_PI;

                /*
                    Angular Voronoi width for this point.
                    Uniform points would have width = 2*pi/N.
                */
                const double angularWidth = 0.5 * (prevGap + nextGap);
                double w = angularWidth * static_cast<double>(N) / TWO_PI;
                w = clamp(w, minWeight, maxWeight);
                samples[idx].baseWeight = w;
            }
        }

        static std::vector<double> fitWeightedRegularizedFourier(
            const std::vector<Sample>& samples,
            const std::vector<double>& robustWeights,
            int harmonics,
            double lambda)
        {
            const int numCoeffs = 1 + 2 * harmonics;

            std::vector< std::vector<double> > normal(
                static_cast<std::size_t>(numCoeffs),
                std::vector<double>(static_cast<std::size_t>(numCoeffs), 0.0)
            );

            std::vector<double> rhs(
                static_cast<std::size_t>(numCoeffs),
                0.0
            );

            std::vector<double> basis;

            for (std::size_t i = 0; i < samples.size(); ++i)
            {
                makeBasis(samples[i].theta, harmonics, basis);

                const double w =
                    samples[i].baseWeight * robustWeights[i];

                if (w <= 0.0)
                    continue;

                for (int row = 0; row < numCoeffs; ++row)
                {
                    rhs[static_cast<std::size_t>(row)] +=
                        w *
                        basis[static_cast<std::size_t>(row)] *
                        samples[i].r;

                    for (int col = 0; col < numCoeffs; ++col)
                    {
                        normal[static_cast<std::size_t>(row)]
                            [static_cast<std::size_t>(col)] +=
                            w *
                            basis[static_cast<std::size_t>(row)] *
                            basis[static_cast<std::size_t>(col)];
                    }
                }
            }

            /*
                Smoothness penalty.

                Penalize higher harmonics more strongly:
                    penalty ~ lambda * k^4

                This suppresses high-frequency boundary ripple while preserving
                low-frequency disc shape.
            */
            for (int k = 1; k <= harmonics; ++k)
            {
                const double kk = static_cast<double>(k);
                const double penalty = lambda * kk * kk * kk * kk;

                normal[static_cast<std::size_t>(2 * k - 1)]
                    [static_cast<std::size_t>(2 * k - 1)] += penalty;

                normal[static_cast<std::size_t>(2 * k)]
                    [static_cast<std::size_t>(2 * k)] += penalty;
            }

            /*
                Tiny ridge for numerical stability.
            */
            for (int j = 0; j < numCoeffs; ++j)
            {
                normal[static_cast<std::size_t>(j)]
                    [static_cast<std::size_t>(j)] += 1e-10;
            }

            return solveLinearSystem(normal, rhs);
        }
    };
}