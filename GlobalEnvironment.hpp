#pragma once

#include <Wt/WApplication.h>
#include <string>
#include <chrono>
#include <vector>

class GlobalEnvironment {
public:
    static GlobalEnvironment& instance() {
        static GlobalEnvironment instance;
        return instance;
    }

    void initialize(const Wt::WEnvironment& env) {
        screenWidth_ = env.screenWidth();
        screenHeight_ = env.screenHeight();
        timeZoneOffset_ = env.timeZoneOffset().count();
        timeZoneName_ = env.timeZoneName();
        locale_ = env.locale().name();
        dpiScale_ = env.dpiScale();
        userAgent_ = env.userAgent();
        referer_ = env.referer();
        clientAddress_ = env.clientAddress();
        serverSignature_ = env.serverSignature();
        serverSoftware_ = env.serverSoftware();
        serverAdmin_ = env.serverAdmin();
    }

    int screenWidth() const { return screenWidth_; }
    int screenHeight() const { return screenHeight_; }
    int timeZoneOffset() const { return timeZoneOffset_; }
    const std::string& timeZoneName() const { return timeZoneName_; }
    const std::string& locale() const { return locale_; }
    double dpiScale() const { return dpiScale_; }
    const std::string& userAgent() const { return userAgent_; }
    const std::string& referer() const { return referer_; }
    const std::string& clientAddress() const { return clientAddress_; }
    const std::string& serverSignature() const { return serverSignature_; }
    const std::string& serverSoftware() const { return serverSoftware_; }
    const std::string& serverAdmin() const { return serverAdmin_; }

private:
    GlobalEnvironment() = default;
    GlobalEnvironment(const GlobalEnvironment&) = delete;
    GlobalEnvironment& operator=(const GlobalEnvironment&) = delete;

    int screenWidth_ = 0;
    int screenHeight_ = 0;
    int timeZoneOffset_ = 0;
    std::string timeZoneName_;
    std::string locale_;
    double dpiScale_ = 1.0;
    std::string userAgent_;
    std::string referer_;
    std::string clientAddress_;
    std::string serverSignature_;
    std::string serverSoftware_;
    std::string serverAdmin_;
};

