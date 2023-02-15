# Simple_Vector

The template class __SimpleVector`<`Type`>`__ is a container that is a simplified analogue of the __std::vector__ class. The elements are stored contiguously in a special template class __ArrayPtr__ in heap. It can be accessed through iterators and using offsets to regular pointers to elements. The storage of the vector is handled automatically, being expanded as needed. The memory is freed automatically when the __SimpleVector__ is destroyed.

## Implemented functionality

#### Constructors

- default
- Парметризованный конструктор
- Конструктор из __std::initializer_list__

#### Element access

- At
- operarator[]

#### Iterators

- begin, cbegin
- end, cend

#### Capacity

- GetSize
- GetCapacity
- IsEmpty

#### Modifiers

- Clear
- Resize

## ArrayPtr

The template class __ArrayPtr`<`Type`>`__ is a smart pointer to an array in heap that has the following functionality:

- Constructors: default, parameterized
- prohibition of copy and assignment operations
- swap
- Release
- Get
- operarator[]


The memory is freed automatically when the __ArrayPtr__ is destroyed.