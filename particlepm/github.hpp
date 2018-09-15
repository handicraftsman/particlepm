#pragma once

#include "includes.hpp"
#include "declarations.hpp"
#include "dependency.hpp"
#include "selector.hpp"

namespace PPM {

  class GitHub : public Dependency {
  public:
    GitHub(const std::string& user_, const std::string& repo_);
    virtual ~GitHub();

    GitHub& operator=(const GitHub& other);

    std::string user() const;
    std::string repo() const;
    std::string branch() const;

    virtual void fetch();
    virtual std::string flags(const std::string& target);
    virtual std::string dir();
    virtual std::string name();
    virtual std::optional<PackagePtr> pkg();

    std::vector<PPM::Selector> selectors;

  
    bool is_fetched;
    PackagePtr pkg_;
    std::string user_;
    std::string repo_;
    std::string branch_;
  };

}