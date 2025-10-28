#include <Wt/WApplication.h>
#include <Wt/WServer.h>
#include <Wt/WEnvironment.h>
#include <Wt/Http/Cookie.h>
#include "ParakeetApp.hpp"
#include <chrono>

std::unique_ptr<Wt::WApplication> createApplication(const Wt::WEnvironment& env) {
    auto app = std::make_unique<ParakeetApp>(env);
    //Wt::Http::Cookie cookie("sessionid", "");
    //cookie.setMaxAge(std::chrono::seconds(3600*24*30));  // 30 jours
    //app->setCookie(cookie);
    return app;
}

int main(int argc, char **argv)
{
  try {
    Wt::WServer server{argc, argv, WTHTTP_CONFIGURATION};

    server.addEntryPoint(Wt::EntryPointType::Application, createApplication);

    server.run();
  } catch (Wt::WServer::Exception& e) {
    std::cerr << e.what() << '\n';
  } catch (Wt::Dbo::Exception &e) {
    std::cerr << "Dbo exception: " << e.what() << '\n';
  } catch (std::exception &e) {
    std::cerr << "exception: " << e.what() << '\n';
  }
}

