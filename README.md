# ParticlePM

A package manager and build system for C/++ applications and libraries.

Its main goal is to allow creating program distributions which then can be
easily deployed to the other machine without installing any self-written dependencies.

## Example manifest

This is an example manifest which can be used to build particlepm itself:

```cpp
#include <particlepm.hpp>

using namespace PPM::Utils;

PPM_PACKAGE(p) {
  p->name("particlepm");
  p->version(PPM::Version("v0.1.3"));
  p->description("A package manager and build system for C/++ applications and libraries");
  p->github(PPM::GitHub("handicraftsman", "particlepm"));
  p->license(PPM::License("MIT", "https://github.com/handicraftsman/particlepm/blob/master/LICENSE.txt"));

  PPM::libs.insert("boost_program_options");

  // you can use
  //    p->git_repo(user, repo, selectors?)
  // or p->github_repo(name, url, selectors?)
  // or p->pkgconfig(name)
  // to specify dependencies

  PPM::TargetPtr lib = p->library("libparticlepm");
  lib->name("particlepm");
  lib->cpp("c++17");
  lib->cpp_files(std::vector<std::string> {
    "particlepm/dependency.cpp",
    "particlepm/github.cpp",
    "particlepm/libdeclarations.cpp",
    "particlepm/package.cpp",
    "particlepm/selector.cpp",
    "particlepm/utils.cpp",
    "particlepm/file.cpp",
    "particlepm/gitrepo.cpp",
    "particlepm/license.cpp",
    "particlepm/pkgconfig.cpp",
    "particlepm/target.cpp",
    "particlepm/version.cpp"
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
      "-lboost_program_options",

      ("-I" + p->pkg_dir())
    })
  );
  exe->depends(lib);
}
```

## Environment

You may create an environment file which contains library and include directories of libraries you build manually.

This is my instance of that file at the moment of writing this readme (where USERNAME is my username):

```json
{
  "library_dirs": [
    "/home/USERNAME/Tools/boost/boost/lib/"
  ],
  "include_dirs": [
    "/home/USERNAME/Tools/boost/boost/include/"
  ]
}
```

You can tell particlepm to use this file by passing `--env /path/to/particlepm.json` argument.

You can also add next line to your `.bashrc`:

```bash
alias particlepm="particlepm --env /path/to/particlepm.json"
```
