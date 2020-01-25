#include "llvm-c/Object.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DIBuilder.h"
#include "llvm-c/Core.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/ARMTargetParser.h"
#include "llvm/Object/MachOUniversal.h"
#include "llvm/Object/ObjectFile.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Transforms/IPO.h"

extern "C" {
  // Not to be upstreamed: They support the hacks that power our dynamic member
  // lookup machinery for intrinsics.
  const char *LLVMSwiftGetIntrinsicAtIndex(size_t index);
  size_t LLVMSwiftCountIntrinsics(void);

  // Not to be upstreamed: There's no value in this without a full Triple
  // API.  And we have chosen to port instead of wrap.
  const char *LLVMGetARMCanonicalArchName(const char *Name, size_t NameLen);

  typedef enum {
    LLVMARMProfileKindInvalid = 0,
    LLVMARMProfileKindA,
    LLVMARMProfileKindR,
    LLVMARMProfileKindM
  } LLVMARMProfileKind;

  LLVMARMProfileKind LLVMARMParseArchProfile(const char *Name, size_t NameLen);
  unsigned LLVMARMParseArchVersion(const char *Name, size_t NameLen);

  // https://reviews.llvm.org/D60378
  LLVMBinaryRef LLVMUniversalBinaryCopyObjectForArchitecture(LLVMBinaryRef BR, const char *Arch, size_t ArchLen, char **ErrorMessage);

  // Not to be upstreamed: It's not clear there's value in having this outside
  // of PGO passes.
  uint64_t LLVMGlobalGetGUID(LLVMValueRef Global);

  // https://reviews.llvm.org/D62456
  void LLVMAddInternalizePassWithMustPreservePredicate(
   LLVMPassManagerRef PM, void *Context,
   LLVMBool (*MustPreserve)(LLVMValueRef, void *));
}

using namespace llvm;
using namespace llvm::object;

inline static section_iterator *unwrap(LLVMSectionIteratorRef SI) {
  return reinterpret_cast<section_iterator*>(SI);
}

inline static LLVMSectionIteratorRef
wrap(const section_iterator *SI) {
  return reinterpret_cast<LLVMSectionIteratorRef>
  (const_cast<section_iterator*>(SI));
}

inline static symbol_iterator *unwrap(LLVMSymbolIteratorRef SI) {
  return reinterpret_cast<symbol_iterator*>(SI);
}

inline static LLVMSymbolIteratorRef
wrap(const symbol_iterator *SI) {
  return reinterpret_cast<LLVMSymbolIteratorRef>
  (const_cast<symbol_iterator*>(SI));
}

LLVMBinaryRef LLVMUniversalBinaryCopyObjectForArchitecture(LLVMBinaryRef BR, const char *Arch, size_t ArchLen, char **ErrorMessage) {
  assert(LLVMBinaryGetType(BR) == LLVMBinaryTypeMachOUniversalBinary);
  auto universal = cast<MachOUniversalBinary>(unwrap(BR));
  Expected<std::unique_ptr<ObjectFile>> ObjOrErr(
    universal->getObjectForArch({Arch, ArchLen}));
  if (!ObjOrErr) {
    *ErrorMessage = strdup(toString(ObjOrErr.takeError()).c_str());
    return nullptr;
  }
  return wrap(ObjOrErr.get().release());
}

size_t LLVMSwiftCountIntrinsics(void) {
  return llvm::Intrinsic::num_intrinsics;
}

const char *LLVMSwiftGetIntrinsicAtIndex(size_t index) {
  return llvm::Intrinsic::getName(static_cast<llvm::Intrinsic::ID>(index)).data();
}

LLVMARMProfileKind LLVMARMParseArchProfile(const char *Name, size_t NameLen) {
  return static_cast<LLVMARMProfileKind>(llvm::ARM::parseArchProfile({Name, NameLen}));
}

unsigned LLVMARMParseArchVersion(const char *Name, size_t NameLen) {
  return llvm::ARM::parseArchVersion({Name, NameLen});
}

const char *LLVMGetARMCanonicalArchName(const char *Name, size_t NameLen) {
  return llvm::ARM::getCanonicalArchName({Name, NameLen}).data();
}

uint64_t LLVMGlobalGetGUID(LLVMValueRef Glob) {
  return unwrap<GlobalValue>(Glob)->getGUID();
}

void LLVMAddInternalizePassWithMustPreservePredicate(
  LLVMPassManagerRef PM, void *Context,
  LLVMBool (*Pred)(LLVMValueRef, void *)) {
  unwrap(PM)->add(createInternalizePass([=](const GlobalValue &GV) {
    return Pred(wrap(&GV), Context) == 0 ? false : true;
  }));
}
