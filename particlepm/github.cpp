#include "libincludes.hpp"

#include "github.hpp"
#include "version.hpp"

#include <algorithm>
#include <sstream>

extern "C" {
  #include <unistd.h>
}

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
  PPM::Utils::exec("git submodule update --init --recursive");
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