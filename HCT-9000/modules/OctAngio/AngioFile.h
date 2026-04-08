#pragma once

#include "OctAngioDef.h"


namespace CppUtil {
	class CvImage;
}


namespace OctAngio
{
	class AngioLayout;
	class AngioDecorr;
	class AngioMotion;

	class OCTANGIO_DLL_API AngioFile
	{
	public:
		AngioFile();
		virtual ~AngioFile();

		AngioFile(AngioFile&& rhs);
		AngioFile& operator=(AngioFile&& rhs);

	public:
		static bool saveAngioDataFile(const std::string dirPath, const std::string fileName, 
									OctAngio::AngioLayout& layout, OctAngio::AngioDecorr& decorr, OctAngio::AngioMotion& motion);
		static bool loadAngioDataFile(const std::string dirPath, const std::string fileName,
									OctAngio::AngioLayout& layout, OctAngio::AngioDecorr& decorr, OctAngio::AngioMotion& motion);

	protected:
		struct DecorrTable {
			std::vector<int> indice;
			std::vector<unsigned short> dc_vals;
			std::vector<unsigned short> df_vals;

			DecorrTable(int size) {
				initialize(size);
			}

			void initialize(int size) {
				indice.resize(size, 0);
				dc_vals.resize(size, 0);
				df_vals.resize(size, 0);
			}
		};

		static bool readDataHeader(std::ifstream& file, OctAngio::AngioLayout& layout, OctAngio::AngioDecorr& decorr, OctAngio::AngioMotion& motion);
		static bool readDecorrSizes(std::ifstream& file, std::vector<int>& dsizes, OctAngio::AngioLayout& layout, OctAngio::AngioDecorr& decorr);
		static bool readMotionShifts(std::ifstream& file, OctAngio::AngioLayout& layout, OctAngio::AngioMotion& motion);
		static bool readDecorrTables(std::ifstream& file, std::vector<int> dsizes, OctAngio::AngioLayout& layout, OctAngio::AngioDecorr& decorr);

		static std::vector<char> makeDataHeader(OctAngio::AngioLayout& layout, OctAngio::AngioDecorr& decorr, OctAngio::AngioMotion& motion);
		static std::vector<int> makeDecorrSizes(OctAngio::AngioLayout& layout, OctAngio::AngioDecorr& decorr);
		static std::vector<int> makeMotionShifts(OctAngio::AngioLayout& layout, OctAngio::AngioMotion& motion);
		static AngioFile::DecorrTable makeDecorrTable(int index, int dsize, OctAngio::AngioLayout& layout, OctAngio::AngioDecorr& decorr);

	private:
		struct AngioFileImpl;
		std::unique_ptr<AngioFileImpl> d_ptr;
		AngioFileImpl& getImpl(void) const;
	};
}

