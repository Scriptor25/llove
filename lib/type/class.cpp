#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/error.hpp>
#include <llove/type.hpp>

llove::ClassType::ClassType(std::string name)
    : m_Name(std::move(name))
{
    Assert(!m_Name.empty(), "name must not be empty");
}

llove::ClassType::ClassType(
    std::string name,
    std::vector<ClassFieldReference> members,
    std::vector<ClassFunctionReference> functions)
    : m_Name(std::move(name)),
      m_Members(std::move(members)),
      m_Functions(std::move(functions))
{
    Assert(!name.empty(), "name must not be empty");
    Assert(!members.empty(), "members must not be empty");
}

const std::string &llove::ClassType::GetName() const
{
    return m_Name;
}

bool llove::ClassType::IsOpaque() const
{
    return m_Members.empty();
}

bool llove::ClassType::HasMember(const std::string &name) const
{
    Assert(!m_Members.empty(), "members must not be empty");

    return std::ranges::any_of(
        m_Members,
        [&name](auto &member)
        {
            return member.Name == name;
        });
}

unsigned llove::ClassType::GetMemberIndex(const std::string &name) const
{
    Assert(!m_Members.empty(), "members must not be empty");

    for (unsigned i = 0; i < m_Members.size(); ++i)
        if (m_Members.at(i).Name == name)
            return i;

    Error("no member with name '{}'", name);
}

unsigned llove::ClassType::GetMemberCount() const
{
    Assert(!m_Members.empty(), "members must not be empty");

    return m_Members.size();
}

const llove::Field &llove::ClassType::GetMember(const unsigned index) const
{
    Assert(!m_Members.empty(), "members must not be empty");
    Assert(index < m_Members.size(), "index out of bounds");

    return m_Members.at(index).Info;
}

std::optional<llove::ClassFunctionReference> llove::ClassType::GetFunction(
    const std::string &name,
    const bool mutable_,
    const std::vector<Field> &parameters,
    const bool variadic,
    const Field &result) const
{
    for (auto &function : m_Functions)
    {
        if (function.Name != name)
            continue;
        if (function.Mutable != mutable_)
            continue;
        if (function.Variadic != variadic)
            continue;
        if (function.Parameters.size() != parameters.size())
            continue;
        if (function.Result != result)
            continue;
        unsigned i;
        for (i = 0; i < function.Parameters.size(); ++i)
            if (function.Parameters.at(i) != parameters.at(i))
                break;
        if (i < function.Parameters.size())
            continue;
        return function;
    }
    return std::nullopt;
}

bool llove::ClassType::HasFunction(const std::string &name) const
{
    return std::ranges::any_of(
        m_Functions,
        [&name](auto &function)
        {
            return function.Name == name;
        });
}

std::vector<llove::ClassFunctionReference> llove::ClassType::GetFunctions(const std::string &name) const
{
    std::vector<ClassFunctionReference> functions;
    for (auto &function : m_Functions)
        if (function.Name == name)
            functions.push_back(function);
    return functions;
}

std::vector<llove::ClassFunctionReference> llove::ClassType::GetConstructors() const
{
    std::vector<ClassFunctionReference> constructors;
    for (auto &function : m_Functions)
        if (function.Name == "create")
            constructors.emplace_back(function);
    return constructors;
}

std::optional<llove::ClassFunctionReference> llove::ClassType::GetDestructor() const
{
    for (auto &function : m_Functions)
        if (function.Name == "delete")
            return function;
    return std::nullopt;
}

void llove::ClassType::SetMembers(std::vector<ClassFieldReference> members)
{
    m_IRType = nullptr;
    m_DIType = nullptr;

    m_Members = std::move(members);
}

void llove::ClassType::SetFunctions(std::vector<ClassFunctionReference> functions)
{
    m_Functions = std::move(functions);
}

llove::TypeId llove::ClassType::GetId() const
{
    return TypeId_Class;
}

bool llove::ClassType::IsClass() const
{
    return true;
}

llvm::StructType *llove::ClassType::GenIR(Builder &builder)
{
    if (!m_IRType)
    {
        if (m_Members.empty())
        {
            m_IRType = builder.GetOrCreateNamedStructType(m_Name);
        }
        else
        {
            std::vector<llvm::Type *> elements;
            for (auto &member : m_Members)
                elements.emplace_back(member.Info.GenIRType(builder));

            // TODO: packed struct
            m_IRType = builder.GetOrCreateNamedStructType(m_Name, elements, false);
        }
    }

    return llvm::dyn_cast<llvm::StructType>(m_IRType);
}

llvm::DIType *llove::ClassType::GenDI(Builder &builder)
{
    if (m_DIType)
        return m_DIType;

    m_DIType = builder.GetDebug().GetClassType(m_Name);

    if (m_Members.empty())
        return m_DIType;

    std::vector<llvm::Metadata *> members;

    unsigned offset = 0;
    for (auto &member : m_Members)
    {
        const auto size = member.Info.SizeBits(builder);
        members.emplace_back(
            builder.GetDebug().GetFieldType(
                member.Name,
                member.Info.GenDIType(builder),
                size,
                offset));
        offset += size;
    }

    return m_DIType = builder.GetDebug().GetClassType(m_Name, members, offset);
}

llove::TypePtr llove::ClassType::Reflect(Context &context) const
{
    return context.GetClass(m_Name);
}

std::string llove::ClassType::Mangle() const
{
    return 'c' + std::to_string(m_Name.size()) + '_' + m_Name;
}

std::ostream &llove::ClassType::Print(std::ostream &stream) const
{
    return stream << "class " << m_Name;
}
