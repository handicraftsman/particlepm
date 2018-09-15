#include "libincludes.hpp"

#include "selector.hpp"

PPM::Selector::Selector()
: ver(0, 1, 0)
{}

PPM::Selector PPM::Selector::exact(const PPM::Version& ver) {
  PPM::Selector sel;
  sel.type = PPM::Selector::Type::Exact;
  sel.ver = ver;
  return sel;
}

PPM::Selector PPM::Selector::same(const PPM::Version& ver) {
  PPM::Selector sel;
  sel.type = PPM::Selector::Type::Same;
  sel.ver = ver;
  return sel;
}

PPM::Selector PPM::Selector::greater(const PPM::Version& ver) {
  PPM::Selector sel;
  sel.type = PPM::Selector::Type::Greater;
  sel.ver = ver;
  return sel;
}

PPM::Selector PPM::Selector::greatere(const PPM::Version& ver) {
  PPM::Selector sel;
  sel.type = PPM::Selector::Type::GreaterE;
  sel.ver = ver;
  return sel;
}

PPM::Selector PPM::Selector::less(const PPM::Version& ver) {
  PPM::Selector sel;
  sel.type = PPM::Selector::Type::Less;
  sel.ver = ver;
  return sel;
}

PPM::Selector PPM::Selector::lesse(const PPM::Version& ver) {
  PPM::Selector sel;
  sel.type = PPM::Selector::Type::LessE;
  sel.ver = ver;
  return sel;
}

std::vector<PPM::Version> PPM::Selector::apply(const std::vector<PPM::Version>& versions, const std::vector<PPM::Selector>& selectors) {
  std::vector<PPM::Version> vec(versions);
  for (PPM::Selector sel : selectors) {
    auto it = std::remove_if(vec.begin(), vec.end(), [&sel] (const PPM::Version& ver) {
      if (sel.type == PPM::Selector::Type::Exact) {
        bool b = !((sel.ver.vmajor() == ver.vmajor()) && (sel.ver.vminor() == ver.vminor()) && (sel.ver.vpatch() == ver.vpatch()));
        return b;
      } else if (sel.type == PPM::Selector::Type::Same) {
        bool b = !((sel.ver.vmajor() == ver.vmajor()) && (sel.ver.vminor() == ver.vminor()));
        return b;
      } else if (sel.type == PPM::Selector::Type::Greater) {
        int v1 = (ver.vmajor() * 1000 * 1000) + (ver.vminor() * 1000) + ver.vpatch();
        int v2 = (sel.ver.vmajor() * 1000 * 1000) + (sel.ver.vminor() * 1000) + sel.ver.vpatch();
        bool b = !(v1 > v2);
        return b;
      } else if (sel.type == PPM::Selector::Type::GreaterE) {
        int v1 = (ver.vmajor() * 1000 * 1000) + (ver.vminor() * 1000) + ver.vpatch();
        int v2 = (sel.ver.vmajor() * 1000 * 1000) + (sel.ver.vminor() * 1000) + sel.ver.vpatch();
        bool b = !(v1 >= v2);
        return b;
      } else if (sel.type == PPM::Selector::Type::Less) {
        int v1 = (ver.vmajor() * 1000 * 1000) + (ver.vminor() * 1000) + ver.vpatch();
        int v2 = (sel.ver.vmajor() * 1000 * 1000) + (sel.ver.vminor() * 1000) + sel.ver.vpatch();
        bool b = !(v1 < v2);
        return b;
      } else if (sel.type == PPM::Selector::Type::LessE) {
        int v1 = (ver.vmajor() * 1000 * 1000) + (ver.vminor() * 1000) + ver.vpatch();
        int v2 = (sel.ver.vmajor() * 1000 * 1000) + (sel.ver.vminor() * 1000) + sel.ver.vpatch();
        bool b = !(v1 <= v2);
        return b;
      } else {
        std::cerr << "Warning: unknown selector. Ignoring" << std::endl;
        return false;
      }
    });
    vec.erase(it, vec.end());
  }
  return vec;
}