#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/type.hpp>

llove::TypeId llove::VariadicType::GetId() const
{
    return TypeId_Variadic;
}

bool llove::VariadicType::IsVariadic() const
{
    return true;
}

llvm::Type *llove::VariadicType::GenIR(Builder &builder)
{
    if (!m_IRType)
        m_IRType = builder.GetVariadicType();

    return m_IRType;
}

llvm::DIType *llove::VariadicType::GenDI(Builder &builder)
{
    if (!m_DIType)
        m_DIType = builder.GetDebug().GetVariadicType();

    return m_DIType;
}

llove::TypePtr llove::VariadicType::Reflect(Context &context) const
{
    return context.GetVariadic();
}

bool llove::VariadicType::TypeInfo(Builder &builder, std::vector<llvm::Constant *> &dst) const
{
    dst.emplace_back(builder.GetI32(ID));
    return true;
}

std::string llove::VariadicType::Mangle() const
{
    return "z";
}

std::ostream &llove::VariadicType::Print(std::ostream &stream) const
{
    return stream << "variadic";
}
