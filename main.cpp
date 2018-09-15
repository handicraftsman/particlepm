#include <cstdio>
#include <cstdlib>

#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <stdexcept>

extern "C" {
  #include <unistd.h>
  #include <sys/stat.h>
}

#include <particlepm.hpp>

#include <boost/program_options.hpp>
#include <boost/range/algorithm_ext/erase.hpp>

namespace po = boost::program_options;

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

static void cp_libs(PPM::PackagePtr libs) {
  PPM::Utils::mkdir(PPM::Utils::to_path(std::vector<std::string> { "dist" }));

  for (std::string lib : PPM::libs) {
    PPM::Utils::ExecStatus st = PPM::Utils::exec("echo \"$(ldconfig -p | grep lib" + lib + ".so | tr ' ' '\\n' | grep /)\"");
    if (st.code != 0) {
      std::cerr << st.data << std::endl;
      exit(1);
    }
    std::stringstream ss(st.data);
    std::string path;
    ss >> path;
    boost::remove_erase(path, '\r');
    boost::remove_erase(path, '\n');
    std::string to = PPM::Utils::to_path(std::vector<std::string> { "dist", path.substr(path.find_last_of("/\\") + 1) });

    auto do_it = [&] () {
      st = PPM::Utils::exec("cp " + path + " " + to);
      if (st.code != 0) {
        std::cerr << st.data << std::endl;
        exit(1);
      }
      st = PPM::Utils::exec("patchelf --set-rpath '$ORIGIN' " + to);
      if (st.code != 0) {
        std::cerr << st.data << std::endl;
        exit(1);
      }
    };

    if (::access(to.c_str(), 0) == 0) {
      struct stat a, b;
      ::stat(path.c_str(), &a);
      ::stat(to.c_str(), &b);
      if (a.st_mtime > b.st_mtime) {
        do_it();
      }
    } else {
      do_it();
    }
  }
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
 
  po::options_description global("Global options");
  global.add_options()
    ("help", "show this message")
    ("command", po::value<std::string>(), "command to execute")
    ("subargs", po::value<std::vector<std::string>>(), "arguments for command");

  po::positional_options_description pos;
  pos
    .add("command", 1)
    .add("subargs", -1);

  po::variables_map vm;

  po::parsed_options parsed = po::command_line_parser(argc, argv)
    .options(global)
    .positional(pos)
    .allow_unregistered()
    .run();

  po::store(parsed, vm);

  auto print_help = [&] () {
    std::cerr << argv[0] << " <subcommand> [...subargs]" << std::endl << std::endl;

    std::cerr << global << std::endl;

    std::cerr << "Available subcommands: " << std::endl;
    std::cerr << "  build     - build all packages in the workspace" << std::endl;
    std::cerr << "  build-dev - build all packages in debug mode" << std::endl;
    std::cerr << "  cp-libs   - copy all libraries specified in packages into the dist directory" << std::endl;
    std::cerr << "  fetch     - fetch all packages specified in your manifest recursively" << std::endl;

    exit(2);
  };

  if (vm.count("help")) {
    print_help();
  }

  if (!vm.count("command")) {
    print_help();
  }

  std::string cmd = vm["command"].as<std::string>();

  if (cmd == "build") {
    build(PPM::Package::from_path("./package.cpp", "./"));
  } else if (cmd == "build-dev") {
    PPM::dev = true;
    build(PPM::Package::from_path("./package.cpp", "./"));
  } else if (cmd == "cp-libs") {
    cp_libs(PPM::Package::from_path("./package.cpp", "./"));
  } else if (cmd == "fetch") {
    fetch(PPM::Package::from_path("./package.cpp", "./"));
  } else {
    throw std::runtime_error("Unknown command: " + cmd);
  }


}