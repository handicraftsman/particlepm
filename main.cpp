#include <cstdio>
#include <cstdlib>

#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <stdexcept>

#include <particlepm.hpp>

std::map<std::string, PPM::PackagePtr> packages;

static void build_pkg(PPM::PackagePtr pkg) {
  if (!pkg || pkg->marked) {
    return;
  }

  pkg->marked = true;

  for (std::pair<size_t, PPM::DependencyPtr> pp : pkg->deps) {
    std::string pkgname = pp.second->name();
    if (pkgname != "pkg-config") {
      PPM::PackagePtr pkg = packages[pkgname];
      if (!pkg) {
        std::cerr << "Cannot find " << pkgname << std::endl;
        exit(1);
      }
      build_pkg(pkg);
    }
  }

  std::cout << "Going to build " << pkg->name() << std::endl;
  pkg->build();
}

static void build(PPM::PackagePtr p) {
  static std::regex rgx("^.*?-(\\d+)$");

  std::ifstream f(PPM::Utils::to_path(std::vector<std::string> { ".particlepm", "packages.txt" }));
  std::string name;
  while (f >> name) {
    std::string dir = PPM::Utils::to_path(std::vector<std::string> { ".particlepm", name });
    std::string manifest = PPM::Utils::to_path(std::vector<std::string> { ".particlepm", name, "package.cpp" });
    size_t hash = 0;

    std::smatch m {};
    if (std::regex_match(name, m, rgx)) {
      hash = std::stoull(m[1]);
    } else {
      throw std::runtime_error("Cannot understand next name in package list: " + name);
    }

    PPM::PackagePtr pkg = PPM::Package::from_path(manifest, dir, hash);
    pkg->dir(dir);

    packages[name] = pkg;
  }

  build_pkg(p);
}

static void fetch(PPM::PackagePtr p) {
  std::string ptxt = PPM::Utils::to_path(std::vector<std::string> { ".particlepm", "packages.txt" });
  ::remove(ptxt.c_str());
  do {
    PPM::fetched = 0;
    for (std::pair<size_t, PPM::DependencyPtr> p : PPM::Package::deps) {
      p.second->fetch();
    }
  } while (PPM::fetched != 0);
}

int main(int argc, char** argv) {
  char* cc = ::getenv("CC");
  if (cc != NULL) {
    PPM_CC = std::string(cc);
  }

  char* cxx = ::getenv("CXX");
  if (cxx != NULL) {
    PPM_CXX = std::string(cxx);
  }

  char* ld = ::getenv("LD");
  if (ld != NULL) {
    PPM_LD = std::string(ld);
  }

  if (argc < 2) {
    std::cerr << "Not enough arguments" << std::endl;
    exit(2);
  }
  std::string arg(argv[1]);

  if (arg == "build") {
    build(PPM::Package::from_path("./package.cpp", "./"));
  } else if (arg == "build-dev") {
    PPM::dev = true;
    build(PPM::Package::from_path("./package.cpp", "./"));
  } else if (arg == "fetch") {
    fetch(PPM::Package::from_path("./package.cpp", "./"));
  } else {
    throw std::runtime_error("Unknown command: " + arg);
  }
}