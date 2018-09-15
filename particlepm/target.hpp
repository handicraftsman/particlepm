#pragma once

#include "includes.hpp"
#include "declarations.hpp"

namespace PPM {

  class Target {
  public:
    enum class Type {
      Executable,
      Library,
    };

    Target(const std::string& name, const std::string& dir, Type type);

    void depends(const TargetPtr& other);
    virtual void build();

    std::string name();
    void name(const std::string& value);

    std::string c();
    void c(const std::string& value);

    std::string cpp();
    void cpp(const std::string& value);

    void c_files(const std::vector<std::string>& filenames);
    void cpp_files(const std::vector<std::string>& filenames);

    void c_flags(const std::string& flags);
    void cpp_flags(const std::string& flags);

    bool marked;
    bool has_cpp;

    std::set<TargetPtr> deps_;

    Type type_;
    std::string name_;
    std::string dir_;
    std::string c_;
    std::string cpp_;
    std::string c_flags_;
    std::string cpp_flags_;
    std::set<FilePtr> files_;
    bool is_dynamic_;
  };

}