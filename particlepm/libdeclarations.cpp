#include "libincludes.hpp"

#include "libincludes.hpp"
#include "libdeclarations.hpp"

bool PPM::dev = false;
int PPM::fetched = 0;
std::string PPM::dist_dir = PPM::Utils::to_path(std::vector<std::string>{ "dist" });
std::set<std::string> PPM::libs;

std::string PPM_CC = "cc";
std::string PPM_CXX = "c++";
std::string PPM_LD = "ld";

