#include "fsclang/ASTSupport/ASTGlobal.h"

#include <iomanip>
#include <sstream>

#include "clang/Lex/Lexer.h"

namespace fsclang {

void ASTGlobal::init(clang::Sema *_sema) {
  sema = _sema;
  astNameGenerator =
      std::make_unique<clang::ASTNameGenerator>(sema->getASTContext());
}

std::string ASTGlobal::getMangledName(const clang::NamedDecl *decl) const {
  if (decl && decl->getDeclName()) {
    if (llvm::isa<clang::RequiresExprBodyDecl>(decl->getDeclContext())) {
      return "";
    }
    auto *varDecl = llvm::dyn_cast<clang::VarDecl>(decl);
    if (varDecl && varDecl->hasLocalStorage()) {
      return "";
    }
    return astNameGenerator->getName(decl);
  }
  return "";
}
//
// static clang::SourceLocation expandToAttrBracket( clang::SourceLocation attrNameLoc,
//                                    const  clang::SourceManager &SM,
//                                    const  clang::LangOptions &Lang) {
//   auto decomposed = SM.getDecomposedLoc(attrNameLoc);
//   clang::FileID FID = decomposed.first;
//   unsigned offset = decomposed.second;
//
//   // walk backward from attrNameLoc character-by-character and scan ,to find two consecutive '[' token.
//   // If found, return the SourceLocation of the first '['; otherwise return an invalid location.
//   while (offset > 0) {
//     --offset;
//     clang::SourceLocation loc = SM.getLocForStartOfFile(FID).getLocWithOffset(offset);
//
//     // get token at this loc
//     clang::Token Tok;
//     if (clang::Lexer::getRawToken(loc, Tok, SM, Lang, /*IgnoreWhiteSpace=*/true))
//       break;
//
//     // find '[' token
//     if (Tok.is(clang::tok::l_square)) {
//       // get next token，check whether it is '['
//       clang::SourceLocation nextLoc = Tok.getLocation().getLocWithOffset(1);
//       clang::Token nextTok;
//       clang::Lexer::getRawToken(nextLoc, nextTok, SM, Lang, true);
//       if (nextTok.is(clang::tok::l_square)) {
//         // find "[[",return the first '['
//         return Tok.getLocation();
//       }
//     }
//   }
//   clang::SourceLocation Bad;
//   return Bad; // error
// }
//
//
// illvm::SourceInterval
// ASTGlobal::getDeclSourceInterval(const clang::Decl *decl) const {
//   illvm::SourceInterval res{};
//
//   res.isValid = false;
//
//   const auto sr = decl->getSourceRange();
//   const auto &sm =  getSourceManager();
//   clang::SourceLocation Begin = sr.getBegin();
//
//   // fix for special sr: [[Attr]] func
//   if (const auto funcDecl = llvm::dyn_cast<clang::FunctionDecl>(decl)) {
//     if (funcDecl->hasAttrs()) {
//       for (auto *A : funcDecl->attrs()) {
//         if (A->isCXX11Attribute()) {
//           auto newBegin = expandToAttrBracket(A->getLocation(), sm, getLangOpts());
//           if (newBegin.isValid()) {
//             Begin = newBegin;
//           }
//         }
//       }
//     }
//   }
//
//   // start
//   const clang::FullSourceLoc startFullSourceLoc(Begin, sm);
//   if (startFullSourceLoc.isInvalid()) {
//     return res;
//   }
//   res.startLine = startFullSourceLoc.getExpansionLineNumber();
//   res.startColumn = startFullSourceLoc.getExpansionColumnNumber();
//
//   // end
//   const clang::SourceLocation endSourceLoc = clang::Lexer::getLocForEndOfToken(
//       sr.getEnd(), 0, sm, getLangOpts());
//   const clang::FullSourceLoc endFullSourceLoc(endSourceLoc, sm);
//   if (endFullSourceLoc.isInvalid()) {
//     return res;
//   }
//   res.endLine = endFullSourceLoc.getExpansionLineNumber();
//   res.endColumn = endFullSourceLoc.getExpansionColumnNumber();
//
//   // [)
//   res.startOffset = startFullSourceLoc.getFileOffset();
//   res.endOffset = endFullSourceLoc.getFileOffset();
//
//   res.filename = "";
//
//   res.isValid = true;
//
//   return res;
// }
//
// std::string ASTGlobal::dumpDecl(const clang::Decl *decl) const {
//   if (decl == nullptr) {
//     return "nullptr";
//   }
//
//   std::ostringstream oss;
//
//   oss << "[" << decl->getDeclKindName() << "] " << decl << " ";
//
//   if (auto *namedDecl = llvm::dyn_cast<clang::NamedDecl>(decl)) {
//     oss << namedDecl->getNameAsString() + "(" + getMangledName(namedDecl) + ")";
//   }
//
//   oss << getDeclSourceInterval(decl).toString();
//
//   return oss.str();
// }

// void ASTGlobal::addDisableWarningDecl(const clang::Decl *decl) {
//   disableWarningDecls.insert(decl->getCanonicalDecl());
// }
//
// bool ASTGlobal::isDisableWarningDecl(const clang::Decl *decl) const {
//   return disableWarningDecls.find(decl->getCanonicalDecl()) !=
//          disableWarningDecls.end();
// }

} // namespace fsclang