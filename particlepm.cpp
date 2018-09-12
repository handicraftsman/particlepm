#include <particlepm.hpp>

#include <cstdio>

#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>
#include <iterator>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

extern "C" {
  #include <dlfcn.h>
  #include <getopt.h>
  #include <unistd.h>
  #include <sys/types.h>
  #include <sys/stat.h>
}

#define PPM_MKDIR_FLAGS S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH

using namespace PPM::Utils;

bool PPM::dev = false;
int PPM::fetched = 0;
std::string PPM::dist_dir = PPM::Utils::to_path(std::vector<std::string>{ "dist" });

std::string PPM_CC = "cc";
std::string PPM_CXX = "c++";
std::string PPM_LD = "ld";

/*
 * Version
 */

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

static bool version_sorter(const PPM::Version& a, const PPM::Version& b) {
  int v1 = (a.vmajor() * 1000 * 1000) + (a.vminor() * 1000) + a.vpatch();
  int v2 = (b.vmajor() * 1000 * 1000) + (b.vminor() * 1000) + b.vpatch();
  return v1 > v2;
}

/*
 * Selector
 */

PPM::Selector::Selector()
: ver(0, 1, 0)
{}

PPM::Selector PPM::Selector::exact(const PPM::Version& ver) {
  PPM::Selector sel;
  sel.type = PPM::Selector::Type::Exact;
  sel.ver = ver;
  return sel;
}

PPM::Selector PPM::Selector::same(const PPM::Version& ver) {
  PPM::Selector sel;
  sel.type = PPM::Selector::Type::Same;
  sel.ver = ver;
  return sel;
}

PPM::Selector PPM::Selector::greater(const PPM::Version& ver) {
  PPM::Selector sel;
  sel.type = PPM::Selector::Type::Greater;
  sel.ver = ver;
  return sel;
}

PPM::Selector PPM::Selector::greatere(const PPM::Version& ver) {
  PPM::Selector sel;
  sel.type = PPM::Selector::Type::GreaterE;
  sel.ver = ver;
  return sel;
}

PPM::Selector PPM::Selector::less(const PPM::Version& ver) {
  PPM::Selector sel;
  sel.type = PPM::Selector::Type::Less;
  sel.ver = ver;
  return sel;
}

PPM::Selector PPM::Selector::lesse(const PPM::Version& ver) {
  PPM::Selector sel;
  sel.type = PPM::Selector::Type::LessE;
  sel.ver = ver;
  return sel;
}

std::vector<PPM::Version> PPM::Selector::apply(const std::vector<PPM::Version>& versions, const std::vector<PPM::Selector>& selectors) {
  std::vector<PPM::Version> vec(versions);
  for (PPM::Selector sel : selectors) {
    auto it = std::remove_if(vec.begin(), vec.end(), [&sel] (const PPM::Version& ver) {
      if (sel.type == PPM::Selector::Type::Exact) {
        bool b = !((sel.ver.vmajor() == ver.vmajor()) && (sel.ver.vminor() == ver.vminor()) && (sel.ver.vpatch() == ver.vpatch()));
        return b;
      } else if (sel.type == PPM::Selector::Type::Same) {
        bool b = !((sel.ver.vmajor() == ver.vmajor()) && (sel.ver.vminor() == ver.vminor()));
        return b;
      } else if (sel.type == PPM::Selector::Type::Greater) {
        int v1 = (ver.vmajor() * 1000 * 1000) + (ver.vminor() * 1000) + ver.vpatch();
        int v2 = (sel.ver.vmajor() * 1000 * 1000) + (sel.ver.vminor() * 1000) + sel.ver.vpatch();
        bool b = !(v1 > v2);
        return b;
      } else if (sel.type == PPM::Selector::Type::GreaterE) {
        int v1 = (ver.vmajor() * 1000 * 1000) + (ver.vminor() * 1000) + ver.vpatch();
        int v2 = (sel.ver.vmajor() * 1000 * 1000) + (sel.ver.vminor() * 1000) + sel.ver.vpatch();
        bool b = !(v1 >= v2);
        return b;
      } else if (sel.type == PPM::Selector::Type::Less) {
        int v1 = (ver.vmajor() * 1000 * 1000) + (ver.vminor() * 1000) + ver.vpatch();
        int v2 = (sel.ver.vmajor() * 1000 * 1000) + (sel.ver.vminor() * 1000) + sel.ver.vpatch();
        bool b = !(v1 < v2);
        return b;
      } else if (sel.type == PPM::Selector::Type::LessE) {
        int v1 = (ver.vmajor() * 1000 * 1000) + (ver.vminor() * 1000) + ver.vpatch();
        int v2 = (sel.ver.vmajor() * 1000 * 1000) + (sel.ver.vminor() * 1000) + sel.ver.vpatch();
        bool b = !(v1 <= v2);
        return b;
      } else {
        std::cerr << "Warning: unknown selector. Ignoring" << std::endl;
        return false;
      }
    });
    vec.erase(it, vec.end());
  }
  return vec;
}

/*
 * License
 */

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

/*
 * Dependency
 */

PPM::Dependency::~Dependency() {}

/*
 * GitHub
 */

PPM::GitHub::GitHub(const std::string& user_, const std::string& repo_)
: user_(user_)
, repo_(repo_)
, branch_("master")
, is_fetched(false)
{}

PPM::GitHub::~GitHub() {}

PPM::GitHub& PPM::GitHub::operator=(const PPM::GitHub& other) {
  if (this != &other) {
    user_ = other.user();
    repo_ = other.repo();
    branch_ = other.branch();
  }
  return *this;
}

std::string PPM::GitHub::user() const { return user_; }
std::string PPM::GitHub::repo() const { return repo_; }
std::string PPM::GitHub::branch() const { return branch_; }

static void gh_clone(PPM::GitHub* dep, std::string p, std::string d) {
  PPM::Utils::exec("git clone https://github.com/" + dep->user() + "/" + dep->repo() + " -b " + dep->branch() + " .");
}

static void gh_pull(PPM::GitHub* dep, std::string p, std::string d) {
  PPM::Utils::exec("git pull");
}

void PPM::GitHub::fetch() {
  if (is_fetched) return;
  is_fetched = true;
  ++PPM::fetched;

  std::string d = dir();
  char* p_ = getcwd(NULL, 1024);
  std::string p(p_);
  free(p_);

  PPM::Utils::mkdir(d);
  PPM::Utils::chdir(d);
  if (access(d.c_str(), 0) == 0) {
    if (access((d + "/.git").c_str(), 0) != 0) {
      gh_clone(this, p, d);
    } else {
      gh_pull(this, p, d);
    }
  } else {
    gh_clone(this, p, d);
  }

  std::string b("master");

  PPM::Utils::ExecStatus st = PPM::Utils::exec("git tag -l v*");
  if (st.code != 0) {
    throw std::runtime_error("Unable to list tags");
  }
  std::stringstream ss(st.data);

  std::vector<PPM::Version> versions;

  std::string vs;
  while (ss >> vs) {
    PPM::Version v(vs);
    versions.push_back(v);
  }

  versions = PPM::Selector::apply(versions, selectors);

  std::sort(versions.begin(), versions.end(), version_sorter);

  if (versions.empty()) {
    throw std::runtime_error("Unable to find a matching version for github repo " + user() + "/" + repo());
  }

  PPM::Version v = versions[0];
  b = "v" + std::to_string(v.vmajor()) + "." + std::to_string(v.vminor()) + "." + std::to_string(v.vpatch());

  PPM::Utils::exec("git checkout " + b);
  PPM::Utils::chdir(p);

  std::string ppath = dir() + "/package.cpp";
  std::cerr << ppath << std::endl;
  
  std::hash<std::string> hasher;
  size_t hash = hasher("github-" + user_ + "-" + repo_);

  std::ofstream f(PPM::Utils::to_path(std::vector<std::string>{ ".particlepm", "packages.txt" }), std::ios_base::app);
  f << "github-" << std::to_string(hash) << std::endl;
  f.close();

  pkg_ = PPM::Package::from_path(ppath, dir(), hash);
}

std::string PPM::GitHub::flags(const std::string& target) {
  return "";
}

std::string PPM::GitHub::dir() {
  std::hash<std::string> hasher;
  size_t hash = hasher("github-" + user_ + "-" + repo_);
  return PPM::Utils::to_path(std::vector<std::string>{ ".particlepm", ("github-" + std::to_string(hash)) });
}

std::string PPM::GitHub::name() {
  std::hash<std::string> hasher;
  size_t hash = hasher("github-" + user_ + "-" + repo_);
  return ("github-" + std::to_string(hash));
}

std::optional<PPM::PackagePtr> PPM::GitHub::pkg() {
  return pkg_;
}

/*
 * GitRepo
 */

PPM::GitRepo::GitRepo(const std::string& name_, const std::string& url_)
: name_(name_)
, url_(url_)
, branch_("master")
, is_fetched(false)
{}

PPM::GitRepo::~GitRepo() {}

PPM::GitRepo& PPM::GitRepo::operator=(const PPM::GitRepo& other) {
  if (this != &other) {
    name_ = other.name();
    url_ = other.url();
    branch_ = other.branch();
  }
  return *this;
}

std::string PPM::GitRepo::name() const { return name_; }
std::string PPM::GitRepo::url() const { return url_; }
std::string PPM::GitRepo::branch() const { return branch_; }

static void g_clone(PPM::GitRepo* dep, std::string p, std::string d) {
  PPM::Utils::exec("git clone " + dep->url() + " -b " + dep->branch() + " .");
}

static void g_pull(PPM::GitRepo* dep, std::string p, std::string d) {
  PPM::Utils::exec("git pull");
}

void PPM::GitRepo::fetch() {
  if (is_fetched) return;
  is_fetched = true;
  ++PPM::fetched;

  std::string d = dir();
  char* p_ = getcwd(NULL, 1024);
  std::string p(p_);
  free(p_);

  PPM::Utils::mkdir(d);
  PPM::Utils::chdir(d);
  if (access(d.c_str(), 0) == 0) {
    if (access((d + "/.git").c_str(), 0) != 0) {
      g_clone(this, p, d);
    } else {
      g_pull(this, p, d);
    }
  } else {
    g_clone(this, p, d);
  }

  std::string b("master");

  PPM::Utils::ExecStatus st = PPM::Utils::exec("git tag -l v*");
  if (st.code != 0) {
    throw std::runtime_error("Unable to list tags");
  }
  std::stringstream ss(st.data);

  std::vector<PPM::Version> versions;

  std::string vs;
  while (ss >> vs) {
    PPM::Version v(vs);
    versions.push_back(v);
  }

  versions = PPM::Selector::apply(versions, selectors);

  std::sort(versions.begin(), versions.end(), version_sorter);

  if (versions.empty()) {
    throw std::runtime_error("Unable to find a matching version for git repo " + name() + " (" + url() + ")");
  }

  PPM::Version v = versions[0];
  b = "v" + std::to_string(v.vmajor()) + "." + std::to_string(v.vminor()) + "." + std::to_string(v.vpatch());

  PPM::Utils::exec("git checkout " + b);
  PPM::Utils::chdir(p);

  std::string ppath = dir() + "/package.cpp";
  std::cerr << ppath << std::endl;
  
  std::hash<std::string> hasher;
  size_t hash = hasher("git-" + url_);

  std::ofstream f(PPM::Utils::to_path(std::vector<std::string>{ ".particlepm", "packages.txt" }), std::ios_base::app);
  f << "git-" << std::to_string(hash) << std::endl;
  f.close();

  pkg_ = PPM::Package::from_path(ppath, dir(), hash);
}

std::string PPM::GitRepo::flags(const std::string& target) {
  return "";
}

std::string PPM::GitRepo::dir() {
  std::hash<std::string> hasher;
  size_t hash = hasher("git-" + url_);
  return PPM::Utils::to_path(std::vector<std::string>{ ".particlepm", ("git-" + std::to_string(hash)) });
}

std::string PPM::GitRepo::name() {
  std::hash<std::string> hasher;
  size_t hash = hasher("git-" + url_);
  return ("git-" + std::to_string(hash));
}

std::optional<PPM::PackagePtr> PPM::GitRepo::pkg() {
  return pkg_;
}

/*
 * PkgConfig
 */

PPM::PkgConfig::PkgConfig() {}

PPM::PkgConfig::~PkgConfig() {}

void PPM::PkgConfig::fetch() {}

std::string PPM::PkgConfig::flags(const std::string& target) {
  PPM::Utils::ExecStatus st = PPM::Utils::exec("pkg-config --libs --cflags " + target);
  if (st.code != 0) {
    throw std::runtime_error("Unable to find `" + target + "` pkg-config package!");
  }
  st.data.erase(std::remove(st.data.begin(), st.data.end(), '\r'), st.data.end());
  st.data.erase(std::remove(st.data.begin(), st.data.end(), '\n'), st.data.end());
  return st.data;
}

std::string PPM::PkgConfig::dir() {
  return PPM::Utils::to_path(std::vector<std::string>{ ".particlepm", "pkgconfig" });
}

std::string PPM::PkgConfig::name() {
  return "pkg-config";
}

std::optional<PPM::PackagePtr> PPM::PkgConfig::pkg() {
  return std::nullopt;
}

/*
 * Package
 */

std::map<size_t, PPM::DependencyPtr> PPM::Package::deps;

PPM::Package::Package()
: marked(false)
, pkgconfig_(new PkgConfig())
, name_("unnamed")
, version_(0, 1, 0)
, description_("")
, github_(PPM::GitHub("", ""))
, license_("UNLICENSED", "")
{}

PPM::PackagePtr PPM::Package::from_path(const std::string& cpppath, const std::string& pkg_dir, size_t hash) {
  PPM::PackagePtr p(new PPM::Package());
  p->pkg_dir(pkg_dir);

  std::string dir = PPM::Utils::to_path(std::vector<std::string>{ ".particlepm" });
  if (access(dir.c_str(), 0) != 0) {
    PPM::Utils::mkdir(dir.c_str());
  }
  p->dir(dir);

  if (hash == 0) {
    std::hash<std::string> hasher;
    hash = hasher(dir);
  }
  p->hash(hash);

  std::string opath = PPM::Utils::to_path(std::vector<std::string>{ ".particlepm", "pkg-" + std::to_string(hash) + ".so" });

  auto build = [&] () {
    PPM::Utils::ExecStatus st = PPM::Utils::exec(PPM_CXX + " -std=c++17 -fPIC -g -I. -L. -lparticlepm -Wl,-rpath=./ -fPIC -shared " + cpppath + " -o " + opath);
    if (st.code != 0) {
      std::cerr << st.data;
      throw std::runtime_error("Unable to compile package file! (" + cpppath + ")");
    }
  };
  if (::access(opath.c_str(), 0) == 0) {
    struct stat a, b;
    ::stat(cpppath.c_str(), &a);
    ::stat(opath.c_str(), &b);
    if (a.st_mtime > b.st_mtime) {
      build();
    }
  } else {
    build();
  }

  void* h = dlopen(opath.c_str(), RTLD_LAZY);
  if (!h) {
    throw std::runtime_error(std::string(dlerror()));
  }

  p->handle = std::shared_ptr<void>(h, dlclose);

  PPMPackageConfigurator c = (PPMPackageConfigurator) dlsym(p->handle.get(), "ppm_package");
  c(p);
  char* err;
  if ((err = dlerror()) != NULL) {
    throw std::runtime_error(std::string(err));
  }
  std::cerr << "Package: " << p->name() << " v" << p->version().vmajor() << "." << p->version().vminor() << "." << p->version().vpatch() << std::endl;

  return p;
}

PPM::GitHubPtr PPM::Package::github_repo(
  const std::string& user,
  const std::string& repo,
  const std::vector<PPM::Selector>& selectors
) {
  std::hash<std::string> hasher;
  size_t hash = hasher("github-" + user + "-" + repo);

  if (deps.find(hash) != deps.end()) {
    std::cerr << "Found github repository " << user << "/" << repo << std::endl;
    auto p = std::dynamic_pointer_cast<PPM::GitHub>(deps[hash]);
    p->selectors = p->selectors + selectors;
    return p;
  }

  PPM::GitHubPtr p(new PPM::GitHub(user, repo));
  p->selectors = p->selectors + selectors;
  deps[hash] = p;
  std::cerr << "Added github repository " << user << "/" << repo << std::endl;
  return p;
}

PPM::GitRepoPtr PPM::Package::git_repo(
  const std::string& name,
  const std::string& url,
  const std::vector<PPM::Selector>& selectors
) {
  std::hash<std::string> hasher;
  size_t hash = hasher("git-" + url);

  if (deps.find(hash) != deps.end()) {
    std::cerr << "Found git repository " << name << " (" << url << ")" << std::endl;
    auto p = std::dynamic_pointer_cast<PPM::GitRepo>(deps[hash]);
    p->selectors = p->selectors + selectors;
    return p;
  }
  
  PPM::GitRepoPtr p(new PPM::GitRepo(name, url));
  p->selectors = p->selectors + selectors;
  deps[hash] = p;
  std::cerr << "Added git repository " << name << " (" << url << ")" << std::endl;
  return p;
}

std::string PPM::Package::pkgconfig(const std::string& name) {
  std::cerr << "Requested pkg-config flags for " << name << std::endl;
  return pkgconfig_->flags(name);
}

std::string PPM::Package::name() {
  return name_;
}

void PPM::Package::name(const std::string& val) {
  name_ = val;
}

PPM::Version PPM::Package::version() {
  return version_;
}

void PPM::Package::version(const PPM::Version& val) {
  version_ = val;
}

std::string PPM::Package::description() {
  return description_;
}

void PPM::Package::description(const std::string& val) {
  description_ = val;
}

PPM::GitHub PPM::Package::github() {
  return github_;
}

void PPM::Package::github(const PPM::GitHub& val) {
  github_ = val;
}

PPM::License PPM::Package::license() {
  return license_;
}

void PPM::Package::license(const PPM::License& val) {
  license_ = val;
}

std::string PPM::Package::dir() {
  return dir_;
}

void PPM::Package::dir(const std::string& val) {
  dir_ = val;
}

std::string PPM::Package::pkg_dir() {
  return pkg_dir_;
}

void PPM::Package::pkg_dir(const std::string& val) {
  pkg_dir_ = val;
}

size_t PPM::Package::hash() {
  return hash_;
}

void PPM::Package::hash(const size_t val) {
  hash_ = val;
}

void PPM::Package::build() {
  for (std::pair<std::string, PPM::TargetPtr> tp : targets) {
    tp.second->build();
  }
}

PPM::TargetPtr PPM::Package::executable(const std::string& name) {
  PPM::TargetPtr t(new PPM::Target(name, PPM::Target::Type::Executable));
  targets[name] = t;
  return t;
}

PPM::TargetPtr PPM::Package::library(const std::string& name) {
  PPM::TargetPtr t(new PPM::Target(name, PPM::Target::Type::Library));
  targets[name] = t;
  return t;
}

/*
 * Target
 */

PPM::Target::Target(const std::string& name, PPM::Target::Type type)
: marked(false)
, has_cpp(false)
, type_(type)
, name_(name)
, c_("c11")
, cpp_("c++11")
, c_flags_("")
, cpp_flags_("")
{}

void PPM::Target::depends(const PPM::TargetPtr& other) {
  deps_.insert(other);
}

void PPM::Target::build() {
  if (marked) {
    return;
  }
  marked = true;

  for (PPM::TargetPtr dep : deps_) {
    dep->build();
  }

  std::cerr << "Building " << name() << "(" << (int) type_ << ")" << std::endl;

  for (PPM::FilePtr file : files_) {
    if (file->built) {
      continue;
    }
    file->built = true;
    auto do_build = [&] () {
      std::string flags;
      std::string std;
      if (file->compiler == PPM_CC) {
        flags = c_flags_;
        std = c_;
      } else if (file->compiler == PPM_CXX) {
        flags = cpp_flags_;
        std = cpp_;
      }
      PPM::Utils::ExecStatus st = PPM::Utils::exec(file->compiler + " " + flags + " -fPIC -std=" + std + " -c " + file->ifile + " -o " + file->ofile);
      if (st.code != 0) {
        std::cerr << st.data << std::endl;
        ::exit(st.code);
      }
    };
    if (::access(file->ofile.c_str(), 0) == 0) {
      struct stat a, b;
      ::stat(file->ifile.c_str(), &a);
      ::stat(file->ofile.c_str(), &b);
      if (a.st_mtime > b.st_mtime) {
        do_build();
      }
    } else {
      do_build();
    }
  }

  std::string compiler = has_cpp ? PPM_CXX : PPM_CC;
  PPM::Utils::mkdir(PPM::dist_dir);
  std::string out = PPM::Utils::to_path(std::vector<std::string>{ "dist", ((type_ == PPM::Target::Type::Executable) ? name() : ("lib" + name() + ".so")) }); 
  std::string filenames = "";
  std::string dbg = PPM::dev ? "-g -ggdb" : "";
  for (PPM::FilePtr file : files_) {
    filenames += (" " + file->ofile);
  }

  PPM::Utils::ExecStatus st;
  if (type_ == PPM::Target::Type::Executable) {
    st = PPM::Utils::exec(compiler + " " + dbg + " -fPIC -o " + out + " " + filenames + " " + cpp_flags_ + " " + c_flags_);
  } else {
    st = PPM::Utils::exec(compiler + " " + dbg + " -rdynamic -shared -fPIC -o " + out + " " + filenames + " " + cpp_flags_ + " " + c_flags_);
  }
  if (st.code != 0) {
    std::cerr << st.data << std::endl;
    ::exit(st.code);
  }
}

std::string PPM::Target::name() {
  return name_;
}

void PPM::Target::name(const std::string& value) {
  name_ = value;
}

std::string PPM::Target::c() {
  return c_;
}

void PPM::Target::c(const std::string& value) {
  c_ = value;
}

std::string PPM::Target::cpp() {
  return cpp_;
}

void PPM::Target::cpp(const std::string& value) {
  cpp_ = value;
}

void PPM::Target::c_files(const std::vector<std::string>& filenames) {
  std::string suffix = (PPM::dev == true ? ".dev.o" : ".o");
  for (const std::string& filename : filenames) {
    files_.insert(PPM::FilePtr(new PPM::File(filename, filename + suffix, PPM_CC)));
  }
}

void PPM::Target::cpp_files(const std::vector<std::string>& filenames) {
  std::string suffix = (PPM::dev == true ? ".dev.o" : ".o");
  has_cpp = true;
  for (const std::string& filename : filenames) {
    files_.insert(PPM::FilePtr(new PPM::File(filename, filename + suffix, PPM_CXX)));
  }
}

void PPM::Target::c_flags(const std::string& flags) {
  c_flags_ += (" " + flags);
}

void PPM::Target::cpp_flags(const std::string& flags) {
  cpp_flags_ += (" " + flags);
}

/*
 * File
 */

PPM::File::File(const std::string& ifile, const std::string& ofile, const std::string& compiler)
: built(false)
, ifile(ifile)
, ofile(ofile)
, compiler(compiler)
{}

/*
 * Utilities
 */

std::string PPM::Utils::to_path(std::vector<std::string> sections) {
  char* p = getcwd(NULL, 1024);
  std::stringstream path;
  path << p;
  for (const std::string& section : sections) {
    path << "/" << section;
  }
  free(p);
  return path.str();
}

PPM::Utils::ExecStatus PPM::Utils::exec(const std::string& cmd) {
  std::cerr << "++ " << cmd << std::endl;

  PPM::Utils::ExecStatus st;

  std::array<char, 128> buffer;
  std::shared_ptr<FILE> pipe(popen(cmd.c_str(), "r"), [&] (FILE* f) {
    st.code = WEXITSTATUS(pclose(f));
  });
  if (!pipe) throw std::runtime_error("popen() failed!");
  while (!feof(pipe.get())) {
    if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
      st.data += buffer.data();
  }

  return st;
}

void PPM::Utils::chdir(const std::string& path) {
  std::cerr << "Navigating to " << path << std::endl;
  if (::chdir(path.c_str()) == -1) {
    perror("PPM::Utils::chdir");
    exit(1);
  }
}

void PPM::Utils::mkdir(const std::string& path) {
  std::cerr << "Creating directory: " << path << std::endl;
  if (::mkdir(path.c_str(), PPM_MKDIR_FLAGS) == -1 && errno != EEXIST) {
    perror("PPM::Utils::mkdir");
    exit(1);
  }
}

template<typename Out>
static void split(const std::string& s, char delim, Out result) {
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
    *(result++) = item;
  }
}

std::vector<std::string> PPM::Utils::split(const std::string& str, char sep) {
  std::vector<std::string> elems;
  ::split(str, sep, std::back_inserter(elems));
  return elems;
}

std::string PPM::Utils::flagcat(const std::vector<std::string>& vec) {
  auto it = vec.begin();
  std::stringstream s;
  s << *it;
  for (it = ++it; it != vec.end(); ++it) {
    s << " " << *it;
  }
  return s.str();
}
