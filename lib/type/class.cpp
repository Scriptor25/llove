#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/error.hpp>
#include <llove/type.hpp>

llove::ClassType::ClassType(std::string name)
    : m_Name(std::move(name))
{
    Assert(!m_Name.empty(), "name must not be empty");
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
    return m_ParentClass && (m_ParentClass == type || m_ParentClass->InheritsFrom(type));
}

bool llove::ClassType::HasParentClass() const
{
    return m_ParentClass != nullptr;
}

llove::ClassType::Ptr llove::ClassType::GetParentClass() const
{
    Assert(m_ParentClass != nullptr, "parent class must not be null");
    return m_ParentClass;
}

bool llove::ClassType::HasMember(const std::string &name) const
{
    Assert(m_ParentClass != nullptr || !m_Members.empty(), "members must not be empty");

    return std::ranges::any_of(
               m_Members,
               [&name](auto &member)
               {
                   return member.Name == name;
               }) || (m_ParentClass && m_ParentClass->HasMember(name));
}

unsigned llove::ClassType::GetMemberIndex(const std::string &name) const
{
    Assert(m_ParentClass != nullptr || !m_Members.empty(), "members must not be empty");

    for (unsigned i = 0; i < m_Members.size(); ++i)
        if (m_Members.at(i).Name == name)
            return i + (m_ParentClass ? m_ParentClass->GetMemberCount() : 0);

    if (m_ParentClass)
        return m_ParentClass->GetMemberIndex(name);

    Error("no member with name '{}'", name);
}

unsigned llove::ClassType::GetMemberCount() const
{
    Assert(m_ParentClass != nullptr || !m_Members.empty(), "members must not be empty");

    return m_Members.size() + (m_ParentClass ? m_ParentClass->GetMemberCount() : 0);
}

llove::Field llove::ClassType::GetMember(unsigned index) const
{
    Assert(m_ParentClass != nullptr || !m_Members.empty(), "members must not be empty");

    if (m_ParentClass)
    {
        if (index < m_ParentClass->GetMemberCount())
            return m_ParentClass->GetMember(index);
        index -= m_ParentClass->GetMemberCount();
    }

    Assert(index < m_Members.size(), "index out of bounds");

    return m_Members.at(index).Info;
}

void llove::ClassType::ForEachMember(const std::function<void(unsigned, const ClassMemberReference &)> &callback) const
{
    auto offset = 0u;
    if (m_ParentClass)
    {
        m_ParentClass->ForEachMember(callback);
        offset = m_ParentClass->GetMemberCount();
    }

    for (unsigned i = 0; i < m_Members.size(); ++i)
        callback(i + offset, m_Members.at(i));
}

llove::ClassType::OptRef<llove::ClassFunctionReference> llove::ClassType::GetFunction(
    const Ptr &self,
    const std::string &name,
    const bool is_mutable,
    const std::vector<Field> &parameters,
    const bool has_variadic,
    const Field &result) const
{
    for (auto &function : m_Functions)
    {
        if (function.Name != name)
            continue;
        if (function.IsMutable != is_mutable)
            continue;
        if (function.HasVariadic != has_variadic)
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

        return { { self, function } };
    }

    if (m_ParentClass)
        if (auto ref = m_ParentClass->GetFunction(m_ParentClass, name, is_mutable, parameters, has_variadic, result))
            return ref;

    return std::nullopt;
}

bool llove::ClassType::HasFunction(const std::string &name) const
{
    return std::ranges::any_of(
               m_Functions,
               [&name](auto &function)
               {
                   return function.Name == name;
               }) || (m_ParentClass && m_ParentClass->HasFunction(name));
}

llove::ClassType::VecRef<llove::ClassFunctionReference> llove::ClassType::GetFunctions(
    const Ptr &self,
    const std::string &name) const
{
    VecRef<ClassFunctionReference> functions;
    for (auto &function : m_Functions)
        if (function.Name == name)
            functions.emplace_back(self, function);

    if (m_ParentClass)
        for (auto &ref : m_ParentClass->GetFunctions(m_ParentClass, name))
            functions.emplace_back(std::move(ref));

    return functions;
}

llove::ClassType::VecRef<llove::ClassFunctionReference> llove::ClassType::GetConstructors(const Ptr &self) const
{
    VecRef<ClassFunctionReference> constructors;
    for (auto &function : m_Functions)
        if (function.Name == "create")
            constructors.emplace_back(self, function);
    return constructors;
}

llove::ClassType::OptRef<llove::ClassFunctionReference> llove::ClassType::GetDestructor(const Ptr &self) const
{
    for (auto &function : m_Functions)
        if (function.Name == "delete")
            return { { self, function } };

    if (m_ParentClass)
        if (auto ref = m_ParentClass->GetDestructor(m_ParentClass))
            return ref;

    return std::nullopt;
}

void llove::ClassType::SetParentClass(Ptr parent_class_type)
{
    m_IRType = nullptr;
    m_DIType = nullptr;

    m_ParentClass = std::move(parent_class_type);
}

void llove::ClassType::SetMembers(std::vector<ClassMemberReference> members)
{
    Assert(m_ParentClass != nullptr || !members.empty(), "members must not be empty");

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
    Assert(m_ParentClass != nullptr || !m_Members.empty(), "members must not be empty");

    std::vector<llvm::Type *> elements;

    if (m_ParentClass)
    {
        auto base_elements = m_ParentClass->GenIRElements(builder);
        elements.insert(elements.end(), base_elements.begin(), base_elements.end());
    }

    for (auto &member : m_Members)
        elements.emplace_back(member.Info.GenIRType(builder));

    return elements;
}

std::pair<std::vector<llvm::Metadata *>, unsigned> llove::ClassType::GenDIElements(Builder &builder)
{
    Assert(m_ParentClass != nullptr || !m_Members.empty(), "members must not be empty");

    std::vector<llvm::Metadata *> elements;
    auto base_offset = 0u;

    const auto layout = builder.GetDataLayout().getStructLayout(GenIR(builder));

    if (m_ParentClass)
    {
        auto [base_elements, base_size] = m_ParentClass->GenDIElements(builder);
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
        if (!m_ParentClass && m_Members.empty())
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

    if (!m_ParentClass && m_Members.empty())
        return m_DIType;

    const auto base = m_ParentClass ? m_ParentClass->GenDI(builder) : nullptr;
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
