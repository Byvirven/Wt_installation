#include "AdminPage.hpp"

AdminPage::AdminPage(Database& db, const std::string& username, int screenWidth, int screenHeight, int windowWidth, int windowHeight, int timeZoneOffset)
    : MainPage(db, username, User::Role::Admin, screenWidth, screenHeight, windowWidth, windowHeight, timeZoneOffset)
{
    initializeInterface();
}

void AdminPage::initializeInterface() {
	auto mainLayout = setLayout(std::make_unique<Wt::WVBoxLayout>());
	mainLayout->setContentsMargins(0, 0, 0, 0);

	// Créer le header avec le menu
	createHeader();

	// Créer le conteneur principal pour le contenu de l'admin
	mainContainer_ = mainLayout->addWidget(std::make_unique<Wt::WContainerWidget>());
	auto contentLayout = mainContainer_->setLayout(std::make_unique<Wt::WVBoxLayout>());

	viewerContainer_ = contentLayout->addWidget(std::make_unique<Wt::WContainerWidget>());
	viewerContainer_->setStyleClass("viewer-container");
	userManagementContainer_ = contentLayout->addWidget(std::make_unique<Wt::WContainerWidget>());
	userManagementContainer_->setStyleClass("viewer-container");
	fileManagementContainer_ = contentLayout->addWidget(std::make_unique<Wt::WContainerWidget>());
	fileManagementContainer_->setStyleClass("viewer-container");

	viewerContainer_->hide();
	userManagementContainer_->hide();
	fileManagementContainer_->hide();

	// Créer le footer avec le ticker
	createFooter();

	// Afficher le viewer par défaut
	showViewer();
}

void AdminPage::onViewerClicked() {
	showViewer();
}

void AdminPage::onUserManagementClicked() {
	showUserManagement();
}

void AdminPage::onFileManagementClicked() {
	showFileManagement();
}

void AdminPage::showViewer() {
	fileManagementContainer_->hide();
	userManagementContainer_->hide();
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
	currencyComboBox_->changed().connect(this, &AdminPage::onCurrencyChanged);

	// Ajouter le fileSelectionBox_ ensuite (en dessous)
	fileSelectionBox_ = fileListLayout->addWidget(std::make_unique<Wt::WSelectionBox>());
	fileSelectionBox_->setStyleClass("showViewer-file-selection-box");
	fileSelectionBox_->setSelectionMode(Wt::SelectionMode::Single);
	fileSelectionBox_->changed().connect(this, &AdminPage::onFileSelected);

	// Charger les fichiers HDF5 initiaux
	createPairsList();

	// Ajouter les widgets pour la visualisation des données et les détails des bougies
	chartWidget_ = layout->addWidget(std::make_unique<Wt::WContainerWidget>());
	chartWidget_->setStyleClass("showViewer-chart-widget");

	candleDetailsWidget_ = layout->addWidget(std::make_unique<Wt::WContainerWidget>());
	candleDetailsWidget_->setStyleClass("showViewer-candle-details-widget");

	// Définir les proportions des colonnes
	layout->setStretchFactor(fileListContainer, 5);  // 5% pour la partie des paires
	layout->setStretchFactor(chartWidget_, 85);      // 85% pour la partie d'affichage du graphique
	layout->setStretchFactor(candleDetailsWidget_, 10); // 10% pour la partie de visualisation des données
}

void AdminPage::showUserManagement() {
	userManagementContainer_->clear();
	userManagementContainer_->show();
	fileManagementContainer_->hide();
	viewerContainer_->hide();

	auto userTable = userManagementContainer_->addNew<Wt::WTable>();
	auto users = db_.getAllUsers();

	userTable->elementAt(0, 0)->addNew<Wt::WText>("<b>Username</b>");
	userTable->elementAt(0, 1)->addNew<Wt::WText>("<b>Role</b>");
	userTable->elementAt(0, 2)->addNew<Wt::WText>("<b>Action</b>");

	int row = 1;
	for (const auto& user : users) {
		userTable->elementAt(row, 0)->addNew<Wt::WText>(user.username);
		userTable->elementAt(row, 1)->addNew<Wt::WText>(user.role == User::Role::Admin ? "Admin" : "User");

		auto editButton = userTable->elementAt(row, 2)->addNew<Wt::WPushButton>("Edit");
		editButton->clicked().connect([=] { showEditUserDialog(user.username); });

		auto deleteButton = userTable->elementAt(row, 2)->addNew<Wt::WPushButton>("Delete");
		deleteButton->clicked().connect([=] { onDeleteUserClicked(user.username); });

		row++;
	}

	auto addUserButton = userManagementContainer_->addNew<Wt::WPushButton>("Add User");
	addUserButton->clicked().connect(this, &AdminPage::showAddUserDialog);
}

void AdminPage::showFileManagement() {
	fileManagementContainer_->clear();
	fileManagementContainer_->show();
	userManagementContainer_->hide();
	viewerContainer_->hide();

	auto fileTable = fileManagementContainer_->addNew<Wt::WTable>();
	auto files = db_.getAllFiles();

	fileTable->elementAt(0, 0)->addNew<Wt::WText>("<b>Type</b>");
	fileTable->elementAt(0, 2)->addNew<Wt::WText>("<b>Path</b>");
	fileTable->elementAt(0, 3)->addNew<Wt::WText>("<b>Action</b>");

	int row = 1;
	for (const auto& file : files) {
		fileTable->elementAt(row, 0)->addNew<Wt::WText>(file.type);

		auto pathEdit = fileTable->elementAt(row, 2)->addNew<Wt::WLineEdit>();
		pathEdit->setText(file.path);
		pathEdit->setReadOnly(true);

		auto editButton = fileTable->elementAt(row, 3)->addNew<Wt::WPushButton>("Edit");
		editButton->clicked().connect([=] {
			showEditFileDialog(file.type);
		});

		row++;
	}
}

void AdminPage::showAddUserDialog() {
	auto dialog = addChild(std::make_unique<Wt::WDialog>("Add New User"));

	auto username = dialog->contents()->addNew<Wt::WLineEdit>();
	username->setPlaceholderText("Username");

	auto password = dialog->contents()->addNew<Wt::WLineEdit>();
	password->setPlaceholderText("Password");
	password->setEchoMode(Wt::EchoMode::Password);

	auto role = dialog->contents()->addNew<Wt::WComboBox>();
	role->addItem("User");
	role->addItem("Admin");

	auto addButton = dialog->footer()->addNew<Wt::WPushButton>("Add");
	addButton->clicked().connect([=] {
		if (username->text().empty() || password->text().empty()) {
			Wt::WMessageBox::show("Error", "Username and password cannot be empty", Wt::StandardButton::Ok);
			return;
		}
		if (db_.addUser(username->text().toUTF8(), password->text().toUTF8(), role->currentIndex() == 1 ? User::Role::Admin : User::Role::Normal)) {
			showUserManagement();
			dialog->accept();
		} else {
			Wt::WMessageBox::show("Error", "Failed to add user", Wt::StandardButton::Ok);
		}
	});

	auto cancelButton = dialog->footer()->addNew<Wt::WPushButton>("Cancel");
	cancelButton->clicked().connect(dialog, &Wt::WDialog::reject);

	dialog->show();
}

void AdminPage::showEditUserDialog(const std::string& username) {
	auto userOpt = db_.getUser(username);
	if (!userOpt) {
		Wt::WMessageBox::show("Error", "User not found", Wt::StandardButton::Ok);
		return;
	}

	const User& user = *userOpt;

	auto dialog = addChild(std::make_unique<Wt::WDialog>("Edit User"));

	auto usernameEdit = dialog->contents()->addNew<Wt::WLineEdit>();
	usernameEdit->setText(user.username);

	auto passwordEdit = dialog->contents()->addNew<Wt::WLineEdit>();
	passwordEdit->setPlaceholderText("New Password (leave empty to keep current)");
	passwordEdit->setEchoMode(Wt::EchoMode::Password);

	auto roleEdit = dialog->contents()->addNew<Wt::WComboBox>();
	roleEdit->addItem("User");
	roleEdit->addItem("Admin");
	roleEdit->setCurrentIndex(user.role == User::Role::Admin ? 1 : 0);

	auto saveButton = dialog->footer()->addNew<Wt::WPushButton>("Save");
	saveButton->clicked().connect([=] {
		if (usernameEdit->text().empty()) {
			Wt::WMessageBox::show("Error", "Username cannot be empty", Wt::StandardButton::Ok);
			return;
		}
		User::Role newRole = roleEdit->currentIndex() == 1 ? User::Role::Admin : User::Role::Normal;
		if (db_.updateUserProfile(username, usernameEdit->text().toUTF8(), passwordEdit->text().toUTF8(), newRole)) {
			showUserManagement();
			dialog->accept();
		} else {
			Wt::WMessageBox::show("Error", "Failed to update user", Wt::StandardButton::Ok);
		}
	});

	auto cancelButton = dialog->footer()->addNew<Wt::WPushButton>("Cancel");
	cancelButton->clicked().connect(dialog, &Wt::WDialog::reject);

	dialog->show();
}

void AdminPage::onDeleteUserClicked(const std::string& username) {
	auto messageBox = addChild(std::make_unique<Wt::WMessageBox>(
		"Confirm Deletion",
		Wt::WString("Are you sure you want to delete user {1}?").arg(username),
		Wt::Icon::Question, Wt::StandardButton::Yes | Wt::StandardButton::No
	));

	messageBox->buttonClicked().connect([=] (Wt::StandardButton button) {
		if (button == Wt::StandardButton::Yes) {
			if (db_.deleteUser(username)) {
				showUserManagement();
			} else {
				Wt::WMessageBox::show("Error", "Failed to delete user", Wt::StandardButton::Ok);
			}
		}
		removeChild(messageBox);
	});

	messageBox->show();
}

void AdminPage::showEditFileDialog(const std::string& fileType) {
	auto fileOpt = db_.getFile(fileType);
	if (!fileOpt) {
		Wt::WMessageBox::show("Error", "File not found", Wt::StandardButton::Ok);
		return;
	}

	const File& file = *fileOpt;

	auto dialog = addChild(std::make_unique<Wt::WDialog>("Edit File Path"));

	//auto typeText = dialog->contents()->addNew<Wt::WText>("Type: " + file.type);

	auto pathEdit = dialog->contents()->addNew<Wt::WLineEdit>();
	pathEdit->setText(file.path);

	auto saveButton = dialog->footer()->addNew<Wt::WPushButton>("Save");
	saveButton->clicked().connect([=] {
		if (pathEdit->text().empty()) {
			Wt::WMessageBox::show("Error", "Path cannot be empty", Wt::StandardButton::Ok);
			return;
		}
		auto confirmBox = addChild(std::make_unique<Wt::WMessageBox>(
			"Confirm Change",
			Wt::WString("Are you sure you want to change the path for {1}?").arg(file.type),
			Wt::Icon::Question, Wt::StandardButton::Yes | Wt::StandardButton::No
		));
		confirmBox->buttonClicked().connect([=] (Wt::StandardButton result) {
			if (result == Wt::StandardButton::Yes) {
				if (db_.updateFilePath(file.type, pathEdit->text().toUTF8())) {
					showFileManagement();
					dialog->accept();
				} else {
					Wt::WMessageBox::show("Error", "Failed to update file path", Wt::StandardButton::Ok);
				}
			}
			removeChild(confirmBox);
		});
		confirmBox->show();
	});

	auto cancelButton = dialog->footer()->addNew<Wt::WPushButton>("Cancel");
	cancelButton->clicked().connect(dialog, &Wt::WDialog::reject);

	dialog->show();
}

