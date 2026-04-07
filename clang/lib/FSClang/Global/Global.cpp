#include "fsclang/Global/Global.h"
#include "illvm/Support/FileSystem.h"

#include "clang/Tooling/Transformer/Stencil.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/JSON.h"
#include "llvm/Support/Path.h"
#include <fstream>
#include "illvm/Support/Diagnostics.h"

namespace fsclang {
void Global::initClientUsed() {
  std::vector<std::string> readInfos;
  std::string toReadFile = illvm::FileSystem::linkPath(workPath,"isUsed.txt");
  readInfos = illvm::FileSystem::readLines(toReadFile);
  for (const auto &info : readInfos) {
    if (info != "" && info != "\n")
      ClientisUsed.insert(info);
  }
}

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

  const char *env2 = std::getenv("Project");
  project = env2 ? env2 : "";

  // llvm::SmallString<256> file(inputFile);
  // llvm::StringRef stem = llvm::sys::path::stem(file);
  // workPath
  // workPath = illvm::FileSystem::linkPath(currentPath,stem.str() + ".Info");
  workPath = inputPath + ".Info";
}
static void saveSet(const std::string &dirPath,const std::string &toWriteFileName,
  const std::unordered_set<std::string> &Set) {
  std::string path = illvm::FileSystem::linkPath(dirPath,toWriteFileName);
  illvm::FileSystem::saveSet(path,Set,true);
}

void Global::saveAllMangledNames() {
  if (workPath.empty())
    return;

  if (auto err = illvm::FileSystem::mkdir(workPath,true)) {
    llvm::errs() << workPath << "\n";
    llvm::consumeError(std::move(err));
    return;
  }

  initIsUsed();

  saveSet(workPath,"Method.txt",Method);
  saveSet(workPath,"Function.txt",Function);
  saveSet(workPath,"Instantiation.txt",Instantiation);
  saveSet(workPath,"CodeGen.txt",CodeGen);
  saveSet(workPath,"isUsed.txt",isUsed);

}

void Global::initIsUsed() {
  // Method + Function + Instantiation
  std::unordered_set<std::string> all;
  all.insert(Method.begin(),Method.end());
  all.insert(Function.begin(),Function.end());
  all.insert(Instantiation.begin(),Instantiation.end());

  for (const auto &N : CodeGen) {
    if (all.find(N) != all.end()) {
      // is used
      isUsed.insert(N);
    }
  }
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

  root["inputFilePath"] = inputPath;
  root["OutputFilePath"] = outputPath;

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

