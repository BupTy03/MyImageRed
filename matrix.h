#ifndef MATRIX_H
#define MATRIX_H

#include<iostream>
#include<exception>

using Index = long;

using namespace std;

//  Матрица на стеке
template<typename T, const Index d1, const Index d2>
class SMatrix
{
private:
    T data[d1][d2] = {};
    const Index dm1;
    const Index dm2;

public:
    SMatrix() : dm1(d1), dm2(d2) {}
    SMatrix(std::initializer_list<T> init_list) : dm1(d1), dm2(d2)
    {
        if(init_list.size() == 1)
        {
            const T& val = *(init_list.begin());
            for(Index i = 0; i < dm1; i++)
            {
                for(Index j = 0; j < dm2; j++)
                {
                    data[i][j] = val;
                }
            }
            return;
        }

        if(init_list.size() != dm1 * dm2)
            throw invalid_argument{"Invalid argument for constructor SMatrix<T>::SMatrix(std::initializer_list<T>)"};

        std::copy(init_list.begin(), init_list.end(), this->begin());
    }

    Index size_dim1() const { return dm1; }
    Index size_dim2() const { return dm2; }

    SMatrix& operator =(const SMatrix&) = delete;
    SMatrix& operator =(SMatrix&&) = delete;

    //  доступ с проверкой диапазона
    const T& operator ()(const Index i, const Index j) const
    {
        if(i < 0 || i >= dm1 || j < 0 || j >= dm2)
            throw out_of_range("Index is outside of SMatrix");

        return data[i][j];
    }
    T& operator ()(const Index i, const Index j)
    {
        if(i < 0 || i >= dm1 || j < 0 || j >= dm2)
            throw out_of_range("Index is outside of SMatrix");

        return data[i][j];
    }

    //  доступ без проверки диапазона
    const T* operator [](Index i) const { return data[i]; }
    T* operator [](Index i) { return data[i]; }

    void fill(T&& val)
    {
        for(Index i = 0; i < dm1; i++)
            for(Index j = 0; j < dm2; j++)
                data[i][j] = move(val);
    }

    // применяет переданную функцию/функциональный объект к каждому M[i][j]
    template<typename F, typename... Args>
    void apply(F func, Args&&... args)
    {
        for(Index i = 0; i < dm1; i++)
            for(Index j = 0; j < dm2; j++)
                data[i][j] = func(data[i][j], std::forward<Args>(args)...);
    }

    friend ostream& operator <<(ostream& os, const SMatrix& m)
    {
        os << "{ ";
        for(Index i = 0; i < m.dm1; i++)
        {
            os << "{ ";
            for(Index j = 0; j < m.dm2; j++)
            {
                os << m[i][j];

                if(j != m.dm2 - 1)
                    os << ", ";
            }
            os << " }";

            if(i != m.dm1 - 1)
                os << ", ";
        }
        os << " }";

        return os;
    }

    using iterator = T*;
    using const_iterator = const T*;

    iterator begin() { return (T*)data; }
    iterator end() { return (T*)data + dm1 * dm2; }

    const_iterator cbegin() const { return (const T*)data; }
    const_iterator cend() const { return (const T*)data + dm1 * dm2; }
};


//  Матрица на куче
template<typename T>
class Matrix
{
private:
    T** data;
    Index dm1;
    Index dm2;
    Index sz;
    Index space_d1;    // общий размер для dm1
    Index space_d2;    // общий размер для dm2

public:
    Matrix() noexcept : data(nullptr), dm1(0), dm2(0), sz(0), space_d1(0), space_d2(0) {}
    Matrix(const Index x, const Index y) : dm1(x), dm2(y), sz(dm1 * dm2), space_d1(dm1), space_d2(dm2)
    {
        if (dm1 <= 0 || dm2 <= 0) {
           throw invalid_argument("Invalid argument for Matrix<T>::Matrix(const Index, const Index)");
           return;
        }

        data = new T*[dm1];

        for(Index i = 0; i < dm1; i++)
            data[i] = new T[dm2]();

    }
    Matrix(const Index x, const Index y, const T& val) : Matrix(x, y)
    {
        for(Index i = 0; i < dm1; i++)
        {
            for(Index j = 0; j < dm2; j++)
            {
                data[i][j] = val;
            }
        }
    }

    Matrix(const Matrix& other) : dm1(other.dm1), dm2(other.dm2), sz(other.sz), space_d1(other.space_d1), space_d2(other.space_d2)
    {
        copy(other.begin(), other.end(), this->begin());
    }

    Matrix(Matrix&& other) : Matrix()
    {
        swap(data, other.data);
        swap(dm1, other.dm1);
        swap(dm2, other.dm2);
        swap(sz, other.sz);
        swap(space_d1, other.space_d1);
        swap(space_d2, other.space_d2);
    }

    Matrix& operator=(const Matrix&) = default;
    Matrix& operator =(Matrix&&) = default;

    Index size_dim1() const noexcept { return dm1; }
    Index size_dim2() const noexcept { return dm2; }
    Index size() const noexcept { return sz; }

    //  индексация с проверкой диапазона
    const T& operator ()(Index i,  Index j) const
    {
        if(i < 0 || i >= dm1 || j < 0 || j >= dm2)
            throw out_of_range("Index is outside of Matrix");

        return data[i][j];
    }
    T& operator ()(Index i, Index j)
    {
        if(i < 0 || i >= dm1 || j < 0 || j >= dm2)
            throw out_of_range("Index is outside of Matrix");

        return data[i][j];
    }

    const T* operator [](Index i) const { return data[i]; }
    T* operator [](Index i) { return data[i]; }

    void fill(T&& val)
    {
        for(Index i = 0; i < dm1; i++)
            for(Index j = 0; j < dm2; j++)
                data[i][j] = move(val);
    }

    // применяет переданную функцию/функциональный объект к каждому M[i][j]
    template<typename F, typename... Args>
    void apply(F func, Args&&... args)
    {
        for(Index i = 0; i < dm1; i++)
            for(Index j = 0; j < dm2; j++)
                data[i][j] = func(data[i][j], std::forward<Args>(args)...);
    }


    void reserve_d1(Index newalloc)
    {
        if(newalloc <= space_d1) return;

        T** p = new T*[newalloc];

        for(Index i = 0; i < dm1; ++i)
            p[i] = data[i];

        delete[] data;
        data = p;
        space_d1 = newalloc;
    }
    Index capacity_d1() const { return space_d1; }
    void resize_d1(Index newsize)
    {
        if(newsize <= dm1) return;

        reserve_d1(newsize);
        for(Index i = dm1; i < newsize; ++i)
            data[i] = new T[space_d2]();

        dm1 = newsize;
        sz = dm1 * dm2;
    }
    void add_d1(const T& val)
    {
        if(space_d1 == 0)
            reserve_d1(8);
        else if(dm1 == space_d1)
            reserve_d1(2 * space_d1);

        data[dm1] = new T[space_d2];

        for(Index i = 0; i < dm2; i++)
            data[dm1][i] = val;

        ++dm1;
        sz = dm1 * dm2;
    }
    template<typename Iter>
    void add_d1(Iter first, Iter last)
    {
        if(std::distance(first, last) != dm2)
            return;

        if(space_d1 == 0)
            reserve_d1(8);
        else if(dm1 == space_d1)
            reserve_d1(2 * space_d1);

        data[dm1] = new T[space_d2];

        for(Index i = 0; i < dm2; i++)
        {
            data[dm1][i] = *first;
            first++;
        }

        ++dm1;
        sz = dm1 * dm2;
    }


    void reserve_d2(Index newalloc)
    {
        if(newalloc <= space_d2) return;

        for(Index i = 0; i < dm1; i++)
        {
            T* p = new T[newalloc];

            for(Index j = 0; j < dm2; j++)
                p[j] = data[i][j];

            delete[] data[i];
            data[i] = p;
        }

        space_d2 = newalloc;
    }
    Index capacity_d2() const { return space_d2; }
    void resize_d2(Index newsize)
    {
        if(newsize <= dm2) return;

        reserve_d2(newsize);
        for(Index i = 0; i < dm1; i++)
        {
            for(Index j = dm2; j < newsize; j++)
                data[i][j] = {};
        }

        dm2 = newsize;
        sz = dm1 * dm2;
    }
    void add_d2(const T& val)
    {
        if(space_d2 == 0)
            reserve_d2(8);
        else if(dm2 == space_d2)
            reserve_d2(2 * space_d2);

        for(Index i = 0; i < dm1; i++)
            data[i][dm2] = val;

        ++dm2;
        sz = dm1 * dm2;
    }
    template<typename Iter>
    void add_d2(Iter first, Iter last)
    {
        if(std::distance(first, last) != dm1)
            return;

        if(space_d2 == 0)
            reserve_d2(8);
        else if(dm2 == space_d2)
            reserve_d2(2 * space_d2);

        for(Index i = 0; i < dm1; i++)
        {
            data[i][dm2] = *first;
            first++;
        }

        ++dm2;
        sz = dm1 * dm2;
    }


    friend ostream& operator <<(ostream& os, const Matrix& m)
    {
        os << "{" << endl;
        for(Index i = 0; i < m.dm1; i++)
        {
            os << "{ ";
            for(Index j = 0; j < m.dm2; j++)
            {
                os << m[i][j];

                if(j != m.dm2 - 1)
                    os << ", ";
            }
            os << " }";

            if(i != m.dm1 - 1)
                os << ", ";

            os << endl;
        }
        os << "}";

        return os;
    }

    class MatrixIterator : public std::iterator<std::input_iterator_tag, T>
    {
        friend class Matrix;
    private:
        MatrixIterator(T** p, Index s) : p(p), step(s), left(0) {}
    public:
        MatrixIterator(const MatrixIterator &it) : p(it.p), step(it.step), left(it.left) {}
        MatrixIterator& operator =(const MatrixIterator&) = default;

        bool operator!=(MatrixIterator const& other) const { return p != other.p || left != other.left || step != other.step; }
        bool operator==(MatrixIterator const& other) const { return p == other.p && left == other.left && step == other.step; }
        T& operator*() const { return *(*p + left); }
        MatrixIterator& operator++()
        {
            if(left >= step)
            {
                left = 0;
                ++p;
                return *this;
            }
            left++;
            return *this;
        }
        MatrixIterator& operator++(int)
        {
            if(left >= step)
            {
                left = 0;
                ++p;
                return *this;
            }
            left++;
            return *this;
        }
    private:
        T** p;
        Index step;
        Index left;
    };

    class ConstMatrixIterator : public std::iterator<std::input_iterator_tag, T>
    {
        friend class Matrix;
    private:
        ConstMatrixIterator(T** p, Index s) : p(const_cast<const T**>(p)), step(s), left(0) {}
    public:
        ConstMatrixIterator(const ConstMatrixIterator &it) : p(it.p), step(it.step), left(it.left) {}
        ConstMatrixIterator& operator =(const ConstMatrixIterator&) = default;

        bool operator!=(ConstMatrixIterator const& other) const { return p != other.p || left != other.left || step != other.step; }
        bool operator==(ConstMatrixIterator const& other) const { return p == other.p && left == other.left && step == other.step; }
        T const& operator*() const { return *(*p + left); }
        ConstMatrixIterator& operator++()
        {
            if(left >= step)
            {
                left = 0;
                ++p;
                return *this;
            }
            left++;
            return *this;
        }
        ConstMatrixIterator& operator++(int)
        {
           if(left >= step)
           {
                left = 0;
                ++p;
                return *this;
            }
            left++;
            return *this;
        }
    private:
        const T** p;
        Index step;
        Index left;
    };

    using iterator = MatrixIterator;
    using const_iterator = ConstMatrixIterator;

    iterator begin() { return iterator(data, dm2 - 1); }
    iterator end() { return iterator(data + dm1, dm2 - 1); }

    const_iterator cbegin() { return const_iterator(data, dm2 - 1); }
    const_iterator cend() { return const_iterator(data + dm1, dm2 - 1); }

    ~Matrix()
    {
        if(data == nullptr)
            return;

        for(Index i = 0; i < dm1; i++)
            delete[] data[i];

        delete[] data;
    }
};

#endif // MATRIX_H
