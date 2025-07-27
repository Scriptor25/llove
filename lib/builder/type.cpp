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

llvm::DIType *llove::Builder::GetDbgVoidType()
{
    return m_DIBuilder.createUnspecifiedType("void");
}

llvm::DIType *llove::Builder::GetDbgIntType(const bool sign, const unsigned bits)
{
    return m_DIBuilder.createBasicType(
        (sign ? 'i' : 'u') + std::to_string(bits),
        bits,
        sign ? llvm::dwarf::DW_ATE_signed : llvm::dwarf::DW_ATE_unsigned);
}

llvm::DIType *llove::Builder::GetDbgFltType(const unsigned bits)
{
    return m_DIBuilder.createBasicType('f' + std::to_string(bits), bits, llvm::dwarf::DW_ATE_float);
}

llvm::DIType *llove::Builder::GetDbgPointerType()
{
    return m_DIBuilder.createPointerType(GetDbgVoidType(), 64);
}

llvm::DIType *llove::Builder::GetDbgPointerType(llvm::DIType *base)
{
    return m_DIBuilder.createPointerType(base, 64);
}

llvm::DIType *llove::Builder::GetDbgArrayType(llvm::DIType *base, const unsigned size)
{
    return m_DIBuilder.createArrayType(size, 0, base, {});
}

llvm::DIType *llove::Builder::GetDbgStructType(const std::vector<llvm::Metadata *> &fields, const unsigned size)
{
    return m_DIBuilder.createStructType(
        nullptr,
        {},
        nullptr,
        0u,
        size,
        0u,
        llvm::DINode::FlagZero,
        nullptr,
        m_DIBuilder.getOrCreateArray(fields));
}

llvm::DIType *llove::Builder::GetDbgFieldType(
    const std::string &name,
    llvm::DIType *type,
    const unsigned size,
    const unsigned offset)
{
    return m_DIBuilder.createMemberType(
        nullptr,
        name,
        nullptr,
        0u,
        size,
        0u,
        offset,
        llvm::DINode::FlagZero,
        type);
}

llvm::DIType *llove::Builder::GetDbgClassType(const std::string &name)
{
    return m_DIBuilder.createClassType(
        nullptr,
        name,
        nullptr,
        0u,
        0u,
        0u,
        0u,
        llvm::DINode::FlagZero,
        nullptr,
        {});
}

llvm::DIType *llove::Builder::GetDbgClassType(const std::string &name, const std::vector<llvm::Metadata *> &fields)
{
    return m_DIBuilder.createClassType(
        nullptr,
        name,
        nullptr,
        0u,
        0u,
        0u,
        0u,
        llvm::DINode::FlagZero,
        nullptr,
        m_DIBuilder.getOrCreateArray(fields));
}

llvm::DISubroutineType *llove::Builder::GetDbgFunctionType(
    const std::vector<llvm::Metadata *> &parameters,
    llvm::DIType *result)
{
    std::vector<llvm::Metadata *> elements;
    elements.emplace_back(result);

    for (auto &parameter : parameters)
        elements.emplace_back(parameter);

    return m_DIBuilder.createSubroutineType(m_DIBuilder.getOrCreateTypeArray(elements));
}
