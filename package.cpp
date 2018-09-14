#include <particlepm.hpp>

using namespace PPM::Utils;

PPM_PACKAGE(p) {
  p->name("particlepm");
  p->version(PPM::Version("v0.1.1"));
  p->description("A package manager and build system for C/++ applications and libraries");
  p->github(PPM::GitHub("handicraftsman", "particlepm"));
  p->license(PPM::License("MIT", "https://github.com/handicraftsman/particlepm/blob/master/LICENSE.txt"));

  // you can use
  //    p->git_repo(user, repo, selectors?)
  // or p->github_repo(name, url, selectors?)
  // or p->pkgconfig(name)
  // to specify dependencies

  PPM::TargetPtr lib = p->library("libparticlepm");
  lib->name("particlepm");
  lib->cpp("c++17");
  lib->cpp_files(std::vector<std::string> {
    "particlepm.cpp"
  });
  lib->cpp_flags(
    flagcat({
      "-ldl",

      ("-I" + p->pkg_dir())
    })
  );

  PPM::TargetPtr exe = p->executable("particlepm");
  exe->cpp("c++17");
  exe->cpp_files(std::vector<std::string> {
    "main.cpp"
  });
  exe->cpp_flags(
    flagcat({
      ("-L" + PPM::dist_dir),

      "-lparticlepm",

      ("-I" + p->pkg_dir())
    })
  );
  exe->depends(lib);
}