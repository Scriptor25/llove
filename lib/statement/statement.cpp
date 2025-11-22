#include <llove/tree.hpp>

llove::Statement::Statement(Location loc)
    : m_Loc(std::move(loc))
{
}

const llove::Location& llove::Statement::Loc() const
{
    return m_Loc;
}
