#include <llove/builder.hpp>

llvm::Type *llove::Builder::GetVoidType()
{
    return llvm::Type::getVoidTy(m_LLVMContext);
}

llvm::IntegerType *llove::Builder::GetIntegerType(const unsigned bits)
{
    return llvm::IntegerType::get(m_LLVMContext, bits);
}

llvm::IntegerType *llove::Builder::GetPointerSizeType()
{
    return GetDataLayout().getIntPtrType(m_LLVMContext);
}

llvm::Type *llove::Builder::GetFloatType(const unsigned bits)
{
    switch (bits)
    {
    case 16:
        return llvm::Type::getHalfTy(m_LLVMContext);
    case 32:
        return llvm::Type::getFloatTy(m_LLVMContext);
    case 64:
        return llvm::Type::getDoubleTy(m_LLVMContext);
    default:
        Error("bits must be either 16, 32 or 64");
    }
}

llvm::ArrayType *llove::Builder::GetArrayType(llvm::Type *base, const unsigned size)
{
    Assert(base != nullptr, "base must not be null");

    (void) m_LLVMContext;
    return llvm::ArrayType::get(base, size);
}

llvm::PointerType *llove::Builder::GetPointerType()
{
    return llvm::PointerType::getUnqual(m_LLVMContext);
}

llvm::FunctionType *llove::Builder::GetFunctionType(llvm::Type *result, const std::vector<llvm::Type *> &parameters)
{
    Assert(result != nullptr, "result must not be null");

    (void) m_LLVMContext;
    return llvm::FunctionType::get(result, parameters, false);
}

llvm::StructType *llove::Builder::GetStructType(const std::vector<llvm::Type *> &fields, const bool packed)
{
    Assert(!fields.empty(), "fields must not be empty");

    return llvm::StructType::get(m_LLVMContext, fields, packed);
}

llvm::StructType *llove::Builder::GetNamedStructType(const std::string &name)
{
    Assert(!name.empty(), "name must not be empty");

    return llvm::StructType::getTypeByName(m_LLVMContext, name);
}

llvm::StructType *llove::Builder::GetOrCreateNamedStructType(const std::string &name)
{
    Assert(!name.empty(), "name must not be empty");

    if (const auto type = llvm::StructType::getTypeByName(m_LLVMContext, name))
        return type;

    return llvm::StructType::create(m_LLVMContext, name);
}

llvm::StructType *llove::Builder::GetOrCreateNamedStructType(
    const std::string &name,
    const std::vector<llvm::Type *> &fields,
    const bool packed)
{
    Assert(!name.empty(), "name must not be empty");
    Assert(!fields.empty(), "fields must not be empty");

    if (const auto type = llvm::StructType::getTypeByName(m_LLVMContext, name))
    {
        type->setBody(fields, packed);
        return type;
    }

    return llvm::StructType::create(m_LLVMContext, fields, name, packed);
}

llvm::StructType *llove::Builder::GetVariadicType()
{
    return GetOrCreateNamedStructType("variadic", { GetIntegerType(32), GetPointerType() }, false);
}
