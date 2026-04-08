#pragma once

#include "KerFocusDef.h"

#include <vector>
#include <array>
#include <memory>

namespace KerFocus
{
	struct KerSetupData {
		float m;
		float n;
		float a;
		float mlen;
		float flen;
		float fdif;
		bool valid;

		template<class Archive>
		void serialize(Archive& archive) {
			archive(m, n, a, mlen, flen, fdif, valid);
		}
	};

	
	struct KerSetupDiopter {
		float mm;
		float diopt;

		std::array<KerSetupData, KER_SETUP_FOCUS_STEP_NUM> steps;

		template<class Archive>
		void serialize(Archive& archive) {
			archive(mm, diopt, steps);
		}
	};

	
	struct KerSetupTable {
		std::array<KerSetupDiopter, KER_SETUP_DIOPTER_NUM> diopts;

		template<class Archive> 
		void serialize(Archive& archive) {
			archive(diopts);
		}
	};


	class KeratoImage;


	class KERFOCUS_DLL_API KeratoSetup
	{
	public:
		KeratoSetup();
		virtual ~KeratoSetup();

		KeratoSetup(KeratoSetup&& rhs);
		KeratoSetup& operator=(KeratoSetup&& rhs);

	private:
		struct KeratoSetupImpl;
		std::unique_ptr<KeratoSetupImpl> d_ptr;

	public:
		void initialize(void);
		KerSetupData* getKerSetupData(int diopt, int istep) const;
		KerSetupDiopter* getKerSetupDiopter(int diopt) const;
		bool isValidSetupData(int diopt, int istep) const;

		bool isSetupDataIndex(int diopt) const;
		bool isSetupDataIndex(int diopt, int step) const;
		bool checkIfValidFocusStepData(int diopt, int istep, float flen, float mlen) const;

		bool getFocusStepLine(int diopt, float& a, float& b) const;
		bool getDiopterSetupLine(float& a, float& b) const;
		bool getDiopterSection(float mlen, float flen, int& idx1, int& idx2) const;

	public:
		bool updateFocusStepData(int diopt, int istep, const KeratoImage& image);

		bool getFocusStepData(int diopt, int istep, float& mlen, float& flen, float& fdif, float& m, float& n, float& a) const;
		bool getDiopterSetupData(int diopt, float& mlen, float& flen, float& fdif, float& m, float& n, float& a, float& k) const;
		void clearFocusStepData(int diopt = -1, int istep = -1);
		bool getFocusDistance(float mlen, float flen, float& dist) const;

		bool loadDataFile(const char* path = KER_SETUP_DATA_FILE_NAME);
		bool saveDataFile(const char* path = KER_SETUP_DATA_FILE_NAME);

		float getSetupDiopter(int diopt) const;
		float getSetupDiameter(int diopt) const;
		float getInitialDiopter(int diopt) const;
		float getInitialDiameter(int diopt) const;
	};
}
