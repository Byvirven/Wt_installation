#ifndef ADMIN_PAGE_H
#define ADMIN_PAGE_H

#include "MainPage.hpp"
#include <Wt/WTable.h>

class AdminPage : public MainPage {
public:
    AdminPage(Database& db, const std::string& username, int screenWidth, int screenHeight, int windowWidth, int windowHeight, int timeZoneOffset);
    ~AdminPage() override = default;

protected:
    void initializeInterface() override;
    void showViewer() override;
    void onViewerClicked() override;
    void onUserManagementClicked() override;
    void onFileManagementClicked() override;

private:
    void showUserManagement();
    void showFileManagement();
    void onAddUserClicked();
    void onDeleteUserClicked(const std::string& username);
    void showAddUserDialog();
    void showEditUserDialog(const std::string& username);
    void showEditFileDialog(const std::string& fileType);

    Wt::WContainerWidget* mainContainer_;
    Wt::WContainerWidget* viewerContainer_;
    Wt::WContainerWidget* userManagementContainer_;
    Wt::WContainerWidget* fileManagementContainer_;
};


#endif // ADMIN_PAGE_H

