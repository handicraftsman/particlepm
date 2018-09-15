#pragma once

#include "includes.hpp"
#include "declarations.hpp"

namespace PPM {

  class Version {
  public:
    Version(const uint32_t vmajor, const uint32_t vminor, const uint32_t vpatch);
    Version(const std::string& vs);
    Version& operator=(const Version& other);

    uint32_t vmajor() const;
    uint32_t vminor() const;
    uint32_t vpatch() const;

  
    uint32_t vmajor_;
    uint32_t vminor_;
    uint32_t vpatch_;
  };

  namespace Utils {
    bool version_sorter(const Version& a, const Version& b);
  }

}
