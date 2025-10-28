#include "AuthWidget.hpp"

AuthWidget::AuthWidget(Database& db)
    : db_(db)
{
    verifier_.addHashFunction(std::make_unique<Wt::Auth::BCryptHashFunction>(7));
    
    auto layout = std::make_unique<Wt::WVBoxLayout>();

    auto usernameLayout = std::make_unique<Wt::WHBoxLayout>();
    usernameLayout->addWidget(std::make_unique<Wt::WLabel>("Username:"));
    usernameEdit_ = usernameLayout->addWidget(std::make_unique<Wt::WLineEdit>());
    layout->addLayout(std::move(usernameLayout));

    auto passwordLayout = std::make_unique<Wt::WHBoxLayout>();
    passwordLayout->addWidget(std::make_unique<Wt::WLabel>("Password:"));
    passwordEdit_ = passwordLayout->addWidget(std::make_unique<Wt::WLineEdit>());
    passwordEdit_->setEchoMode(Wt::EchoMode::Password);
    layout->addLayout(std::move(passwordLayout));

    loginButton_ = layout->addWidget(std::make_unique<Wt::WPushButton>("Login"));
    loginButton_->clicked().connect(this, &AuthWidget::onLoginClicked);

    messageText_ = layout->addWidget(std::make_unique<Wt::WText>());
    messageText_->setStyleClass("error");

    setLayout(std::move(layout));
}

/*void AuthWidget::onLoginClicked() {
    std::string username = usernameEdit_->text().toUTF8();
    std::string password = passwordEdit_->text().toUTF8();

    User::Role role;
    if (db_.authenticateUser(username, password, role)) {
        authenticated_.emit(username, role);
    } else {
        messageText_->setText("Invalid username or password");
    }
}
*/
void AuthWidget::onLoginClicked() {
    std::string username = usernameEdit_->text().toUTF8();
    std::string password = passwordEdit_->text().toUTF8();

    User::Role role;
    if (db_.authenticateUser(username, password, role)) {
        authenticated_.emit(username, role);
    } else {
        messageText_->setText("Nom d'utilisateur ou mot de passe invalide");
    }
}

