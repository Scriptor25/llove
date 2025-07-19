#include <llove/builder.hpp>

llvm::Type *llove::Builder::GetVoidType()
{
    return llvm::Type::getVoidTy(m_Context);
}

llvm::IntegerType *llove::Builder::GetIntType(const unsigned bits)
{
    return llvm::IntegerType::get(m_Context, bits);
}

llvm::Type *llove::Builder::GetFltType(const unsigned bits)
{
    switch (bits)
    {
    case 16:
        return llvm::Type::getHalfTy(m_Context);
    case 32:
        return llvm::Type::getFloatTy(m_Context);
    case 64:
        return llvm::Type::getDoubleTy(m_Context);
    default:
        return nullptr;
    }
}

llvm::ArrayType *llove::Builder::GetArrayType(llvm::Type *base, const unsigned size)
{
    (void) m_Context;
    return llvm::ArrayType::get(base, size);
}

llvm::PointerType *llove::Builder::GetPointerType()
{
    return llvm::PointerType::getUnqual(m_Context);
}

llvm::PointerType *llove::Builder::GetPointerType(llvm::Type *base)
{
    (void) m_Context;
    return llvm::PointerType::getUnqual(base);
}

llvm::StructType *llove::Builder::GetStructType(const std::vector<llvm::Type *> &fields, const bool packed)
{
    return llvm::StructType::get(m_Context, fields, packed);
}

llvm::FunctionType *llove::Builder::GetFunctionType(
    llvm::Type *result,
    const std::vector<llvm::Type *> &parameters,
    const bool vararg)
{
    (void) m_Context;
    return llvm::FunctionType::get(result, parameters, vararg);
}

llvm::StructType *llove::Builder::GetNamedStructType(const std::string &name)
{
    return llvm::StructType::getTypeByName(m_Context, name);
}

llvm::StructType *llove::Builder::GetOrCreateNamedStructType(const std::string &name)
{
    if (const auto type = llvm::StructType::getTypeByName(m_Context, name))
        return type;
    return llvm::StructType::create(m_Context, name);
}

llvm::StructType *llove::Builder::GetOrCreateNamedStructType(
    const std::string &name,
    const std::vector<llvm::Type *> &fields,
    const bool packed)
{
    if (const auto type = llvm::StructType::getTypeByName(m_Context, name))
    {
        type->setBody(fields, packed);
        return type;
    }
    return llvm::StructType::create(m_Context, fields, name, packed);
}
