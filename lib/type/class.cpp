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
    Ptr base_type,
    std::vector<ClassMemberReference> members,
    std::vector<ClassFunctionReference> functions)
    : m_Name(std::move(name)),
      m_BaseType(std::move(base_type)),
      m_Members(std::move(members)),
      m_Functions(std::move(functions))
{
    Assert(!m_Name.empty(), "name must not be empty");
    Assert(m_BaseType != nullptr || !m_Members.empty(), "members must not be empty");
}

const std::string &llove::ClassType::GetName() const
{
    return m_Name;
}

bool llove::ClassType::IsOpaque() const
{
    return m_Members.empty();
}

bool llove::ClassType::InheritsFrom(const TypePtr &type) const
{
    return m_BaseType && (m_BaseType == type || m_BaseType->InheritsFrom(type));
}

bool llove::ClassType::HasMember(const std::string &name) const
{
    Assert(m_BaseType != nullptr || !m_Members.empty(), "members must not be empty");

    return std::ranges::any_of(
               m_Members,
               [&name](auto &member)
               {
                   return member.Name == name;
               }) || (m_BaseType && m_BaseType->HasMember(name));
}

unsigned llove::ClassType::GetMemberIndex(const std::string &name) const
{
    Assert(m_BaseType != nullptr || !m_Members.empty(), "members must not be empty");

    for (unsigned i = 0; i < m_Members.size(); ++i)
        if (m_Members.at(i).Name == name)
            return i + (m_BaseType ? m_BaseType->GetMemberCount() : 0);

    if (m_BaseType)
        return m_BaseType->GetMemberIndex(name);

    Error("no member with name '{}'", name);
}

unsigned llove::ClassType::GetMemberCount() const
{
    Assert(m_BaseType != nullptr || !m_Members.empty(), "members must not be empty");

    return m_Members.size() + (m_BaseType ? m_BaseType->GetMemberCount() : 0);
}

const llove::Field &llove::ClassType::GetMember(unsigned index) const
{
    Assert(m_BaseType != nullptr || !m_Members.empty(), "members must not be empty");

    if (m_BaseType)
    {
        if (index < m_BaseType->GetMemberCount())
            return m_BaseType->GetMember(index);
        index -= m_BaseType->GetMemberCount();
    }

    Assert(index < m_Members.size(), "index out of bounds");

    return m_Members.at(index).Info;
}

std::optional<llove::ClassFunctionReference> llove::ClassType::GetFunction(
    const std::string &name,
    const bool is_mutable,
    const std::vector<Field> &parameters,
    const bool is_variadic,
    const Field &result) const
{
    for (auto &function : m_Functions)
    {
        if (function.Name != name)
            continue;
        if (function.IsMutable != is_mutable)
            continue;
        if (function.IsVariadic != is_variadic)
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

    if (m_BaseType)
        return m_BaseType->GetFunction(name, is_mutable, parameters, is_variadic, result);

    return std::nullopt;
}

bool llove::ClassType::HasFunction(const std::string &name) const
{
    return std::ranges::any_of(
               m_Functions,
               [&name](auto &function)
               {
                   return function.Name == name;
               }) || (m_BaseType && m_BaseType->HasFunction(name));
}

std::vector<llove::ClassFunctionReference> llove::ClassType::GetFunctions(const std::string &name) const
{
    std::vector<ClassFunctionReference> functions;
    for (auto &function : m_Functions)
        if (function.Name == name)
            functions.push_back(function);

    if (m_BaseType)
        for (auto &function : m_BaseType->GetFunctions(name))
            functions.emplace_back(std::move(function));

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

    if (m_BaseType)
        return m_BaseType->GetDestructor();

    return std::nullopt;
}

void llove::ClassType::SetBaseType(Ptr base_type)
{
    m_IRType = nullptr;
    m_DIType = nullptr;

    m_BaseType = std::move(base_type);
}

void llove::ClassType::SetMembers(std::vector<ClassMemberReference> members)
{
    Assert(m_BaseType != nullptr || !members.empty(), "members must not be empty");

    m_IRType = nullptr;
    m_DIType = nullptr;

    m_Members = std::move(members);
}

void llove::ClassType::SetFunctions(std::vector<ClassFunctionReference> functions)
{
    m_IRType = nullptr;
    m_DIType = nullptr;

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

std::vector<llvm::Type *> llove::ClassType::GenIRElements(Builder &builder) const
{
    Assert(m_BaseType != nullptr || !m_Members.empty(), "members must not be empty");

    std::vector<llvm::Type *> elements;

    if (m_BaseType)
    {
        auto base_elements = m_BaseType->GenIRElements(builder);
        elements.insert(elements.end(), base_elements.begin(), base_elements.end());
    }

    for (auto &member : m_Members)
        elements.emplace_back(member.Info.GenIRType(builder));

    return elements;
}

std::pair<std::vector<llvm::Metadata *>, unsigned> llove::ClassType::GenDIElements(Builder &builder)
{
    Assert(m_BaseType != nullptr || !m_Members.empty(), "members must not be empty");

    std::vector<llvm::Metadata *> elements;
    auto base_offset = 0u;

    const auto layout = builder.GetDataLayout().getStructLayout(GenIR(builder));

    if (m_BaseType)
    {
        auto [base_elements, base_size] = m_BaseType->GenDIElements(builder);
        elements.insert(elements.end(), base_elements.begin(), base_elements.end());
        base_offset = base_size;
    }

    for (unsigned i = 0; i < m_Members.size(); ++i)
    {
        auto &member = m_Members.at(i);
        const auto size = member.Info.SizeBits(builder);
        const auto offset = base_offset + layout->getElementOffsetInBits(i);

        elements.emplace_back(
            builder.GetDebug().GetFieldType(
                member.Name,
                member.Info.GenDIType(builder),
                size,
                offset));
    }

    return { elements, layout->getSizeInBits() };
}

llvm::StructType *llove::ClassType::GenIR(Builder &builder)
{
    if (!m_IRType)
    {
        if (!m_BaseType && m_Members.empty())
        {
            m_IRType = builder.GetOrCreateNamedStructType(m_Name);
        }
        else
        {
            const auto elements = GenIRElements(builder);
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

    if (!m_BaseType && m_Members.empty())
        return m_DIType;

    const auto base = m_BaseType ? m_BaseType->GenDI(builder) : nullptr;
    auto [elements, size] = GenDIElements(builder);

    return m_DIType = builder.GetDebug().GetClassType(m_Name, base, elements, size);
}

llove::TypePtr llove::ClassType::Reflect(Context &context) const
{
    return context.GetClass(m_Name);
}

bool llove::ClassType::TypeInfo(Builder &builder, std::vector<llvm::Constant *> &dst) const
{
    return false;
}

std::string llove::ClassType::Mangle() const
{
    return 'c' + std::to_string(m_Name.size()) + '_' + m_Name;
}

std::ostream &llove::ClassType::Print(std::ostream &stream) const
{
    return stream << "class " << m_Name;
}
