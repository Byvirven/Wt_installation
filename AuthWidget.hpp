#ifndef AUTH_WIDGET_H
#define AUTH_WIDGET_H

#include <Wt/WContainerWidget.h>
#include <Wt/WVBoxLayout.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>
#include <Wt/WLabel.h>
#include <Wt/Auth/PasswordVerifier.h>
#include <Wt/Auth/HashFunction.h>
#include "Database.hpp"

class AuthWidget : public Wt::WContainerWidget {
public:
    AuthWidget(Database& db);

    Wt::Signal<std::string, User::Role>& authenticated() { return authenticated_; }

private:
    Database& db_;
    Wt::WLineEdit* usernameEdit_;
    Wt::WLineEdit* passwordEdit_;
    Wt::WPushButton* loginButton_;
    Wt::WText* messageText_;

    Wt::Signal<std::string, User::Role> authenticated_;
    Wt::Auth::PasswordVerifier verifier_;
    void onLoginClicked();
};

#endif // AUTH_WIDGET_H

