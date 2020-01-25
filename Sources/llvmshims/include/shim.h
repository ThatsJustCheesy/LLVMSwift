#include <stddef.h>
#include "llvm-c/Types.h"
#include "llvm-c/Object.h"
#include "llvm-c/DebugInfo.h"

#ifndef LLVMSWIFT_LLVM_SHIM_H
#define LLVMSWIFT_LLVM_SHIM_H

size_t LLVMSwiftCountIntrinsics(void);
const char *LLVMSwiftGetIntrinsicAtIndex(size_t index);
const char *LLVMGetARMCanonicalArchName(const char *Name, size_t NameLen);

typedef enum {
  LLVMARMProfileKindInvalid = 0,
  LLVMARMProfileKindA,
  LLVMARMProfileKindR,
  LLVMARMProfileKindM
} LLVMARMProfileKind;
LLVMARMProfileKind LLVMARMParseArchProfile(const char *Name, size_t NameLen);
unsigned LLVMARMParseArchVersion(const char *Name, size_t NameLen);

LLVMBinaryRef LLVMUniversalBinaryCopyObjectForArchitecture(LLVMBinaryRef BR, const char *Arch, size_t ArchLen, char **ErrorMessage);

uint64_t LLVMGlobalGetGUID(LLVMValueRef Global);

void LLVMAddInternalizePassWithMustPreservePredicate(
  LLVMPassManagerRef PM, void *Context,
  LLVMBool (*MustPreserve)(LLVMValueRef, void *));

#endif /* LLVMSWIFT_LLVM_SHIM_H */
