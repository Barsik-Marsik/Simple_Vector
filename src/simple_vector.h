#pragma once

#include <algorithm>
#include <iterator>
#include <cassert>
#include <stdexcept>
#include <initializer_list>

#include "array_ptr.h"

struct ReserveProxyObj {
    explicit ReserveProxyObj(size_t new_capacity)
        : capacity_(new_capacity) {
    }

    size_t capacity_;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size)
        : items_(size),
          size_(size),
          capacity_(size)
    {
        std::fill(begin(), end(), Type());
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value)
        : items_(size),
          size_(size),
          capacity_(size)

    {
        std::fill(begin(), end(), value);
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init)
        : items_(init.size()),
          size_(init.size()),
          capacity_(init.size())
    {
        size_t i = 0;
        for (const Type& item : init) {
            items_[i] = item;
            ++i;
        }
    }

    SimpleVector(const SimpleVector& other) {
        ArrayPtr<Type> new_items(other.GetSize());
        std::copy(other.begin(), other.end(), new_items.Get());
        items_.swap(new_items);
        size_ = other.GetSize();
        capacity_ = other.GetCapacity();
    }
    
    SimpleVector(const ReserveProxyObj& proxyObj) {
        Reserve(proxyObj.capacity_);
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this == &rhs) {
            return *this;
        }
        SimpleVector new_vector(rhs);
        swap(new_vector);
        return *this;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        assert(index < size_);
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        assert(index < size_);
        const Type& const_ref = items_[index];
        return const_ref;
    }

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type& item) {
        if (size_ == capacity_) {
            ArrayPtr<Type> new_items(capacity_ == 0 ? 2 : capacity_ * 2);
            std::copy(begin(), end(), new_items.Get());
            items_.swap(new_items);
            capacity_ = capacity_ == 0 ? 2 : capacity_ * 2;
        }
        items_[size_] = item;
        ++size_;
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        //assert(pos >= begin() && pos < end());
        if (size_ == capacity_) {
        	size_t new_capacity = (capacity_ == 0 ? 1 : capacity_ * 2);
            ArrayPtr<Type> new_items(new_capacity);
            const auto dist = std::distance(cbegin(), pos);
            std::copy(begin(), const_cast<Iterator>(pos), new_items.Get());
            std::copy(const_cast<Iterator>(pos), end(), new_items.Get() + dist + 1);
            new_items[dist] = value;
            items_.swap(new_items);
            ++size_;
            capacity_ = new_capacity;
            return begin() + dist;
        }
        std::copy_backward(const_cast<Iterator>(pos), end(), end() + 1);
        items_[pos - items_.Get()] = value;
        ++size_;
        return const_cast<Iterator>(pos);
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        assert(size_ > 0);
        --size_;
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        Iterator new_pos = begin() + (pos - cbegin());
        std::copy(new_pos + 1, end(), new_pos);
        --size_;
        return new_pos;
    }

    void Reserve(const size_t new_capacity) {
        if (new_capacity > capacity_) {
            ArrayPtr<Type> new_items(new_capacity);
            std::copy(begin(), end(), new_items.Get());
            items_.swap(new_items);
            capacity_ = new_capacity;
        }
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("");
        }
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("");
        }
        return items_[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if (new_size <= size_) {
            size_ = new_size;
            return;
        }
        if (new_size < capacity_) {
            for (size_t i = size_; i < new_size; ++i) {
                items_[i] = Type();
            }
            size_ = new_size;
            return;
        }
        else {
            size_t new_capacity =  2 * new_size;
            ArrayPtr<Type> new_items(new_capacity);
            std::copy(begin(), end(), new_items.Get());
            items_.swap(new_items);
            size_ = new_size;
            capacity_ = new_capacity;
            return;
        }
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        items_.swap(other.items_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return items_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return items_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return items_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return items_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        const Type* begin = items_.Get();
        return begin;
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        const Type* end = items_.Get() + size_;
        return end;
    }

private:
    ArrayPtr<Type> items_;
    size_t size_ = 0;
    size_t capacity_ = 0;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(rhs < lhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}