#include "MainPage.hpp"

MainPage::MainPage(Database& db, const std::string& username, User::Role role, int screenWidth, int screenHeight, int windowWidth, int windowHeight, int timeZoneOffset)
    : db_(db), username_(username), role_(role), screenWidth_(screenWidth), screenHeight_(screenHeight), windowWidth_(windowWidth), windowHeight_(windowHeight), timezoneOffset_(timeZoneOffset)
{
	filereader_ = new HDF5Reader(db_.getFile("HDF5")->path); // activer le lecteur
	
	setupResizing();
	initializeInterface();
}

MainPage::~MainPage() {
	delete filereader_;
}

void MainPage::setupResizing() {
	screenSignal_ = std::make_unique<Wt::JSignal<int, int>>(this, "screenSignal");
	screenSignal_->connect(this, &MainPage::handleResize);

	std::string screenSignalJavaScript = 
		"function updateResize() {"
		"  var width = window.innerWidth || document.documentElement.clientWidth || document.body.clientWidth;"
		"  var height = window.innerHeight || document.documentElement.clientHeight || document.body.clientHeight;" +
		screenSignal_->createCall({"width", "height"}) + ";"
		"}"
		"window.addEventListener('resize', updateResize);";

	doJavaScript(screenSignalJavaScript);
}

void MainPage::handleResize(int width, int height) {
	#ifdef DEBUG_MODE
	Wt::log("info") << __FUNCTION__ ;
	#endif  
	windowWidth_ = width; 
	windowHeight_ = height;
	
    this->resize(width, height);
    
    // Redimensionner les widgets enfants si nécessaire
    //if (chartWidget_) {
    //    chartWidget_->resize(windowWidth_, windowHeight_);
    //}
}

void MainPage::initializeInterface() {
	auto mainLayout = setLayout(std::make_unique<Wt::WVBoxLayout>());
	mainLayout->setSpacing(0);
	mainLayout->setContentsMargins(0, 0, 0, 0);

	createHeader();
	createContent();
	createFooter();

}


void MainPage::createHeader() {
	auto container = std::make_unique<Wt::WContainerWidget>();
	headerContainer_ = container.get();
	layout()->addWidget(std::move(container));
	headerContainer_->setStyleClass("header");

	auto navBar = headerContainer_->addWidget(std::make_unique<Wt::WNavigationBar>());
	createMenu(navBar);
}

void MainPage::createContent() {
	auto container = std::make_unique<Wt::WContainerWidget>();
	contentContainer_ = container.get();
	layout()->addWidget(std::move(container));
	contentContainer_->setStyleClass("content");
	//contentContainer_->setHeight (Wt::WLength(90.0, Wt::LengthUnit::ViewportHeight ));

	auto viewerContainer = std::make_unique<Wt::WContainerWidget>();
	viewerContainer_ = viewerContainer.get();
	contentContainer_->addWidget(std::move(viewerContainer));
	viewerContainer_->setStyleClass("viewer-container");

	showViewer();
}

void MainPage::createFooter() {
    auto container = std::make_unique<Wt::WContainerWidget>();
    footerContainer_ = container.get();
    layout()->addWidget(std::move(container));
    footerContainer_->setStyleClass("footer");

    ticker_ = footerContainer_->addWidget(std::make_unique<Wt::WText>("Real-time Information Ticker"));
    ticker_->setStyleClass("ticker");
}

void MainPage::createMenu(Wt::WNavigationBar* navBar) {
    // Create a popup menu
    auto popupPtr = std::make_unique<Wt::WPopupMenu>();
    popupMenu_ = popupPtr.get();

    popupMenu_->addItem("👤 Profil")->triggered().connect(this, &MainPage::onProfileClicked);
    popupMenu_->addItem("👋 Logout")->triggered().connect(this, &MainPage::onLogoutClicked);

    if (role_ == User::Role::Admin) {
        popupMenu_->addItem("🗠 Chart")->triggered().connect(this, &MainPage::onViewerClicked);
        popupMenu_->addItem("🪪 User Management")->triggered().connect(this, &MainPage::onUserManagementClicked);
        popupMenu_->addItem("⚙️ File Management")->triggered().connect(this, &MainPage::onFileManagementClicked);
    }
    // Add a button to trigger the popup menu
    auto menuButton = navBar->addWidget(std::make_unique<Wt::WPushButton>("🏠 "+username_));
    menuButton->setMenu(std::move(popupPtr));
}


void MainPage::showViewer() {
    viewerContainer_->clear();
    viewerContainer_->show();

    auto layout = viewerContainer_->setLayout(std::make_unique<Wt::WHBoxLayout>());

    // Créer un conteneur vertical pour le currencyComboBox_ et le fileSelectionBox_
    auto fileListContainer = layout->addWidget(std::make_unique<Wt::WContainerWidget>());
    fileListContainer->setStyleClass("showViewer-file-list-container");
    auto fileListLayout = fileListContainer->setLayout(std::make_unique<Wt::WVBoxLayout>());

    // Ajouter le currencyComboBox_ en premier (au-dessus)
    currencyComboBox_ = fileListLayout->addWidget(std::make_unique<Wt::WComboBox>());
    currencyComboBox_->setStyleClass("showViewer-currency-combo-box");
    for (const auto& currency : exchangeCurrencies_) {
        currencyComboBox_->addItem(currency);
    }
    currencyComboBox_->changed().connect(this, &MainPage::onCurrencyChanged);

    // Ajouter le fileSelectionBox_ ensuite (en dessous)
    fileSelectionBox_ = fileListLayout->addWidget(std::make_unique<Wt::WSelectionBox>());
    fileSelectionBox_->setStyleClass("showViewer-file-selection-box");
    fileSelectionBox_->setSelectionMode(Wt::SelectionMode::Single);
    fileSelectionBox_->changed().connect(this, &MainPage::onFileSelected);

    // Ajouter les widgets pour la visualisation des données et les détails des bougies
    chartWidget_ = layout->addWidget(std::make_unique<Wt::WContainerWidget>());
    chartWidget_->setStyleClass("showViewer-chart-widget");

    candleDetailsWidget_ = layout->addWidget(std::make_unique<Wt::WContainerWidget>());
    candleDetailsWidget_->setStyleClass("showViewer-candle-details-widget");

    // Définir les proportions des colonnes
    layout->setStretchFactor(fileListContainer, 5);  // 5% pour la partie des paires
    layout->setStretchFactor(chartWidget_, 85);      // 85% pour la partie d'affichage du graphique
    layout->setStretchFactor(candleDetailsWidget_, 10); // 10% pour la partie de visualisation des données

    // Charger BTCUSDT par défaut
    auto it = std::find(exchangeCurrencies_.begin(), exchangeCurrencies_.end(), "USDT");
    if (it != exchangeCurrencies_.end()) {
        int index = std::distance(exchangeCurrencies_.begin(), it);
        currencyComboBox_->setCurrentIndex(index);
        onCurrencyChanged();  // Mettre à jour la liste des fichiers ou créer la liste initiale

        // Trouver et sélectionner BTCUSDT
        for (int i = 0; i < fileSelectionBox_->count(); ++i) {
            if (fileSelectionBox_->itemText(i) == "BTC") {
                fileSelectionBox_->setCurrentIndex(i);
                onFileSelected();  // Charger les données et afficher le graphique
                break;
            }
        }
    }
}


void MainPage::onViewerClicked() {
	showViewer();
}

void MainPage::onUserManagementClicked() {
	// To be implemented in AdminPage
}

void MainPage::onFileManagementClicked() {
	// To be implemented in AdminPage
}

void MainPage::onLogoutClicked() {
	Wt::Http::Cookie usernameCookie("username", "");
	//usernameCookie.setMaxAge(std::chrono::seconds(0));
	Wt::WApplication::instance()->setCookie(usernameCookie);

	Wt::Http::Cookie roleCookie("role", "");
	//roleCookie.setMaxAge(std::chrono::seconds(0));
	Wt::WApplication::instance()->setCookie(roleCookie);
	// Obtenir l'URL de base de l'application
	std::string baseUrl = Wt::WApplication::instance()->url();

	// Supprimer les paramètres de l'URL (tout ce qui suit '?')
	size_t questionMarkPos = baseUrl.find('?');
	if (questionMarkPos != std::string::npos) {
		baseUrl = baseUrl.substr(0, questionMarkPos);
	}

	// Clear application state
	Wt::WApplication::instance()->root()->clear();
	Wt::WApplication::instance()->root()->addWidget(std::make_unique<AuthWidget>(db_));

	// Rediriger vers la page de connexion
	Wt::WApplication::instance()->redirect(baseUrl);
	//Wt::WApplication::instance()->refresh();
}

void MainPage::onProfileClicked() {
	auto dialog = addChild(std::make_unique<Wt::WDialog>("Profil utilisateur"));
	auto container = dialog->contents()->addWidget(std::make_unique<Wt::WContainerWidget>());
	auto layout = container->setLayout(std::make_unique<Wt::WVBoxLayout>());

	// Champ pour le nom d'utilisateur
	auto usernameEdit = layout->addWidget(std::make_unique<Wt::WLineEdit>(), 0, Wt::AlignmentFlag::Center);
	usernameEdit->setPlaceholderText("Nom d'utilisateur");
	usernameEdit->setText(username_);

	// Champ pour le nouveau mot de passe
	auto passwordEdit = layout->addWidget(std::make_unique<Wt::WLineEdit>(), 0, Wt::AlignmentFlag::Center);
	passwordEdit->setPlaceholderText("Nouveau mot de passe");
	passwordEdit->setEchoMode(Wt::EchoMode::Password);

	// Champ pour confirmer le nouveau mot de passe
	auto confirmPasswordEdit = layout->addWidget(std::make_unique<Wt::WLineEdit>(), 0, Wt::AlignmentFlag::Center);
	confirmPasswordEdit->setPlaceholderText("Confirmer le nouveau mot de passe");
	confirmPasswordEdit->setEchoMode(Wt::EchoMode::Password);

	// Upload de la photo de profil
	auto fileUpload = layout->addWidget(std::make_unique<Wt::WFileUpload>(), 0, Wt::AlignmentFlag::Center);
	fileUpload->setFileTextSize(96);
	fileUpload->setProgressBar(std::make_unique<Wt::WProgressBar>());
	fileUpload->setMargin(10, Wt::Side::Top | Wt::Side::Bottom);
	fileUpload->setFilters("image/*");
	fileUpload->uploaded().connect([=] {
		// Traitement de l'image uploadée
		// Vous devrez implémenter la logique pour sauvegarder l'image
	});

	// Bouton de mise à jour
	auto updateButton = layout->addWidget(std::make_unique<Wt::WPushButton>("Mettre à jour"), 0, Wt::AlignmentFlag::Center);
	updateButton->setStyleClass("btn-primary");

	updateButton->clicked().connect([=] {
		if (passwordEdit->text() != confirmPasswordEdit->text()) {
			dialog->footer()->addWidget(std::make_unique<Wt::WText>("Les mots de passe ne correspondent pas."));
			return;
		}

		// Mettre à jour les informations de l'utilisateur dans la base de données
		bool success = db_.updateUserProfile(username_, usernameEdit->text().toUTF8(), passwordEdit->text().toUTF8(), role_);
		if (success) {
			username_ = usernameEdit->text().toUTF8();
			dialog->accept();
		} else {
			dialog->footer()->addWidget(std::make_unique<Wt::WText>("Erreur lors de la mise à jour du profil."));
		}
	});

	dialog->footer()->addWidget(std::make_unique<Wt::WPushButton>("Annuler"))->clicked().connect([=] {
		dialog->reject();
	});
	
	dialog->show();
}

void MainPage::createPairsList() {
	fileSelectionBox_->clear();

	if (!filereader_->directoryExists()) {
		Wt::WMessageBox::show("Error", "HDF5 directory does not exist or is not accessible", Wt::StandardButton::Ok);
		return;
	}
	
	std::vector<std::string> filteredFiles = filereader_->getFilesList(currencyComboBox_->currentText().toUTF8());

	// Ajouter les fichiers triés au WSelectionBox
	for (const auto& displayName : filteredFiles) {
		fileSelectionBox_->addItem(displayName);
	}
}

void MainPage::onFileSelected() {
    filereader_->setDatasetName( fileSelectionBox_->currentText().toUTF8() + currencyComboBox_->currentText().toUTF8() );

    chartWidget_->clear();
    auto newChart = createFullChartWithHover();
        if (newChart) {
        chartWidget_->addWidget(std::unique_ptr<Wt::WWidget>(newChart));
    } else {
        Wt::log("error") << "Failed to create chart";
    }
}

void MainPage::onCurrencyChanged() {
	createPairsList();
}

// créer la zone d'affichage des données des bougies
Wt::WContainerWidget* MainPage::createDataDisplay() {
	auto container = new Wt::WContainerWidget();
	container->setStyleClass("candle-details");
	container->setMinimumSize(200, 150);  // Ajustez ces valeurs selon vos besoins
	container->addNew<Wt::WText>("Hover over the chart to see details");
	return container;
}

// Assembler les graphiques ; la fonction appelera les fonctions ci-dessus pour créer et assembler les graphiques
Wt::WContainerWidget* MainPage::createFullChartWithHover() {
    //auto data = filereader->loadHDF5Data();
    //if (data.empty()) {
    //    Wt::log("error") << "No data loaded from HDF5 file";
    //    return nullptr;
    //}

	auto container = new Wt::WContainerWidget();
	container->setStyleClass("full-chart-container");
	container->resize(Wt::WLength (100, Wt::LengthUnit::Percentage), 	Wt::WLength (100, Wt::LengthUnit::Percentage));
	auto layout = container->setLayout(std::make_unique<Wt::WVBoxLayout>());

	
	// créer le modèle
	//auto model = createTOHLCVTModel(data);
	// créer le graphique
	auto ohlcChart = new Financial::Chart(filereader_, timezoneOffset_);//Financial::Chart::createModel(data, timezoneOffset_));
	// Définissez la configuration des séries
	ohlcChart->setSeriesConfigurations(
	{
		{ Financial::ChartType::Candlestick, {1, 2, 3, 4} },
		{ Financial::ChartType::Surface, {2, 3}, 0, 0, 255, 64 },
		{ Financial::ChartType::Line, {1}, 255 }
	});
	
	layout->addWidget(std::unique_ptr<Wt::WWidget>(ohlcChart), 3);
	
	const Wt::Chart::WDataSeries& series = ohlcChart->series(4);
	auto sliderWidget = layout->addWidget(std::make_unique<Wt::Chart::WAxisSliderWidget>(const_cast<Wt::Chart::WDataSeries*>(&series)));

	sliderWidget->setStyleClass("full-chart-slider");
	sliderWidget->resize(windowWidth_*0.80, 50);
	sliderWidget->setMaximumSize(windowWidth_*0.80, 50);
	sliderWidget->setSelectionAreaPadding(40, Wt::Side::Left | Wt::Side::Right);
	sliderWidget->setMargin(Wt::WLength::Auto, Wt::Side::Left | Wt::Side::Right);
	//sliderWidget->setAutoLayoutEnabled(true);
	sliderWidget->setLabelsEnabled(false);
	sliderWidget->setYAxisZoomEnabled(true);

	candleDetailsWidget_->clear();
	candleDetailsWidget_->addNew<Wt::WText>("Hover over the chart to see details");

	//ohlcChart->setupClientMouseHover(candleDetailsWidget_, data);
	
	return container;
}

void MainPage::connectMouseHover(Wt::Chart::WCartesianChart *chart, Wt::WContainerWidget* displayWidget, const std::vector<HDF5Data::Candle>& data) {
    chart->mouseMoved().connect([=](const Wt::WMouseEvent& event) {
        Wt::WPointF point = chart->mapFromDevice(Wt::WPointF(event.widget().x, event.widget().y));
		
        // Obtenir la valeur X
        int64_t xValue = static_cast<int64_t>(point.x()) ;
        size_t index = ((data.front().timestamp - 30) - xValue >= 0) 
        	? 0 : 
        	(
        		(xValue - (data.back().timestamp - 30 ) >= 0) 
        		? data.size() - 1 
        		: static_cast<int64_t>((xValue - (data.front().timestamp - 30))/60)
     		) ;

        // Récupérer les données à l'index trouvé
        const auto& candle = data[index];

        // Afficher les informations dans le widget
        std::stringstream ss;
        ss << "Date: " << Wt::WDateTime::fromTime_t(candle.timestamp).toString() << "\n"
           << "Open: " << std::setprecision(8) << candle.open << "\n"
           << "High: " << std::setprecision(8) << candle.high << "\n"
           << "Low: " << std::setprecision(8) << candle.low << "\n"
           << "Close: " << std::setprecision(8) << candle.close << "\n"
           << "Volume: " << std::setprecision(8) << candle.volume << "\n"
           << "Trades: " << std::setprecision(8) << candle.trades;

        displayWidget->clear();
        auto text = displayWidget->addNew<Wt::WText>();
        text->setText(Wt::WString::fromUTF8(ss.str()));
        text->setTextFormat(Wt::TextFormat::Plain);
    });
}
