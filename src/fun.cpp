#include "fun.hpp"

#include <set>
#include <string>
#include <vector>
#include <algorithm>
#include <functional>

template<typename ElT, typename AllocT>
struct vector_data
{
    typedef ElT el_t;
    typedef std::vector<ElT, AllocT> container_t;
    
    template<typename OtherElT> struct other_t
    {
        typedef vector_data<OtherElT, std::allocator<OtherElT>> type;
    };
    
    vector_data()
    {
    }
    
    vector_data( const container_t& container ) : m_container( container )
    {
    }
    
    void add( const el_t& el )
    {
        m_container.push_back( el );
    }
    
    container_t m_container;
};

template<typename ElT, typename CompareT, typename AllocT>
struct set_data
{
    typedef ElT el_t;
    typedef std::set<ElT, CompareT, AllocT> container_t;
    
    template<typename OtherElT> struct other_t
    {
        typedef set_data<OtherElT, std::less<OtherElT>, std::allocator<OtherElT>> type;
    };
    
    set_data()
    {
    }
    
    set_data( const container_t& container ) : m_container( container )
    {
    }
    
    void add( const el_t& el )
    {
        m_container.insert( el );
    }
    
    container_t m_container;
};


template<typename container_data>
struct container_wrapper
{
    typedef container_wrapper<container_data> self_t;
    
    container_wrapper( container_data data ) : m_data(data)
    {
    }
    
    container_data m_data;
    
    template<typename res_t>
    auto map( std::function<res_t( const typename container_data::el_t& )> fn ) ->
        container_wrapper<typename container_data::template other_t<res_t>::type>
    {
        typedef typename container_data::template other_t<res_t>::type res_t;
        
        res_t res;
        
        for ( auto v : m_data.m_container )
        {
            res.add( fn(v) );
        }
        
        return container_wrapper<res_t>( res );
    }
    
    self_t sort( std::function<bool(const typename container_data::el_t&, const typename container_data::el_t&)> fn )
    {
        container_data res;
        for ( auto v : m_data.m_container ) res.add(v);
        std::sort( res.m_container.begin(), res.m_container.end(), fn );
        
        return self_t( res );
    }
};


template<typename ElT, typename AllocT>
container_wrapper<vector_data<ElT, AllocT>> fwrap( std::vector<ElT, AllocT>& container )
{
    typedef vector_data<ElT, AllocT> type_data_t;
    
    return container_wrapper<type_data_t>( type_data_t(container) );
}

template<typename ElT, typename CompareT, typename AllocT>
container_wrapper<set_data<ElT, CompareT, AllocT>> fwrap( std::set<ElT, CompareT, AllocT>& container )
{
    typedef set_data<ElT, CompareT, AllocT> type_data_t;
    
    return container_wrapper<type_data_t>( type_data_t(container) );
}




void test()
{
    std::vector<int> a = { 3, 4, 1, 2, 3, 6, 7, 4, 2, 8 };
    std::set<int> s = { 3, 4, 1, 2, 3, 6, 7, 4, 2, 8 };
    
    auto wa = fwrap(a)
        .map<int>( []( const int& v ) { return v+1; } )
        .map<double>( []( const int&v ) { return v * 1.5; } )
        .sort( []( const double& l, const double& r ) { return l < r; } );
        
    auto sa = fwrap(s).map<double>( []( const int& v ) { return v * 3.0; } );
    
    
}


