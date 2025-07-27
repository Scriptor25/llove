#include <llove/builder.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

void llove::Builder::CreateDbgParameter(const std::string &name, const unsigned index, const ValuePtr &value)
{
    const auto dbg_var = m_DIBuilder.createParameterVariable(
        GetDbgScope(),
        name,
        index,
        GetDbgFile(),
        0u,
        value->GetType()->GenDbg(*this),
        true);

    if (value->IsReferenceable())
        m_DIBuilder.insertDeclare(
            value->GetPointer(),
            dbg_var,
            m_DIBuilder.createExpression(),
            llvm::DILocation::get(m_Context, 0u, 0u, GetDbgScope()),
            m_Builder.GetInsertBlock());
    else
        m_DIBuilder.insertDbgValueIntrinsic(
            value->Load(*this),
            dbg_var,
            m_DIBuilder.createExpression(),
            llvm::DILocation::get(m_Context, 0u, 0u, GetDbgScope()),
            m_Builder.GetInsertBlock());
}

void llove::Builder::CreateDbgVariable(const std::string &name, const ValuePtr &value)
{
    const auto dbg_var = m_DIBuilder.createAutoVariable(
        GetDbgScope(),
        name,
        GetDbgFile(),
        0u,
        value->GetType()->GenDbg(*this),
        true);

    if (value->IsReferenceable())
        m_DIBuilder.insertDeclare(
            value->GetPointer(),
            dbg_var,
            m_DIBuilder.createExpression(),
            llvm::DILocation::get(m_Context, 0u, 0u, GetDbgScope()),
            m_Builder.GetInsertBlock());
    else
        m_DIBuilder.insertDbgValueIntrinsic(
            value->Load(*this),
            dbg_var,
            m_DIBuilder.createExpression(),
            llvm::DILocation::get(m_Context, 0u, 0u, GetDbgScope()),
            m_Builder.GetInsertBlock());
}

void llove::Builder::EmitLoc()
{
    m_Builder.SetCurrentDebugLocation({});
}

void llove::Builder::EmitLoc(const Location &loc)
{
    m_Builder.SetCurrentDebugLocation(llvm::DILocation::get(m_Context, loc.Row, loc.Col, GetDbgScope()));
}

void llove::Builder::EmitLoc(const GlobalPtr &ptr)
{
    m_Builder.SetCurrentDebugLocation(llvm::DILocation::get(m_Context, ptr->Loc().Row, ptr->Loc().Col, GetDbgScope()));
}

void llove::Builder::EmitLoc(const StatementPtr &ptr)
{
    m_Builder.SetCurrentDebugLocation(llvm::DILocation::get(m_Context, ptr->Loc().Row, ptr->Loc().Col, GetDbgScope()));
}
