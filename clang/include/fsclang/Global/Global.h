#ifndef FSCLANG_GLOBAL_H
#define FSCLANG_GLOBAL_H

#include <string>
#include <unordered_set>

#include "clang/Driver/Action.h"
#include "clang/Driver/InputInfo.h"
#include "clang/AST/Decl.h"
#include "fsclang/ASTSupport/ASTGlobal.h"

namespace fsclang {

enum class RunMode {
  Test,
  Client,
  Origin
};


enum class FSClangMode {
  Normal,
  Master,
  Client,
  Origin // Nothing to do with FSClang
};


enum class MangledNameParts {
  Method,
  Function,
  Instantiation,
  CodeGen
};

enum class PassMode {
  Normal, // Origin
  Master, // Collection
  Client, // Skip
};


class Global {
public:
  long long NormalTimeMs = 0;
  long long MasterTimeMs = 0;
  long long ClientTimeMs = 0;
  long long ASTGenTimeStartMs = 0;
  long long ASTGenTimeEndMs = 0;
  long long endTimeMs = 0;

  RunMode runMode = RunMode::Origin;

  FSClangMode Mode = FSClangMode::Origin;

  std::string inputFile = "";
  std::string inputPath = "";
  std::string project = "";

  std::string outputFile = "";
  std::string outputPath = "";

  std::string currentPath = "";
  std::string workPath = "";

  std::unordered_set<std::string> all_func;
  std::unordered_set<std::string> all_head;
  std::unordered_set<std::string> all_instant;

  // MangledNames of three parts
  std::unordered_set<std::string> Method;
  std::unordered_set<std::string> Function;
  int used_head = 0;
  int ast_head = 0;
  std::unordered_set<std::string> Instantiation;
  int used_instant = 0;
  int ast_instant = 0;
  // MangledNames that are truly CodeGened
  std::unordered_set<std::string> CodeGen;
  // CodeGen find in ( Method + Function + Instantiation ) = isUsed
  std::unordered_set<std::string> Used;
  size_t Used_txt_size = 0;
  size_t original_size = 0;

  int used_func_count = 0;
  int ast_func_count = 0;

  // Client Mode to skip
  std::unordered_set<std::string> ClientUsed;

  Global() {}
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

  void addMangledName(const std::string &MangledName,MangledNameParts Part) {
    if (MangledName != "") {
      if (Part == MangledNameParts::Method) { Method.insert(MangledName); }
      else if (Part == MangledNameParts::Function) { Function.insert(MangledName); }
      else if (Part == MangledNameParts::Instantiation) { Instantiation.insert(MangledName); }
      else if (Part == MangledNameParts::CodeGen) { CodeGen.insert(MangledName);  }
    }
  }

  void saveAllMangledNames();

  void initUsed();

  void saveUsedFuncs();

  bool loadUsedFuncs();

  // bool initClientUsed();

  bool clientCanSkip(const std::string &MangledName) {
    if (MangledName == "") return false;
    return ClientUsed.find(MangledName) == ClientUsed.end();
  }

  // void serialize();

  void RunMode_Test_Analysis();
  //
  // void master_handle(clang::Decl* decl,MangledNameParts Part) {
  //   auto &astGlobal = fsclang::ASTGlobal::getInstance();
  //   auto *funcDecl = llvm::dyn_cast<clang::FunctionDecl>(decl);
  //   if (funcDecl == nullptr) {
  //     return;
  //   }
  //   if (!astGlobal.isValidFuncHeader(funcDecl)) {
  //     return;
  //   }
  //   std::string mangledName = astGlobal.getMangledName(funcDecl);
  //   if (mangledName.empty()) {
  //     return;
  //   }
  //   addMangledName(mangledName,)
  // }
};

} // namespace fsclang

#endif // FSCLANG_GLOBAL_H
