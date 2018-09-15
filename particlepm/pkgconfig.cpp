#include "libincludes.hpp"

#include "pkgconfig.hpp"

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