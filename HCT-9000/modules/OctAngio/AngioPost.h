#include "OctAngioDef.h"

#include <memory>


namespace CppUtil {
	class CvImage;
}


namespace OctAngio
{
	class AngioLayout;
	class AngioLayers;

	class OCTANGIO_DLL_API AngioPost
	{
	public:
		AngioPost();
		virtual ~AngioPost();

		AngioPost(AngioPost&& rhs);
		AngioPost& operator=(AngioPost&& rhs);
		AngioPost(const AngioPost& rhs);
		AngioPost& operator=(const AngioPost& rhs);

	public:
		bool createProjectionMask(const AngioLayout& layout, const std::vector<float>& profile, std::vector<float>& mask, bool kernel=true);
		bool performPostProcessing(int width, int height, std::vector<float>& profile, bool outFlows = false);
		bool performVesselProcessing(int width, int height, std::vector<float>& profile);
		bool performFilterProcessing(int width, int height, std::vector<float>& profile);

		bool removeProjectionStripeNoise(const AngioLayout& layout, std::vector<float>& profile, bool vertical);
		bool removeProjectionStripeNoise2(const AngioLayout& layout, std::vector<float>& profile, bool vertical);

		bool applyNoiseReduction(const AngioLayout& layout, std::vector<float>& profile, float rate);
		bool removeFoveaRegionNoise(const AngioLayout& layout, const AngioLayers& layers, std::vector<float>& profile);

		bool applyBiasFieldCorrection(const AngioLayout& layout, std::vector<float>& profile, bool isFovea);

		int& garborFilerOrients(void);
		float& garborFilterSigma(void);
		float& garborFilterDivider(void);
		float& garborFilterWeight(void);

	protected:
		void detectVessels(int width, int height, std::vector<float>& profile, float thresh1, float thresh2);

	private:
		struct AngioPostImpl;
		std::unique_ptr<AngioPostImpl> d_ptr;
		AngioPostImpl& getImpl(void) const;
	};
}

