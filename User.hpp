#ifndef USER_H
#define USER_H

#include <string>
#include <Wt/Dbo/Dbo.h>

class User {
public:
    enum class Role {
        Normal,
        Admin
    };

    std::string username;
    std::string passwordHash;
    Role role;
    std::string profilePicture;

    template<class Action>
    void persist(Action& a) {
        Wt::Dbo::id(a, username, "username");
        Wt::Dbo::field(a, passwordHash, "password_hash");
        Wt::Dbo::field(a, role, "role");
        Wt::Dbo::field(a, profilePicture, "profile_picture");
    }
};

namespace Wt {
  namespace Dbo {
    template<>
    struct dbo_traits<User> : public dbo_default_traits {
      typedef std::string IdType;
      static IdType invalidId() { return std::string(); }
      static const char *surrogateIdField() { return nullptr; }
    };
  }
}

#endif // USER_H

