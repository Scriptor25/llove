#include "llove/context.hpp"
#include "llove/type.hpp"

#include <llove/builder.hpp>
#include <llove/forward.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/InlineAsm.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <memory>
#include <utility>
#include <vector>

llove::InlineExpression::InlineExpression(
    Location loc,
    std::string asm_string,
    std::vector<InlineOperand> dst_operands,
    std::vector<InlineOperand> src_operands,
    std::vector<std::string> clobbers,
    bool sideeffect,
    bool alignstack,
    bool inteldialect,
    bool unwind)
    : Expression(std::move(loc)),
      m_AsmString(std::move(asm_string)),
      m_DstOperands(std::move(dst_operands)),
      m_SrcOperands(std::move(src_operands)),
      m_Clobbers(std::move(clobbers)),
      m_SideEffect(std::move(sideeffect)),
      m_AlignStack(std::move(alignstack)),
      m_IntelDialect(std::move(inteldialect)),
      m_Unwind(std::move(unwind))
{
}

llove::ValuePtr llove::InlineExpression::GenVal(Builder &builder, TypePtr /* expect */) const
{
    std::vector<Field> elements, parameters;
    std::vector<llvm::Type *> element_types, parameter_types;
    std::string constraints;

    for (auto it = m_DstOperands.begin(); it != m_DstOperands.end(); ++it)
    {
        auto &type = it->second;

        elements.emplace_back(type);
        element_types.push_back(type->GenIR(builder));

        if (it != m_DstOperands.begin())
            constraints += ',';
        constraints += it->first;
    }

    for (auto it = m_SrcOperands.begin(); it != m_SrcOperands.end(); ++it)
    {
        auto &type = it->second;

        parameters.emplace_back(type);
        parameter_types.push_back(type->GenIR(builder));

        if (!m_DstOperands.empty() || it != m_SrcOperands.begin())
            constraints += ',';
        constraints += it->first;
    }

    for (auto it = m_Clobbers.begin(); it != m_Clobbers.end(); ++it)
    {
        if (!m_DstOperands.empty() || !m_SrcOperands.empty() || it != m_Clobbers.begin())
            constraints += ',';
        constraints += "~{" + *it + "}";
    }

    Field result;
    llvm::Type *result_type;

    if (element_types.empty())
    {
        result = Field(builder.GetContext().GetVoid());
        result_type = builder.GetVoidType();
    }
    else if (element_types.size() == 1)
    {
        result = Field(elements.front());
        result_type = element_types.front();
    }
    else
    {
        result = Field(builder.GetContext().GetTuple(std::move(elements)));
        result_type = builder.GetStructType(element_types);
    }

    auto function_type = llvm::FunctionType::get(result_type, parameter_types, false);
    auto inline_asm = llvm::InlineAsm::get(
        function_type,
        m_AsmString,
        constraints,
        m_SideEffect,
        m_AlignStack,
        m_IntelDialect ? llvm::InlineAsm::AD_Intel : llvm::InlineAsm::AD_ATT,
        m_Unwind);

    return Value::CreateR(builder.GetContext().GetFunction(result, std::move(parameters)), inline_asm);
}

llove::StatementPtr llove::InlineExpression::Reflect(Context &context) const
{
    std::vector<InlineOperand> dst_operands, src_operands;

    for (const auto &[fst, snd] : m_DstOperands)
    {
        TypePtr type;
        Type::Reflect(context, snd, type);
        dst_operands.emplace_back(fst, std::move(type));
    }

    for (const auto &[fst, snd] : m_SrcOperands)
    {
        TypePtr type;
        Type::Reflect(context, snd, type);
        src_operands.emplace_back(fst, std::move(type));
    }

    return std::make_unique<InlineExpression>(
        m_Loc,
        m_AsmString,
        std::move(dst_operands),
        std::move(src_operands),
        m_Clobbers,
        m_SideEffect,
        m_AlignStack,
        m_IntelDialect,
        m_Unwind);
}

std::ostream &llove::InlineExpression::Print(std::ostream &stream) const
{
    stream << "inline(" << m_AsmString;

    if (m_SideEffect || m_AlignStack || m_IntelDialect || m_Unwind)
    {
        stream << " [";
        if (m_SideEffect)
            stream << "sideeffect";
        if (m_AlignStack)
            stream << "alignstack";
        if (m_IntelDialect)
            stream << "inteldialect";
        if (m_Unwind)
            stream << "unwind";
        stream << "]";
    }

    if (!m_DstOperands.empty())
    {
        stream << " | ";
        for (auto it = m_DstOperands.begin(); it != m_DstOperands.end(); ++it)
        {
            if (it != m_DstOperands.begin())
                stream << ", ";
            stream << '"' << it->first << '"' << ':' << it->second;
        }
    }

    if (!m_SrcOperands.empty())
    {
        if (m_DstOperands.empty())
            stream << " |";

        stream << " | ";
        for (auto it = m_SrcOperands.begin(); it != m_SrcOperands.end(); ++it)
        {
            if (it != m_SrcOperands.begin())
                stream << ", ";
            stream << '"' << it->first << '"' << ':' << it->second;
        }
    }

    if (!m_Clobbers.empty())
    {
        if (m_SrcOperands.empty())
        {
            if (m_DstOperands.empty())
                stream << " |";
            stream << " |";
        }

        stream << " | ";
        for (auto it = m_Clobbers.begin(); it != m_Clobbers.end(); ++it)
        {
            if (it != m_Clobbers.begin())
                stream << ", ";
            stream << '"' << *it << '"';
        }
    }

    return stream << ")";
}
