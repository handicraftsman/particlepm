#include "libincludes.hpp"

#include "utils.hpp"

std::string PPM::Utils::to_path(std::vector<std::string> sections) {
  char* p = getcwd(NULL, 1024);
  std::stringstream path;
  path << p;
  for (const std::string& section : sections) {
    path << "/" << section;
  }
  free(p);
  return path.str();
}

PPM::Utils::ExecStatus PPM::Utils::exec(const std::string& cmd) {
  std::cerr << "++ " << cmd << std::endl;

  PPM::Utils::ExecStatus st;

  std::array<char, 128> buffer;
  std::shared_ptr<FILE> pipe(popen(cmd.c_str(), "r"), [&] (FILE* f) {
    st.code = WEXITSTATUS(pclose(f));
  });
  if (!pipe) throw std::runtime_error("popen() failed!");
  while (!feof(pipe.get())) {
    if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
      st.data += buffer.data();
  }

  return st;
}

void PPM::Utils::chdir(const std::string& path) {
  std::cerr << "Navigating to " << path << std::endl;
  if (::chdir(path.c_str()) == -1) {
    perror("PPM::Utils::chdir");
    exit(1);
  }
}

void PPM::Utils::mkdir(const std::string& path) {
  std::cerr << "Creating directory: " << path << std::endl;
  if (::mkdir(path.c_str(), PPM_MKDIR_FLAGS) == -1 && errno != EEXIST) {
    perror("PPM::Utils::mkdir");
    exit(1);
  }
}

template<typename Out>
static void split(const std::string& s, char delim, Out result) {
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
    *(result++) = item;
  }
}

std::vector<std::string> PPM::Utils::split(const std::string& str, char sep) {
  std::vector<std::string> elems;
  ::split(str, sep, std::back_inserter(elems));
  return elems;
}

std::string PPM::Utils::flagcat(const std::vector<std::string>& vec) {
  auto it = vec.begin();
  std::stringstream s;
  s << *it;
  for (it = ++it; it != vec.end(); ++it) {
    s << " " << *it;
  }
  return s.str();
}
