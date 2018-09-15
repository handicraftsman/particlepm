#pragma once

#include <iostream>
#include <ostream>

#include "includes.hpp"
#include "declarations.hpp"

namespace PPM {

  namespace Utils {
    std::string to_path(std::vector<std::string> sections);

    template<typename T>
    std::vector<T> operator+(const std::vector<T>& a, const std::vector<T>& b) {
      std::vector<T> c;
      c.reserve(a.size() + b.size());
      c.insert(c.end(), a.begin(), a.end());
      c.insert(c.end(), b.begin(), b.end());
      return c;
    }

    struct ExecStatus {
      std::string data;
      int code;
    };

    ExecStatus exec(const std::string& cmd);
    void chdir(const std::string& path);
    void mkdir(const std::string& path);

    std::vector<std::string> split(const std::string& str, char sep);

    std::string flagcat(const std::vector<std::string>& strs);
  }

}