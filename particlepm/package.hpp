#pragma once

#include "includes.hpp"
#include "declarations.hpp"

namespace PPM {

  class Package {
  public:
    Package();

    static PackagePtr from_path(const std::string& cpppath, const std::string& pkg_dir, size_t hash = 0);

    //void build();

    GitHubPtr github_repo(const std::string& user, const std::string& repo, const std::vector<Selector>& selectors = std::vector<PPM::Selector> {});
    GitRepoPtr git_repo(const std::string& name, const std::string& url, const std::vector<Selector>& selectors = std::vector<PPM::Selector> {});
    std::string pkgconfig(const std::string& name);

    std::string name();
    void name(const std::string& val);

    Version version();
    void version(const Version& val);

    std::string description();
    void description(const std::string& val);

    GitHub github();
    void github(const GitHub& val);

    License license();
    void license(const License& val);

    std::string dir();
    void dir(const std::string& val);

    std::string pkg_dir();
    void pkg_dir(const std::string& val);

    size_t hash();
    void hash(const size_t val);

    void build();

    TargetPtr executable(const std::string& name);
    TargetPtr library(const std::string& name);

    static std::map<size_t, DependencyPtr> deps;
    std::map<std::string, TargetPtr> targets;
    bool marked;
  
    std::shared_ptr<void> handle;
    PkgConfigPtr pkgconfig_;

    std::string name_;
    Version version_;
    std::string description_;
    GitHub github_;
    License license_;
    std::string dir_;
    std::string pkg_dir_;
    size_t hash_;
  };

}