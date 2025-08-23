#include <llove/builder.hpp>
#include <llove/value.hpp>

llove::ValuePtr llove::Builder::GetPointerElement(const ValuePtr &pointer, const ValuePtr &index)
{
    Assert(pointer != nullptr, "pointer must not be null");
    Assert(index != nullptr, "index must not be null");

    const auto pointer_type = pointer->GetType();
    const auto index_type = index->GetType();

    Assert(pointer_type->IsPointer(), "pointer type must be a pointer type");
    Assert(index_type->IsInteger(), "index type must be an integer type");

    const auto type = As<PointerType>(pointer_type);
    const auto base_type = type->GetBase();
    const auto value = m_LLVMBuilder.CreateGEP(
        base_type->GenIR(*this),
        pointer->Load(*this),
        index->Load(*this));
    return Value::CreateL(base_type, value, type->IsMutable());
}

llove::ValuePtr llove::Builder::GetArrayElement(const ValuePtr &array, const ValuePtr &index)
{
    Assert(array != nullptr, "array must not be null");
    Assert(index != nullptr, "index must not be null");

    const auto array_type = array->GetType();
    const auto index_type = index->GetType();

    Assert(array_type->IsArray(), "array type must be an array type");
    Assert(index_type->IsInteger(), "index type must be an integer type");

    const auto type = As<ArrayType>(array_type);
    const auto base_type = type->GetBase();

    if (array->IsReference())
    {
        const auto element_pointer = m_LLVMBuilder.CreateInBoundsGEP(
            type->GenIR(*this),
            array->GetPointer(),
            {
                llvm::Constant::getNullValue(index_type->GenIR(*this)),
                index->Load(*this),
            });
        return Value::CreateL(base_type, element_pointer, array->IsMutable());
    }

    if (const auto const_index_value = llvm::dyn_cast<llvm::ConstantInt>(index->Load(*this)))
    {
        const auto element = m_LLVMBuilder.CreateExtractValue(array->Load(*this), const_index_value->getLimitedValue());
        return Value::CreateR(base_type, element);
    }

    const auto pointer = CreateAlloca(type->GenIR(*this));
    m_LLVMBuilder.CreateStore(array->Load(*this), pointer);

    const auto element_pointer = m_LLVMBuilder.CreateInBoundsGEP(
        type->GenIR(*this),
        pointer,
        {
            llvm::Constant::getNullValue(index_type->GenIR(*this)),
            index->Load(*this),
        });
    return Value::CreateL(base_type, element_pointer, false);
}
