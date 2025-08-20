#include <llove/builder.hpp>
#include <llove/tree.hpp>

llove::StatementPtr llove::ScopeStatement::Wrap(StatementPtr ptr)
{
    if (dynamic_cast<ScopeStatement *>(ptr.get()))
        return ptr;

    auto loc = ptr->Loc();

    std::vector<StatementPtr> content;
    content.emplace_back(std::move(ptr));
    return std::make_unique<ScopeStatement>(std::move(loc), std::move(content));
}

llove::ScopeStatement::ScopeStatement(Location loc, std::vector<StatementPtr> content)
    : Statement(std::move(loc)),
      m_Content(std::move(content))
{
}

void llove::ScopeStatement::Gen(Builder &builder) const try
{
    builder.EmitLoc(m_Loc);
    builder.PushFrame(m_Loc);
    for (auto &ptr : m_Content)
        ptr->Gen(builder);
    builder.PopFrame();
}
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

llove::StatementPtr llove::ScopeStatement::Reflect(Context &context) const try
{
    std::vector<StatementPtr> content;
    for (auto &statement : m_Content)
        statement->Reflect(context, content.emplace_back());

    return std::make_unique<ScopeStatement>(m_Loc, std::move(content));
}
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

std::ostream &llove::ScopeStatement::Print(std::ostream &stream) const
{
    const auto cur = std::string(PrintDepth += 2, ' ');

    stream << '{' << std::endl;
    for (auto &ptr : m_Content)
        stream << cur << ptr << std::endl;
    return stream << std::string(PrintDepth -= 2, ' ') << '}';
}
