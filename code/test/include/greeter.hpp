#ifndef GREETER_HPP
#define GREETER_HPP

#include <string>

class Greeter {
  public:
    explicit Greeter(std::string name);
    std::string greet() const;

  private:
    std::string name_;
};

#endif // GREETER_HPP
