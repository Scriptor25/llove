#include <llove/builder.hpp>
#include <llove/tree.hpp>

llove::StatementPtr llove::ScopeStatement::Wrap(StatementPtr ptr)
{
    if (dynamic_cast<ScopeStatement *>(ptr.get()))
        return ptr;

    std::vector<StatementPtr> content;
    content.emplace_back(std::move(ptr));
    return std::make_unique<ScopeStatement>(std::move(content));
}

llove::ScopeStatement::ScopeStatement(std::vector<StatementPtr> content)
    : m_Content(std::move(content))
{
}

void llove::ScopeStatement::Gen(Builder &builder) const
{
    builder.PushFrame();
    for (auto &ptr : m_Content)
        ptr->Gen(builder);
    builder.PopFrame();
}

llove::StatementPtr llove::ScopeStatement::Reflect(Context &types) const
{
    std::vector<StatementPtr> content(m_Content.size());
    for (unsigned i = 0; i < m_Content.size(); ++i)
        m_Content.at(i)->Reflect(types, content.at(i));

    return std::make_unique<ScopeStatement>(std::move(content));
}

std::ostream &llove::ScopeStatement::Print(std::ostream &stream) const
{
    const auto cur = std::string(PrintDepth += 2, ' ');

    stream << '{' << std::endl;
    for (auto &ptr : m_Content)
        stream << cur << ptr << std::endl;
    return stream << std::string(PrintDepth -= 2, ' ') << '}';
}
