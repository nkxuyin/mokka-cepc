/*! \file phys_primitives.hh
    \brief Defines some primitives usefull for physical software development.

Copyright (c) 2005-2011 Konstantin Androsov <konstantin.androsov@gmail.com>.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include <exception>
#include <cmath>
#include <ostream>

/// Some primitives usefull for physical software development.
namespace phys
{
    /// Out of range exception.
    class out_of_range_exception : public std::exception
    {
    public:
        virtual const char* what() const throw();
    };

    /// N-dimensional vector.
    template<typename T, unsigned N>
    class vector
    {
    private:
        T values[N];

    public:
        vector() throw()
        {
            for(unsigned n = 0; n < N; ++n)
                values[n] = 0;
        }

        virtual ~vector() throw() {}

        T& operator[] (unsigned n) throw(out_of_range_exception)
        {
            if (n >= N)
                throw out_of_range_exception();
            return values[n];
        }

        const T& operator[] (unsigned n) const throw(out_of_range_exception)
        {
            if (n >= N)
                throw out_of_range_exception();
            return values[n];
        }

        unsigned dimension() const throw()
        {
            return N;
        }

        vector& operator+=(const vector& v) throw()
        {
            for(unsigned n = 0; n < N; ++n)
                values[n] += v.values[n];
            return *this;
        }

        vector& operator-=(const vector& v) throw()
        {
            for(unsigned n = 0; n < N; ++n)
                values[n] -= v.values[n];
            return *this;
        }

        vector& operator+(const vector& v) const throw()
        {
            vector result = *this;
            result += v;
            return result;
        }

        vector& operator-(const vector& v) const throw()
        {
            vector result = *this;
            result -= v;
            return result;
        }
    };

    template<typename T, unsigned N>
    std::ostream& operator<<(std::ostream& o, const vector<T, N>& v)
    {
        o << "(";
        if(N > 0)
        {
            o << v[0];
            for(unsigned n = 1; n < N; ++n)
                o << ", " << v[n];
        }
        o << ")";
        return o;
    }


    /// Specialization vector<T, N> for 1-dimensional vector.
    template<typename T>
    class vector<T, 1>
    {
    public:
        static const unsigned X_INDEX = 0;

    public:
        T x;

    public:
        vector() throw() : x(0) {}
        vector(const T& p_x) throw() : x(p_x) {}
        virtual ~vector() throw() {}

        T& operator[] (unsigned n) throw(out_of_range_exception)
        {
            if (!n)
                return x;
            throw out_of_range_exception();
        }

        const T& operator[] (unsigned n) const throw(out_of_range_exception)
        {
            if (!n)
                return x;
            throw out_of_range_exception();
        }

        unsigned dimension() const throw()
        {
            return 1;
        }

        vector& operator+=(const vector& v) throw()
        {
            x += v.x;
            return *this;
        }

        vector& operator-=(const vector& v) throw()
        {
            x -= v.x;
            return *this;
        }

        vector& operator+(const vector& v) const throw()
        {
            return vector(x + v.x);
        }

        vector& operator-(const vector& v) const throw()
        {
            return vector(x - v.x);
        }
    };

    /// Specialization vector<T, N> for 2-dimensional vector.
    template<typename T>
    class vector<T, 2>
    {
    public:
        static const unsigned X_INDEX = 0, Y_INDEX = 1;

    public:
        T x, y;

    private:
        static const unsigned N = 2;
        typedef T vector::*pointer_to_member;

        static inline pointer_to_member get_pointer(unsigned n) throw(out_of_range_exception)
        {
            static const pointer_to_member pointers[] = { &vector::x, &vector::y };
            if(n >= N)
                throw out_of_range_exception();
            return pointers[n];
        }

    public:
        vector() throw() : x(0), y(0) {}
        vector(const T& p_x, const T& p_y) throw() : x(p_x), y(p_y) {}
        virtual ~vector() throw() {}

        T& operator[] (unsigned n) throw(out_of_range_exception)
        {
            return this->*get_pointer(n);
        }

        const T& operator[] (unsigned n) const throw(out_of_range_exception)
        {
            return this->*get_pointer(n);
        }

        unsigned dimension() const throw()
        {
            return N;
        }

        vector& operator+=(const vector& v) throw()
        {
            x += v.x;
            y += v.y;
            return *this;
        }

        vector& operator-=(const vector& v) throw()
        {
            x -= v.x;
            y -= v.y;
            return *this;
        }

        vector& operator+(const vector& v) const throw()
        {
            return vector(x + v.x, y + v.y);
        }

        vector& operator-(const vector& v) const throw()
        {
            return vector(x - v.x, y - v.y);
        }
    };

    /// Specialization vector<T, N> for 3-dimensional vector.
    template<typename T>
    class vector<T, 3>
    {
    public:
        static const unsigned X_INDEX = 0, Y_INDEX = 1, Z_INDEX = 2;

    public:
        T x, y, z;

    private:
        static const unsigned N = 3;
        typedef T vector::*pointer_to_member;

        static inline pointer_to_member get_pointer(unsigned n) throw(out_of_range_exception)
        {
            static const pointer_to_member pointers[] = { &vector::x, &vector::y, &vector::z };
            if(n >= N)
                throw out_of_range_exception();
            return pointers[n];
        }

    public:
        vector() throw() : x(0), y(0), z(0) {}
        vector(const T& p_x, const T& p_y, const T& p_z) throw() : x(p_x), y(p_y), z(p_z) {}
        virtual ~vector() throw() {}

        T& operator[] (unsigned n) throw(out_of_range_exception)
        {
            return this->*get_pointer(n);
        }

        const T& operator[] (unsigned n) const throw(out_of_range_exception)
        {
            return this->*get_pointer(n);
        }

        unsigned dimension() const throw()
        {
            return N;
        }

        vector& operator+=(const vector& v) throw()
        {
            x += v.x;
            y += v.y;
            z += v.z;
            return *this;
        }

        vector& operator-=(const vector& v) throw()
        {
            x -= v.x;
            y -= v.y;
            z -= v.z;
            return *this;
        }

        vector& operator+(const vector& v) const throw()
        {
            return vector(x + v.x, y + v.y, z + v.z);
        }

        vector& operator-(const vector& v) const throw()
        {
            return vector(x - v.x, y - v.y, z - v.z);
        }
    };

    /// N-dimensional matrix.
    template<typename T, unsigned N, unsigned M>
    class matrix
    {
    private:
        static const unsigned L = N * M;
        T values[L];

    public:
        class matrix_row
        {
        private:
            T* data;

        private:
            matrix_row(T* d) throw() : data(d) {}

        public:
            T& operator[] (unsigned m) throw(out_of_range_exception)
            {
                if(m >= M)
                    throw out_of_range_exception();
                return data[m];
            }

            const T& operator[] (unsigned m) const throw(out_of_range_exception)
            {
                if(m >= M)
                    throw out_of_range_exception();
                return data[m];
            }
        };

        class const_matrix_row
        {
        private:
            const T* data;

        private:
            const_matrix_row(const T* d) throw() : data(d) {}

        public:
            const T& operator[] (unsigned m) const throw(out_of_range_exception)
            {
                if(m >= M)
                    throw out_of_range_exception();
                return data[m];
            }
        };

        matrix()
        {
            for(unsigned l = 0; l < L; ++l)
                values[l] = 0;
        }

        matrix_row operator[] (unsigned n) throw(out_of_range_exception)
        {
            if (n >= N)
                throw out_of_range_exception();
            return matrix_row(values + n * M);
        }

        const_matrix_row operator[] (unsigned n) const throw(out_of_range_exception)
        {
            if (n >= N)
                throw out_of_range_exception();
            return const_matrix_row(values + n * M);
        }

        unsigned number_of_rows() const throw()
        {
            return N;
        }

        unsigned number_of_columns() const throw()
        {
            return M;
        }
    };

    /// N-dimensional Euclidean vector.
    template<typename T, unsigned N>
    class euclidean_vector : public vector<T, N>
    {
    private:
        bool bound;

    public:
        euclidean_vector() : bound(true) {}
        euclidean_vector(bool p_bound) : bound(p_bound) {}

        T norm() const throw()
        {
            T sum_square;
            for(unsigned n = 0; n < N; ++n)
                sum_square += (*this)[n] * (*this)[n];
            return std::sqrt(sum_square);
        }

        bool is_bound() const throw()
        {
            return bound;
        }
    };

    /// Specialization euclidean_vector<T, N> for 3-dimensional Euclidean vector.
    template<typename T>
    class euclidean_vector<T, 3> : public vector<T, 3>
    {
    private:
        bool bound;

    public:
        euclidean_vector() throw() : vector<T, 3>(), bound(true) {}
        euclidean_vector(const T& p_x, const T& p_y, const T& p_z) throw()
            : vector<T, 3>(p_x, p_y, p_z), bound(true) {}
        euclidean_vector(bool p_bound) throw() : vector<T, 3>(), bound(p_bound) {}
        euclidean_vector(bool p_bound, const T& p_x, const T& p_y, const T& p_z) throw()
            : vector<T, 3>(p_x, p_y, p_z), bound(p_bound) {}

        void rotate_x(T angle) throw()
        {
            T c = std::cos(angle);
            T s = std::sin(angle);
            T new_y = this->y * c - this->z * s;
            T new_z = this->y * s + this->z * c;
            this->y = new_y;
            this->z = new_z;
        }

        void rotate_y(T angle) throw()
        {
            T c = std::cos(angle);
            T s = std::sin(angle);
            T new_x = this->x * c + this->z * s;
            T new_z = -this->x * s + this->z * c;
            this->x = new_x;
            this->z = new_z;
        }

        void rotate_z(T angle) throw()
        {
            T c = std::cos(angle);
            T s = std::sin(angle);
            T new_x = this->x * c - this->y * s;
            T new_y = this->x * s + this->y * c;
            this->x = new_x;
            this->y = new_y;
        }

        T norm() const throw()
        {
            return std::sqrt(this->x + this->y * this->y + this->z * this->z);
        }

        bool is_bound() const throw()
        {
            return bound;
        }
    };

    /// N-dimensional solid angle.
    template<typename T, unsigned N>
    class solid_angle
    {
    private:
        T values[N];

    public:
        solid_angle() throw()
        {
            for(unsigned n = 0; n < N; ++n)
                values[n] = 0;
        }

        T& operator[] (unsigned n) throw(out_of_range_exception)
        {
            if (n >= N)
                throw out_of_range_exception();
            return values[N];
        }

        const T& operator[] (unsigned n) const throw(out_of_range_exception)
        {
            if (n >= N)
                throw out_of_range_exception();
            return values[N];
        }

        unsigned dimension() const throw()
        {
            return N;
        }
    };

    /// Specialization solid_angle<T, N> for 1-dimensional solid angle.
    template<typename T>
    class solid_angle<T, 1>
    {
    public:
        static const unsigned PSI_INDEX = 0;

    public:
        T psi;

    public:
        solid_angle() throw() : psi(0) {}
        solid_angle(const T& p_psi) throw() : psi(p_psi) {}

        T& operator[] (unsigned n) throw(out_of_range_exception)
        {
            if (!n)
                return psi;
            throw out_of_range_exception();
        }

        const T& operator[] (unsigned n) const throw(out_of_range_exception)
        {
            if (!n)
                return psi;
            throw out_of_range_exception();
        }

        unsigned dimension() const throw()
        {
            return 1;
        }
    };

    /// Specialization solid_angle<T, N> for 2-dimensional solid angle.
    template<typename T>
    class solid_angle<T, 2>
    {
    public:
        static const unsigned PSI_INDEX = 0, THETA_INDEX = 1;

    public:
        T psi, theta;

    private:
        static const unsigned N = 2;
        typedef T solid_angle::*pointer_to_member;

        static inline pointer_to_member get_pointer(unsigned n) throw(out_of_range_exception)
        {
            static const pointer_to_member pointers[] = { &solid_angle::psi, &solid_angle::theta };
            if(n >= N)
                throw out_of_range_exception();
            return pointers[n];
        }

    public:
        solid_angle() throw() : psi(0), theta(0) {}
        solid_angle(const T& p_psi, const T& p_theta) throw() : psi(p_psi), theta(p_theta) {}

        T& operator[] (unsigned n) throw(out_of_range_exception)
        {
            return this->*get_pointer(n);
        }

        const T& operator[] (unsigned n) const throw(out_of_range_exception)
        {
            return this->*get_pointer(n);
        }

        unsigned dimension() const throw()
        {
            return N;
        }
    };

    /// Specialization solid_angle<T, N> for 3-dimensional solid angle.
    template<typename T>
    class solid_angle<T, 3>
    {
    public:
        static const unsigned PSI_INDEX = 0, THETA_INDEX = 1, PHI_INDEX = 2;

    public:
        T psi, theta, phi;

    private:
        static const unsigned N = 3;
        typedef T solid_angle::*pointer_to_member;

        static inline pointer_to_member get_pointer(unsigned n) throw(out_of_range_exception)
        {
            static const pointer_to_member pointers[] = { &solid_angle::psi, &solid_angle::theta, &solid_angle::phi };
            if(n >= N)
                throw out_of_range_exception();
            return pointers[n];
        }

    public:
        solid_angle() throw() : psi(0), theta(0), phi(0) {}
        solid_angle(const T& p_psi, const T& p_theta, const T& p_phi) throw()
            : psi(p_psi), theta(p_theta), phi(p_phi) {}

        T& operator[] (unsigned n) throw(out_of_range_exception)
        {
            return this->*get_pointer(n);
        }

        const T& operator[] (unsigned n) const throw(out_of_range_exception)
        {
            return this->*get_pointer(n);
        }

        unsigned dimension() const throw()
        {
            return N;
        }
    };
}
