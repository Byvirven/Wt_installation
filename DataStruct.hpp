#ifndef DATASTRUCT_HPP
#define DATASTRUCT_HPP

#include <Wt/WColor.h>
#include <string>
#include <vector>

namespace Financial {

enum ChartType {
	Candlestick, 
	Surface, 
	Line
};

struct SeriesConfig {
	ChartType type;
	std::vector<unsigned short> seriesIds;
	unsigned char red = 0 ;
	unsigned char green = 0 ;
	unsigned char blue = 0 ;
	unsigned char opacity = 255 ;
};

} // namespace Financial

namespace HDF5Data {

struct Candle {
	int64_t timestamp; // unix in millisecond
	double open;
	double high;
	double low;
	double close;
	double volume;
	int64_t trades;
};

} // namespace HDF5Data

#endif // DATASTRUCT_HPP
