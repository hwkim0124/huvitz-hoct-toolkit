#include "stdafx.h"
#include "BscanSegmentator.h"
#include "LayeredBscan.h"
#include "Sampling.h"
#include "Bordering.h"
#include "Measuring.h"

using namespace SemtSegm;
using namespace std;


struct BscanSegmentator::BscanSegmentatorImpl
{
	const LayeredBscan* bscan = nullptr;

	unique_ptr<Sampling> sampling;
	unique_ptr<Bordering> bordering;
	unique_ptr<Classifier> classifier;
	unique_ptr<Measuring> measuring;

	unique_ptr<LayerInner> layerInn;
	unique_ptr<LayerOuter> layerOut;

	unique_ptr<LayerILM> layerILM;
	unique_ptr<LayerNFL> layerNFL;
	unique_ptr<LayerIPL> layerIPL;
	unique_ptr<LayerOPL> layerOPL;
	unique_ptr<LayerONL> layerONL;
	unique_ptr<LayerIOS> layerIOS;
	unique_ptr<LayerRPE> layerRPE;
	unique_ptr<LayerBRM> layerBRM;
	unique_ptr<LayerOPR> layerOPR;

	BscanSegmentatorImpl()
	{
	}
};


BscanSegmentator::BscanSegmentator(LayeredBscan* bscan) :
	d_ptr(make_unique<BscanSegmentatorImpl>())
{
	getImpl().bscan = bscan;
}


SemtSegm::BscanSegmentator::~BscanSegmentator() = default;
SemtSegm::BscanSegmentator::BscanSegmentator(BscanSegmentator && rhs) = default;
BscanSegmentator & SemtSegm::BscanSegmentator::operator=(BscanSegmentator && rhs) = default;


const LayeredBscan * SemtSegm::BscanSegmentator::getLayeredBscan() const
{
	return getImpl().bscan;
}

const OctScanImage& SemtSegm::BscanSegmentator::getImageSource() const 
{
	return getImpl().bscan->getImageSource();
}

Sampling * SemtSegm::BscanSegmentator::getSampling() const
{
	return getImpl().sampling.get();
}

Bordering * SemtSegm::BscanSegmentator::getBodering() const
{
	return getImpl().bordering.get();
}

Classifier * SemtSegm::BscanSegmentator::getClassifier() const
{
	return getImpl().classifier.get();
}

Measuring * SemtSegm::BscanSegmentator::getMeasuring() const
{
	return getImpl().measuring.get();
}

LayerInner * SemtSegm::BscanSegmentator::getLayerInner() const
{
	return getImpl().layerInn.get();
}

LayerOuter * SemtSegm::BscanSegmentator::getLayerOuter() const
{
	return getImpl().layerOut.get();
}

LayerILM * SemtSegm::BscanSegmentator::getLayerILM() const
{
	return getImpl().layerILM.get();
}

LayerNFL * SemtSegm::BscanSegmentator::getLayerNFL() const
{
	return getImpl().layerNFL.get();
}

LayerIPL * SemtSegm::BscanSegmentator::getLayerIPL() const
{
	return  getImpl().layerIPL.get();
}

LayerOPL * SemtSegm::BscanSegmentator::getLayerOPL() const
{
	return getImpl().layerOPL.get();
}

LayerONL * SemtSegm::BscanSegmentator::getLayerONL() const
{
	return getImpl().layerONL.get();
}

LayerIOS * SemtSegm::BscanSegmentator::getLayerIOS() const
{
	return getImpl().layerIOS.get();
}

LayerRPE * SemtSegm::BscanSegmentator::getLayerRPE() const
{
	return getImpl().layerRPE.get();
}

LayerBRM * SemtSegm::BscanSegmentator::getLayerBRM() const
{
	return getImpl().layerBRM.get();
}

LayerOPR * SemtSegm::BscanSegmentator::getLayerOPR() const
{
	return getImpl().layerOPR.get();
}

int SemtSegm::BscanSegmentator::getImageIndex() const
{
	return getImpl().bscan->getImageIndex();
}

float SemtSegm::BscanSegmentator::getImageRangeX() const
{
	return getImpl().bscan->getImageRangeX();
}

void SemtSegm::BscanSegmentator::resetAlgorithms()
{
	getImpl().sampling = move(unique_ptr<Sampling>(createSampling()));
	getImpl().bordering = move(unique_ptr<Bordering>(createBordering()));
	getImpl().classifier = move(unique_ptr<Classifier>(createClassifier()));
	getImpl().measuring = move(unique_ptr<Measuring>(createMeasuring()));

	getImpl().layerInn = move(unique_ptr<LayerInner>(createLayerInner()));
	getImpl().layerOut = move(unique_ptr<LayerOuter>(createLayerOuter()));

	getImpl().layerILM = move(unique_ptr<LayerILM>(createLayerILM()));
	getImpl().layerNFL = move(unique_ptr<LayerNFL>(createLayerNFL()));
	getImpl().layerIPL = move(unique_ptr<LayerIPL>(createLayerIPL()));
	getImpl().layerOPL = move(unique_ptr<LayerOPL>(createLayerOPL()));
	getImpl().layerONL = move(unique_ptr<LayerONL>(createLayerONL()));
	getImpl().layerIOS = move(unique_ptr<LayerIOS>(createLayerIOS()));
	getImpl().layerRPE = move(unique_ptr<LayerRPE>(createLayerRPE()));
	getImpl().layerBRM = move(unique_ptr<LayerBRM>(createLayerBRM()));
	getImpl().layerOPR = move(unique_ptr<LayerOPR>(createLayerOPR()));
}

Sampling * SemtSegm::BscanSegmentator::createSampling() 
{
	return new Sampling();
}

Bordering * SemtSegm::BscanSegmentator::createBordering() 
{
	return new Bordering(this);
}

Classifier * SemtSegm::BscanSegmentator::createClassifier()
{
	return new Classifier(this);
}

Measuring * SemtSegm::BscanSegmentator::createMeasuring()
{
	return new Measuring(this);
}

LayerInner * SemtSegm::BscanSegmentator::createLayerInner() 
{
	return new LayerInner(this);
}

LayerOuter * SemtSegm::BscanSegmentator::createLayerOuter() 
{
	return new LayerOuter(this);
}

LayerILM * SemtSegm::BscanSegmentator::createLayerILM()
{
	return new LayerILM(this);
}

LayerNFL * SemtSegm::BscanSegmentator::createLayerNFL()
{
	return new LayerNFL(this);
}

LayerIPL * SemtSegm::BscanSegmentator::createLayerIPL()
{
	return new LayerIPL(this);
}

LayerOPL * SemtSegm::BscanSegmentator::createLayerOPL()
{
	return new LayerOPL(this);
}

LayerONL * SemtSegm::BscanSegmentator::createLayerONL()
{
	return new LayerONL(this);
}

LayerIOS * SemtSegm::BscanSegmentator::createLayerIOS()
{
	return new LayerIOS(this);
}

LayerRPE * SemtSegm::BscanSegmentator::createLayerRPE()
{
	return new LayerRPE(this);
}

LayerBRM * SemtSegm::BscanSegmentator::createLayerBRM()
{
	return new LayerBRM(this);
}

LayerOPR * SemtSegm::BscanSegmentator::createLayerOPR()
{
	return new LayerOPR(this);
}


BscanSegmentator::BscanSegmentatorImpl & SemtSegm::BscanSegmentator::getImpl(void) const
{
	return *d_ptr;
}


