#include <vector>

#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

class RadialCorrFactor
{
public:
    RadialCorrFactor();
    RadialCorrFactor(std::vector<float>& p, float R, int N = 2000);
    virtual ~RadialCorrFactor();

    float at(float r);
    std::vector<float>& getTable() {
        return factor;
    }

private:
    std::vector<float> factor;
    int n;
    float radius;
    float step;
};

class LevelCorrFactor
{
public:
    LevelCorrFactor();
    LevelCorrFactor(std::vector<int>& xs, std::vector<int>& ys, bool useFundusFILR = false);
    virtual ~LevelCorrFactor();

    float at(int val);
    std::vector<float>& getTable() {
        return factor;
    }

private:
    std::vector<float> factor;
};

class cvMatCorrector
{
public:
    cvMatCorrector();
    virtual ~cvMatCorrector();

    void setROI(int x0, int y0, int x1, int y1);
    void setRadialParameter(float p4, float p3, float p2, float p1, float p0);
    void setKnees(std::vector<int> xs, std::vector<int> ys);
	void setCeil(int c);
	bool useFundusFILR(bool iset, bool flag = false);
	void correctRadial(cv::Mat& m);
    void correctLevel(cv::Mat& m);
    void correctBoth(cv::Mat& m);

private:
	int ceil;
    int roi_x0;
    int roi_y0;
    int roi_x1;
    int roi_y1;
    int roi_cx;
    int roi_cy;
	bool isFundusFILR;
    std::vector<float> radParm;
    std::vector<int> knee_xs;
    std::vector<int> knee_ys;
    RadialCorrFactor radCorr;
    LevelCorrFactor levCorr;
};
