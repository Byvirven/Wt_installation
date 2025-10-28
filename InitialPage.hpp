// InitialPage.hpp
#include <Wt/WContainerWidget.h>
#include <Wt/WJavaScript.h>

class InitialPage : public Wt::WContainerWidget {
public:
    InitialPage();
    Wt::Signal<int, int>& dimensionsReceived() { return dimensionsReceived_; }

private:
    Wt::JSignal<int, int> dimensionsSignal_;
    Wt::Signal<int, int> dimensionsReceived_;
};
