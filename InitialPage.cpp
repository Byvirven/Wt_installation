// InitialPage.cpp
#include "InitialPage.hpp"

InitialPage::InitialPage() : dimensionsSignal_(this, "dimensionsSignal") {
    dimensionsSignal_.connect([this](int width, int height) {
        dimensionsReceived_.emit(width, height);
    });

    doJavaScript(
        "function sendDimensions() {"
        "  var width = window.innerWidth || document.documentElement.clientWidth || document.body.clientWidth;"
        "  var height = window.innerHeight || document.documentElement.clientHeight || document.body.clientHeight;" +
        dimensionsSignal_.createCall({"width", "height"}) + ";"
        "}"
        "sendDimensions();"
    );
}
