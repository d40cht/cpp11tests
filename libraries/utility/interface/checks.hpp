#pragma once

#include <iostream>

template<typename T>
inline void check_equal( const char* FILE, int line, const T& lhs, const T& rhs )
{
    if ( lhs != rhs )
    { 
        std::cerr << "Test check failure: (" << FILE << ", " << line << "): " << lhs << " != " << rhs << std::endl;
    }
}

inline void check( const char* FILE, int line, bool predicate, const char* predicateString )
{
    if ( !predicate )
    { 
        std::cerr << "Test check failure: (" << FILE << ", " << line << "): " << predicateString << std::endl;
    }
}

#define CHECK_EQUAL( lhs, rhs ) check_equal( __FILE__, __LINE__, (lhs), (rhs) )
#define CHECK( predicate ) check( __FILE__, __LINE__, (predicate), #predicate )

class AssertionFailureException : public std::exception
{
public:
    AssertionFailureException( const char* message ) : m_message(message)
    {
    }
    
private:
    const char* m_message;
};

inline void throwing_assert( bool predicate, const char* message )
{
    if ( !predicate ) throw AssertionFailureException( message );
}

