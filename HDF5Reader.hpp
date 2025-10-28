#ifndef HDF5_READER_HPP
#define HDF5_READER_HPP

#include "H5Cpp.h"

#include "DataStruct.hpp"

#include <filesystem>
#include <vector>
#include <string>
#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>

class HDF5Reader {
public:
	HDF5Reader(std::string path, std::string extension = ".h5");
	~HDF5Reader();
	
	void setPath(std::string path);
	std::string getPath();
	
	void setDatasetName(std::string datasetName);
	std::string getDatasetName();
	
	void setFilename(std::string filename);
	std::string getFilename();
	
	void setFilePath();
	std::string getFilePath();
	
	bool directoryExists();
	bool fileExists();
	
	std::vector<std::string> getFilesList(std::string selectedCurrency) ;
	std::vector<HDF5Data::Candle> loadHDF5Data() ;
	
	// Démarrer/arrêter la surveillance
	void startMonitoring(std::function<void()> onChangeCallback = nullptr);
	void stopMonitoring();
	bool isMonitoring();
	
protected:

private:
	std::string path_;
	std::string extension_;
	std::string dataset_name_;
	std::string filename_;
	std::string filepath_;
	std::filesystem::file_time_type currentModifiedTime;
	std::thread monitoringThread;
	std::atomic<bool> stopMonitoringFlag;
	std::mutex filepathMutex;
	std::function<void()> onChangeCallback_;
	
	// Fonction pour surveiller les modifications du fichier HDF5
	void monitorFile();
};

#endif // HDF5_READER_HPP

