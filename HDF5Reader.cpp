#include "HDF5Reader.hpp"

HDF5Reader::HDF5Reader(std::string path, std::string extension) : path_(path), extension_(extension) {
	stopMonitoringFlag = false;
}

HDF5Reader::~HDF5Reader() {
	stopMonitoring();
}

void HDF5Reader::setPath(std::string path) {
	path_ = path;
}

std::string HDF5Reader::getPath() {
	return path_;
}

void HDF5Reader::setDatasetName(std::string datasetName) {
	dataset_name_ = datasetName;
	setFilename(datasetName);
}

std::string HDF5Reader::getDatasetName() {
	return dataset_name_ ;
}

void HDF5Reader::setFilename(std::string filename) {
	filename_ = filename + extension_;
	setFilePath();
}

std::string HDF5Reader::getFilename() {
	return filename_;
}

void HDF5Reader::setFilePath() {
	std::lock_guard<std::mutex> lock(filepathMutex);
	filepath_ = path_ + "/" + filename_;
}

std::string HDF5Reader::getFilePath() {
	std::lock_guard<std::mutex> lock(filepathMutex);
	return filepath_;
}

bool HDF5Reader::directoryExists() {
	return std::filesystem::exists(path_);
}

bool HDF5Reader::fileExists() {
	return std::filesystem::exists(filepath_);
}

std::vector<std::string> HDF5Reader::getFilesList(std::string selectedCurrency) {
	std::vector<std::string> filteredFiles;

	for (const auto& entry : std::filesystem::directory_iterator(path_)) {
		if (entry.path().extension() == ".h5") {
			std::string filename = entry.path().filename().string();
			// Vérifier si le fichier se termine par la devise sélectionnée
			if (filename.length() >= selectedCurrency.length() + 3 &&
				filename.substr(filename.length() - selectedCurrency.length() - 3) == selectedCurrency + ".h5") {

				// Extraire le nom du token
				std::string displayName = filename.substr(0, filename.length() - selectedCurrency.length() - 3);
				filteredFiles.push_back(displayName);
			}
		}
	}
	// Trier par ordre alphabétique
	std::sort(filteredFiles.begin(), filteredFiles.end());
	
	return filteredFiles ;
}

std::vector<HDF5Data::Candle> HDF5Reader::loadHDF5Data() {
	std::vector<HDF5Data::Candle> data;
	try {
        // Ouvrir le fichier en mode lecture seule avec l'option SWMR_ACCESS
        H5::H5File file(getFilePath(), H5F_ACC_RDONLY | H5F_ACC_SWMR_READ);

        // Ouvrir le dataset
        H5::DataSet dataset = file.openDataSet(getDatasetName());
        H5::DataSpace dataspace = dataset.getSpace();

        hsize_t dims[2];
        dataspace.getSimpleExtentDims(dims, nullptr);
        data.resize(dims[0]);

        H5::CompType mtype(sizeof(HDF5Data::Candle));
        mtype.insertMember("timestamp", HOFFSET(HDF5Data::Candle, timestamp), H5::PredType::NATIVE_INT64);
        mtype.insertMember("open", HOFFSET(HDF5Data::Candle, open), H5::PredType::NATIVE_DOUBLE);
        mtype.insertMember("high", HOFFSET(HDF5Data::Candle, high), H5::PredType::NATIVE_DOUBLE);
        mtype.insertMember("low", HOFFSET(HDF5Data::Candle, low), H5::PredType::NATIVE_DOUBLE);
        mtype.insertMember("close", HOFFSET(HDF5Data::Candle, close), H5::PredType::NATIVE_DOUBLE);
        mtype.insertMember("volume", HOFFSET(HDF5Data::Candle, volume), H5::PredType::NATIVE_DOUBLE);
        mtype.insertMember("trades", HOFFSET(HDF5Data::Candle, trades), H5::PredType::NATIVE_INT64);

        // Lire les données du dataset
        dataset.read(data.data(), mtype);

	} catch (H5::FileIException& error) {
		std::cerr << "Error opening file: " << error.getDetailMsg() << std::endl;
	} catch (H5::DataSetIException& error) {
		std::cerr << "Error accessing dataset: " << error.getDetailMsg() << std::endl;
	} catch (H5::DataSpaceIException& error) {
		std::cerr << "Error with dataspace: " << error.getDetailMsg() << std::endl;
	} catch (H5::DataTypeIException& error) {
		std::cerr << "Error with datatype: " << error.getDetailMsg() << std::endl;
	} catch (std::exception& error) {
		std::cerr << "Unexpected error: " << error.what() << std::endl;
	}

	// init currentModifiedTime
	currentModifiedTime = std::filesystem::last_write_time(getFilePath());
	
	return data;
}

// Démarre le thread qui surveille les modifications du fichier
void HDF5Reader::startMonitoring(std::function<void()> onChangeCallback) {
	stopMonitoringFlag = false;
	onChangeCallback_ = onChangeCallback;  // Stocke le callback
	monitoringThread = std::thread(&HDF5Reader::monitorFile, this);
}

// Arrête proprement le thread de surveillance
void HDF5Reader::stopMonitoring() {
	stopMonitoringFlag = true;
	if (monitoringThread.joinable()) {
		monitoringThread.join();  // Attend la fin du thread
	}
}

bool HDF5Reader::isMonitoring() {
	return !stopMonitoringFlag ;
}

// Fonction pour surveiller les modifications du fichier HDF5
void HDF5Reader::monitorFile() {
	while (!stopMonitoringFlag) {
		std::filesystem::file_time_type newModifiedTime = std::filesystem::last_write_time(getFilePath());
		
		if (newModifiedTime != currentModifiedTime) {
			currentModifiedTime = newModifiedTime;
			std::cout << "Le fichier a été modifié : " << getFilePath() << std::endl;
			
			if (onChangeCallback_) {
				onChangeCallback_();  // Appelle le callback lorsqu'une modification est détectée
			}
		}
		
		// Vérification toutes les 60 secondes
		std::this_thread::sleep_for(std::chrono::seconds(60));
	}
}

