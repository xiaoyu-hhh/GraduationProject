set(LLVM_LINK_COMPONENTS
  Option
  ProfileData
  ILLVMSupport
  )

add_clang_library(clangFSClangGlobal
  Global.cpp

  DEPENDS
  ClangDriverOptions

  LINK_LIBS
  clangBasic
  ${system_libs}
)