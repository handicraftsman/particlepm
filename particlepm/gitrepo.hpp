#pragma once

#include "includes.hpp"
#include "declarations.hpp"
#include "dependency.hpp"
#include "selector.hpp"

namespace PPM {

  class GitRepo : public Dependency {
  public:
    GitRepo(const std::string& name_, const std::string& url_);
    virtual ~GitRepo();

    GitRepo& operator=(const GitRepo& other);

    std::string name() const;
    std::string url() const;
    std::string branch() const;

    virtual void fetch();
    virtual std::string flags(const std::string& target);
    virtual std::string dir();
    virtual std::string name();
    virtual std::optional<PackagePtr> pkg();

    std::vector<PPM::Selector> selectors;

  
    bool is_fetched;
    PackagePtr pkg_;
    std::string name_;
    std::string url_;
    std::string branch_;
  };

}