#ifndef FILE_H
#define FILE_H

#include <string>
#include <Wt/Dbo/Dbo.h>

class File {
public:
    std::string path;
    std::string type;
    std::string description;

    template<class Action>
    void persist(Action& a)
    {
        Wt::Dbo::id(a, path, "path");  // Définit 'path' comme identifiant naturel
        Wt::Dbo::field(a, type, "type");
        Wt::Dbo::field(a, description, "description");
    }
};

namespace Wt {
  namespace Dbo {
    template<>
    struct dbo_traits<File> : public dbo_default_traits {
      typedef std::string IdType;
      static IdType invalidId() { return std::string(); }
      static const char *surrogateIdField() { return nullptr; }
    };
  }
}

#endif // FILE_H

