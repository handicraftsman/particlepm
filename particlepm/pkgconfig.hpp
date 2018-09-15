#pragma once

#include "includes.hpp"
#include "declarations.hpp"
#include "dependency.hpp"

namespace PPM {

  class PkgConfig : public Dependency {
  public:
    PkgConfig();
    virtual ~PkgConfig();

    virtual void fetch();
    virtual std::string flags(const std::string& target);
    virtual std::string dir();
    virtual std::string name();
    virtual std::optional<PackagePtr> pkg();
  };

}