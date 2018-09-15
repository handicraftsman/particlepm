#pragma once

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

#include "libdeclarations.hpp"