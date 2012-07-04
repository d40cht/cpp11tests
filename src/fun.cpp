#include "fun.hpp"

#include <set>
#include <string>
#include <vector>
#include <algorithm>
#include <functional>

template<class ContainerType>
class Builder
{
};

template<typename ElType, typename AllocT>
class Builder<std::vector<ElType, AllocT>>
{
public:
    void add( const ElType& el )
    {
        m_data.push_back(el);
    }
    
    std::vector<ElType, AllocT>& get() { return m_data; }

private:
    std::vector<ElType, AllocT> m_data;
};

template<typename ElType>
class Builder<std::set<ElType>>
{
public:
    void add( const ElType& el )
    {
        m_data.insert(el);
    }
    
    std::set<ElType>& get() { return m_data; }

private:
    std::set<ElType> m_data;
};

template<typename InEl, typename OutEl>
std::vector<OutEl> fmap( const std::vector<InEl>& container, std::function<OutEl(const InEl&)> fn )
{
    Builder<std::vector<OutEl>> res;
    
    for ( auto v : container )
    {
        res.add( fn(v) );
    }
    
    return res.get();
}

template<typename ElT>
std::vector<ElT> fsort( const std::vector<ElT>& container, std::function<bool(const ElT&, const ElT&)> ordFn )
{
    Builder<std::vector<ElT>> res;
    for ( auto v : container ) res.add(v);

    std::sort( res.get().begin(), res.get().end(), ordFn );
    
    return res.get();
}

template<typename In, typename Out>
Out apply( std::function<Out(In)> fn, In val ) { return fn(val); }

template<template<class, class> class Container, typename ElType, typename AllocT>
Container<std::string, AllocT> toString( const Container<ElType, AllocT>& inp )
{
    Builder<Container<std::string, AllocT>> res;
    for ( auto v : inp )
    {
        res.add( std::string("ook") );
    }
    
    return res.get();
}


void test()
{

    std::vector<int> a = { 3, 4, 1, 2, 3, 6, 7, 4, 2, 8 };
    
    auto z1 = apply<int, int>( []( int q ) { return q+1; }, 12 );
    auto z2 = apply<int, double>( []( int q ) { return q * 1.5; }, 12 );
    
    Builder<std::vector<double>> b1;
    Builder<std::set<double>> b2;
    b1.add(1.0);
    b2.add(3.0);
    
    auto ooks = toString(a);

    auto b = fmap<int, double>( a, []( const int& v ) { return v*2.0; } );
    auto c = fsort<double>( b, []( const double& x, const double& y ){ return x < y; } );
}

