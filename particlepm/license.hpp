#pragma once

#include "includes.hpp"
#include "declarations.hpp"

namespace PPM {

  class License {
  public:
    License(const std::string& name_, const std::string& url_);
    License& operator=(const License& other);

    std::string name() const;
    std::string url() const;

  
    std::string name_;
    std::string url_;
  };

}