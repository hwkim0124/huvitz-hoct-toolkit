#pragma once

#include "KerFocusDef.h"

#include <vector>
#include <array>
#include <memory>


namespace KerFocus
{
	struct KerSetupData2 {
		float m;
		float n;
		float a;
		float mlen;
		float flen;
		float fd;
		bool valid;

		template<class Archive>
		void serialize(Archive& archive) {
			archive(m, n, a, mlen, flen, fd, valid);
		}
	};


	struct KerSetupDiopter2 {
		float mm;
		float diopt;

		std::array<KerSetupData2, FOCUS_SETUP_STEP_NUM> steps;

		template<class Archive>
		void serialize(Archive& archive) {
			archive(mm, diopt, steps);
		}
	};


	struct KerSetupTable2 {
		std::array<KerSetupDiopter2, FOCUS_SETUP_DIOPTER_NUM> diopts;

		template<class Archive>
		void serialize(Archive& archive) {
			archive(diopts);
		}
	};

	class KeratoImage2;

	class KeratoSetup2
	{
	public:
		KeratoSetup2();
		virtual ~KeratoSetup2();

		KeratoSetup2(KeratoSetup2&& rhs);
		KeratoSetup2& operator=(KeratoSetup2&& rhs);

	private:
		struct KeratoSetup2Impl;
		std::unique_ptr<KeratoSetup2Impl> d_ptr;

		void initialize(void);
		KerSetupData* getKerSetupData(int diopt, int step) const;
		KerSetupDiopter* getKerSetupDiopter(int diopt) const;

	};
}
