#pragma once

#include "array_ptr.h"
#include <cassert>
#include <initializer_list>
#include <array>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <utility>


struct ReserveProxyObj {
    explicit ReserveProxyObj(size_t capacity)
            : capacity_to_reserve(capacity)
    {
    }

    size_t capacity_to_reserve;
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
    explicit SimpleVector(size_t size) {

        ArrayPtr<Type> smart_ptr(size);
        data_.swap(smart_ptr);
        size_ = size;
        capacity_ = size;
    }

    explicit SimpleVector(ReserveProxyObj res) {
        ArrayPtr<Type> smart_ptr(res.capacity_to_reserve);
        data_.swap(smart_ptr);
        capacity_ = res.capacity_to_reserve;
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value) {

        ArrayPtr<Type> smart_ptr(size);
        for (auto i = smart_ptr.Get(); i < smart_ptr.Get() + size; ++i) {
            *i = value;
        }

        data_.swap(smart_ptr);
        size_ = size;
        capacity_ = size;
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) {

        ArrayPtr<Type> smart_ptr(init.size());
        int counter = 0;
        for (auto i : init) {
            *(smart_ptr.Get() + counter) = i;
            ++counter;
        }

        data_.swap(smart_ptr);
        size_ = init.size();
        capacity_ = init.size();
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
        return data_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        return data_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Index out of range.");
        }
        return data_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Index out of range.");
        }
        return data_[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if (new_size < size_) {
            size_ = new_size;
        } else if (capacity_ >= new_size && new_size > size_) {
            for (size_t i = size_; i < new_size; ++i) {
                Type dummy{};
                data_[i] = std::move(dummy);
            }
            size_ = new_size;
        } else {
            size_t size = (new_size > 2 * capacity_ ? new_size : 2 * capacity_);
            ArrayPtr<Type> smart_ptr(size);
            std::move(begin(), end(), smart_ptr.Get());
            data_.swap(smart_ptr);
            size_ = size;
            capacity_ = size;
        }
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return &data_[0];
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return &data_[size_];
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return &data_[0];
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return &data_[size_];
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return const_cast<Type*>(&data_[0]);
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return const_cast<Type*>(&data_[size_]);
    }

    SimpleVector(const SimpleVector& other) {
        if (!other.IsEmpty()) {
            ArrayPtr<Type> tmp(other.size_);
            std::copy(other.begin(), other.end(), tmp.Get());
            data_.swap(tmp);
            size_ = other.size_;
            capacity_ = other.size_;
        } else {
            SimpleVector(0);
        }
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (&rhs != this) {
            SimpleVector tmp(rhs);
            swap(tmp);
        }
        return *this;
    }

    SimpleVector(SimpleVector&& other) {
        if (!other.IsEmpty()) {
            ArrayPtr<Type> tmp(other.size_);
            std::move(other.begin(), other.end(), tmp.Get());
            data_.swap(tmp);
            size_ = other.size_;

            other.size_ = 0;
            capacity_ = other.size_;
        } else {
            SimpleVector(0);
        }
    }

    SimpleVector& operator=(SimpleVector&& rhs) {
        if (&rhs != this) {
            SimpleVector tmp(rhs);
            swap(tmp);
        }
        return *this;
    }

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type& item) {
        if (size_ < capacity_) {
            data_[size_] = item;
            ++size_;
        } else {
            size_t new_capacity = (IsEmpty() ? 1 : 2 * capacity_);
            ArrayPtr<Type> tmp(new_capacity);
            std::copy(cbegin(), cend(), tmp.Get());
            tmp[size_] = item;

            data_.swap(tmp);
            capacity_ = new_capacity;
            ++size_;
        }
    }

    void PushBack(Type&& item) {
        if (size_ < capacity_) {
            data_[size_] = std::move(item);
            ++size_;
        } else {
            size_t new_capacity = (IsEmpty() ? 1 : 2 * capacity_);
            ArrayPtr<Type> tmp(new_capacity);
            std::move(begin(), end(), tmp.Get());
            tmp[size_] = std::move(item);

            data_.swap(tmp);
            capacity_ = new_capacity;
            ++size_;
        }
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        if (size_ < capacity_) {
            Iterator npos = Iterator(pos);
            std::copy_backward(pos, cend(), end() + 1);
            *npos = value;
            ++size_;
            return npos;
        } else {
            if (IsEmpty()) {
                ArrayPtr<Type> tmp(1);
                *tmp.Get() = value;
                data_.swap(tmp);
                capacity_ = 1;
                ++size_;
                return begin();
            }

            size_t new_capacity = 2 * capacity_;
            ArrayPtr<Type> tmp(new_capacity);

            auto last_copy_it = std::copy(cbegin(), pos, tmp.Get());

            *last_copy_it = value;

            if ((cbegin() - pos) == (cbegin() - cend())) {
                data_.swap(tmp);
                capacity_ = new_capacity;
                ++size_;
                return Iterator(last_copy_it);
            }

            Iterator next_pos = last_copy_it + 1;
            std::copy(ConstIterator(pos + 1), cend(), next_pos);
            data_.swap(tmp);
            capacity_ = new_capacity;
            ++size_;
            return last_copy_it;

        }
    }

    // Rvalue insert
    Iterator Insert(ConstIterator pos, Type&& value) {
        if (size_ < capacity_) {
            Iterator npos = Iterator(pos);
            std::move_backward(Iterator(pos), end(), end() + 1);
            *npos = std::move(value);
            ++size_;
            return npos;
        } else {
            if (IsEmpty()) {
                ArrayPtr<Type> tmp(1);
                *tmp.Get() = std::move(value);
                data_.swap(tmp);
                capacity_ = 1;
                ++size_;
                return begin();
            }

            size_t new_capacity = 2 * capacity_;
            ArrayPtr<Type> tmp(new_capacity);

            auto last_copy_it = std::move(begin(), Iterator(pos), tmp.Get());

            *last_copy_it = std::move(value);

            if ((cbegin() - pos) == (cbegin() - cend())) {
                data_.swap(tmp);
                capacity_ = new_capacity;
                ++size_;
                return Iterator(last_copy_it);
            }

            Iterator next_pos = last_copy_it + 1;
            std::move(Iterator(pos + 1), end(), next_pos);
            data_.swap(tmp);
            capacity_ = new_capacity;
            ++size_;
            return last_copy_it;

        }
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        if (!IsEmpty()) {
            --size_;
        }
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        Iterator npos = Iterator(pos);
        Iterator next = Iterator(pos + 1);
        std::move(next, end(), npos);
        --size_;
        return npos;
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        std::swap(other.size_, size_);
        std::swap(other.capacity_, capacity_);
        data_.swap(other.data_);
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            ArrayPtr<Type> tmp(new_capacity);
            std::move(begin(), end(), tmp.Get());
            data_.swap(tmp);
            capacity_ = new_capacity;
        }
    }

private:
    ArrayPtr<Type> data_;
    size_t capacity_ = 0;
    size_t size_ = 0;
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
    return lhs == rhs || lhs < rhs;
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return lhs == rhs || lhs > rhs;
}
