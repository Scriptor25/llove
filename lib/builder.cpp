#include <llove/builder.hpp>
#include <llove/type.hpp>

llove::Builder::Builder(Context &types)
    : m_Types(types),
      m_Builder(m_Context),
      m_Module("main", m_Context)
{
}

llove::Context &llove::Builder::GetTypes() const
{
    return m_Types;
}

llvm::Type *llove::Builder::GetVoidType()
{
    return m_Builder.getVoidTy();
}

llvm::IntegerType *llove::Builder::GetIntType(const unsigned bits)
{
    return m_Builder.getIntNTy(bits);
}

llvm::Type *llove::Builder::GetFltType(const unsigned bits)
{
    switch (bits)
    {
    case 16:
        return m_Builder.getHalfTy();
    case 32:
        return m_Builder.getFloatTy();
    case 64:
        return m_Builder.getDoubleTy();
    default:
        return nullptr;
    }
}

llvm::ArrayType *llove::Builder::GetArrayType(llvm::Type *base, const unsigned size)
{
    return llvm::ArrayType::get(base, size);
}

llvm::PointerType *llove::Builder::GetPtrType(llvm::Type *base)
{
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
    return llvm::FunctionType::get(result, parameters, vararg);
}

llvm::StructType *llove::Builder::GetNamedStructType(const std::string_view &name)
{
    return llvm::StructType::getTypeByName(m_Context, name);
}

llvm::StructType *llove::Builder::GetOrCreateNamedStructType(const std::string_view &name)
{
    if (const auto type = llvm::StructType::getTypeByName(m_Context, name))
        return type;
    return llvm::StructType::create(m_Context, name);
}

llvm::StructType *llove::Builder::GetOrCreateNamedStructType(
    const std::string_view &name,
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

llvm::Value *llove::Builder::CreateLoad(llvm::Value *pointer, const TypePtr &type)
{
    return m_Builder.CreateLoad(type->Gen(*this), pointer);
}

llvm::Value *llove::Builder::CreateStore(llvm::Value *pointer, llvm::Value *value, const bool volatile_)
{
    return m_Builder.CreateStore(value, pointer, volatile_);
}

llvm::Function *llove::Builder::CreateFunction(
    const std::string_view &name,
    const FunctionType::Ptr &type,
    const bool external)
{
    return llvm::Function::Create(
        type->Gen(*this),
        external ? llvm::Function::ExternalLinkage : llvm::Function::InternalLinkage,
        name,
        m_Module);
}
