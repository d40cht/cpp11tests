#include <iostream>

#include <vector>
#include <algorithm>

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
    
    // The return type is implicit (void here)
    std::for_each( test1.begin(), test1.end(), []( int& b ) { b *= b; } );
    CHECK_EQUAL( test1[0], 81 );
    CHECK_EQUAL( test1[1], 64 );
    CHECK_EQUAL( test1[2], 49 );
}


