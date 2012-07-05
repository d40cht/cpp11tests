#include "fun.hpp"

#include <set>
#include <string>
#include <vector>
#include <algorithm>
#include <functional>
#include <type_traits>

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
    
    /*template<typename res_t>
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
    }*/
    
    // http://stackoverflow.com/questions/11344063/c11-type-inference-with-lambda-and-stdfunction
    
    template<typename Functor>
    struct map_ret_type_helper
    {
        typedef decltype(std::declval<Functor>()( std::declval<typename container_data::el_t>() )) functorRet_t;
        typedef typename container_data::template other_t<functorRet_t>::type resContainerData_t;
        typedef container_wrapper<resContainerData_t> res_t;
    };
    
    template<typename Functor>
    typename map_ret_type_helper<Functor>::res_t map( Functor fn )
    {
        typedef typename map_ret_type_helper<Functor>::resContainerData_t resContainerData_t;
        
        resContainerData_t res;
        
        for ( auto v : m_data.m_container )
        {
            res.add( fn(v) );
        }
        
        return container_wrapper<resContainerData_t>( res );
    }
    
    template<typename Functor>
    self_t sort( Functor fn )
    {
        container_data res;
        for ( auto v : m_data.m_container ) res.add(v);
        std::sort( res.m_container.begin(), res.m_container.end(), fn );
        
        return self_t( res );
    }
    
    template<typename Functor>
    self_t filter( Functor fn )
    {
        container_data res;
        for ( auto v : m_data.m_container ) if (fn(v)) res.add(v);
        
        return self_t( res );
    }
    
    template<typename res_t, typename Functor>
    res_t foldLeft( res_t acc, Functor fn )
    {
        for ( auto v : m_data.m_container )
        {
            acc = fn(acc, v);
        }
        
        return acc;
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

// TODO: fwrap construction currently takes a copy. Add additional set of types
// that wrap using a reference but build into new containers

// filter, reverse, iterator views making lazy expression chains etc.
// toList, toSet, groupBy, unique etc.


template<typename InT, typename Functor>
auto unary_apply( InT val, Functor fn ) -> decltype(fn(val))
{
    return fn(val);
}


class BoomT
{
public:
    double operator()( const int& v ) { return 3.0 * v; }
};

void test()
{
    std::vector<int> a = { 3, 4, 1, 2, 3, 6, 7, 4, 2, 8 };
    std::set<int> s = { 3, 4, 1, 2, 3, 6, 7, 4, 2, 8 };
    
    double blah = unary_apply( 2, []( int v ) { return 3.0 * v; } );
    
    auto fn = []( int v ) { return 3.0 * v; };
    
    typedef decltype(fn) lambda_t;
    decltype(fn( std::declval<int>() )) blah2 = 3.0;
    decltype(std::declval<lambda_t>()( std::declval<int>() )) blah3 = 4.0;

    auto wa = fwrap(a)
        .map( []( const int& v ) { return v+1; } )
        .map( []( const int& v ) { return v * 1.5; } )
        .sort( []( const double& l, const double& r ) { return l < r; } );
        
    //auto wb = fwrap(a).toSet();
        
    double sum = wa.foldLeft(0.0, []( const double& l, const double& r ) { return l + r; } );
    double max = wa.foldLeft(std::numeric_limits<double>::min(), []( const double& l, const double& r ) { return std::max(l, r); } );
        
    auto sa = fwrap(s).map( []( const int& v ) { return v * 3.0; } ).filter( [](const double& v) { return v > 10.0; } );
    
    
}


