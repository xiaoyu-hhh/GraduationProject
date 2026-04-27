//===--- ASTGlobal.h - FSClang AST global data -----------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===/
//
// FSClang AST global data.
// This module was originally part of Utils. However, when link to llvm, it will
// introduce AST dependency for the llvm citizens that do not need
// AST dependency. Therefore, we decouple ASTUtils from Utils.
//
//===----------------------------------------------------------------------===/

#ifndef FSCLANG_ASTGLOBAL_H
#define FSCLANG_ASTGLOBAL_H

#include <memory>
#include <string>

#include "clang/AST/Mangle.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Sema/Sema.h"
#include "illvm/Support/Interval.h"
#include "illvm/Support/FileSystem.h"
#include <fstream>

namespace fsclang {

class ASTGlobal {
private:

  clang::Sema *sema = nullptr;

  // Mangled name generator.
  std::unique_ptr<clang::ASTNameGenerator> astNameGenerator = nullptr;

  ASTGlobal() = default;

public:
  ASTGlobal(const ASTGlobal &) = delete;
  ASTGlobal &operator=(const ASTGlobal &) = delete;

  static ASTGlobal &getInstance() {
    static ASTGlobal instance;
    return instance;
  }

  void init(clang::Sema *_sema);

  clang::Sema &getSema() {
    assert(sema != nullptr);
    return *sema;
  }

  const clang::ASTContext &getContext() const {
    assert(sema != nullptr);
    return sema->getASTContext();
  }

  const clang::SourceManager &getSourceManager() const {
    assert(sema != nullptr);
    return sema->getSourceManager();
  }

  const clang::LangOptions &getLangOpts() const {
    assert(sema != nullptr);
    return sema->getLangOpts();
  }

  std::string getMangledName(const clang::NamedDecl *decl) const;

  illvm::SourceInterval getDeclSourceInterval(const clang::Decl *decl) const;

  std::string dumpDecl(const clang::Decl *decl) const;


  bool isMainFileDecl(const clang::Decl *decl) const {
    const auto loc = decl->getLocation();
    return loc.isValid() && getSourceManager().isInMainFile(loc);
  }

  const char* dumpOriginalCode(const clang::SourceLocation &loc) const {
    return getSourceManager().getCharacterData(loc);
  }

  static bool hasAutoReturn(const clang::FunctionDecl *FD) {
    const clang::QualType RT = FD->getReturnType();
    const clang::Type *T = RT.getTypePtr();

    if (llvm::dyn_cast<clang::AutoType>(T) != nullptr) {
      return true;
    }

    if (const clang::DeducedType *DT = T->getContainedDeducedType()) {
      if (llvm::isa<clang::AutoType>(DT)) {
        return true;
      }
    }

    return false;
  }

  bool isValidFuncHeader(const clang::FunctionDecl *funcDecl) const {

    if (llvm::dyn_cast<clang::CXXConstructorDecl>(funcDecl) != nullptr ||
        llvm::dyn_cast<clang::CXXDestructorDecl>(funcDecl) != nullptr)
      return false;

    if (const clang::CXXMethodDecl *cxxMethodDecl =
            llvm::dyn_cast<const clang::CXXMethodDecl>(funcDecl)) {
      if (cxxMethodDecl->isVirtual()) {
        return false;
      }
    }

    if (funcDecl->isImplicit())
      return false;

    if (funcDecl->getLinkageAndVisibility().getLinkage() ==
        clang::Linkage::UniqueExternalLinkage)
      return false;

    if (funcDecl->isTemplated() || funcDecl->isTemplateInstantiation() ||
        funcDecl->isFunctionTemplateSpecialization())
      return false;

    if (funcDecl->isConstexpr() || hasAutoReturn(funcDecl))
      return false;

    if (funcDecl->hasAttr<clang::AlwaysInlineAttr>() ||
        funcDecl->hasAttr<clang::ConstructorAttr>() ||
        funcDecl->hasAttr<clang::DestructorAttr>())
      return false;

    // New
    if (funcDecl->getOverloadedOperator() !=
      clang::OverloadedOperatorKind::OO_None)
      return false;

    // opencv
    // clang::SourceLocation loc = funcDecl->getLocation();
    // if (loc.isMacroID()) {
    //   // ✅ 这个函数名 / 声明 来自宏展开
    //   return false;
    // }

    // deleted / defaulted
    // if (funcDecl->isDeleted() || funcDecl->isDefaulted())
    //   return false;

    // // 没有 body
    // if (!funcDecl->doesThisDeclarationHaveABody())
    //   return false;

    // internal linkage（anonymous namespace）
    // if (funcDecl->getLinkageAndVisibility().getLinkage()  == clang::Linkage::InternalLinkage) {
    //   return false;
    // }


    // // inline（建议保守）
    // if (funcDecl->isInlined())
    //   return false;

    // if (funcDecl->getName().str() == "swap") {
    //   std::string targetPath = "/root/debug.txt";
    //   std::ofstream ofs(targetPath,std::ios::app);
    //   ofs << dumpDecl(funcDecl) << "\n";
    //   ofs.close();
    //   return false;
    // }

    return true;
  }


  bool isValidTempFuncHeader(const clang::FunctionDecl *funcDecl) const {

    if (llvm::dyn_cast<clang::CXXConstructorDecl>(funcDecl) != nullptr ||
        llvm::dyn_cast<clang::CXXDestructorDecl>(funcDecl) != nullptr)
      return false;

    if (const clang::CXXMethodDecl *cxxMethodDecl =
            llvm::dyn_cast<const clang::CXXMethodDecl>(funcDecl)) {
      if (cxxMethodDecl->isVirtual()) {
        return false;
      }
    }

    if (funcDecl->isImplicit())
      return false;

    if (funcDecl->getLinkageAndVisibility().getLinkage() ==
        clang::Linkage::UniqueExternalLinkage)
      return false;

    if (funcDecl->isTemplated() ||
        funcDecl->isFunctionTemplateSpecialization())
      return false;

    if (funcDecl->isConstexpr() || hasAutoReturn(funcDecl))
      return false;

    if (funcDecl->hasAttr<clang::AlwaysInlineAttr>() ||
        funcDecl->hasAttr<clang::ConstructorAttr>() ||
        funcDecl->hasAttr<clang::DestructorAttr>())
      return false;

    // New
    if (funcDecl->getOverloadedOperator() !=
      clang::OverloadedOperatorKind::OO_None)
      return false;

    // opencv
    // clang::SourceLocation loc = funcDecl->getLocation();
    // if (loc.isMacroID()) {
    //   return false;
    // }

    // deleted / defaulted
    // if (funcDecl->isDeleted() || funcDecl->isDefaulted())
    //   return false;

    // // 没有 body
    // if (!funcDecl->doesThisDeclarationHaveABody())
    //   return false;

    // internal linkage（anonymous namespace）
    // if (funcDecl->getLinkageAndVisibility().getLinkage()  == clang::Linkage::InternalLinkage) {
    //   return false;
    // }


    // // inline（建议保守）
    // if (funcDecl->isInlined())
    //   return false;

    // if (funcDecl->getName().str() == "swap") {
    //   std::string targetPath = "/root/debug.txt";
    //   std::ofstream ofs(targetPath,std::ios::app);
    //   ofs << dumpDecl(funcDecl) << "\n";
    //   ofs.close();
    //   return false;
    // }

    return true;
  }



  // bool isValidFuncHeader(const clang::FunctionDecl *funcDecl) const {
  //   if (funcDecl->isImplicit() || !isMainFileDecl(funcDecl) ||
  //       funcDecl->getLinkageAndVisibility().getLinkage() ==
  //           clang::Linkage::UniqueExternalLinkage ||
  //       funcDecl->isTemplated() || funcDecl->isTemplateInstantiation() ||
  //       funcDecl->isFunctionTemplateSpecialization() ||
  //       llvm::dyn_cast<clang::CXXConstructorDecl>(funcDecl) != nullptr ||
  //       llvm::dyn_cast<clang::CXXDestructorDecl>(funcDecl) != nullptr ||
  //       funcDecl->getOverloadedOperator() !=
  //           clang::OverloadedOperatorKind::OO_None ||
  //       llvm::dyn_cast<clang::CXXConversionDecl>(funcDecl) != nullptr ||
  //       funcDecl->isConstexpr() ||
  //       hasAutoReturn(funcDecl) ||
  //       funcDecl->hasAttr<clang::AlwaysInlineAttr>() ||
  //       funcDecl->hasAttr<clang::ConstructorAttr>() ||
  //       funcDecl->hasAttr<clang::DestructorAttr>()) {
  //     return false;
  //   }
  //   if (const clang::CXXMethodDecl *cxxMethodDecl =
  //           llvm::dyn_cast<const clang::CXXMethodDecl>(funcDecl)) {
  //     if (cxxMethodDecl->isVirtual()) {
  //       return false;
  //     }
  //     if (!cxxMethodDecl->isOutOfLine()) {
  //       return false;
  //     }
  //   }
  //   if (getMangledName(funcDecl).empty()) {
  //     return false;
  //   }
  //   return true;
  // }

  // bool isValidFuncBody(const clang::FunctionDecl *funcDecl) const {
  //   const auto compoundStmt =
  //    llvm::dyn_cast<clang::CompoundStmt>(funcDecl->getBody());
  //   if (compoundStmt == nullptr || compoundStmt->getLBracLoc().isInvalid()) {
  //     return false;
  //   }
  //   const auto loc = compoundStmt->getLBracLoc();
  //   const char *locChar = dumpOriginalCode(loc);
  //   if (locChar == nullptr || *locChar != '{') {
  //     return false;
  //   }
  //   return true;
  // }

};

} // namespace fsclang

#endif // FSCLANG_ASTGLOBAL_H
