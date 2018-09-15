#include "libincludes.hpp"

#include "license.hpp"

PPM::License::License(const std::string& name_, const std::string& url_)
: name_(name_)
, url_(url_)
{}

PPM::License& PPM::License::operator=(const PPM::License& other) {
  if (this != &other) {
    name_ = other.name();
    url_ = other.url();
  }
  return *this;
}

std::string PPM::License::name() const { return name_; }
std::string PPM::License::url() const { return url_; }