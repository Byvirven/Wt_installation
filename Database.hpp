// Database.hpp
#ifndef DATABASE_H
#define DATABASE_H

#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/Session.h>
#include <Wt/Dbo/backend/Sqlite3.h>
#include <Wt/Dbo/Transaction.h>
#include <Wt/Dbo/Query.h>
#include <Wt/Auth/PasswordVerifier.h>
#include <Wt/Auth/HashFunction.h>
#include <Wt/Auth/PasswordHash.h>
#include "User.hpp"
#include "File.hpp"
#include <memory>

class Database {
public:
    Database(const std::string& dbFile);
    bool authenticateUser(const std::string& username, const std::string& password, User::Role& role);
    std::vector<std::string> getHDF5Files();
    std::vector<User> getAllUsers();
    bool addUser(const std::string& username, const std::string& password, User::Role role);
    bool deleteUser(const std::string& username);
    std::vector<File> getAllFiles();
    bool addFile(const std::string& path);
    bool deleteFile(const std::string& path);
    bool updateUserProfile(const std::string& oldUsername, const std::string& newUsername, const std::string& newPassword, User::Role newRole);
    std::optional<User> getUser(const std::string& username);
    std::optional<File> getFile(const std::string& fileType);
    bool updateFilePath(const std::string& fileType, const std::string& newPath);


private:
    Wt::Dbo::Session session_;
    std::unique_ptr<Wt::Dbo::SqlConnection> connection_;
};

#endif // DATABASE_H

