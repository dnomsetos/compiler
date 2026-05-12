#include <ir_builder/builder.hpp>
#include <ir_builder/ir_utilities.hpp>
#include <semantic_analysis/type_storage.hpp>

llvm::Type* get_llvm_type(TypeStore& type_store, llvm::IRBuilder<>& builder,
                          llvm::LLVMContext& context, tp::TypeId type_id) {
  auto& tp_type = type_store.get_type(type_id);

  return std::visit(
      [&](auto&& type) -> llvm::Type* {
        using T = std::decay_t<decltype(type)>;

        if constexpr (std::is_same_v<T, tp::I8>) {
          return builder.getInt8Ty();
        } else if constexpr (std::is_same_v<T, tp::I16>) {
          return builder.getInt16Ty();
        } else if constexpr (std::is_same_v<T, tp::I32>) {
          return builder.getInt32Ty();
        } else if constexpr (std::is_same_v<T, tp::I64>) {
          return builder.getInt64Ty();
        } else if constexpr (std::is_same_v<T, tp::U8>) {
          return builder.getInt8Ty();
        } else if constexpr (std::is_same_v<T, tp::U16>) {
          return builder.getInt16Ty();
        } else if constexpr (std::is_same_v<T, tp::U32>) {
          return builder.getInt32Ty();
        } else if constexpr (std::is_same_v<T, tp::U64>) {
          return builder.getInt64Ty();
        } else if constexpr (std::is_same_v<T, tp::F32>) {
          return builder.getFloatTy();
        } else if constexpr (std::is_same_v<T, tp::F64>) {
          return builder.getDoubleTy();
        } else if constexpr (std::is_same_v<T, tp::Bool>) {
          return builder.getInt1Ty();
        } else if constexpr (std::is_same_v<T, tp::Char>) {
          return builder.getInt8Ty();
        } else if constexpr (std::is_same_v<T, tp::Void>) {
          return llvm::StructType::get(context, {});
        } else if constexpr (std::is_same_v<T, tp::FunctionType>) {
          auto* ret =
              get_llvm_type(type_store, builder, context, type.return_type);
          std::vector<llvm::Type*> params;
          for (auto a : type.args) {
            params.push_back(get_llvm_type(type_store, builder, context, a));
          }
          return llvm::FunctionType::get(ret, params, false);
        }

        throw std::runtime_error("Unknown type");
      },
      tp_type.type);
}
