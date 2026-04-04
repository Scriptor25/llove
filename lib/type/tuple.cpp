#include <string>
#include <utility>

#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/field.hpp>
#include <llove/forward.hpp>
#include <llove/type.hpp>

#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/Type.h>

llove::TupleType::TupleType(std::vector<Field> fields)
    : m_Fields(std::move(fields))
{
}

unsigned llove::TupleType::GetFieldCount() const
{
    return m_Fields.size();
}

const llove::Field &llove::TupleType::GetField(const unsigned index) const
{
    return m_Fields.at(index);
}

llove::TypeId llove::TupleType::GetId() const
{
    return ID;
}

bool llove::TupleType::IsTuple() const
{
    return true;
}

llvm::StructType *llove::TupleType::GenIR(Builder &builder)
{
    if (!m_IRType)
    {
        std::vector<llvm::Type *> fields;
        for (auto &field : m_Fields)
            fields.push_back(field.GenIRType(builder));

        m_IRType = builder.GetStructType(fields);
    }

    return llvm::dyn_cast<llvm::StructType>(m_IRType);
}

llvm::DIType *llove::TupleType::GenDI(Builder &builder)
{
    if (!m_DIType)
    {
        const auto layout = builder.GetDataLayout().getStructLayout(GenIR(builder));

        std::vector<llvm::Metadata *> elements;

        for (unsigned i = 0; i < m_Fields.size(); ++i)
        {
            auto &field = m_Fields.at(i);
            const auto size = field.SizeBits(builder);
            const auto offset = layout->getElementOffsetInBits(i);

            elements.push_back(
                builder.GetDebug().GetFieldType(std::to_string(i), field.GenDIType(builder), size, offset));
        }

        m_DIType = builder.GetDebug().GetStructType(elements, layout->getSizeInBits());
    }

    return m_DIType;
}

llove::TypePtr llove::TupleType::Reflect(Context &context) const
{
    std::vector<Field> fields;
    for (auto &field : m_Fields)
        field.Reflect(context, fields.emplace_back());

    return context.GetTuple(std::move(fields));
}

bool llove::TupleType::TypeInfo(Builder &builder, std::vector<llvm::Constant *> &dst) const
{
    dst.push_back(builder.GetI32(ID));
    dst.push_back(builder.GetI32(m_Fields.size()));
    for (auto &field : m_Fields)
        if (!field.TypeInfo(builder, dst))
            return false;
    return true;
}

std::string llove::TupleType::Mangle() const
{
    std::string fields;
    for (auto &field : m_Fields)
        fields += field.Mangle();
    return 'm' + std::to_string(m_Fields.size()) + '_' + fields;
}

std::ostream &llove::TupleType::Print(std::ostream &stream) const
{
    stream << "[ ";
    for (auto it = m_Fields.begin(); it != m_Fields.end(); ++it)
    {
        if (it != m_Fields.begin())
            stream << ", ";
        stream << *it;
    }
    return stream << " ]";
}
