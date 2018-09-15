#pragma once

#include "includes.hpp"
#include "declarations.hpp"

namespace PPM {

  class Dependency {
  public:
    virtual ~Dependency();
    virtual void fetch() = 0;
    virtual std::string flags(const std::string& target) = 0;
    virtual std::string dir() = 0;
    virtual std::string name() = 0;
    virtual std::optional<PackagePtr> pkg() = 0;
  };

}