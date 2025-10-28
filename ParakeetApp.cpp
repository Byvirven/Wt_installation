#include "ParakeetApp.hpp"

ParakeetApp::ParakeetApp(const Wt::WEnvironment& env)
    : Wt::WApplication(env), db_("parakeet.db"), currentUserRole_(User::Role::Normal)
{
	setTitle("Parakeet Weather - Loading...");
	GlobalEnvironment::instance().initialize(const_cast<Wt::WEnvironment&>(env));
	auto initialPage = root()->addWidget(std::make_unique<InitialPage>());
   initialPage->dimensionsReceived().connect(this, &ParakeetApp::onDimensionsReceived);
}

void ParakeetApp::onDimensionsReceived(int width, int height) {
	#ifdef DEBUG_MODE
	Wt::log("info") << __FUNCTION__ ;
	#endif  
	// Get window variables
	windowWidth_ = width; 
	windowHeight_ = height;
	
	// Get environment variables
	screenWidth_ = GlobalEnvironment::instance().screenWidth();
	screenHeight_ = GlobalEnvironment::instance().screenHeight();
	timeZoneOffset_ = GlobalEnvironment::instance().timeZoneOffset() ;
	
	// clear current empty page
	root()->clear();
	
	// load the true page
	//useStyleSheet("resources/themes/bootstrap/3/bootstrap.min.css");
	//useStyleSheet("resources/themes/bootstrap/3/wt.css");
	setTitle("Parakeet Weather");
	useStyleSheet("resources/css/custom.css");
	manageSessionCookie();
	loadUserSession();
}

void ParakeetApp::showAuthWidget() {
    auto authWidget = root()->addWidget(std::make_unique<AuthWidget>(db_));
    authWidget->authenticated().connect(this, &ParakeetApp::onUserAuthenticated);
}

void ParakeetApp::onUserAuthenticated(const std::string& username, User::Role role) {
    currentUsername_ = username;
    currentUserRole_ = role;

    Wt::Http::Cookie usernameCookie("username", username);
    //usernameCookie.setMaxAge(std::chrono::hours(24));
    setCookie(usernameCookie);

    Wt::Http::Cookie roleCookie("role", std::to_string(static_cast<int>(role)));
    //roleCookie.setMaxAge(std::chrono::hours(24));
    setCookie(roleCookie);

    root()->clear();
    if (role == User::Role::Admin) {
        root()->addWidget(std::make_unique<AdminPage>(db_, username, screenWidth_, screenHeight_, windowWidth_, windowHeight_, timeZoneOffset_));
    } else {
        root()->addWidget(std::make_unique<MainPage>(db_, username, role, screenWidth_, screenHeight_, windowWidth_, windowHeight_, timeZoneOffset_));
    }
}

void ParakeetApp::handleLogout() {
    currentUsername_.clear();
    currentUserRole_ = User::Role::Normal;
    showAuthWidget();
}

void ParakeetApp::manageSessionCookie() {
    const std::string* sessionId = environment().getCookie("sessionId");
    if (!sessionId || sessionId->empty()) {
        // Créer un nouveau cookie de session avec un ID unique
        std::string newSessionId = "";//Wt::WRandom::generateId(32);
        Wt::Http::Cookie cookie("sessionId", newSessionId);
        //cookie.setMaxAge(std::chrono::seconds(3600 * 24 * 30)); // 30 jours
        setCookie(cookie);
    }
}


void ParakeetApp::loadUserSession() {
    const std::string* username = environment().getCookie("username");
    const std::string* roleStr = environment().getCookie("role");
    
    if (username && roleStr && !username->empty() && !roleStr->empty()) {
        currentUsername_ = *username;
        currentUserRole_ = static_cast<User::Role>(std::stoi(*roleStr));
        
        if (currentUserRole_ == User::Role::Admin) {
            root()->addWidget(std::make_unique<AdminPage>(db_, currentUsername_, screenWidth_, screenHeight_, windowWidth_, windowHeight_, timeZoneOffset_));
        } else {
            root()->addWidget(std::make_unique<MainPage>(db_, currentUsername_, currentUserRole_, screenWidth_, screenHeight_, windowWidth_, windowHeight_, timeZoneOffset_));
            
        }
    } else {
        showAuthWidget();
    }
}


void ParakeetApp::resetApplication() {
    currentUsername_.clear();
    currentUserRole_ = User::Role::Normal;
    root()->clear();
    showAuthWidget();
}

