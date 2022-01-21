#pragma once

#include <cassert>
#include <initializer_list>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <utility>
#include <iterator>

#include "array_ptr.h"

struct ReserveProxyObj
{
    size_t capacity = 0;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj({capacity_to_reserve});
}

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size) {
        size_ = capacity_ = size;
        ArrayPtr<Type>arr(size);
        array_.swap(arr);
        std::fill(array_.Get(), array_.Get() + size, NULL);
    }

    SimpleVector(const SimpleVector& other) {
        size_ = capacity_ = other.GetSize();
        ArrayPtr<Type>arr(size_);
        array_.swap(arr);
        std::copy(other.begin(), other.end(), begin());
    }

    SimpleVector(SimpleVector&& other) noexcept{
        size_ = capacity_ = other.GetSize();
        ArrayPtr<Type>arr(size_);
        array_.swap(arr);
        std::move(other.begin(), other.end(), begin());
        other.capacity_ = other.size_ = 0;
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, Type value) {
        size_ = capacity_ = size;
        ArrayPtr<Type>arr(size);
        array_.swap(arr);
        std::fill(begin(), end(), std::move(value));
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) {
        size_ = capacity_ = init.size();
        ArrayPtr<Type>arr(init.size());
        array_.swap(arr);
        std::copy(init.begin(), init.end(), begin());
    }

    SimpleVector(ReserveProxyObj element) {
        Reserve(element.capacity);
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            SimpleVector<Type>arr(rhs);
            this->swap(arr);
        }
        return *this;
    }

    SimpleVector& operator=(SimpleVector&& rhs) {
        if (this != &rhs) {
            SimpleVector<Type>arr(std::move(rhs));
            this->swap(arr);
        }
        return *this;
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            ArrayPtr<Type>arr(new_capacity);
            std::copy(begin(), end(), arr.Get());
            capacity_ = new_capacity;
            array_.swap(arr);
        }
    }

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(Type item) {
        this->Insert(end(), std::move(item));
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1

    Iterator Insert(ConstIterator pos, Type value) {
        Iterator position = Iterator(pos);
        assert(position >= begin() && position <= end());
        size_t size = capacity_;
        if (size_ == capacity_)
            capacity_ = size = std::max(static_cast<size_t>(1), 2 * size_);
        ArrayPtr<Type>arr(size);
        const auto posistion_in_arr = arr.Get() + (position - begin());
        std::move(begin(), position, arr.Get());
        *posistion_in_arr = std::move(value);
        std::move(position, end(), posistion_in_arr + 1);
        array_.swap(arr);
        ++size_;
        return begin() + (position - arr.Get());
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        this->Erase(end() - 1);
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        assert(pos >= begin() && pos < end());
        std::move(Iterator(pos) + 1, end(), Iterator(pos));
        --size_;
        return Iterator(pos);
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
        array_.swap(other.array_);
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

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        return array_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        return array_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_)
            throw std::out_of_range(std::string("Индекс выходит за пределы массива!"));
        return array_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_)
            throw std::out_of_range(std::string("Индекс выходит за пределы массива!"));
        return array_[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if (new_size > size_ && new_size <= capacity_)
            std::fill(end(), begin() + new_size, NULL);
        else if (new_size > size_ && new_size > capacity_) {
            capacity_ = std::max(new_size, 2 * capacity_);
            ArrayPtr<Type>arr(capacity_);
            std::copy(begin(), end(), arr.Get());
            std::fill(arr.Get() + size_, arr.Get() + new_size, NULL);
            array_.swap(arr);
        }
        size_ = new_size;
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return array_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return array_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return array_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return array_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return array_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return array_.Get() + size_;
    }
private:
    size_t size_ = 0;
    size_t capacity_ = 0;
    ArrayPtr<Type> array_;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin());
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
    return lhs < rhs || lhs == rhs;
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs <= rhs);
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}