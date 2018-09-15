#pragma once

#define PPM_PACKAGE(p) extern "C" void ppm_package(PPM::PackagePtr p)

extern std::string PPM_CC;
extern std::string PPM_CXX;
extern std::string PPM_LD;

namespace PPM {

  extern bool dev;
  extern int fetched;
  extern std::string dist_dir;
  extern std::set<std::string> libs;

  /*
   * Declarations
   */

  class Version;
  class License;
  class Dependency;
  class GitHub;
  class GitRepo;
  class PkgConfig;
  class Package;
  class Target;
  class File;

  typedef std::shared_ptr<Package> PackagePtr;
  typedef std::shared_ptr<Target> TargetPtr;
  typedef std::shared_ptr<Dependency> DependencyPtr;
  typedef std::shared_ptr<GitHub> GitHubPtr;
  typedef std::shared_ptr<GitRepo> GitRepoPtr;
  typedef std::shared_ptr<PkgConfig> PkgConfigPtr;
  typedef std::shared_ptr<Target> TargetPtr;
  typedef std::shared_ptr<File> FilePtr;

}

typedef void (*PPMPackageConfigurator)(PPM::PackagePtr p);