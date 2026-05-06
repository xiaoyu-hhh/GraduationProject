#include "fsclang/Global/Global.h"
#include "illvm/Support/FileSystem.h"

#include "clang/Tooling/Transformer/Stencil.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/JSON.h"
#include "llvm/Support/Path.h"
#include <fstream>
#include <sstream>
#include "illvm/Support/Diagnostics.h"

namespace fsclang {
// bool Global::initClientUsed() {
//   std::vector<std::string> readInfos;
//   std::string toReadFile = illvm::FileSystem::linkPath(workPath,"Used.txt");
//   readInfos = illvm::FileSystem::readLines(toReadFile);
//   if (readInfos.empty()) return false;
//   for (const auto &info : readInfos) {
//     if (info != "" && info != "\n")
//       ClientisUsed.insert(info);
//   }
//   return true;
// }

static bool assembleJobCheck(const clang::driver::Action::ActionClass &kind) {
  return kind == clang::driver::Action::AssembleJobClass;
}

void Global::init(const clang::driver::Action::ActionClass &kind,
                  const std::vector<clang::driver::InputInfo> &inputInfos,
                  const std::vector<std::string> &outputFilenames,
                  const llvm::SmallVector<const char *, 128> &originalArgv) {
  if (inputInfos.size() != 1 || !inputInfos[0].isFilename() ||
      outputFilenames.size() != 1 || !assembleJobCheck(kind)) {
    runMode = RunMode::Origin;
    return;
      }
  const char* env = std::getenv("FSClang");
  std::string mode = env ? env : "";
  if (mode != "") {
    if (mode == "Test")
      runMode = RunMode::Test;
    else if (mode == "Client") {
      runMode = RunMode::Client;
      const char* tmp = std::getenv("ClientMode");
      std::string ClientMode = tmp ? tmp : "";
      if (ClientMode != "") {
        if (ClientMode == "Normal")
          Mode = FSClangMode::Normal;
        else if (ClientMode == "Master")
          Mode = FSClangMode::Master;
        else if (ClientMode == "Client")
          Mode = FSClangMode::Client;
        else
          Mode = FSClangMode::Origin;
      }
      else {
        Mode = FSClangMode::Origin;
      }
    }
    else
      runMode = RunMode::Origin;
  }
  else {
    runMode = RunMode::Origin;
    return;
  }

  currentPath = illvm::FileSystem::getCurrentPath();
  inputFile = inputPath = inputInfos[0].getFilename();
  // inputPath = illvm::FileSystem::linkPath(currentPath,inputInfos[0].getFilename());
  outputFile = outputFilenames[0];
  outputPath = illvm::FileSystem::linkPath(currentPath,outputFilenames[0]);

  ast_func_count = 0;
  used_func_count = 0;

  const char *env2 = std::getenv("Project");
  project = env2 ? env2 : "";

  // llvm::SmallString<256> file(inputFile);
  // llvm::StringRef stem = llvm::sys::path::stem(file);
  // workPath
  // workPath = illvm::FileSystem::linkPath(currentPath,stem.str() + ".Info");

  // std::string savepath = inputPath;
  // for (int i = 0; i < savepath.length(); i++) {
  //   char c = savepath[i];
  //   if (c == '/' || c == '\\')
  //     savepath[i] = '_';
  // }
  // savepath = savepath + ".Info";
  // const char *env3 = std::getenv("benchmark");
  // std::string benchmark_path = "/root/ibenchmark";
  // if (env3)
  //   benchmark_path = env3;
  // workPath = illvm::FileSystem::linkPath(benchmark_path + "/" + project + "/Infos" ,savepath);
  //
  // if (runMode == RunMode::Client && Mode == FSClangMode::Master)
  // if (auto err = illvm::FileSystem::mkdir(workPath,true)) {
  //   Mode = FSClangMode::Origin;
  //   runMode = RunMode::Origin;
  //   return;
  // }
  //
  // if (runMode == RunMode::Client && Mode == FSClangMode::Client) {
  //   if (!llvm::sys::fs::exists(illvm::FileSystem::linkPath(workPath,"Used.txt"))) {
  //     Mode = FSClangMode::Origin;
  //     runMode = RunMode::Origin;
  //     return;
  //   }
  // }

  workPath = inputPath + ".Info";

}
// static void saveSet(const std::string &dirPath,const std::string &toWriteFileName,
//   const std::unordered_set<std::string> &Set) {
//   std::string path = illvm::FileSystem::linkPath(dirPath,toWriteFileName);
//   illvm::FileSystem::saveSet(path,Set,true);
// }

static size_t commonPrefixLength(const std::string& a, const std::string& b) {
  size_t i = 0;
  size_t minLength = std::min(a.size(), b.size());
  while (i < minLength && a[i] == b[i]) {
    ++i;
  }
  return i;
}

void Global::saveUsedFuncs() {
  std::set<std::string> orderedRes(Used.begin(), Used.end());

  std::ostringstream oss;
  oss << orderedRes.size() << "\n";
  std::string preStr;
  for (const auto &elem : orderedRes) {
    original_size += elem.size();
    size_t prefixLength = commonPrefixLength(preStr, elem);
    oss << prefixLength << " ";
    for (size_t i = prefixLength; i < elem.length(); i++) {
      oss << elem[i];
    }
    oss << "\n";
    preStr = elem;
  }

  std::string path = illvm::FileSystem::linkPath(workPath,"Used.txt");
  std::ofstream ofs(path);
  if (!ofs.is_open()) {
    llvm::errs() << "cant open " << path << "\n";
    return;
  }
  std::string content = oss.str();
  Used_txt_size = content.size();
  ofs << content;
  ofs.close();
}

bool Global::loadUsedFuncs() {
  std::string path = illvm::FileSystem::linkPath(workPath,"Used.txt");
  std::istringstream iss(illvm::FileSystem::readAll(path));

  size_t n, prefixLength;
  iss >> n;
  std::string suffix, preStr, curStr;
  for (size_t i = 0; i < n; i++) {
    iss >> prefixLength >> suffix;
    curStr = preStr.substr(0, prefixLength);
    curStr += suffix;
    ClientUsed.insert(curStr);
    preStr = curStr;
  }
  return true;
}

void Global::saveAllMangledNames() {
  if (workPath.empty())
    return;

  if (auto err = illvm::FileSystem::mkdir(workPath,true)) {
    llvm::errs() << workPath << "\n";
    llvm::consumeError(std::move(err));
    return;
  }

  initUsed();

  // saveSet(workPath,"Method.txt",Method);
  // saveSet(workPath,"Function.txt",Function);
  // saveSet(workPath,"Instantiation.txt",Instantiation);
  // saveSet(workPath,"CodeGen.txt",CodeGen);
  // saveSet(workPath,"Used.txt",isUsed);
  saveUsedFuncs();

}



void Global::initUsed() {
  // Method + Function + Instantiation
  std::unordered_set<std::string> all;
  all.insert(Method.begin(),Method.end());
  all.insert(Function.begin(),Function.end());
  all.insert(Instantiation.begin(),Instantiation.end());

  std::unordered_set<std::string> MethodAndFunction;
  MethodAndFunction.insert(Method.begin(),Method.end());
  MethodAndFunction.insert(Function.begin(),Function.end());
  HeadFuncCount = MethodAndFunction.size();
  InstantiationCount = Instantiation.size();

  int used_head = 0;
  int used_instantiation = 0;

  ast_func_count = all.size();

  for (const auto &N : CodeGen) {
    if (all.find(N) != all.end()) {
      // is used
      Used.insert(N);
    }
    if (MethodAndFunction.find(N) != MethodAndFunction.end()) {
      used_head++;
    }
    if (Instantiation.find(N) != Instantiation.end()) {
      used_instantiation++;
    }
  }
  HeadCanSkipCount = HeadFuncCount - used_head;
  InstantiationCanSkipCount = InstantiationCount - used_instantiation;
  used_func_count = Used.size();
}
//
// void Global::serialize() {
//   llvm::json::Object root;
//   if (Mode == FSClangMode::Master)
//     root["MasterTimeMs"] = MasterTimeMs;
//   else if (Mode == FSClangMode::Client)
//     root["ClientTimeMs"] = ClientTimeMs;
//   else if (Mode == FSClangMode::Normal)
//     root["NormalTimeMs"] = NormalTimeMs;
//
//   root["inputFilePath"] = inputPath;
//   root["OutputFilePath"] = outputPath;
//
//   const auto rootValue = llvm::json::Value(std::move(root));
//   const auto content = llvm::formatv("{0:2}", rootValue).str();
//
//   std::string targetPath = "";
//   std::string savePath = "/root/ibenchmark/" + project + "/CompileInfos";;
//   if (Mode == FSClangMode::Master)
//     targetPath = illvm::FileSystem::linkPath(savePath,"MasterCompile.json");
//   else if (Mode == FSClangMode::Client)
//     targetPath = illvm::FileSystem::linkPath(savePath,"ClientCompile.json");
//   else if (Mode == FSClangMode::Normal)
//     targetPath = illvm::FileSystem::linkPath(savePath,"NormalCompile.json");
//
//   if (auto err = illvm::FileSystem::mkdir(savePath,true)) {
//     llvm::consumeError(std::move(err));
//     return;
//   }
//
//   std::ofstream ofs(targetPath,std::ios::app);
//   ILLVM_FCHECK(ofs.is_open(), "Can not open " + targetPath);
//   ofs << content << "\n";
//   ofs.close();
//
//   // illvm::FileSystem::saveStr(targetPath, content);
// }


void Global::RunMode_Test_Analysis() {

  std::string result = "/root/ibenchmark/" + project + "/CompileInfos";

  llvm::json::Object root;
  root["MasterTimeMs"] = MasterTimeMs;
  root["ClientTimeMs"] = ClientTimeMs;
  root["NormalTimeMs"] = NormalTimeMs;
  root["SkipTimeMs"] = NormalTimeMs - ClientTimeMs;
  root["ASTGenTimeMs"] = ASTGenTimeEndMs - ASTGenTimeStartMs;
  root["backendTimeMS"] = endTimeMs - ASTGenTimeEndMs;
  root["original_size"] = original_size;
  root["Used_txt_size"] = Used_txt_size;

  root["inputFilePath"] = inputPath;
  root["OutputFilePath"] = outputPath;

  root["unused_func_count"] = ast_func_count - used_func_count;
  root["all_func"] = all_func.size();

  root["HeadFuncCount"] = HeadFuncCount;
  root["HeadCanSkipCount"] = HeadCanSkipCount;
  root["InstantiationCount"] = InstantiationCount;
  root["InstantiationCanSkipCount"] = InstantiationCanSkipCount;

  const auto rootValue = llvm::json::Value(std::move(root));
  const auto content = llvm::formatv("{0:2}", rootValue).str();

  std::string targetfile = project + "_Compile.json";
  std::string targetPath = "";
  targetPath = illvm::FileSystem::linkPath(result,targetfile);

  if (auto err = illvm::FileSystem::mkdir(result,true)) {
    llvm::consumeError(std::move(err));
    return;
  }

  std::ofstream ofs(targetPath,std::ios::app);
  ILLVM_FCHECK(ofs.is_open(), "Can not open " + targetPath);
  ofs << content << "\n";
  ofs.close();
  // illvm::FileSystem::saveStr(targetPath, content);

}

}

