#pragma once

#include "OctDataDef.h"
#include "ProtocolData.h"

#include <string>
#include <memory>
#include <vector>


namespace OctData
{
	class LineScanData;
	class CubeScanData;
	class CircleScanData;
	class CrossScanData;
	class RadialScanData;
	class RasterScanData;


	class OCTDATA_DLL_API ProtocolDataset
	{
	public:
		ProtocolDataset();
		virtual ~ProtocolDataset();

		ProtocolDataset(ProtocolDataset&& rhs);
		ProtocolDataset& operator=(ProtocolDataset&& rhs);
		ProtocolDataset(const ProtocolDataset& rhs) = delete;
		ProtocolDataset& operator=(const ProtocolDataset& rhs) = delete;

	public:
		ProtocolData* getProtocolData(int index);
		LineScanData* getOrCreateLineScanData(int index);
		CubeScanData* getOrCreateCubeScanData(int index);
		CircleScanData* getOrCreateCircleScanData(int index);
		CrossScanData* getOrCreateCrossScanData(int index);
		RadialScanData* getOrCreateRadialScanData(int index);
		RasterScanData* getOrCreateRasterScanData(int index);

		void clear(void);

	protected:
		template <typename T>
		ProtocolData* emplaceProtocolData(int index);

		template <typename T>
		ProtocolData* fetchProtocolData(int index);

		template <typename T>
		ProtocolData * createProtocolData(int index);

	private:
		struct ProtocolDatasetImpl;
		std::unique_ptr<ProtocolDatasetImpl> d_ptr;
		ProtocolDatasetImpl& getImpl(void) const;
	};


	template<typename T>
	inline ProtocolData * ProtocolDataset::emplaceProtocolData(int index)
	{
		auto data = fetchProtocolData<T>(index);
		if (data == nullptr) {
			return createProtocolData<T>(index);
		}
		return data;
	}

	template<typename T>
	inline ProtocolData * ProtocolDataset::fetchProtocolData(int index)
	{
		auto iter = d_ptr->dataset.find(index);
		if (iter != end(d_ptr->dataset)) {
			return dynamic_cast<T*>(iter->second.get());
		}
		return nullptr;
	}

	template<typename T>
	inline ProtocolData * ProtocolDataset::createProtocolData(int index)
	{
		if (index >= 0) {
			d_ptr->dataset[index] = make_unique<T>();
			return d_ptr->dataset[index].get();
		}
		return nullptr;
	}
}
