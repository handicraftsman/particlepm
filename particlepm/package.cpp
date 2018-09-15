#include "libincludes.hpp"

#include "package.hpp"

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
    PPM::Utils::ExecStatus st = PPM::Utils::exec(PPM_CXX + " " + PPM::envflags + " -std=c++17 -fPIC -g -I. -L. -lparticlepm -Wl,-rpath=./ -fPIC -shared " + cpppath + " -o " + opath);
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
  PPM::TargetPtr t(new PPM::Target(name, pkg_dir(), PPM::Target::Type::Executable));
  targets[name] = t;
  return t;
}

PPM::TargetPtr PPM::Package::library(const std::string& name) {
  PPM::TargetPtr t(new PPM::Target(name, pkg_dir(), PPM::Target::Type::Library));
  targets[name] = t;
  return t;
}