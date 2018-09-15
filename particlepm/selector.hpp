#pragma once

#include "includes.hpp"
#include "declarations.hpp"
#include "version.hpp"

namespace PPM {

  class Selector {
  public: 
    enum class Type {
      Exact,
      Same,
      Greater,
      GreaterE,
      Less,
      LessE,
    };

    Selector();

    Type type;
    Version ver;

    static Selector exact(const Version& ver);
    static Selector same(const Version& ver);
    static Selector greater(const Version& ver);
    static Selector greatere(const Version& ver);
    static Selector less(const Version& ver);
    static Selector lesse(const Version& ver);

    static std::vector<Version> apply(const std::vector<Version>& versions, const std::vector<Selector>& selectors);
  };

}