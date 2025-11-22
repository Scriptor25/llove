#include <llove/tree.hpp>

llove::Global::Global(Location loc)
    : m_Loc(std::move(loc))
{
}

const llove::Location& llove::Global::Loc() const
{
    return m_Loc;
}
