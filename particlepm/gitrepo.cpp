#include "libincludes.hpp"

#include "gitrepo.hpp"
#include "version.hpp"

#include <algorithm>
#include <sstream>

extern "C" {
  #include <unistd.h>
}

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
  PPM::Utils::exec("git submodule update --recursive --remote");
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