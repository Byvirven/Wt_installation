// Database.cpp
#include "Database.hpp"

Database::Database(const std::string& dbFile)
{
    connection_ = std::make_unique<Wt::Dbo::backend::Sqlite3>(dbFile);
    session_.setConnection(std::move(connection_));
    session_.mapClass<User>("user");
    session_.mapClass<File>("file");
}

bool Database::authenticateUser(const std::string& username, const std::string& password, User::Role& role) {
    Wt::Dbo::Transaction transaction(session_);
    Wt::Dbo::ptr<User> user = session_.find<User>().where("username = ?").bind(username);
    if (user) {
        Wt::Auth::PasswordVerifier verifier;
        verifier.addHashFunction(std::make_unique<Wt::Auth::BCryptHashFunction>());

        Wt::Auth::PasswordHash storedHash("bcrypt", "", user->passwordHash);

        if (verifier.verify(password, storedHash)) {
            role = user->role;
            return true;
        }
    }
    return false;
}

std::vector<std::string> Database::getHDF5Files() {
    std::vector<std::string> files;
    Wt::Dbo::Transaction transaction(session_);
    Wt::Dbo::collection<Wt::Dbo::ptr<File>> fileCollection = session_.find<File>();
    for (const auto& filePtr : fileCollection) {
        files.push_back(filePtr->path);
    }
    return files;
}

std::vector<User> Database::getAllUsers() {
    std::vector<User> users;
    Wt::Dbo::Transaction transaction(session_);
    Wt::Dbo::collection<Wt::Dbo::ptr<User>> userCollection = session_.find<User>();
    for (const auto& userPtr : userCollection) {
        users.push_back(*userPtr);
    }
    return users;
}

bool Database::addUser(const std::string& username, const std::string& password, User::Role role) {
    try {
        Wt::Dbo::Transaction transaction(session_);
        auto user = std::make_unique<User>();
        user->username = username;
        Wt::Auth::BCryptHashFunction bcrypt;
        user->passwordHash = bcrypt.compute(password, "");
        user->role = role;
        session_.add(std::move(user));
        transaction.commit();
        return true;
    } catch (const std::exception& e) {
        // Log the error
        return false;
    }
}

bool Database::deleteUser(const std::string& username) {
    try {
        Wt::Dbo::Transaction transaction(session_);
        Wt::Dbo::ptr<User> user = session_.find<User>().where("username = ?").bind(username);
        if (user) {
            user.remove();
            transaction.commit();
            return true;
        }
        return false;
    } catch (const std::exception& e) {
        // Log the error
        return false;
    }
}

std::vector<File> Database::getAllFiles() {
    std::vector<File> files;
    Wt::Dbo::Transaction transaction(session_);
    Wt::Dbo::collection<Wt::Dbo::ptr<File>> fileCollection = session_.find<File>();
    for (const auto& filePtr : fileCollection) {
        files.push_back(*filePtr);
    }
    return files;
}

bool Database::addFile(const std::string& path) {
    try {
        Wt::Dbo::Transaction transaction(session_);
        auto file = std::make_unique<File>();
        file->path = path;
        session_.add(std::move(file));
        transaction.commit();
        return true;
    } catch (const std::exception& e) {
        // Log the error
        return false;
    }
}

bool Database::deleteFile(const std::string& path) {
    try {
        Wt::Dbo::Transaction transaction(session_);
        Wt::Dbo::ptr<File> file = session_.find<File>().where("path = ?").bind(path);
        if (file) {
            file.remove();
            transaction.commit();
            return true;
        }
        return false;
    } catch (const std::exception& e) {
        // Log the error
        return false;
    }
}

bool Database::updateUserProfile(const std::string& oldUsername, const std::string& newUsername, const std::string& newPassword, User::Role newRole) {
    try {
        Wt::Dbo::Transaction transaction(session_);

        // Vérifiez si l'utilisateur existe
        Wt::Dbo::ptr<User> user = session_.find<User>().where("username = ?").bind(oldUsername);
        if (!user) {
            std::cerr << "Utilisateur non trouvé : " << oldUsername << std::endl;
            return false;
        }

        // Vérifiez si le nouveau nom d'utilisateur est déjà utilisé
        if (oldUsername != newUsername) {
            Wt::Dbo::ptr<User> existingUser = session_.find<User>().where("username = ?").bind(newUsername);
            if (existingUser) {
                std::cerr << "Le nouveau nom d'utilisateur existe déjà : " << newUsername << std::endl;
                return false;
            }
        }

        // Mettez à jour le nom d'utilisateur
        user.modify()->username = newUsername;

        // Mettez à jour le mot de passe si nécessaire
        if (!newPassword.empty()) {
            Wt::Auth::BCryptHashFunction bcrypt;
            user.modify()->passwordHash = bcrypt.compute(newPassword, "");
        }

        // Mettez à jour le rôle
        user.modify()->role = newRole;

        transaction.commit();
        std::cout << "Mise à jour réussie pour l'utilisateur : " << newUsername << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Erreur lors de la mise à jour du profil : " << e.what() << std::endl;
        return false;
    }
}


std::optional<User> Database::getUser(const std::string& username) {
    try {
        Wt::Dbo::Transaction transaction(session_);
        Wt::Dbo::ptr<User> user = session_.find<User>().where("username = ?").bind(username);
        if (user) {
            return *user;
        }
        return std::nullopt;
    } catch (const std::exception& e) {
        std::cerr << "Erreur lors de la récupération de l'utilisateur : " << e.what() << std::endl;
        return std::nullopt;
    }
}

std::optional<File> Database::getFile(const std::string& fileType) {
    try {
        Wt::Dbo::Transaction transaction(session_);
        Wt::Dbo::ptr<File> file = session_.find<File>().where("type = ?").bind(fileType);
        if (file) {
            return *file;
        }
        return std::nullopt;
    } catch (const std::exception& e) {
        std::cerr << "Erreur lors de la récupération du fichier : " << e.what() << std::endl;
        return std::nullopt;
    }
}

bool Database::updateFilePath(const std::string& fileType, const std::string& newPath) {
    try {
        Wt::Dbo::Transaction transaction(session_);
        Wt::Dbo::ptr<File> file = session_.find<File>().where("type = ?").bind(fileType);
        if (file) {
            file.modify()->path = newPath;
            transaction.commit();
            return true;
        }
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Erreur lors de la mise à jour du chemin du fichier : " << e.what() << std::endl;
        return false;
    }
}


