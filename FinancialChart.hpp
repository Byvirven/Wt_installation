#ifndef FINANCIALCHART_HPP
#define FINANCIALCHART_HPP

#include <Wt/Chart/WCartesianChart.h>
#include <Wt/Chart/WAbstractChartModel.h>
#include <Wt/Chart/WDataSeries.h>
#include <Wt/WAbstractItemModel.h>
#include <Wt/WPainter.h>
#include <Wt/WPaintDevice.h>
#include <Wt/WLogger.h>
#include <Wt/Chart/WAxisSliderWidget.h>
#include <Wt/WAbstractItemView.h>
#include <Wt/Chart/WAxis.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WStandardItemModel.h>
#include <Wt/WShadow.h>
#include <Wt/WPaintedWidget.h>
#include <Wt/WInteractWidget.h>
#include <Wt/WDateTime.h>
#include <Wt/WDate.h>
#include <Wt/WStandardItem.h> 
#include <Wt/WStackedWidget.h>
#include <Wt/WJavaScript.h>
#include <Wt/Json/Array.h>
#include <Wt/Json/Object.h>
#include <Wt/Json/Serializer.h>
#include <Wt/WLength.h>
#include <Wt/WTimer.h>

#include "DataStruct.hpp"
#include "HDF5Reader.hpp"

#include <vector>
#include <cfloat>
#include <cmath>

namespace Financial {

class Chart : public Wt::Chart::WCartesianChart
{
public:
	//Chart(const std::shared_ptr< Wt::WAbstractItemModel > & model);  // Modifié pour ne pas prendre de parent
	Chart(HDF5Reader * reader, int timezoneOffset);  // Modifié pour ne pas prendre de parent
	~Chart();
	//static std::shared_ptr<Wt::WStandardItemModel> createModel(const std::vector<HDF5Data::Candle>& data, int timezoneOffset);
	
	void setSeriesConfigurations(const std::vector<Financial::SeriesConfig>& configs);
	
	void setupClientMouseHover(Wt::WContainerWidget* displayWidget, const std::vector<HDF5Data::Candle>& data);

protected:
	void renderOther(Wt::WPainter &painter) const override;

private:
	std::vector<Financial::SeriesConfig> seriesConfigs_;
	mutable double last_timestamp_;
	bool monitoring_;
	HDF5Reader * filereader_;
	int timezoneOffset_ ;
	double minPrice_;
	double maxPrice_;
	double minTime_;
	double maxTime_;
	
	std::shared_ptr<Wt::WStandardItemModel> model_ ;
	 Wt::WTimer* timer_;
	
	void createModel();
	void updateModel();
	void updateAxisRanges();
	void setupSeries();
	void setupAxes();
	void setupCrosshair();
	void startMonitoring() ;
	void stopMonitoring() ;
	void onFileModified() ;
	
	Wt::WPainterPath drawCandle(double x, double open, double high, double low, double close, double candleWidth) const ;

};

} // namespace Financial

#endif // FINANCIALCHART_HPP
