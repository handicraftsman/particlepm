#pragma once

#include "includes.hpp"
#include "declarations.hpp"

namespace PPM {

  class File {
  public:
    File(const std::string& ifile, const std::string& ofile, const std::string& compiler);

    bool built;
    std::string ifile;
    std::string ofile;
    std::string compiler;
  };
  
}