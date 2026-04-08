#pragma once

#include "OctAngioDef.h"

#include <memory>


namespace OctAngio
{
	class OCTANGIO_DLL_API AngioLayers
	{
	public:
		AngioLayers();
		virtual ~AngioLayers();

		AngioLayers(AngioLayers&& rhs);
		AngioLayers& operator=(AngioLayers&& rhs);

	public:
		void setupAngioLayers(int lines, int points, int repeats);
		void setupSlabRange(OcularLayerType upper, OcularLayerType lower, float upperOffset, float lowerOffset);

		bool loadLayerSegmentFiles(const std::string dirPath);
		void testLayerSegments(void);

		bool importLayersFromSegmentFiles(
						OcularLayerType upper, OcularLayerType lower, float upperOffset, float lowerOffset, 
						const std::string dirPath, bool vascular=false);

		bool isSuperficialFlows(void) const;
		bool isDeepFlows(void) const ;
		bool isOuterRetinaFlows(void) const;
		bool isChoroidalFlows(void) const;

		void setLayerPoints(int line, OcularLayerType type, std::vector<int> points);
		LayerArrays getLayerArrays(OcularLayerType type, float offset = 0.0f) const;
		LayerArrays getUpperLayers(OcularLayerType type, float offset = 0.0f) const;
		LayerArrays getLowerLayers(OcularLayerType type, float offset = 0.0f) const;

		LayerArrays getUpperLayersOfSlab(void) const;
		LayerArrays getLowerLayersOfSlab(void) const;
		LayerArrays getUpperLayersOfProjectMask(void) const;
		LayerArrays getLowerLayersOfProjectMask(void) const;
		LayerArrays getUpperLayersOfShadowed(void) const;
		LayerArrays getLowerLayersOfShadowed(void) const;
		LayerArrays getUpperLayersOfShadowed2(void) const;
		LayerArrays getLowerLayersOfShadowed2(void) const;

		LayerArrays getUpperLayersOfVasculature(void) const;
		LayerArrays getLowerLayersOfVasculature(void) const;
		LayerArrays getUpperLayersOfVasculature2(void) const;
		LayerArrays getLowerLayersOfVasculature2(void) const;

		LayerArrays getUpperLayersOfVariance(void) const;
		LayerArrays getLowerLayersOfVariance(void) const;
		LayerArrays getUpperLayersOfVariance2(void) const;
		LayerArrays getLowerLayersOfVariance2(void) const;
		LayerArrays getUpperLayersOfSuperficial(void) const;
		LayerArrays getLowerLayersOfSuperficial(void) const;
		LayerArrays getUpperLayersOfNonVascular(void) const;
		LayerArrays getLowerLayersOfNonVascular(void) const;

		void setUpperLayers(const std::vector<std::vector<int>>& layers);
		void setLowerLayers(const std::vector<std::vector<int>>& layers);
		const std::vector<std::vector<int>>& upperLayers(void) const;
		const std::vector<std::vector<int>>& lowerLayers(void) const;

		LayerArrays getUpperLayers(void) const;
		LayerArrays getLowerLayers(void) const;

		OcularLayerType& upperLayerType(void);
		OcularLayerType& lowerLayerType(void);

	private:
		struct AngioLayersImpl;
		std::unique_ptr<AngioLayersImpl> d_ptr;
		AngioLayersImpl& getImpl(void) const;
	};
}

