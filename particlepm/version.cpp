#include "libincludes.hpp"

#include "version.hpp"

PPM::Version::Version(const uint32_t vmajor_, const uint32_t vminor_, const uint32_t vpatch_)
: vmajor_(vmajor_)
, vminor_(vminor_)
, vpatch_(vpatch_)
{}

PPM::Version::Version(const std::string& vs) {
  static std::regex rgx("^v?(\\d+)\\.(\\d+)\\.(\\d+)$");

  std::smatch m {};
  
  if (std::regex_match(vs, m, rgx)) {
    vmajor_ = std::stoi(m[1]);
    vminor_ = std::stoi(m[2]);
    vpatch_ = std::stoi(m[3]);
  } else {
    throw std::runtime_error("unable to parse version string: " + vs);
  }
}

PPM::Version& PPM::Version::operator=(const PPM::Version& other) {
  if (this != &other) {
    vmajor_ = other.vmajor();
    vminor_ = other.vminor();
    vpatch_ = other.vpatch();
  }
  return *this;
}

uint32_t PPM::Version::vmajor() const { return vmajor_; }
uint32_t PPM::Version::vminor() const { return vminor_; }
uint32_t PPM::Version::vpatch() const { return vpatch_; }

bool PPM::Utils::version_sorter(const PPM::Version& a, const PPM::Version& b) {
  int v1 = (a.vmajor() * 1000 * 1000) + (a.vminor() * 1000) + a.vpatch();
  int v2 = (b.vmajor() * 1000 * 1000) + (b.vminor() * 1000) + b.vpatch();
  return v1 > v2;
}
