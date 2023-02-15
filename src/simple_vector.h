#pragma once

#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <iterator>
#include <stdexcept>
#include <functional>
#include <utility>

#include "array_ptr.h"


class ReserveProxyObj {
public:
    ReserveProxyObj(size_t capacity)
        : capacity_(capacity) {
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
        std::generate(begin(), end(), [](){return Type();});
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
        std::move(init.begin(), init.end(), begin());
    }

    // Создаёт пустой вектор и резервирует необходимую память
    explicit SimpleVector(const ReserveProxyObj& proxyObj) {
        Reserve(proxyObj.capacity_);
    }

    // конструктор копирования
    SimpleVector(const SimpleVector& other) {
        ArrayPtr<Type> new_items(other.GetSize());
        std::copy(other.begin(), other.end(), new_items.Get());
        items_.swap(new_items);
        size_ = other.GetSize();
        capacity_ = other.GetCapacity();
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this == &rhs) {
            return *this;
        }
        SimpleVector new_vector(rhs);
        swap(new_vector);
        return *this;
    }

    SimpleVector(SimpleVector&& other) noexcept
        : items_(std::move(other.items_)),
        size_(other.size_),
        capacity_(other.capacity_)
    {
        other.Clear();
    }

    SimpleVector& operator=(SimpleVector&& rhs) noexcept{
        if (items_.Get() != rhs.items_.Get()) {
            Resize(rhs.size_);
            std::move(rhs.begin(), rhs.end(), begin());
        }
        return *this;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        assert(index < GetSize());
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        assert(index < GetSize());
        return items_[index];

    }

    void PushBack(const Type& item) {
        if (size_ == capacity_) {
            const size_t new_capacity = capacity_ == 0 ? 1 : capacity_ * 2;
            ArrayPtr<Type> new_items(new_capacity);
            std::move(begin(), end(), new_items.Get());
            items_.swap(new_items);
            capacity_ = new_capacity;
        }
        items_[size_] = item;
        ++size_;
    }
    
    void PushBack(Type&& item) {
        if (size_ == capacity_) {
            ArrayPtr<Type> new_items(capacity_ == 0 ? 1 : capacity_ * 2);
            std::move(begin(), end(), new_items.Get());
            items_.swap(new_items);
            capacity_ = capacity_ == 0 ? 1 : capacity_ * 2;
        }
        items_[size_] = std::move(item);
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
            std::move(begin(), const_cast<Iterator>(pos), new_items.Get());
            std::move(const_cast<Iterator>(pos), end(), new_items.Get() + dist + 1);
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

    Iterator Insert(ConstIterator pos, Type&& value) {
        // assert(pos >= begin() && pos < end());
        if (size_ == capacity_) {
            size_t new_capacity = (capacity_ == 0 ? 1 : capacity_ * 2);
            ArrayPtr<Type> new_items(new_capacity);
            const auto dist = std::distance(cbegin(), pos);
            std::move(begin(), const_cast<Iterator>(pos), new_items.Get());
            std::move(const_cast<Iterator>(pos), end(), new_items.Get() + dist + 1);
            new_items[dist] = std::move(value);
            items_.swap(new_items);
            ++size_;
            capacity_ = new_capacity;
            return begin() + dist;
        }
        std::move_backward(const_cast<Iterator>(pos), end(), end() + 1);
        items_[pos - items_.Get()] = std::move(value);
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
        assert(pos >= begin() && pos < end());
        assert(!IsEmpty());
        Iterator new_pos = begin() + (pos - cbegin());
        std::move(new_pos + 1, end(), new_pos);
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
            std::move(begin(), end(), new_items.Get());
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
        return items_.Get();

    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return items_.Get() + size_;
    }

private:
    ArrayPtr<Type> items_{};
    size_t size_ = 0;
    size_t capacity_ = 0;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (lhs.GetSize() == rhs.GetSize())
           && std::equal(lhs.begin(), lhs.end(), rhs.begin());  // может бросить исключение
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);  // может бросить исключение
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());  // может бросить исключение
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(rhs < lhs);  // может бросить исключение
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;  // может бросить исключение
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs <= lhs;  // может бросить исключение
}
