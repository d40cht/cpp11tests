#include <iostream>

#include <vector>
#include <algorithm>

#include "fun.hpp"

// COMPILER CHANGES
//
// * R-value references
// * default constructor syntax (SomeConstructor() = default;).
// * extern template class std::vector<MyClass>; (do not instantiate the template in this translation unit - it's already elsewhere)
// * Constructors can call other constructors
// * Strongly typed enums
// * Right angle bracket (>>) for closing nested templates
// * User-defined literals
// * Variadic templates (recurse on Args): template<typename T, typename... Args>
// * Thread-local storage
// * Static assertions: static_assert((GREEKPI > 3.14) && (GREEKPI < 3.15), "GREEKPI is inaccurate!");
// * alignof and alignas

// LIBRARY CHANGES
//
// * Many threading improvements
// * Tuple types (using variadic templates)
// * Hash tables (std::unordered_set, map, multimap, multiset)
// * Regular expressions
// * Boost-style smart pointers
// * Boost-style random rng
// * std::ref
// * std::function
// * Boost type-traits
// * std::result_of for computed return types


template<typename T>
void check_equal( const char* FILE, int line, const T& lhs, const T& rhs )
{
    if ( lhs != rhs )
    {
        std::cerr << "Assertion Failure: (" << FILE << ", " << line << "): " << lhs << " != " << rhs << std::endl;
    }
}

#define CHECK_EQUAL( lhs, rhs ) check_equal( __FILE__, __LINE__, (lhs), (rhs) )

struct T1
{
    double   a;
    std::string b;
};

// Uniform initialisation
T1 makeAT1() { return {4.0, "hello"}; }

auto funnyFunctionTypeDecl( int x, int y ) -> int
{
    return x + y;
}

template<typename T>
auto funnyFunctionTypeDecl2( const T& t1, const T& t2 ) -> decltype(t1+t2)
{
    return t1 + t2;
}

// Checked override
namespace t1
{
    struct Base {
        virtual void some_func(float);
    };
     
    struct Derived : Base {
        //virtual void some_func(int) override; // ill-formed because it doesn't override a base class method
    };
}

// Checked final
namespace t2
{
    /*struct Base1 final { };
 
    struct Derived1 : Base1 { }; // ill-formed because the class Base1 has been marked final
     
    struct Base2 {
        virtual void f() final;
    };
     
    struct Derived2 : Base2 {
        void f(); // ill-formed because the virtual function Base2::f has been marked final
    };*/
}

// Template type aliasing
namespace t3
{
    template <typename First, typename Second, int third>
    class SomeType;
     
    template <typename Second>
    using TypedefName = SomeType<float, Second, 5>;
}

// Deleted functions (explicitly not available)
namespace t4
{
    struct NoIntNoCopy {
        NoIntNoCopy(const NoIntNoCopy   &) = delete;
        void f(double i);
        void f(int) = delete;
    };
}

void meanMedian()
{
    std::vector<double> values = { 6.0, 6.0, 3.0, 4.0, 5.0, 8.0, 9.0, 6.0, 4.0, 10.0, 22.0, 5.0 };
    
    int n = values.size();
    
    auto sorted = fwrap(values)
        .sort( []( const double& lhs, const double& rhs ) { return lhs < rhs; } );
        
    auto sliced = sorted
        .zipWithIndex()
        .filter( [n]( const std::pair<double, int>& v ) { return v.second >= n/4 && v.second < 3*(n/4); } )
        .map( []( const std::pair<double, int>& v ) { return v.first; } );

    int weight = static_cast<double>( sliced.size() );
    auto mean = sliced
        .foldLeft(0.0, [weight]( const double& acc, const double& v ) { return acc + v/weight; } );
        
    CHECK_EQUAL( mean, 6.0 );
}

class IterRange
{
    class iterator
    {
    public:
        iterator(int value) : m_value(value)
        {
        }
        
        iterator& operator++()
        {
            m_value++;
            return *this;
        }
        
        int operator*()
        {
            return m_value;
        }
        
        bool operator==( const iterator& other ) { return m_value == other.m_value; }
        bool operator!=( const iterator& other ) { return m_value != other.m_value; }
        
    private:
        int m_value;
    };
    
public:
    IterRange( int start, int end ) : m_start(start), m_end(end)
    {
    }
    
    iterator begin() { return iterator(m_start); }
    iterator end() { return iterator(m_end); }

private:
    int m_start, m_end;
};

void otherTests()
{
    std::vector<double> values = { 6.0, 6.0, 3.0, 4.0, 5.0, 8.0, 9.0, 6.0, 4.0, 10.0, 22.0, 5.0 };
    
    CHECK_EQUAL( fwrap(values).toSet().mkString(";"), std::string("3;4;5;6;8;9;10;22") );
    
    double acc = 0.0;
    for ( auto v : IterRange(10, 20) ) acc += v;
    CHECK_EQUAL( acc, 145.0 );
    
    auto viewSum = fwrap(values).toView()
        .filter( []( const double& v ) { return v >= 8.0; } )
        .map( []( const double& v ) { return v * 3.0; } )
        .foldLeft(0.0, []( const double& acc, const double& v ) { return acc+v; } );
        
    auto nonViewSum = fwrap(values)
        .filter( []( const double& v ) { return v >= 8.0; } )
        .map( []( const double& v ) { return v * 3.0; } )
        .foldLeft(0.0, []( const double& acc, const double& v ) { return acc+v; } );
        
    CHECK_EQUAL( viewSum, 147.0 );
    CHECK_EQUAL( nonViewSum, 147.0 );
}

int main( int argc, char* argv[] )
{
    // Extended initialiser lists. Hurray. Uses initialiser list constructor.
    std::vector<int> test1 = { 4, 5, 6, 0, 1, 2, 3, 7, 8, 9 };
    
    CHECK_EQUAL( test1.size(), 10UL );
    
    // Type inference
    for ( auto it = test1.begin(); it != test1.end(); it++ )
    {
        // Same type as.
        decltype(it) it2 = it; 
    }
    
    // A fold
    int folded = std::accumulate( test1.begin(), test1.end(), 4, [](const int& lhs, const int& rhs) { return lhs + rhs; } );
    CHECK_EQUAL( folded, 49 );
    
    // Range-based for loop
    int acc = 4;
    for ( int& x : test1 ) { acc += x; }
    CHECK_EQUAL( acc, 49 );
    
    // Sort with a local lambda
    std::sort( test1.begin(), test1.end(), []( const int& lhs, const int& rhs ) { return lhs < rhs; } );
    
    // A zip function would be great here...
    CHECK_EQUAL( test1[0], 0 );
    CHECK_EQUAL( test1[1], 1 );
    CHECK_EQUAL( test1[2], 2 );
    
    std::sort( test1.begin(), test1.end(), []( const int& lhs, const int& rhs ) { return lhs > rhs; } );
    CHECK_EQUAL( test1[0], 9 );
    CHECK_EQUAL( test1[1], 8 );
    CHECK_EQUAL( test1[2], 7 );
    
    // A named local lambda
    auto squareFn = []( int v ) { return v * v; };
    
    // Capture a local variable (squareFn) before calculation. Lambda lifting.
    std::for_each( test1.begin(), test1.end(), [squareFn]( int& v ){ v = squareFn(v); } );
    CHECK_EQUAL( test1[0], 81 );
    CHECK_EQUAL( test1[1], 64 );
    CHECK_EQUAL( test1[2], 49 );
    
    meanMedian();
    otherTests();   
}


