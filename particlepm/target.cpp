#include "libincludes.hpp"

#include "target.hpp"

#include <functional>
#include <mutex>
#include <queue>
#include <thread>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>

PPM::Target::Target(const std::string& name, const std::string& dir, PPM::Target::Type type)
: marked(false)
, has_cpp(false)
, type_(type)
, name_(name)
, dir_(dir)
, c_("c11")
, cpp_("c++11")
, c_flags_("")
, cpp_flags_("")
, is_dynamic_(true)
{}

void PPM::Target::depends(const PPM::TargetPtr& other) {
  deps_.insert(other);
}

void PPM::Target::build() {
  if (marked) {
    return;
  }
  marked = true;

  for (PPM::TargetPtr dep : deps_) {
    dep->build();
  }

  std::cerr << "Building " << name() << "(" << (int) type_ << ")" << std::endl;

  std::string dbg = PPM::dev ? "-g -ggdb" : "";

  auto has_updated_headers = [&] (const std::string& compiler, const std::string& ifile, const std::string& ofile, std::function<void()> do_build) -> bool {
    std::string flags;
    std::string std;
    if (compiler == PPM_CC) {
      flags = c_flags_;
      std = c_;
    } else if (compiler == PPM_CXX) {
      flags = cpp_flags_;
      std = cpp_;
    }

    PPM::Utils::ExecStatus st = PPM::Utils::exec(compiler + " " + dbg + " " + PPM::envflags + " " + flags + " -fPIC -std=" + std + " -M " + ifile);
    if (st.code != 0) {
      std::cerr << st.data << std::endl;
      exit(1);
    }
    std::vector<std::string> files;
    boost::algorithm::split_regex(files, st.data, boost::regex("[\\\\\\r\\n\\s]+"));
    files.erase(files.begin(), files.begin() + 2);

    for (const std::string& file : files) {
      if (file.empty()) continue;

      if (access(ofile.c_str(), 0) == 0) {
        struct stat a, b;
        ::stat(file.c_str(), &a);
        ::stat(ofile.c_str(), &b);
        if (a.st_mtime > b.st_mtime) {
          do_build();
          return true;
        }
      }
    }
    return false;
  };

  auto runner = [&] (PPM::FilePtr file) {
    if (file->built) {
      return;
    }
    file->built = true;
    auto do_build = [&] () {
      char* p_ = getcwd(NULL, 1024);
      std::string p(p_);
      free(p_);

      PPM::Utils::chdir(dir_);

      std::string flags;
      std::string std;
      if (file->compiler == PPM_CC) {
        flags = c_flags_;
        std = c_;
      } else if (file->compiler == PPM_CXX) {
        flags = cpp_flags_;
        std = cpp_;
      }
      PPM::Utils::ExecStatus st = PPM::Utils::exec(file->compiler + " " + dbg + " " + PPM::envflags + " " + flags + " -Wl,-rpath='$ORIGIN' -fPIC -std=" + std + " -c " + file->ifile + " -o " + file->ofile);
      if (st.code != 0) {
        std::cerr << st.data << std::endl;
        ::exit(st.code);
      }

      PPM::Utils::chdir(p);
    };
    if (::access(file->ofile.c_str(), 0) != 0) {
      do_build();
    } else if (::access(file->ofile.c_str(), 0) == 0) {
      struct stat a, b;
      ::stat(file->ifile.c_str(), &a);
      ::stat(file->ofile.c_str(), &b);
      if (a.st_mtime > b.st_mtime) {
        do_build();
      } else if (has_updated_headers(file->compiler, file->ifile, file->ofile, do_build)) {}
    } else if (has_updated_headers(file->compiler, file->ifile, file->ofile, do_build)) {
    } else {
      do_build();
    }
  };

  std::queue<PPM::FilePtr> q;
  std::mutex qm;
  std::vector<std::thread> threads;
  unsigned n = std::thread::hardware_concurrency();

  auto thread_runner = [&] () {
    while (!q.empty()) {
      PPM::FilePtr f;
      {
        std::lock_guard<std::mutex> ql(qm);
        if (q.empty()) break;
        f = q.front();
        q.pop();
      }
      runner(f);
    }
  };

  for (int i = 0; i < n; ++i) {
    threads.push_back(std::thread(thread_runner));
  }

  for (PPM::FilePtr file : files_) {
    q.push(file);
  }

  for (std::thread& thr : threads) {
    thr.join();
  }

  std::string compiler = has_cpp ? PPM_CXX : PPM_CC;
  PPM::Utils::mkdir(PPM::dist_dir);
  std::string out = PPM::Utils::to_path(std::vector<std::string>{ "dist", ((type_ == PPM::Target::Type::Executable) ? name() : ("lib" + name() + ".so")) }); 
  std::string filenames = "";
  for (PPM::FilePtr file : files_) {
    filenames += (" " + file->ofile);
  }

  std::string lt = is_dynamic_ ? "-rdynamic" : "";

  PPM::Utils::ExecStatus st;
  if (type_ == PPM::Target::Type::Executable) {
    st = PPM::Utils::exec(compiler + " " + dbg + " " + PPM::envflags + " -Wl,-rpath='$ORIGIN' -fPIC -o " + out + " " + filenames + " " + cpp_flags_ + " " + c_flags_);
  } else {
    st = PPM::Utils::exec(compiler + " " + dbg + " " + PPM::envflags + " " + lt + " -shared -Wl,-rpath='$ORIGIN' -fPIC -o " + out + " " + filenames + " " + cpp_flags_ + " " + c_flags_);
  }
  if (st.code != 0) {
    std::cerr << st.data << std::endl;
    ::exit(st.code);
  }
}

std::string PPM::Target::name() {
  return name_;
}

void PPM::Target::name(const std::string& value) {
  name_ = value;
}

std::string PPM::Target::c() {
  return c_;
}

void PPM::Target::c(const std::string& value) {
  c_ = value;
}

std::string PPM::Target::cpp() {
  return cpp_;
}

void PPM::Target::cpp(const std::string& value) {
  cpp_ = value;
}

void PPM::Target::c_files(const std::vector<std::string>& filenames) {
  std::string suffix = (PPM::dev == true ? ".dev.o" : ".o");
  for (const std::string& filename : filenames) {
    std::string f = dir_ + "/" + filename;
    files_.insert(PPM::FilePtr(new PPM::File(f, f + suffix, PPM_CC)));
  }
}

void PPM::Target::cpp_files(const std::vector<std::string>& filenames) {
  std::string suffix = (PPM::dev == true ? ".dev.o" : ".o");
  has_cpp = true;
  for (const std::string& filename : filenames) {
    std::string f = dir_ + "/" + filename;
    files_.insert(PPM::FilePtr(new PPM::File(f, f + suffix, PPM_CXX)));
  }
}

void PPM::Target::c_flags(const std::string& flags) {
  c_flags_ += (" " + flags);
}

void PPM::Target::cpp_flags(const std::string& flags) {
  cpp_flags_ += (" " + flags);
}