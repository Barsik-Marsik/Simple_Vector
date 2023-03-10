# Simple_Vector

The template class __SimpleVector`<`Type`>`__ is a container that is a simplified analogue of the __std::vector__ class. The elements are stored contiguously in a special template class __ArrayPtr__ in heap. It can be accessed through iterators and using offsets to regular pointers to elements. The storage of the vector is handled automatically, being expanded as needed. The memory is freed automatically when the __SimpleVector__ is destroyed.

## Implemented functionality

#### Member functions

- Constructors:
    - default
    - parameterized
    - constructor from __std::initializer_list__
    - copy constructor
    - move constructor
- operator=
- move operator

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
- Reserve

#### Modifiers

- PushBack
- PopBack
- Insert
- Erase
- Clear
- Resize
- swap

#### Non-member functions

- operator==
- operator!=
- operator<
- operator<=
- operator>
- operator>=

## ArrayPtr

The template class __ArrayPtr`<`Type`>`__ is a smart pointer to an array in heap that has the following functionality:

- Constructors: default, parameterized, move
- prohibition of copy and assignment operations
- swap
- Release
- Get
- operarator[]
- move operator


The memory is freed automatically when the __ArrayPtr__ is destroyed.