#pragma once

#include <iostream>

template<typename T>
inline void check_equal( const char* FILE, int line, const T& lhs, const T& rhs )
{
    if ( lhs != rhs )
    { 
        std::cerr << "Assertion Failure: (" << FILE << ", " << line << "): " << lhs << " != " << rhs << std::endl;
    }
}

#define CHECK_EQUAL( lhs, rhs ) check_equal( __FILE__, __LINE__, (lhs), (rhs) ) 

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

