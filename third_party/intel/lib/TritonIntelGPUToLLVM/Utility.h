#ifndef TRITON_CONVERSION_TRITONINTELGPU_TO_LLVM_UTILITY_H
#define TRITON_CONVERSION_TRITONINTELGPU_TO_LLVM_UTILITY_H

#include "intel/include/TritonIntelGPUToLLVM/Passes.h"
#include "mlir/Conversion/LLVMCommon/Pattern.h"
#include "mlir/Dialect/ControlFlow/IR/ControlFlowOps.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "triton/Conversion/MLIRTypes.h"
#include "triton/Dialect/NVGPU/IR/Dialect.h"
#include "triton/Dialect/Triton/IR/Utility.h"
#include "triton/Dialect/TritonGEN/IR/TritonGENDialect.h"
#include "triton/Dialect/TritonIntelGPU/IR/Dialect.h"
#include "triton/Dialect/TritonNvidiaGPU/IR/Dialect.h"
#include <set>

using namespace mlir;
using namespace mlir::triton;

// Shortcuts for some commonly used LLVM ops to keep code simple and intuitive
// Operators
#define inttoptr(...) rewriter.create<LLVM::IntToPtrOp>(loc, __VA_ARGS__)
#define ptrtoint(...) rewriter.create<LLVM::PtrToIntOp>(loc, __VA_ARGS__)
#define zext(...) rewriter.create<LLVM::ZExtOp>(loc, __VA_ARGS__)
#define trunc(...) rewriter.create<LLVM::TruncOp>(loc, __VA_ARGS__)
#define sext(...) rewriter.create<LLVM::SExtOp>(loc, __VA_ARGS__)
#define fpext(...) rewriter.create<LLVM::FPExtOp>(loc, __VA_ARGS__)
#define trunc(...) rewriter.create<LLVM::TruncOp>(loc, __VA_ARGS__)
#define udiv(...) rewriter.create<LLVM::UDivOp>(loc, __VA_ARGS__)
#define urem(...) rewriter.create<LLVM::URemOp>(loc, __VA_ARGS__)
#define add(...) rewriter.create<LLVM::AddOp>(loc, __VA_ARGS__)
#define sub(...) rewriter.create<LLVM::SubOp>(loc, __VA_ARGS__)
#define fadd(...) rewriter.create<LLVM::FAddOp>(loc, __VA_ARGS__)
#define mul(...) rewriter.create<LLVM::MulOp>(loc, __VA_ARGS__)
#define fmul(...) rewriter.create<LLVM::FMulOp>(loc, __VA_ARGS__)
#define lshr(...) rewriter.create<LLVM::LShrOp>(loc, __VA_ARGS__)
#define smax(...) rewriter.create<LLVM::SMaxOp>(loc, __VA_ARGS__)
#define umax(...) rewriter.create<LLVM::UMaxOp>(loc, __VA_ARGS__)
#define fmax(...) rewriter.create<LLVM::MaxNumOp>(loc, __VA_ARGS__)
#define smin(...) rewriter.create<LLVM::SMinOp>(loc, __VA_ARGS__)
#define umin(...) rewriter.create<LLVM::UMinOp>(loc, __VA_ARGS__)
#define fmin(...) rewriter.create<LLVM::MinNumOp>(loc, __VA_ARGS__)
#define shl(...) rewriter.create<LLVM::ShlOp>(loc, __VA_ARGS__)
#define lshr(...) rewriter.create<LLVM::LShrOp>(loc, __VA_ARGS__)
#define and_(...) rewriter.create<LLVM::AndOp>(loc, __VA_ARGS__)
#define xor_(...) rewriter.create<LLVM::XOrOp>(loc, __VA_ARGS__)
#define or_(...) rewriter.create<LLVM::OrOp>(loc, __VA_ARGS__)
#define bitcast(val__, type__)                                                 \
  rewriter.create<LLVM::BitcastOp>(loc, type__, val__)
#define gep(...) rewriter.create<LLVM::GEPOp>(loc, __VA_ARGS__)
#define ptr_ty(...) LLVM::LLVMPointerType::get(__VA_ARGS__)
#define insert_val(...) rewriter.create<LLVM::InsertValueOp>(loc, __VA_ARGS__)
#define extract_val(...) rewriter.create<LLVM::ExtractValueOp>(loc, __VA_ARGS__)
#define insert_element(...)                                                    \
  rewriter.create<LLVM::InsertElementOp>(loc, __VA_ARGS__)
#define extract_element(...)                                                   \
  rewriter.create<LLVM::ExtractElementOp>(loc, __VA_ARGS__)
#define load(...) rewriter.create<LLVM::LoadOp>(loc, __VA_ARGS__)
#define store(...) rewriter.create<LLVM::StoreOp>(loc, __VA_ARGS__)
#define load_dsmem(...) LLVM::utils::createLoadDSmem(loc, rewriter, __VA_ARGS__)
#define store_dsmem(...)                                                       \
  LLVM::utils::createStoreDSmem(loc, rewriter, __VA_ARGS__)
#define fcmp_ogt(lhs, rhs)                                                     \
  rewriter.create<LLVM::FCmpOp>(loc, rewriter.getI1Type(),                     \
                                LLVM::FCmpPredicate::ogt, lhs, rhs)
#define fcmp_olt(lhs, rhs)                                                     \
  rewriter.create<LLVM::FCmpOp>(loc, rewriter.getI1Type(),                     \
                                LLVM::FCmpPredicate::olt, lhs, rhs)
#define fcmp_eq(lhs, rhs)                                                      \
  rewriter.create<LLVM::FCmpOp>(loc, rewriter.getI1Type(),                     \
                                LLVM::FCmpPredicate::oeq, lhs, rhs)
#define icmp_eq(...)                                                           \
  rewriter.create<LLVM::ICmpOp>(loc, LLVM::ICmpPredicate::eq, __VA_ARGS__)
#define icmp_ne(...)                                                           \
  rewriter.create<LLVM::ICmpOp>(loc, LLVM::ICmpPredicate::ne, __VA_ARGS__)
#define icmp_slt(...)                                                          \
  rewriter.create<LLVM::ICmpOp>(loc, LLVM::ICmpPredicate::slt, __VA_ARGS__)
#define icmp_sle(...)                                                          \
  rewriter.create<LLVM::ICmpOp>(loc, LLVM::ICmpPredicate::sle, __VA_ARGS__)
#define icmp_sgt(...)                                                          \
  rewriter.create<LLVM::ICmpOp>(loc, LLVM::ICmpPredicate::sgt, __VA_ARGS__)
#define icmp_sge(...)                                                          \
  rewriter.create<LLVM::ICmpOp>(loc, LLVM::ICmpPredicate::sge, __VA_ARGS__)
#define icmp_ult(...)                                                          \
  rewriter.create<LLVM::ICmpOp>(loc, LLVM::ICmpPredicate::ult, __VA_ARGS__)
#define icmp_ule(...)                                                          \
  rewriter.create<LLVM::ICmpOp>(loc, LLVM::ICmpPredicate::ule, __VA_ARGS__)
#define icmp_ugt(...)                                                          \
  rewriter.create<LLVM::ICmpOp>(loc, LLVM::ICmpPredicate::ugt, __VA_ARGS__)
#define icmp_uge(...)                                                          \
  rewriter.create<LLVM::ICmpOp>(loc, LLVM::ICmpPredicate::uge, __VA_ARGS__)
#define select(...) rewriter.create<LLVM::SelectOp>(loc, __VA_ARGS__)
#define address_of(...) rewriter.create<LLVM::AddressOfOp>(loc, __VA_ARGS__)
#define barrier() rewriter.create<mlir::gpu::BarrierOp>(loc)
#define undef(...) rewriter.create<LLVM::UndefOp>(loc, __VA_ARGS__)
#define null(...) rewriter.create<LLVM::ZeroOp>(loc, __VA_ARGS__)
#define call(...) rewriter.create<LLVM::CallOp>(loc, __VA_ARGS__)
#define addrspacecast(...)                                                     \
  rewriter.create<LLVM::AddrSpaceCastOp>(loc, __VA_ARGS__)

// Types
#define int_ty(width) rewriter.getIntegerType(width)
#define i64_ty rewriter.getIntegerType(64)
#define i32_ty rewriter.getIntegerType(32)
#define i16_ty rewriter.getIntegerType(16)
#define i32_ty rewriter.getIntegerType(32)
#define i64_ty rewriter.getIntegerType(64)
#define ui32_ty rewriter.getIntegerType(32, false)
#define f16_ty rewriter.getF16Type()
#define bf16_ty rewriter.getBF16Type()
#define i8_ty rewriter.getIntegerType(8)
#define i1_ty rewriter.getI1Type()
#define f32_ty rewriter.getF32Type()
#define f64_ty rewriter.getF64Type()
#define vec_ty(type, num) VectorType::get(num, type)
#define void_ty(ctx) LLVM::LLVMVoidType::get(ctx)
#define struct_ty(...) LLVM::LLVMStructType::getLiteral(ctx, __VA_ARGS__)
#define array_ty(elemTy, count) LLVM::LLVMArrayType::get(elemTy, count)

// Constants
#define f16_val(...) LLVM::utils::createConstantF16(loc, rewriter, __VA_ARGS__)
#define f32_val(...) LLVM::utils::createConstantF32(loc, rewriter, __VA_ARGS__)
#define f64_val(...) LLVM::utils::createConstantF64(loc, rewriter, __VA_ARGS__)
#define i32_val(...) LLVM::utils::createConstantI32(loc, rewriter, __VA_ARGS__)
#define i64_val(...) LLVM::utils::createConstantI64(loc, rewriter, __VA_ARGS__)
#define int_val(width, val)                                                    \
  LLVM::utils::createLLVMIntegerConstant(rewriter, loc, width, val)
#define tid_val() getThreadId(rewriter, loc)

// Attributes
#define i32_arr_attr(...) rewriter.getI32ArrayAttr({__VA_ARGS__})
#define i64_arr_attr(...) rewriter.getI64ArrayAttr({__VA_ARGS__})

namespace mlir {
namespace triton {
// namespace intel {

// Delinearize supposing order is [0, 1, .. , n]
template <typename T>
llvm::SmallVector<T> getMultiDimIndexImpl(T linearIndex,
                                          llvm::ArrayRef<T> shape) {
  // shape: {a, b, c, d}  ->  accMul: {1, a, a*b, a*b*c}
  size_t rank = shape.size();
  T accMul = product(shape.drop_back());
  T linearRemain = linearIndex;
  llvm::SmallVector<T> multiDimIndex(rank);
  for (int i = rank - 1; i >= 0; --i) {
    multiDimIndex[i] = linearRemain / accMul;
    linearRemain = linearRemain % accMul;
    if (i != 0) {
      accMul = accMul / shape[i - 1];
    }
  }
  return multiDimIndex;
}

template <typename T>
llvm::SmallVector<T> getMultiDimIndex(T linearIndex, llvm::ArrayRef<T> shape,
                                      llvm::ArrayRef<unsigned> order) {
  size_t rank = shape.size();
  assert(rank == order.size());
  auto reordered = applyPermutation(shape, order);
  auto reorderedMultiDim = getMultiDimIndexImpl<T>(linearIndex, reordered);
  llvm::SmallVector<T> multiDim(rank);
  for (unsigned i = 0; i < rank; ++i) {
    multiDim[order[i]] = reorderedMultiDim[i];
  }
  return multiDim;
}

// Linearize supposing order is [0, 1, .. , n]
template <typename T>
T getLinearIndexImpl(llvm::ArrayRef<T> multiDimIndex, llvm::ArrayRef<T> shape) {
  assert(multiDimIndex.size() == shape.size());
  // shape: {a, b, c, d}  ->  accMul: {1, a, a*b, a*b*c}
  size_t rank = shape.size();
  T accMul = product(shape.drop_back());
  T linearIndex = 0;
  for (int i = rank - 1; i >= 0; --i) {
    linearIndex += multiDimIndex[i] * accMul;
    if (i != 0) {
      accMul = accMul / shape[i - 1];
    }
  }
  return linearIndex;
}

template <typename T>
T getLinearIndex(llvm::ArrayRef<T> multiDimIndex, llvm::ArrayRef<T> shape,
                 llvm::ArrayRef<unsigned> order) {
  assert(shape.size() == order.size());
  return getLinearIndexImpl<T>(applyPermutation(multiDimIndex, order),
                               applyPermutation(shape, order));
}

// } // namespace intel
} // namespace triton

namespace LLVM {
namespace utils {
using namespace mlir::triton;

/// Create a predicated block, using \p cond as the condition and \p ops for the
/// values supplied by the conditional branch to the exit block. The \p
/// thenOpsFn function is used to inject operations in the 'then' branch:
///   cf.cond_br %cond, ^br1, ^br2(%ops)
///   ^br1:
///     %then_ops = `thenOpsFn()`
///     cf.br ^br2(%then_ops)
///   ^br2(%block_ops):
template <typename ThenOpsFn>
Block &createPredicatedBlock(ConversionPatternRewriter &rewriter, Location loc,
                             Value cond, ArrayRef<Value> ops,
                             ThenOpsFn &&thenOpsFn) {
  Block *insertionBlock = rewriter.getInsertionBlock();
  Block *thenBlock =
      rewriter.splitBlock(insertionBlock, rewriter.getInsertionPoint());
  Block *endBlock = rewriter.splitBlock(thenBlock, thenBlock->begin());

  rewriter.setInsertionPointToEnd(insertionBlock);
  rewriter.create<cf::CondBranchOp>(loc, cond, thenBlock, endBlock, ops);

  rewriter.setInsertionPointToStart(thenBlock);
  auto thenOps = thenOpsFn();
  assert(thenOps.size() == ops.size() && "Inconsistent size");
  assert(llvm::all_of(llvm::enumerate(ops, thenOps),
                      [](const auto &enumerator) {
                        auto [index, op, thenOp] = enumerator;
                        return op.getType() == thenOp.getType();
                      }) &&
         "type mismatch found");

  if (thenOps.empty())
    rewriter.create<cf::BranchOp>(loc, endBlock);
  else
    rewriter.create<cf::BranchOp>(loc, endBlock, thenOps);

  for (Value op : thenOps)
    endBlock->addArgument(op.getType(), op.getLoc());

  rewriter.setInsertionPointToStart(endBlock);
  return *endBlock;
}

/// Create a predicated block, using \p cond as the condition and \p thenOpsFn
/// to inject operations in the 'then' branch:
///   cf.cond_br %cond, ^br1, ^br2
///   ^br1:
///     `thenOpsFn()`
///     cf.br ^br2
///   ^br2:
template <typename ThenOpsFn>
Block &createPredicatedBlock(ConversionPatternRewriter &rewriter, Location loc,
                             Value cond, ThenOpsFn &&thenOpsFn) {
  return createPredicatedBlock(rewriter, loc, cond, {}, thenOpsFn);
}

/// Create a 32-bit integer constant.
Value createConstantI32(Location loc, OpBuilder &rewriter, int32_t v);

/// Create a 64-bit integer constant.
Value createConstantI64(Location loc, OpBuilder &rewriter, int64_t v);

/// Create a 16-bit float constant.
Value createConstantF16(Location loc, OpBuilder &rewriter, float v);

/// Create a 32-bit float constant.
Value createConstantF32(Location loc, OpBuilder &rewriter, float v);

/// Create a 64-bit float constant.
Value createConstantF64(Location loc, OpBuilder &rewriter, double v);

/// Create NaN constant of specified type.
Value createNaNConstant(Location loc, OpBuilder &rewriter, Type type);

/// Create an index type constant.
Value createIndexConstant(OpBuilder &builder, Location loc,
                          TypeConverter *converter, int64_t value);

/// Create an integer constant of \param width bits.
Value createLLVMIntegerConstant(OpBuilder &builder, Location loc, short width,
                                int64_t value);

/// Usage of macro load_dsmem
/// (1) load_dsmem(addr, ctaId)
/// (2) load_dsmem(addr, ctaId, vec)
Value createLoadDSmem(Location loc, PatternRewriter &rewriter, Value addr,
                      Value ctaId, Type elemTy);
SmallVector<Value> createLoadDSmem(Location loc, PatternRewriter &rewriter,
                                   Value addr, Value ctaId, unsigned vec,
                                   Type elemTy);

/// Usage of macro store_dsmem
/// (1) store_dsmem(addr, ctaId, value, pred)
/// (2) store_dsmem(addr, ctaId, value)
/// (3) store_dsmem(addr, ctaId, values, pred)
/// (4) store_dsmem(addr, ctaId, values)
void createStoreDSmem(Location loc, PatternRewriter &rewriter, Value addr,
                      Value ctaId, Value value, Value pred);
void createStoreDSmem(Location loc, PatternRewriter &rewriter, Value addr,
                      Value ctaId, Value value);
void createStoreDSmem(Location loc, PatternRewriter &rewriter, Value addr,
                      Value ctaId, ArrayRef<Value> values, Value pred);
void createStoreDSmem(Location loc, PatternRewriter &rewriter, Value addr,
                      Value ctaId, ArrayRef<Value> values);

/// Helper function to get strides from a given shape and its order
SmallVector<Value> getStridesFromShapeAndOrder(ArrayRef<int64_t> shape,
                                               ArrayRef<unsigned> order,
                                               Location loc,
                                               RewriterBase &rewriter);
struct SharedMemoryObject {
  Value base; // i32 ptr. The start address of the shared memory object after
              // the initial allocation or the last slicing operation.
  Type baseElemType;
  // We need to store strides as Values, not integers, because the
  // extract_slice instruction can take a slice at arbitrary offsets.
  // Take $a[16:32, 16:32] as an example; though we know the stride of $a[0] is
  // 32, we need to let the instruction that uses $a be aware of that.
  // Otherwise, when we use $a, we only know that the shape of $a is 16x16. If
  // we store strides into an attribute array of integers, the information
  // cannot pass through block argument assignment because attributes are
  // associated with operations, not Values.
  // TODO(Keren): We may need to figure out a way to store strides as integers
  // if we want to support more optimizations.
  SmallVector<Value>
      strides; // i32 int. The strides of the shared memory object.
  SmallVector<Value> offsets; // i32 int.
  // Offsets are applied at the last slicing operation.
  // We can use offsets to recover the previous base.
  // The offsets are zero at the initial allocation.

  SharedMemoryObject(Value base, Type baseElemType, ArrayRef<Value> strides,
                     ArrayRef<Value> offsets)
      : base(base), baseElemType(baseElemType),
        strides(strides.begin(), strides.end()),
        offsets(offsets.begin(), offsets.end()) {}

  SharedMemoryObject(Value base, Type baseElemType, ArrayRef<int64_t> shape,
                     ArrayRef<unsigned> order, Location loc,
                     RewriterBase &rewriter)
      : base(base), baseElemType(baseElemType) {
    strides = getStridesFromShapeAndOrder(shape, order, loc, rewriter);
    offsets.append(order.size(), i32_val(0));
  }

  SmallVector<Value> getStrides() const { return strides; }
  SmallVector<Value> getOffsets() const { return offsets; }
  Value getBase() const { return base; }
  Type getBaseElemType() const { return baseElemType; }

  SmallVector<Value> getElems() const {
    SmallVector<Value> elems;
    elems.push_back(base);
    elems.append(strides.begin(), strides.end());
    elems.append(offsets.begin(), offsets.end());
    return elems;
  }

  SmallVector<Type> getTypes() const {
    SmallVector<Type> types;
    types.push_back(base.getType());
    types.append(strides.size(), IntegerType::get(base.getContext(), 32));
    types.append(offsets.size(), IntegerType::get(base.getContext(), 32));
    return types;
  }

  Value getCSwizzleOffset(int order) const {
    assert(order >= 0 && order < strides.size());
    return offsets[order];
  }

  Value getBaseBeforeSlice(int order, Location loc,
                           ConversionPatternRewriter &rewriter) const {
    Value cSwizzleOffset = getCSwizzleOffset(order);
    Value offset = sub(i32_val(0), cSwizzleOffset);
    Type type = base.getType();
    return gep(type, baseElemType, base, offset);
  }
};

SharedMemoryObject
getSharedMemoryObjectFromStruct(Location loc, Value llvmStruct, Type elemTy,
                                ConversionPatternRewriter &rewriter);

// Convert an \param linear to a multi-dim coordinate given \param shape and
// \param order.
SmallVector<Value> delinearize(RewriterBase &rewriter, Location loc,
                               Value linear, ArrayRef<unsigned> shape,
                               ArrayRef<unsigned> order);

SmallVector<Value> delinearize(RewriterBase &rewriter, Location loc,
                               unsigned linear, ArrayRef<unsigned> shape);

SmallVector<Value> delinearize(RewriterBase &rewriter, Location loc,
                               Value linear, ArrayRef<unsigned> shape);

Value linearize(ConversionPatternRewriter &rewriter, Location loc,
                ArrayRef<Value> multiDim, ArrayRef<unsigned> shape,
                ArrayRef<unsigned> order);

Value linearize(ConversionPatternRewriter &rewriter, Location loc,
                ArrayRef<Value> multiDim, ArrayRef<unsigned> shape);

Value storeShared(ConversionPatternRewriter &rewriter, Location loc, Value ptr,
                  Value val, Value pred);

Value loadShared(ConversionPatternRewriter &rewriter, Location loc, Value ptr,
                 Type elemTy, Value pred);

Value shflSync(Location loc, ConversionPatternRewriter &rewriter, Value val,
               int i);
Value shflUpSync(Location loc, ConversionPatternRewriter &rewriter, Value val,
                 int i);
Value shflIdxSync(Location loc, ConversionPatternRewriter &rewriter, Value val,
                  int i);
Value shflIdxSync(Location loc, ConversionPatternRewriter &rewriter, Value val,
                  Value i);

Value llGetPid(Location loc, ConversionPatternRewriter &rewriter,
               ModuleOp moduleOp, int axis);

Value addStringToModule(Location loc, ConversionPatternRewriter &rewriter,
                        StringRef key, StringRef content,
                        unsigned addressSpace);

static bool isKernel(FunctionOpInterface funcOp) {
  return funcOp.getVisibility() == SymbolTable::Visibility::Public;
}

static Value getStackPointer(PatternRewriter &rewriter,
                             FunctionOpInterface funcOp) {
  auto mod = funcOp->getParentOfType<ModuleOp>();
  LLVM::LLVMPointerType ptrTy = ptr_ty(
      rewriter.getContext(), TritonGEN::TritonGENMemorySpace::kWorkgroup);
  if (mod->getAttrOfType<IntegerAttr>("triton_gpu.shared").getInt() == 0)
    return rewriter.create<LLVM::UndefOp>(funcOp.getLoc(), ptrTy);
  return funcOp.getArgument(funcOp.getNumArguments() - 1);
}

static Value getSharedMemoryBase(Location loc,
                                 ConversionPatternRewriter &rewriter,
                                 Operation *op) {
  auto ptrTy = LLVM::LLVMPointerType::get(rewriter.getContext(), 3);
  FunctionOpInterface func =
      op->template getParentOfType<FunctionOpInterface>();
  assert(op->hasAttr("allocation.offset"));
  size_t offset = op->getAttr("allocation.offset")
                      .cast<IntegerAttr>()
                      .getValue()
                      .getZExtValue();
  Value offVal = i32_val(offset);
  Value base =
      gep(ptrTy, i8_ty, LLVM::utils::getStackPointer(rewriter, func), offVal);
  return base;
}

// Returns a Value for the format string, which you can reuse.
Value llPrintf(ConversionPatternRewriter &rewriter, StringRef msg,
               ValueRange args);

void llPrintf(ConversionPatternRewriter &rewriter, Value msg, ValueRange args);

} // namespace utils
} // namespace LLVM

/* ------------------------------------ */
// Returns CTA level thread idx
static Value getThreadIdInCTA(RewriterBase &rewriter, Location loc) {
  Value tid =
      rewriter.create<::mlir::gpu::ThreadIdOp>(loc, ::mlir::gpu::Dimension::x);
  return rewriter.create<arith::IndexCastOp>(loc, i32_ty, tid);
}

// Returns CTA level thread idx.
static Value getThreadId(RewriterBase &rewriter, Location loc) {
  Value tid = getThreadIdInCTA(rewriter, loc);
  auto mod = rewriter.getBlock()->getParent()->getParentOfType<ModuleOp>();
  return tid;
}

static Value getModuleWarpSize(RewriterBase &rewriter, Location loc) {
  auto mod = rewriter.getBlock()->getParent()->getParentOfType<ModuleOp>();
  return i32_val(triton::gpu::TritonGPUDialect::getThreadsPerWarp(mod));
}

static Value getClusterCTAId(RewriterBase &rewriter, Location loc) {
  // Clusters of thread blocks aren't supported.
  return rewriter.create<arith::ConstantIntOp>(loc, 0, 32);
}

// -----------------------------------------------------------------------
// Shared memory utilities
// -----------------------------------------------------------------------
using ::mlir::LLVM::utils::delinearize;
using ::mlir::LLVM::utils::SharedMemoryObject;
using ::mlir::triton::getMultiDimIndex;
using ::mlir::triton::gpu::BlockedEncodingAttr;
using ::mlir::triton::gpu::CTALayoutAttr;
using ::mlir::triton::gpu::DotOperandEncodingAttr;
using ::mlir::triton::gpu::NvidiaMmaEncodingAttr;
using ::mlir::triton::gpu::SliceEncodingAttr;
using ::mlir::triton::gpu::intel::DpasEncodingAttr;

static Value dot(RewriterBase &rewriter, Location loc, ArrayRef<Value> offsets,
                 ArrayRef<Value> strides) {
  assert(offsets.size() == strides.size());
  Value ret = i32_val(0);
  for (auto [offset, stride] : llvm::zip(offsets, strides)) {
    ret = add(ret, mul(offset, stride));
  }
  return ret;
}

// -----------------------------------------------------------------------
// Blocked layout indices
// -----------------------------------------------------------------------

// Get an index-base for each dimension for a \param blockedLayout.
static SmallVector<Value>
emitBaseIndexWithinCTAForBlockedLayout(Location loc, RewriterBase &rewriter,
                                       const BlockedEncodingAttr &blockedLayout,
                                       RankedTensorType type) {
  auto shape = type.getShape();
  Value threadId = getThreadId(rewriter, loc);
  Value warpSize = getModuleWarpSize(rewriter, loc);
  Value laneId = urem(threadId, warpSize);
  Value warpId = udiv(threadId, warpSize);
  auto sizePerThread = blockedLayout.getSizePerThread();
  auto threadsPerWarp = blockedLayout.getThreadsPerWarp();
  auto warpsPerCTA = blockedLayout.getWarpsPerCTA();
  auto order = blockedLayout.getOrder();
  auto shapePerCTA = triton::gpu::getShapePerCTA(blockedLayout, shape);
  unsigned rank = shape.size();

  // delinearize threadId to get the base index
  SmallVector<Value> multiDimWarpId =
      delinearize(rewriter, loc, warpId, warpsPerCTA, order);
  SmallVector<Value> multiDimThreadId =
      delinearize(rewriter, loc, laneId, threadsPerWarp, order);

  SmallVector<Value> multiDimBase(rank);
  for (unsigned k = 0; k < rank; ++k) {
    // Wrap around multiDimWarpId/multiDimThreadId in case
    // shapePerCTATile[k] > shapePerCTA[k]
    auto maxWarps =
        ceil<unsigned>(shapePerCTA[k], sizePerThread[k] * threadsPerWarp[k]);
    auto maxThreads = ceil<unsigned>(shapePerCTA[k], sizePerThread[k]);
    multiDimWarpId[k] = urem(multiDimWarpId[k], i32_val(maxWarps));
    multiDimThreadId[k] = urem(multiDimThreadId[k], i32_val(maxThreads));
    // multiDimBase[k] = (multiDimThreadId[k] +
    //                    multiDimWarpId[k] * threadsPerWarp[k]) *
    //                   sizePerThread[k];
    Value threadsPerWarpK = i32_val(threadsPerWarp[k]);
    Value sizePerThreadK = i32_val(sizePerThread[k]);
    multiDimBase[k] =
        mul(sizePerThreadK,
            add(multiDimThreadId[k], mul(multiDimWarpId[k], threadsPerWarpK)));
  }
  return multiDimBase;
}

static SmallVector<SmallVector<unsigned>>
emitOffsetForBlockedLayout(const BlockedEncodingAttr &blockedLayout,
                           RankedTensorType type) {
  auto shape = type.getShape();
  auto sizePerThread = blockedLayout.getSizePerThread();
  auto threadsPerWarp = blockedLayout.getThreadsPerWarp();
  auto warpsPerCTA = blockedLayout.getWarpsPerCTA();
  auto order = blockedLayout.getOrder();
  auto shapePerCTATile = getShapePerCTATile(blockedLayout);
  auto shapePerCTA = triton::gpu::getShapePerCTA(blockedLayout, shape);

  unsigned rank = shape.size();
  SmallVector<unsigned> tilesPerDim(rank);
  for (unsigned k = 0; k < rank; ++k)
    tilesPerDim[k] = ceil<unsigned>(shapePerCTA[k], shapePerCTATile[k]);

  unsigned elemsPerThread = triton::gpu::getTotalElemsPerThread(type);
  unsigned totalSizePerThread = product<unsigned>(sizePerThread);
  SmallVector<SmallVector<unsigned>> reorderedOffset(elemsPerThread);
  for (unsigned n = 0; n < elemsPerThread; ++n) {
    unsigned linearNanoTileId = n / totalSizePerThread;
    unsigned linearNanoTileElemId = n % totalSizePerThread;
    SmallVector<unsigned> multiDimNanoTileId =
        getMultiDimIndex<unsigned>(linearNanoTileId, tilesPerDim, order);
    SmallVector<unsigned> multiDimNanoTileElemId =
        getMultiDimIndex<unsigned>(linearNanoTileElemId, sizePerThread, order);
    for (unsigned k = 0; k < rank; ++k) {
      unsigned reorderedMultiDimId =
          multiDimNanoTileId[k] *
              (sizePerThread[k] * threadsPerWarp[k] * warpsPerCTA[k]) +
          multiDimNanoTileElemId[k];
      reorderedOffset[n].push_back(reorderedMultiDimId);
    }
  }
  return reorderedOffset;
}

// -----------------------------------------------------------------------
// Mma layout indices
// -----------------------------------------------------------------------

static SmallVector<Value>
emitBaseIndexWithinCTAForMmaLayoutV1(Location loc, RewriterBase &rewriter,
                                     const NvidiaMmaEncodingAttr &mmaLayout,
                                     RankedTensorType type) {
  auto shape = type.getShape();
  auto wpt = mmaLayout.getWarpsPerCTA();
  static constexpr std::array<int, 3> fpw{{2, 2, 1}};
  auto [isARow, isBRow, isAVec4, isBVec4, _] =
      mmaLayout.decodeVoltaLayoutStates();

  Value thread = getThreadId(rewriter, loc);
  auto *ctx = thread.getContext();
  Value _1 = i32_val(1);
  Value _2 = i32_val(2);
  Value _4 = i32_val(4);
  Value _16 = i32_val(16);
  Value _32 = i32_val(32);
  Value _fpw0 = i32_val(fpw[0]);
  Value _fpw1 = i32_val(fpw[1]);

  // A info
  auto aRep = mmaLayout.getMMAv1Rep(0);
  auto aSpw = mmaLayout.getMMAv1ShapePerWarp(0);
  // B info
  auto bSpw = mmaLayout.getMMAv1ShapePerWarp(1);
  auto bRep = mmaLayout.getMMAv1Rep(1);

  SmallVector<int, 2> rep({aRep[0], bRep[1]});
  SmallVector<int, 2> spw({aSpw[0], bSpw[1]});
  SmallVector<unsigned, 2> shapePerCTA({spw[0] * wpt[0], spw[1] * wpt[1]});

  Value lane = urem(thread, _32);
  Value warp = udiv(thread, _32);

  Value warp0 = urem(warp, i32_val(wpt[0]));
  Value warp12 = udiv(warp, i32_val(wpt[0]));
  Value warp1 = urem(warp12, i32_val(wpt[1]));

  // warp offset
  Value offWarpM = mul(warp0, i32_val(spw[0]));
  Value offWarpN = mul(warp1, i32_val(spw[1]));
  // quad offset
  Value offQuadM = mul(udiv(and_(lane, _16), _4), _fpw0);
  Value offQuadN = mul(udiv(and_(lane, _16), _4), _fpw1);
  // pair offset
  Value offPairM = udiv(urem(lane, _16), _4);
  offPairM = urem(offPairM, _fpw0);
  offPairM = mul(offPairM, _4);
  Value offPairN = udiv(urem(lane, _16), _4);
  offPairN = udiv(offPairN, _fpw0);
  offPairN = urem(offPairN, _fpw1);
  offPairN = mul(offPairN, _4);
  offPairM = mul(offPairM, i32_val(rep[0] / 2));
  offQuadM = mul(offQuadM, i32_val(rep[0] / 2));
  offPairN = mul(offPairN, i32_val(rep[1] / 2));
  offQuadN = mul(offQuadN, i32_val(rep[1] / 2));
  // quad pair offset
  Value offLaneM = add(offPairM, offQuadM);
  Value offLaneN = add(offPairN, offQuadN);
  // a, b offset
  Value offsetAM = add(offWarpM, offLaneM);
  Value offsetBN = add(offWarpN, offLaneN);
  // m indices
  Value offsetCM = add(and_(lane, _1), offsetAM);
  // n indices
  Value offsetCN = add((and_(lane, _2)), (add(offWarpN, offPairN)));
  return {offsetCM, offsetCN};
}

static SmallVector<SmallVector<unsigned>>
emitOffsetForMmaLayoutV1(const NvidiaMmaEncodingAttr &mmaLayout,
                         RankedTensorType type) {
  auto shape = type.getShape();

  auto [isARow, isBRow, isAVec4, isBVec4, _] =
      mmaLayout.decodeVoltaLayoutStates();

  // TODO: seems like the apttern below to get `rep`/`spw` appears quite often
  // A info
  auto aRep = mmaLayout.getMMAv1Rep(0);
  auto aSpw = mmaLayout.getMMAv1ShapePerWarp(0);
  // B info
  auto bSpw = mmaLayout.getMMAv1ShapePerWarp(1);
  auto bRep = mmaLayout.getMMAv1Rep(1);

  auto wpt = mmaLayout.getWarpsPerCTA();
  static constexpr std::array<int, 3> fpw{{2, 2, 1}};
  SmallVector<int, 2> rep({aRep[0], bRep[1]});
  SmallVector<int, 2> spw({aSpw[0], bSpw[1]});
  SmallVector<unsigned, 2> shapePerCTA({spw[0] * wpt[0], spw[1] * wpt[1]});

  SmallVector<unsigned> idxM;
  for (unsigned m = 0; m < shape[0]; m += shapePerCTA[0])
    for (unsigned mm = 0; mm < rep[0]; ++mm)
      idxM.push_back(m + mm * 2);

  SmallVector<unsigned> idxN;
  for (int n = 0; n < shape[1]; n += shapePerCTA[1]) {
    for (int nn = 0; nn < rep[1]; ++nn) {
      idxN.push_back(n + nn / 2 * 4 + (nn % 2) * 2 * fpw[1] * rep[1]);
      idxN.push_back(n + nn / 2 * 4 + (nn % 2) * 2 * fpw[1] * rep[1] + 1);
    }
  }

  SmallVector<SmallVector<unsigned>> ret;
  for (unsigned x1 : idxN) {   // N
    for (unsigned x0 : idxM) { // M
      SmallVector<unsigned> idx(2);
      idx[0] = x0; // M
      idx[1] = x1; // N
      ret.push_back(std::move(idx));
    }
  }
  return ret;
}

static SmallVector<SmallVector<unsigned>>
emitOffsetForMmaLayoutV2(const NvidiaMmaEncodingAttr &mmaLayout,
                         RankedTensorType type) {
  auto shape = type.getShape();
  auto shapePerCTA = getShapePerCTA(mmaLayout, shape);
  SmallVector<SmallVector<unsigned>> ret;

  auto rank = shape.size();
  for (unsigned i = 0; i < shapePerCTA[rank - 2];
       i += getShapePerCTATile(mmaLayout)[rank - 2]) {
    for (unsigned j = 0; j < shapePerCTA[rank - 1];
         j += getShapePerCTATile(mmaLayout)[rank - 1]) {
      if (rank == 3) {
        ret.push_back({0, i, j});
        ret.push_back({0, i, j + 1});
        ret.push_back({0, i + 8, j});
        ret.push_back({0, i + 8, j + 1});
      } else {
        ret.push_back({i, j});
        ret.push_back({i, j + 1});
        ret.push_back({i + 8, j});
        ret.push_back({i + 8, j + 1});
      }
    }
  }
  return ret;
}

static SmallVector<Value>
emitBaseIndexWithinCTAForMmaLayoutV2V3(Location loc, RewriterBase &rewriter,
                                       const NvidiaMmaEncodingAttr &mmaLayout,
                                       RankedTensorType type) {
  auto shape = type.getShape();
  auto _warpsPerCTA = mmaLayout.getWarpsPerCTA();
  auto rank = shape.size();
  assert(rank == 2 || rank == 3);
  auto order = triton::gpu::getOrder(mmaLayout);
  ArrayRef<unsigned int> instrShape = mmaLayout.getInstrShape();
  SmallVector<Value> warpsPerCTA;
  for (unsigned i = 0; i < rank; ++i)
    warpsPerCTA.push_back(i32_val(_warpsPerCTA[i]));
  auto shapePerCTA = getShapePerCTA(mmaLayout, shape);

  Value threadId = getThreadId(rewriter, loc);
  Value warpSize = i32_val(32);
  Value laneId = urem(threadId, warpSize);
  Value warpId = udiv(threadId, warpSize);

  uint32_t repM =
      (_warpsPerCTA[rank - 2] * instrShape[rank - 2]) / shapePerCTA[rank - 2];
  uint32_t repN =
      (_warpsPerCTA[rank - 1] * instrShape[rank - 1]) / shapePerCTA[rank - 1];

  uint32_t warpsM;
  if (repM > 1)
    warpsM = _warpsPerCTA[rank - 2] / repM;
  else
    warpsM = shape[rank - 2] / instrShape[rank - 2];

  uint32_t warpsN;
  if (repN > 1)
    warpsN = _warpsPerCTA[rank - 1] / repN;
  else
    warpsN = shape[rank - 1] / instrShape[rank - 1];

  SmallVector<Value> multiDimWarpId(rank);
  if (mmaLayout.isHopper()) {
    // TODO[goostavz]: the tiling order from CTA->warp level is different for
    // MMAv2/3. This is a workaround since we don't explicitly have warpGrp
    // level in the layout definition, and the tiling order of warpGrp->warp
    // must be fixed to meet the HW's needs. We may need to consider to
    // explicitly define warpGrpPerCTA for MMAv3 layout.
    assert(rank == 2 && "MMAv3 layout is does not support 3D tensor yet");
    multiDimWarpId[rank - 2] = urem(warpId, warpsPerCTA[rank - 2]);
    multiDimWarpId[rank - 1] =
        urem(udiv(warpId, warpsPerCTA[rank - 2]), warpsPerCTA[rank - 1]);
  } else {
    multiDimWarpId = delinearize(rewriter, loc, warpId, _warpsPerCTA, order);
  }
  Value warpIdM = urem(multiDimWarpId[rank - 2], i32_val(warpsM));
  Value warpIdN = urem(multiDimWarpId[rank - 1], i32_val(warpsN));

  Value offWarpM = mul(warpIdM, i32_val(instrShape[rank - 2]));
  Value offWarpN = mul(warpIdN, i32_val(instrShape[rank - 1]));

  SmallVector<Value> multiDimBase(rank);
  if (rank == 3)
    multiDimBase[0] = multiDimWarpId[0];
  multiDimBase[rank - 2] = add(udiv(laneId, i32_val(4)), offWarpM);
  multiDimBase[rank - 1] =
      add(mul(i32_val(2), urem(laneId, i32_val(4))), offWarpN);
  return multiDimBase;
}

static SmallVector<SmallVector<unsigned>>
emitOffsetForMmaLayoutV3(const NvidiaMmaEncodingAttr &mmaLayout,
                         RankedTensorType type) {
  auto shape = type.getShape();
  auto shapePerCTA = getShapePerCTA(mmaLayout, shape);
  SmallVector<SmallVector<unsigned>> ret;
  ArrayRef<unsigned int> instrShape = mmaLayout.getInstrShape();

  for (unsigned i = 0; i < shapePerCTA[0];
       i += getShapePerCTATile(mmaLayout)[0]) {
    for (unsigned j = 0; j < shapePerCTA[1];
         j += getShapePerCTATile(mmaLayout)[1]) {
      for (unsigned k = 0; k < instrShape[1]; k += 8) {
        ret.push_back({i, j + k});
        ret.push_back({i, j + k + 1});
        ret.push_back({i + 8, j + k});
        ret.push_back({i + 8, j + k + 1});
      }
    }
  }
  return ret;
}

static SmallVector<SmallVector<unsigned>>
emitOffsetForLayout(Attribute layout, RankedTensorType type);
static SmallVector<Value>
emitBaseIndexForDpasLayout(Location loc, RewriterBase &rewriter,
                           const DpasEncodingAttr &dpasLayout,
                           RankedTensorType type);

static SmallVector<SmallVector<unsigned>>
emitOffsetForSliceLayout(const SliceEncodingAttr &sliceLayout,
                         RankedTensorType type) {
  auto parentEncoding = sliceLayout.getParent();
  unsigned dim = sliceLayout.getDim();
  auto parentShape = sliceLayout.paddedShape(type.getShape());
  RankedTensorType parentTy =
      RankedTensorType::get(parentShape, type.getElementType(), parentEncoding);
  auto parentOffsets = emitOffsetForLayout(parentEncoding, parentTy);

  unsigned numOffsets = parentOffsets.size();
  SmallVector<SmallVector<unsigned>> resultOffsets;
  std::set<SmallVector<unsigned>> uniqueOffsets;

  for (unsigned i = 0; i < numOffsets; ++i) {
    SmallVector<unsigned> offsets = parentOffsets[i];
    offsets.erase(offsets.begin() + dim);
    if (uniqueOffsets.find(offsets) == uniqueOffsets.end()) {
      resultOffsets.push_back(offsets);
      uniqueOffsets.insert(offsets);
    }
  }
  return resultOffsets;
}

static void
emitOffsetForDpasLayoutPerCTA(const DpasEncodingAttr &dpasLayout,
                              SmallVector<SmallVector<unsigned>> &offsets,
                              unsigned ctaOffsetX, unsigned ctaOffsetY) {
  SmallVector<unsigned> sizePerThreads = getSizePerThread(dpasLayout);
  uint32_t elemsPerThreadPerGroup = product<unsigned>(sizePerThreads);
  uint32_t rowsPerWarp =
      dpasLayout.getSubGroupSize() / dpasLayout.getExecutionSize();
  SmallVector<unsigned> shapePerCTA =
      triton::gpu::getShapePerCTATile(dpasLayout);

  for (unsigned elem = 0; elem < elemsPerThreadPerGroup; elem++) {
    uint32_t elemRowIndex = (elem / sizePerThreads[1]) * rowsPerWarp;
    uint32_t elemColIndex = elem % sizePerThreads[1];
    offsets.push_back({ctaOffsetX + elemRowIndex, ctaOffsetY + elemColIndex});
  }
}

//

// -----------------------------------------------------------------------
// Get offsets / indices for any layout
// -----------------------------------------------------------------------

static SmallVector<Value> emitCTAOffsetForLayout(Location loc,
                                                 RewriterBase &rewriter,
                                                 Attribute layout,
                                                 ArrayRef<int64_t> shape) {
  unsigned rank = shape.size();
  SmallVector<unsigned> CTAsPerCGA = triton::gpu::getCTAsPerCGA(layout);
  SmallVector<unsigned> CTASplitNum = triton::gpu::getCTASplitNum(layout);
  SmallVector<unsigned> CTAOrder = triton::gpu::getCTAOrder(layout);
  SmallVector<int64_t> shapePerCTA =
      triton::gpu::getShapePerCTA(CTASplitNum, shape);

  // Delinearize clusterCTAId
  Value clusterCTAId = getClusterCTAId(rewriter, loc);
  SmallVector<Value> multiDimClusterCTAId =
      delinearize(rewriter, loc, clusterCTAId, CTAsPerCGA, CTAOrder);

  // CTA Wrapping
  for (unsigned i = 0; i < rank; ++i) {
    // This wrapping rule must be consistent with getShapePerCTA
    unsigned splitNum = std::min<unsigned>(shape[i], CTASplitNum[i]);
    multiDimClusterCTAId[i] = urem(multiDimClusterCTAId[i], i32_val(splitNum));
  }

  SmallVector<Value> CTAOffset(rank);
  for (unsigned i = 0; i < rank; ++i)
    CTAOffset[i] = mul(multiDimClusterCTAId[i], i32_val(shapePerCTA[i]));

  return CTAOffset;
}

static SmallVector<Value>
emitBaseIndexForLayout(Location loc, RewriterBase &rewriter, Attribute layout,
                       RankedTensorType type, bool withCTAOffset) {
  auto shape = type.getShape();

  SmallVector<Value> baseIndex;
  ConversionPatternRewriter::InsertionGuard guard(rewriter);
  SmallVector<Value> result;
  if (auto blockedLayout = layout.dyn_cast<BlockedEncodingAttr>()) {
    result = emitBaseIndexWithinCTAForBlockedLayout(loc, rewriter,
                                                    blockedLayout, type);
  } else if (auto mmaLayout = layout.dyn_cast<NvidiaMmaEncodingAttr>()) {
    if (mmaLayout.isVolta())
      result =
          emitBaseIndexWithinCTAForMmaLayoutV1(loc, rewriter, mmaLayout, type);
    if (mmaLayout.isAmpere() || mmaLayout.isHopper())
      result = emitBaseIndexWithinCTAForMmaLayoutV2V3(loc, rewriter, mmaLayout,
                                                      type);
  } else if (auto dpasLayout = layout.dyn_cast<DpasEncodingAttr>()) {
    result = emitBaseIndexForDpasLayout(loc, rewriter, dpasLayout, type);
  } else if (auto sliceLayout = layout.dyn_cast<SliceEncodingAttr>()) {
    auto parentLayout = sliceLayout.getParent();
    auto parentShape = sliceLayout.paddedShape(type.getShape());
    RankedTensorType parentTy =
        RankedTensorType::get(parentShape, type.getElementType(), parentLayout);
    result = emitBaseIndexForLayout(loc, rewriter, parentLayout, parentTy,
                                    withCTAOffset);
    result.erase(result.begin() + sliceLayout.getDim());
    // CTAOffset has been added in emitBaseIndexForLayout of parentLayout
    return result;
  } else {
    llvm_unreachable("unsupported emitBaseIndexForLayout");
  }
  if (withCTAOffset) {
    auto CTAOffset = emitCTAOffsetForLayout(loc, rewriter, layout, shape);
    assert(CTAOffset.size() == result.size() && "Rank mismatch");
    for (unsigned k = 0; k < result.size(); ++k)
      result[k] = add(result[k], CTAOffset[k]);
  }
  return result;
}

static SmallVector<SmallVector<unsigned>>
emitOffsetForDpasLayout(const DpasEncodingAttr &dpasLayout,
                        RankedTensorType type) {
  ArrayRef<int64_t> shape = type.getShape();
  SmallVector<SmallVector<unsigned>> offsets;
  SmallVector<unsigned> shapePerCTA = getShapePerCTATile(dpasLayout);

  for (unsigned i = 0; i < shape[0]; i += shapePerCTA[0]) {
    for (unsigned j = 0; j < shape[1]; j += shapePerCTA[1]) {
      emitOffsetForDpasLayoutPerCTA(dpasLayout, offsets, i, j);
    }
  }

  return offsets;
}

static SmallVector<SmallVector<unsigned>>
emitOffsetForLayout(Attribute layout, RankedTensorType type) {
  if (auto blockedLayout = layout.dyn_cast<BlockedEncodingAttr>())
    return emitOffsetForBlockedLayout(blockedLayout, type);
  if (auto mmaLayout = layout.dyn_cast<NvidiaMmaEncodingAttr>()) {
    if (mmaLayout.isVolta())
      return emitOffsetForMmaLayoutV1(mmaLayout, type);
    if (mmaLayout.isAmpere())
      return emitOffsetForMmaLayoutV2(mmaLayout, type);
    if (mmaLayout.isHopper())
      return emitOffsetForMmaLayoutV3(mmaLayout, type);
  }
  if (auto dpasLayout = layout.dyn_cast<DpasEncodingAttr>()) {
    return emitOffsetForDpasLayout(dpasLayout, type);
  }
  if (auto sliceLayout = layout.dyn_cast<SliceEncodingAttr>())
    return emitOffsetForSliceLayout(sliceLayout, type);
  llvm_unreachable("unsupported emitOffsetForLayout");
}

// -----------------------------------------------------------------------
// Dpas layout indices
// -----------------------------------------------------------------------

static SmallVector<Value>
emitBaseIndexForDpasLayout(Location loc, RewriterBase &rewriter,
                           const DpasEncodingAttr &dpasLayout,
                           RankedTensorType type) {
  Value threadId = getThreadId(rewriter, loc);
  Value warpSize = i32_val(triton::gpu::getWarpSize(dpasLayout));
  Value warpId = udiv(threadId, warpSize);
  Value laneId = urem(threadId, warpSize);

  auto warpsPerCTA = dpasLayout.getWarpsPerCTA();
  ArrayRef<int64_t> shape = type.getShape();

  auto order = triton::gpu::getOrder(dpasLayout);
  SmallVector<Value> multiDimWarpId =
      delinearize(rewriter, loc, warpId, warpsPerCTA, order);

  // Compute the 2-dim coordinates of the warp containing the tensor element
  // operated on by this thread.
  SmallVector<unsigned> warpShape = dpasLayout.getShapeC();
  Value rowWarpId =
      urem(multiDimWarpId[0], i32_val(std::ceil(shape[0] / warpShape[0])));
  Value colWarpId =
      urem(multiDimWarpId[1], i32_val(std::ceil(shape[1] / warpShape[1])));
  Value rowWarpOffset = mul(rowWarpId, i32_val(warpShape[0]));
  Value colWarpOffset = mul(colWarpId, i32_val(warpShape[1]));

  // Compute the 2-dim coordinates of the first element in the warp operated
  // on by this thread.
  SmallVector<unsigned> threadsPerWarp = getThreadsPerWarp(dpasLayout);
  SmallVector<Value> multiDimBase = {
      add(udiv(laneId, i32_val(threadsPerWarp[1])), rowWarpOffset),
      add(urem(laneId, i32_val(threadsPerWarp[1])), colWarpOffset)};
  return multiDimBase;
}

// Emit indices calculation within each ConversionPattern, and returns a
// [elemsPerThread X rank] index matrix.
static SmallVector<SmallVector<Value>>
emitIndices(Location loc, RewriterBase &rewriter, Attribute layout,
            RankedTensorType type, bool withCTAOffset) {
  // step 1, delinearize threadId to get the base index
  auto multiDimBase =
      emitBaseIndexForLayout(loc, rewriter, layout, type, withCTAOffset);
  // step 2, get offset of each element
  auto offset = emitOffsetForLayout(layout, type);
  // step 3, add offset to base, and reorder the sequence
  // of indices to guarantee that elems in the same
  // sizePerThread are adjacent in order
  auto shape = type.getShape();
  unsigned rank = shape.size();
  unsigned elemsPerThread = offset.size();
  SmallVector<SmallVector<Value>> multiDimIdx(elemsPerThread,
                                              SmallVector<Value>(rank));
  for (unsigned n = 0; n < elemsPerThread; ++n)
    for (unsigned k = 0; k < rank; ++k)
      multiDimIdx[n][k] = add(multiDimBase[k], i32_val(offset[n][k]));
  return multiDimIdx;
}

/* ---------------- */
/* ---------------- */
DenseMap<unsigned, Value> static getSwizzledSharedPtrs(
    Location loc, unsigned inVec, RankedTensorType srcTy,
    triton::gpu::SharedEncodingAttr resSharedLayout, Type resElemTy,
    SharedMemoryObject smemObj, RewriterBase &rewriter,
    SmallVectorImpl<Value> &offsetVals, SmallVectorImpl<Value> &srcStrides) {
  // This utility computes the pointers for accessing the provided swizzled
  // shared memory layout `resSharedLayout`. More specifically, it computes,
  // for all indices (row, col) of `srcEncoding` such that idx % inVec = 0,
  // the pointer: ptr[(row, col)] = base + (rowOff * strides[ord[1]] +
  // colOff) where :
  //   phase = (row // perPhase) % maxPhase
  //   rowOff = row
  //   colOff = colOffSwizzled + colOffOrdered
  //     colOffSwizzled = ((col // outVec) ^ phase) * outVec
  //     colOffOrdered = (col % outVec) // minVec * minVec
  //
  // Note 1:
  // -------
  // Because swizzling happens at a granularity of outVec, we need to
  // decompose the offset into a swizzled factor and a non-swizzled
  // (ordered) factor
  //
  // Note 2:
  // -------
  // If we have x, y, z of the form:
  // x = 0b00000xxxx
  // y = 0byyyyy0000
  // z = 0b00000zzzz
  // then (x + y) XOR z = 0byyyyxxxx XOR 0b00000zzzz = (x XOR z) + y
  // This means that we can use some immediate offsets for shared memory
  // operations.
  auto dstPtrTy = ptr_ty(rewriter.getContext(), 3);
  auto dstOffset = dot(rewriter, loc, offsetVals, smemObj.strides);
  Value dstPtrBase = gep(dstPtrTy, resElemTy, smemObj.base, dstOffset);

  auto srcEncoding = srcTy.getEncoding();
  auto srcShape = srcTy.getShape();
  auto srcShapePerCTA = triton::gpu::getShapePerCTA(srcTy);
  unsigned numElems = triton::gpu::getTotalElemsPerThread(srcTy);
  // swizzling params as described in TritonGPUAttrDefs.td
  unsigned outVec = resSharedLayout.getVec();
  unsigned perPhase = resSharedLayout.getPerPhase();
  unsigned maxPhase = resSharedLayout.getMaxPhase();
  // Order
  auto inOrder = triton::gpu::getOrder(srcEncoding);
  auto outOrder = triton::gpu::getOrder(resSharedLayout);
  assert(maxPhase == 1 ||
         outVec * maxPhase <= srcShape[outOrder[0]] &&
             "Swizzling would generate out of bounds memory accesses");
  // Tensor indices held by the current thread, as LLVM values
  auto srcIndices = emitIndices(loc, rewriter, srcEncoding, srcTy, false);
  // Swizzling with leading offsets (e.g. Hopper GMMA)
  unsigned swizzlingByteWidth = 0;
  if (resSharedLayout.getHasLeadingOffset()) {
    if (perPhase == 4 && maxPhase == 2)
      swizzlingByteWidth = 32;
    else if (perPhase == 2 && maxPhase == 4)
      swizzlingByteWidth = 64;
    else if (perPhase == 1 && maxPhase == 8)
      swizzlingByteWidth = 128;
    else
      llvm::report_fatal_error("Unsupported shared layout.");
  }
  unsigned numElemsPerSwizzlingRow =
      swizzlingByteWidth * 8 / resElemTy.getIntOrFloatBitWidth();
  Value numElemsPerSwizzlingRowVal = i32_val(numElemsPerSwizzlingRow);
  unsigned leadingDimOffset;
  if (outOrder.size() >= 2) {
    leadingDimOffset = numElemsPerSwizzlingRow * srcShapePerCTA[outOrder[1]];
  } else {
    leadingDimOffset = numElemsPerSwizzlingRow;
  }

  Value leadingDimOffsetVal = i32_val(leadingDimOffset);
  // Return values
  DenseMap<unsigned, Value> ret;
  // cache for non-immediate offsets
  DenseMap<unsigned, Value> cacheCol, cacheRow;
  unsigned minVec = std::min(outVec, inVec);
  for (unsigned elemIdx = 0; elemIdx < numElems; elemIdx += minVec) {
    Value offset = i32_val(0);
    // Extract multi dimensional index for current element
    auto idx = srcIndices[elemIdx];
    Value idxCol = idx[outOrder[0]]; // contiguous dimension
    Value idxRow, strideRow;
    if (outOrder.size() >= 2) {
      idxRow = idx[outOrder[1]]; // discontiguous dimension
      strideRow = srcStrides[outOrder[1]];
    } else {
      idxRow = i32_val(0);
      strideRow = i32_val(0);
    }
    Value strideCol = srcStrides[outOrder[0]];
    // compute phase = (row // perPhase) % maxPhase
    Value phase = urem(udiv(idxRow, i32_val(perPhase)), i32_val(maxPhase));
    // extract dynamic/static offset for immediate offsetting
    unsigned immedateOffCol = 0;
    unsigned immedateOffRow = 0;
    if (leadingDimOffset) {
      // hopper
      offset =
          mul(udiv(idxCol, numElemsPerSwizzlingRowVal), leadingDimOffsetVal);
      // Shrink by swizzling blocks
      idxCol = urem(idxCol, numElemsPerSwizzlingRowVal);
      strideRow = numElemsPerSwizzlingRowVal;
    }
    if (auto add = dyn_cast_or_null<LLVM::AddOp>(idxCol.getDefiningOp())) {
      if (auto _cst = dyn_cast_or_null<LLVM::ConstantOp>(
              add.getRhs().getDefiningOp())) {
        unsigned cst =
            _cst.getValue().cast<IntegerAttr>().getValue().getSExtValue();
        unsigned key = cst % (outVec * maxPhase);
        cacheCol.insert({key, idxCol});
        idxCol = cacheCol[key];
        immedateOffCol = cst / (outVec * maxPhase) * (outVec * maxPhase);
      }
    }
    if (auto add = dyn_cast_or_null<LLVM::AddOp>(idxRow.getDefiningOp())) {
      if (auto _cst = dyn_cast_or_null<LLVM::ConstantOp>(
              add.getRhs().getDefiningOp())) {
        unsigned cst =
            _cst.getValue().cast<IntegerAttr>().getValue().getSExtValue();
        unsigned key = cst % (perPhase * maxPhase);
        cacheRow.insert({key, idxRow});
        idxRow = cacheRow[key];
        immedateOffRow = cst / (perPhase * maxPhase) * (perPhase * maxPhase);
      }
    }
    // row offset is simply row index
    Value rowOff = mul(idxRow, strideRow);
    // because swizzling happens at a granularity of outVec, we need to
    // decompose the offset into a swizzled factor and a non-swizzled
    // (ordered) factor: colOffSwizzled = ((col // outVec) ^ phase) * outVec
    // colOffOrdered = (col % outVec) // minVec * minVec
    Value colOffSwizzled = xor_(udiv(idxCol, i32_val(outVec)), phase);
    colOffSwizzled = mul(colOffSwizzled, i32_val(outVec));
    Value colOffOrdered = urem(idxCol, i32_val(outVec));
    colOffOrdered = udiv(colOffOrdered, i32_val(minVec));
    colOffOrdered = mul(colOffOrdered, i32_val(minVec));
    Value colOff = add(colOffSwizzled, colOffOrdered);
    // compute non-immediate offset
    if (outOrder.size() == 3)
      offset = add(offset, mul(idx[outOrder[2]], srcStrides[outOrder[2]]));
    offset = add(offset, add(rowOff, mul(colOff, strideCol)));
    Value currPtr = gep(dstPtrTy, resElemTy, dstPtrBase, offset);
    // compute immediate offset
    Value immediateOff;
    if (outOrder.size() >= 2) {
      immediateOff =
          add(mul(i32_val(immedateOffRow), strideRow), i32_val(immedateOffCol));
    } else {
      immediateOff = i32_val(immedateOffCol);
    }

    ret[elemIdx] = gep(dstPtrTy, resElemTy, currPtr, immediateOff);
  }
  return ret;
}

static SmallVector<Value>
loadSharedToDistributed(Value dst, ArrayRef<SmallVector<Value>> dstIndices,
                        Value src, SharedMemoryObject smemObj, Type elemTy,
                        Location loc, ConversionPatternRewriter &rewriter) {
  auto dstTy = dst.getType().cast<RankedTensorType>();
  auto dstShape = dstTy.getShape();
  assert(dstShape.size() <= 2 && "Unexpected rank of loadSharedToDistributed");
  auto srcTy = src.getType().cast<MemDescType>();
  auto dstDistributedLayout = dstTy.getEncoding();
  if (auto mmaLayout = dstDistributedLayout.dyn_cast<NvidiaMmaEncodingAttr>()) {
    assert((!mmaLayout.isVolta()) &&
           "ConvertLayout Shared->MMAv1 is not supported yet");
  }
  auto srcSharedLayout =
      srcTy.getEncoding().cast<triton::gpu::SharedEncodingAttr>();
  auto srcElemTy = srcTy.getElementType();
  auto dstElemTy = dstTy.getElementType();
  auto inOrd = triton::gpu::getOrder(srcSharedLayout);
  auto outOrd = triton::gpu::getOrder(dstDistributedLayout);
  unsigned outVec = inOrd == outOrd
                        ? triton::gpu::getUniqueContigPerThread(
                              dstDistributedLayout, dstShape)[outOrd[0]]
                        : 1;
  unsigned inVec = srcSharedLayout.getVec();
  unsigned minVec = std::min(outVec, inVec);
  unsigned outElems = triton::gpu::getTotalElemsPerThread(dstTy);
  SmallVector<Value> offsetVals = {smemObj.strides.size(), i32_val(0)};
  assert(outElems == dstIndices.size());

  DenseMap<unsigned, Value> sharedPtrs =
      getSwizzledSharedPtrs(loc, outVec, dstTy, srcSharedLayout, elemTy,
                            smemObj, rewriter, offsetVals, smemObj.strides);
  assert(outElems % minVec == 0 && "Unexpected number of elements");
  unsigned numVecs = outElems / minVec;
  auto wordTy = vec_ty(elemTy, minVec);
  SmallVector<Value> outVals(outElems);
  for (unsigned i = 0; i < numVecs; ++i) {
    Value smemAddr = sharedPtrs[i * minVec];
    smemAddr = bitcast(smemAddr, ptr_ty(rewriter.getContext(), 3));
    Value valVec = load(wordTy, smemAddr);
    for (unsigned v = 0; v < minVec; ++v) {
      Value currVal = extract_element(dstElemTy, valVec, i32_val(v));
      outVals[i * minVec + v] = currVal;
    }
  }
  return outVals;
}

static void storeDistributedToShared(Value src, ArrayRef<Value> inVals,
                                     ArrayRef<Value> dstStrides,
                                     ArrayRef<SmallVector<Value>> srcIndices,
                                     Value dst, Value smemBase, Type elemTy,
                                     Location loc,
                                     ConversionPatternRewriter &rewriter) {
  auto srcTy = src.getType().cast<RankedTensorType>();
  auto srcShape = srcTy.getShape();
  auto rank = srcShape.size();
  assert(rank == 2 ||
         rank == 3 && "Unexpected rank of storeDistributedToShared");
  auto dstTy = dst.getType().cast<MemDescType>();
  auto srcDistributedLayout = srcTy.getEncoding();
  if (auto mmaLayout = srcDistributedLayout.dyn_cast<NvidiaMmaEncodingAttr>()) {
    assert((!mmaLayout.isVolta()) &&
           "ConvertLayout MMAv1->Shared is not supported yet");
  }
  auto dstSharedLayout =
      dstTy.getEncoding().cast<triton::gpu::SharedEncodingAttr>();
  auto dstElemTy = dstTy.getElementType();
  auto inOrd = triton::gpu::getOrder(srcDistributedLayout);
  auto outOrd = dstSharedLayout.getOrder();
  unsigned inVec = inOrd == outOrd
                       ? triton::gpu::getUniqueContigPerThread(
                             srcDistributedLayout, srcShape)[inOrd[0]]
                       : 1;
  unsigned outVec = dstSharedLayout.getVec();
  unsigned minVec = std::min(outVec, inVec);
  unsigned numElems = triton::gpu::getTotalElemsPerThread(srcTy);
  assert(numElems == srcIndices.size());
  auto wordTy = vec_ty(elemTy, minVec);
  Value word;

  SmallVector<Value, 3> srcStrides(dstStrides);
  SmallVector<Value, 3> offsetVals(rank, i32_val(0));
  SharedMemoryObject smemObj(smemBase, elemTy, srcStrides, offsetVals);

  DenseMap<unsigned, Value> sharedPtrs =
      getSwizzledSharedPtrs(loc, inVec, srcTy, dstSharedLayout, elemTy, smemObj,
                            rewriter, offsetVals, srcStrides);

  for (unsigned i = 0; i < numElems; ++i) {
    if (i % minVec == 0)
      word = undef(wordTy);
    word = insert_element(wordTy, word, inVals[i], i32_val(i % minVec));
    if (i % minVec == minVec - 1) {
      Value smemAddr = sharedPtrs[i / minVec * minVec];
      smemAddr = bitcast(smemAddr, ptr_ty(rewriter.getContext(), 3));
      store(word, smemAddr);
    }
  }
}

static Value
getStructFromSharedMemoryObject(Location loc, const SharedMemoryObject &smemObj,
                                ConversionPatternRewriter &rewriter) {
  auto elems = smemObj.getElems();
  auto types = smemObj.getTypes();
  auto structTy =
      LLVM::LLVMStructType::getLiteral(rewriter.getContext(), types);
  // pack into struct
  Value llvmStruct = rewriter.create<LLVM::UndefOp>(loc, structTy);
  for (const auto &v : llvm::enumerate(elems)) {
    assert(v.value() && "can not insert null values");
    llvmStruct = insert_val(structTy, llvmStruct, v.value(), v.index());
  }
  return llvmStruct;
}

static SmallVector<Value>
unpackLLElements(Location loc, Value llvmStruct,
                 ConversionPatternRewriter &rewriter) {
  assert(bool(llvmStruct) && "can not unpack null values");
  if (llvmStruct.getType().isIntOrIndexOrFloat() ||
      llvmStruct.getType().isa<triton::PointerType>() ||
      llvmStruct.getType().isa<LLVM::LLVMPointerType>())
    return {llvmStruct};
  ArrayRef<Type> types =
      llvmStruct.getType().cast<LLVM::LLVMStructType>().getBody();
  SmallVector<Value> results(types.size());
  for (unsigned i = 0; i < types.size(); ++i) {
    Type type = types[i];
    results[i] = extract_val(type, llvmStruct, i);
  }
  return results;
}

static Value packLLElements(Location loc,
                            const LLVMTypeConverter *typeConverter,
                            ValueRange resultVals,
                            ConversionPatternRewriter &rewriter, Type type) {
  auto structType =
      typeConverter->convertType(type).dyn_cast<LLVM::LLVMStructType>();
  if (!structType) {
    assert(resultVals.size() == 1);
    return *resultVals.begin();
  }

  auto elementTypes = structType.getBody();
  if (elementTypes.size() != resultVals.size()) {
    emitError(loc) << " size mismatch when packing elements for LLVM struct"
                   << " expected " << elementTypes.size() << " but got "
                   << resultVals.size();
  }
  Value llvmStruct = rewriter.create<LLVM::UndefOp>(loc, structType);
  for (const auto &v : llvm::enumerate(resultVals)) {
    if (!v.value()) {
      emitError(loc)
          << "cannot insert null values into struct, but tried to insert"
          << v.value();
    }
    if (v.value().getType() != elementTypes[v.index()]) {
      emitError(loc) << "invalid element type in packLLEElements. Expected "
                     << elementTypes[v.index()] << " but got "
                     << v.value().getType();
    }
    llvmStruct = insert_val(structType, llvmStruct, v.value(), v.index());
  }
  return llvmStruct;
}

} // namespace mlir

#endif
