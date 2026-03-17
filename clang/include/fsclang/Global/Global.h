#ifndef FSCLANG_GLOBAL_H
#define FSCLANG_GLOBAL_H

#include <string>
#include <unordered_set>

#include "clang/Driver/Action.h"
#include "clang/Driver/InputInfo.h"

namespace fsclang {

enum class FSClangMode {
  Normal,
  Master,
  Client
};


enum class MangledNameParts {
  Method,
  Function,
  Instantiation
};

class Global {
private:
  Global() {}

public:

  FSClangMode Mode = FSClangMode::Normal;

  std::string inputPath = "";
  std::string outputPath = "";
  std::string currentPath = "";

  // MangledNames of three parts
  std::unordered_set<std::string> Method;
  std::unordered_set<std::string> Function;
  std::unordered_set<std::string> Instantiation;

  Global(const Global &) = delete;
  Global &operator=(const Global &) = delete;

  static Global &getInstance() {
    static Global instance;
    return instance;
  }

  void init(const clang::driver::Action::ActionClass &kind,
                const std::vector<clang::driver::InputInfo> &inputInfos,
                const std::vector<std::string> &outputFilenames,
                const llvm::SmallVector<const char *, 128> &originalArgv);

  void addMangledName(const std::string MangledName,MangledNameParts Part) {
    if (MangledName != "") {
      if (Part == MangledNameParts::Method) { Method.insert(MangledName); }
      else if (Part == MangledNameParts::Function) { Function.insert(MangledName); }
      else if (Part == MangledNameParts::Instantiation) { Instantiation.insert(MangledName); }
    }
  }

  void saveAllMangledNames();

};

} // namespace fsclang

#endif // FSCLANG_GLOBAL_H
