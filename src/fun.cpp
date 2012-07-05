#include "fun.hpp"



void meanMedian()
{
    std::vector<double> values = { 3.0, 4.0, 1.0, 2.0, 3.0, 6.0, 7.0, 4.0, 2.0, 8.0 };
    
    int n = values.size();
    
    auto res = fwrap(values)
        .sort( []( const double& lhs, const double& rhs ) { return lhs < rhs; } )
        .zipWithIndex()
        .filter( [n]( const std::pair<double, int>& v ) { return v.second > n/4 && v.second < 3*(n/4); } )
        .map( []( const std::pair<double, int>& v ) { return v.first; } )
        .foldLeft(0.0, [n]( const double& acc, const double& v ) { return acc + v/static_cast<double>(n); } );
}

void test()
{
    std::vector<int> a = { 3, 4, 1, 2, 3, 6, 7, 4, 2, 8 };
    std::set<int> s = { 3, 4, 1, 2, 3, 6, 7, 4, 2, 8 };
    std::list<int> l = { 4, 3, 1, 6, 7, 5, 10 };
    
    std::map<int, double> mo = { {1, 2.0}, {2, 3.0}, {4, 5.0}, {6, 7.0}, {8, 9.0} };
    std::list<std::pair<int, double>> lo = { {1, 2.0}, {2, 3.0}, {4, 5.0}, {6, 7.0}, {8, 9.0} };
    
    auto pairList = fwrap(lo);
    auto mapO = fwrap(mo);
    
    auto uniquea = fwrap(a).unique();
    
    // Currently not possible as requires a container of pairs and that's not always the case
    //auto genMap = pairList.toMap();
    
    auto reverseAssoc = mapO
        .map( []( const std::pair<int, double>& v ){ return std::make_pair( v.second, v.first ); } )
        .toVector()
        .map( []( const std::pair<double, int>& v ){ return v.second; } );
    
    auto listO = fwrap(l).toSet().toList();
    
    auto orderedUnique = fwrap(a).toSet().toVector();
    
    auto wa = fwrap(a)
        .map( []( const int& v ) { return v+1; } )
        .map( []( const int& v ) { return v * 1.5; } )
        .sort( []( const double& l, const double& r ) { return l < r; } );
        
    double sum = wa.foldLeft(0.0, []( const double& l, const double& r ) { return l + r; } );
    double max = wa.foldLeft(std::numeric_limits<double>::min(), []( const double& l, const double& r ) { return std::max(l, r); } );
        
    auto sa = fwrap(s)
        .map( []( const int& v ) { return v * 3.0; } )
        .filter( [](const double& v) { return v > 10.0; } );
    
}


