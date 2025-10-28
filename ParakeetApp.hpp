#include <Wt/WApplication.h>
#include <Wt/WServer.h>
#include <Wt/WEnvironment.h>
#include <Wt/WTimer.h>
#include <Wt/Auth/AuthModel.h>
#include <Wt/Auth/AuthWidget.h>
#include <Wt/WJavaScript.h>
#include <Wt/Http/Cookie.h>
#include "AuthWidget.hpp"
#include "InitialPage.hpp"
#include "MainPage.hpp"
#include "AdminPage.hpp"
#include "Database.hpp"
#include "GlobalEnvironment.hpp"
#include <chrono>

class ParakeetApp : public Wt::WApplication {
public:
	ParakeetApp(const Wt::WEnvironment& env);

private:
	Database db_;
	std::unique_ptr<Wt::WTimer> sessionCheckTimer_;
	std::string currentUsername_;
	User::Role currentUserRole_;
	
	int screenWidth_ ;
	int screenHeight_ ;
	int windowWidth_;
	int windowHeight_;
	int timeZoneOffset_ ;

	void showAuthWidget();
	void onUserAuthenticated(const std::string& username, User::Role role);
	void handleLogout() ;
	void loadUserSession();
	void manageSessionCookie();
	void resetApplication();
	void onDimensionsReceived(int width, int height);
};

