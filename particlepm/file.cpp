#include "libincludes.hpp"

#include "file.hpp"

PPM::File::File(const std::string& ifile, const std::string& ofile, const std::string& compiler)
: built(false)
, ifile(ifile)
, ofile(ofile)
, compiler(compiler)
{}