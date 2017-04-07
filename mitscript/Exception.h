#include <string>
#include <exception>

class SystemException : public exception {
private:
  string _msg;

public:
  SystemException(const string& msg) : _msg(msg) {}

  virtual string description() const = 0;

  virtual const char* what() const throw() {
    return (new string(description() + ": " + _msg))->c_str();
  }
};
