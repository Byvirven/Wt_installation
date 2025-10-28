#ifndef MAIN_PAGE_H
#define MAIN_PAGE_H

#include <Wt/WEnvironment.h>
#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WPushButton.h>
#include <Wt/WComboBox.h>
#include <Wt/WText.h>
#include <Wt/WImage.h>
#include <Wt/WVBoxLayout.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WTableView.h>
#include <Wt/WLabel.h>
#include <Wt/WMenu.h>
#include <Wt/WPopupMenu.h>
#include <Wt/WNavigationBar.h>
#include <Wt/WStandardItemModel.h>
#include <Wt/WStandardItem.h> 
#include <Wt/WStackedWidget.h>
#include <Wt/WDialog.h>
#include <Wt/WLineEdit.h>
#include <Wt/WFileUpload.h>
#include <Wt/WProgressBar.h>
#include <Wt/WDateTime.h>
#include <Wt/WDate.h>
#include <Wt/WTimer.h>
#include <Wt/WSelectionBox.h>
#include <Wt/WMessageBox.h>
#include <Wt/WLength.h>
#include <Wt/Chart/WAxisSliderWidget.h>
#include <Wt/Chart/WCartesianChart.h>
#include <Wt/Chart/WDataSeries.h>
#include <Wt/Chart/WAbstractChartModel.h>
#include <Wt/WAbstractItemModel.h>
#include <Wt/WAbstractItemView.h>
#include <Wt/Chart/WAxis.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WStandardItemModel.h>
#include <Wt/WShadow.h>
#include <Wt/WPaintedWidget.h>
#include <Wt/WInteractWidget.h>
#include <Wt/WEvent.h>
#include <Wt/WItemDelegate.h>
#include <Wt/WObject.h>
#include <Wt/WString.h>
#include <Wt/Http/Cookie.h>
#include <Wt/WLogger.h>
#include <Wt/WJavaScript.h>
#include <Wt/Json/Array.h>
#include <Wt/Json/Object.h>
#include <Wt/Json/Serializer.h>

#include "H5Cpp.h"

#include "HDF5Reader.hpp"
#include "Database.hpp"
#include "AuthWidget.hpp"
#include "FinancialChart.hpp"
#include "DataStruct.hpp"

#include <filesystem>
#include <vector>
#include <string>
#include <any>
#include <sstream>
#include <iomanip>
#include <cmath>

class MainPage : public Wt::WContainerWidget {
public:
	MainPage(Database& db, const std::string& username, User::Role role, int screenWidth, int screenHeight, int windowWidth, int windowHeight, int timeZoneOffset);
	~MainPage();
	
protected:
	Database& db_;
	std::string username_;
	User::Role role_;
	int screenWidth_ ;
	int screenHeight_ ;
	int windowWidth_;
	int windowHeight_;
	int timezoneOffset_ ;

	virtual void initializeInterface();
	virtual void createHeader();
	virtual void createContent();
	virtual void createFooter();
	virtual void showViewer();
	virtual void onViewerClicked();
	virtual void onUserManagementClicked();
	virtual void onFileManagementClicked();

	void createMenu(Wt::WNavigationBar* navBar) ; 
	void createPairsList();
	void onFileSelected() ;
	void onCurrencyChanged() ;
	void onLogoutClicked();
	void onProfileClicked();
	
	// créer la zone d'affichage des données des bougies
	Wt::WContainerWidget * createDataDisplay() ;
	// créer le conneter la souris pour détecter la position X et indiquer les données relatives à la position
	void connectMouseHover(Wt::Chart::WCartesianChart *chart, Wt::WContainerWidget* displayWidget, const std::vector<HDF5Data::Candle>& data) ;
	// Assembler les graphiques ; la fonction appelera les fonctions ci-dessus pour créer et assembler les graphiques
	Wt::WContainerWidget * createFullChartWithHover() ;


	Wt::WContainerWidget* headerContainer_;
	Wt::WContainerWidget* contentContainer_;
	Wt::WContainerWidget* footerContainer_;
	Wt::WContainerWidget* viewerContainer_;
	Wt::WSelectionBox* fileSelectionBox_;
	Wt::WComboBox* currencyComboBox_;
	std::vector<std::string> exchangeCurrencies_{"USDT", "BTC"};
	Wt::WContainerWidget* chartWidget_;
	Wt::WContainerWidget* candleDetailsWidget_;
	Wt::WPopupMenu* popupMenu_;
	Wt::WText* ticker_;

void setupResizing();	
void handleResize(int width, int height);
std::unique_ptr<Wt::JSignal<int, int>> screenSignal_;


private:
	HDF5Reader * filereader_;
};

#endif // MAIN_PAGE_H

