#include <ostream>
#include <llove/error.hpp>

llove::ErrorStack::ErrorStack(const ErrorStack *cause, std::optional<Location> loc, std::optional<std::string> message)
    : m_Cause(cause),
      m_Loc(std::move(loc)),
      m_Message(std::move(message))
{
}

std::ostream &llove::ErrorStack::Print(std::ostream &stream) const
{
    auto new_line = false;
    if (m_Loc && (!m_Cause || !m_Cause->m_Loc || *m_Cause->m_Loc != *m_Loc))
    {
        new_line = true;
        stream
                << "at "
                << std::filesystem::weakly_canonical(m_Loc->Filepath).string()
                << ':'
                << m_Loc->Row
                << ':'
                << m_Loc->Col
                << ": ";
    }

    if (m_Message)
    {
        new_line = true;
        stream << *m_Message;
    }

    if (new_line)
        stream << std::endl;

    if (m_Cause)
        m_Cause->Print(stream);

    return stream;
}
