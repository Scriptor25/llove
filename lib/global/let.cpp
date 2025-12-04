#include <llove/builder.hpp>
#include <llove/error.hpp>
#include <llove/forward.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>
#include <llvm/IR/Constant.h>
#include <llvm/IR/GlobalValue.h>

llove::LetGlobal::LetGlobal(
    Location loc,
    bool is_export,
    std::string name,
    TypePtr type)
    : Global(std::move(loc)),
      m_IsExport(is_export),
      m_Name(std::move(name)),
      m_Type(std::move(type))
{
}

void llove::LetGlobal::Gen(Builder& builder) const
try
{
    builder.EmitLoc(m_Loc);

    auto type = m_Type->GenIR(builder);
    auto linkage = m_IsExport ? llvm::GlobalValue::ExternalLinkage : llvm::GlobalValue::InternalLinkage;
    auto initializer = llvm::Constant::getNullValue(type);
    auto pointer = builder.CreateGlobal(m_Name, type, false, linkage, initializer);

    builder.SetValue(m_Name, Value::CreateL(m_Type, pointer, true));
}
catch (ref_exception<ErrorStack>& cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

std::pair<
    std::string,
    llove::ValuePtr>
llove::LetGlobal::GenImport(
    Context& /* context */,
    Builder& builder,
    const std::string& as,
    const std::map<
        std::string,
        std::string>& symbols) const
{
    if (!m_IsExport)
        return {};

    if (as.empty() && !symbols.empty() && !symbols.contains(m_Name))
        return {};

    auto type = m_Type->GenIR(builder);
    auto linkage = m_IsExport ? llvm::GlobalValue::ExternalLinkage : llvm::GlobalValue::InternalLinkage;
    auto pointer = builder.CreateGlobal(m_Name, type, true, linkage, nullptr);

    auto value = Value::CreateL(m_Type, pointer, true);

    if ((as.empty() && symbols.empty()) || (symbols.contains(m_Name) && symbols.at(m_Name) == m_Name))
    {
        builder.SetValue(symbols.contains(m_Name) ? symbols.at(m_Name) : m_Name, std::move(value));
        return { m_Name, nullptr };
    }

    return { m_Name, std::move(value) };
}

std::ostream& llove::LetGlobal::Print(std::ostream& stream) const
{
    return stream << (m_IsExport ? "export " : "") << "let " << m_Name << ": " << m_Type << ";";
}
