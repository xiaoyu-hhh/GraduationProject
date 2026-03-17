#include "fsclang/Global/Global.h"
#include "llvm/Support/FileSystem.h"
#

namespace fsclang {


static std::string getCurrentPath() {
  llvm::SmallString<256> res;
  const auto ec = llvm::sys::fs::current_path(res);
  if (ec) {
    llvm::errs() << "getCurrentPath error: " << ec.message() << "\n";
    return "";
  }
  return res.str().str();
}


void Global::init(const clang::driver::Action::ActionClass &kind,
                  const std::vector<clang::driver::InputInfo> &inputInfos,
                  const std::vector<std::string> &outputFilenames,
                  const llvm::SmallVector<const char *, 128> &originalArgv) {
  inputPath = inputInfos[0].getFilename();
  outputPath = outputFilenames[0];
  currentPath = getCurrentPath();

  const char *env = std::getenv("FSClang");
  std::string mode = env ? env : "";
  if (mode != "") {
    if (mode == "Master")
      Mode = FSClangMode::Master;
    else if (mode == "Client")
      Mode = FSClangMode::Client;
    else
      Mode = FSClangMode::Normal;
  }
}


void Global::saveAllMangledNames() {

}

}

