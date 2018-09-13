#pragma once

#define PPM_PACKAGE(p) extern "C" void ppm_package(PPM::PackagePtr p)

#include <set>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <optional>

extern std::string PPM_CC;
extern std::string PPM_CXX;
extern std::string PPM_LD;

namespace PPM {

  extern bool dev;
  extern int fetched;
  extern std::string dist_dir;

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

  /*
   * Version
   */

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

  /*
   * Selector
   */

  class Selector {
  public: 
    enum class Type {
      Exact,
      Same,
      Greater,
      GreaterE,
      Less,
      LessE,
    };

    Selector();

    Type type;
    Version ver;

    static Selector exact(const Version& ver);
    static Selector same(const Version& ver);
    static Selector greater(const Version& ver);
    static Selector greatere(const Version& ver);
    static Selector less(const Version& ver);
    static Selector lesse(const Version& ver);

    static std::vector<Version> apply(const std::vector<Version>& versions, const std::vector<Selector>& selectors);
  };

  /*
   * License
   */

  class License {
  public:
    License(const std::string& name_, const std::string& url_);
    License& operator=(const License& other);

    std::string name() const;
    std::string url() const;

  
    std::string name_;
    std::string url_;
  };

  /*
   * Dependency
   */

  class Dependency {
  public:
    virtual ~Dependency();
    virtual void fetch() = 0;
    virtual std::string flags(const std::string& target) = 0;
    virtual std::string dir() = 0;
    virtual std::string name() = 0;
    virtual std::optional<PackagePtr> pkg() = 0;
  };

  /*
   * GitHub
   */

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

  /*
   * GitRepo
   */

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

  /*
   * PkgConfig
   */

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

  /*
   * Package
   */

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

  /*
   * Target
   */

  class Target {
  public:
    enum class Type {
      Executable,
      Library,
    };

    Target(const std::string& name, const std::string& dir, Type type);

    void depends(const TargetPtr& other);
    void build();

    std::string name();
    void name(const std::string& value);

    std::string c();
    void c(const std::string& value);

    std::string cpp();
    void cpp(const std::string& value);

    void c_files(const std::vector<std::string>& filenames);
    void cpp_files(const std::vector<std::string>& filenames);

    void c_flags(const std::string& flags);
    void cpp_flags(const std::string& flags);

    bool marked;
    bool has_cpp;

    std::set<TargetPtr> deps_;

    Type type_;
    std::string name_;
    std::string dir_;
    std::string c_;
    std::string cpp_;
    std::string c_flags_;
    std::string cpp_flags_;
    std::set<FilePtr> files_;
  };

  /*
   * File
   */

  class File {
  public:
    File(const std::string& ifile, const std::string& ofile, const std::string& compiler);

    bool built;
    std::string ifile;
    std::string ofile;
    std::string compiler;
  };

  /*
   * Utilities
   */

  namespace Utils {
    std::string to_path(std::vector<std::string> sections);

    template<typename T>
    std::vector<T> operator+(const std::vector<T>& a, const std::vector<T>& b) {
      std::vector<T> c;
      c.reserve(a.size() + b.size());
      c.insert(c.end(), a.begin(), a.end());
      c.insert(c.end(), b.begin(), b.end());
      return c;
    }

    struct ExecStatus {
      std::string data;
      int code;
    };

    ExecStatus exec(const std::string& cmd);
    void chdir(const std::string& path);
    void mkdir(const std::string& path);

    std::vector<std::string> split(const std::string& str, char sep);

    std::string flagcat(const std::vector<std::string>& strs);
  }
}

typedef void (*PPMPackageConfigurator)(PPM::PackagePtr p);
