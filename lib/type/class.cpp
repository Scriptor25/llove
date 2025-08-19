#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/error.hpp>
#include <llove/type.hpp>

llove::ClassType::ClassType(std::string name)
    : m_Name(std::move(name)),
      m_Opaque(true)
{
}

llove::ClassType::ClassType(
    std::string name,
    std::vector<ClassFieldReference> fields,
    std::vector<ClassFunctionReference> functions)
    : m_Name(std::move(name)),
      m_Opaque(false),
      m_Fields(std::move(fields)),
      m_Functions(std::move(functions))
{
}

const std::string &llove::ClassType::GetName() const
{
    return m_Name;
}

bool llove::ClassType::IsOpaque() const
{
    return m_Opaque;
}

bool llove::ClassType::HasField(const std::string &name) const
{
    return std::ranges::any_of(
        m_Fields,
        [&name](auto &field)
        {
            return field.Name == name;
        });
}

unsigned llove::ClassType::GetFieldIndex(const std::string &name) const
{
    for (unsigned i = 0; i < m_Fields.size(); ++i)
        if (m_Fields.at(i).Name == name)
            return i;
    Error("no field with name '{}'", name);
}

unsigned llove::ClassType::GetFieldCount() const
{
    return m_Fields.size();
}

const llove::Field &llove::ClassType::GetField(const unsigned index) const
{
    return m_Fields.at(index).Info;
}

std::optional<llove::ClassFunctionReference> llove::ClassType::GetFunction(
    const std::string &name,
    const bool mutable_,
    const std::vector<Field> &parameters,
    const bool vararg,
    const Field &result) const
{
    for (auto &function : m_Functions)
    {
        if (function.Name != name)
            continue;
        if (function.Mutable != mutable_)
            continue;
        if (function.VarArg != vararg)
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

void llove::ClassType::SetFields(std::vector<ClassFieldReference> fields)
{
    m_IRType = nullptr;
    m_DIType = nullptr;

    m_Opaque = fields.empty();
    m_Fields = std::move(fields);
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

unsigned llove::ClassType::SizeBits(Builder &builder) const
{
    auto size = 0u;
    for (auto &field : m_Fields)
        size += field.Info.SizeBits(builder);
    return size;
}

llvm::StructType *llove::ClassType::GenIR(Builder &builder)
{
    if (!m_IRType)
    {
        if (m_Opaque)
        {
            const auto type = builder.GetOrCreateNamedStructType(m_Name);
            m_IRType = type;
            return type;
        }

        std::vector<llvm::Type *> elements;
        for (auto &field : m_Fields)
            elements.emplace_back(field.Info.GenIRType(builder));

        // TODO: packed struct
        m_IRType = builder.GetOrCreateNamedStructType(m_Name, elements, false);
    }

    return llvm::dyn_cast<llvm::StructType>(m_IRType);
}

llvm::DIType *llove::ClassType::GenDI(Builder &builder)
{
    if (m_DIType)
        return m_DIType;

    if (m_Opaque)
        return builder.GetDebug().GetClassType(m_Name);

    std::vector<llvm::Metadata *> fields;

    unsigned offset = 0;
    for (auto &field : m_Fields)
    {
        const auto size = field.Info.SizeBits(builder);
        fields.emplace_back(
            builder.GetDebug().GetFieldType(field.Name, field.Info.GenDIType(builder), size, offset));
        offset += size;
    }

    return builder.GetDebug().GetClassType(m_Name, fields, offset);
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
