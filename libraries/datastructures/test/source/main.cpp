#include "hashtable.hpp"
#include "openaddressinghashtable.hpp"
#include "mergesort.hpp"
#include "quicksort.hpp"
#include "heap.hpp"

#include <iostream>
#include <algorithm>

void hashTest()
{
    HashTable<int, int> testHashTable( 100, 4 );
    
    testHashTable.insert( std::make_pair( 1, 2 ) );
    CHECK_EQUAL( testHashTable.size(), 1 );
    testHashTable.insert( std::make_pair( 3, 4 ) );
    CHECK_EQUAL( testHashTable.size(), 2 );
    testHashTable.insert( std::make_pair( 5, 6 ) );
    CHECK_EQUAL( testHashTable.size(), 3 );
    
    CHECK_EQUAL( testHashTable.find( 1 ), true );
    CHECK_EQUAL( testHashTable.find( 2 ), false );
    CHECK_EQUAL( testHashTable.find( 3 ), true );
    CHECK_EQUAL( testHashTable.find( 4 ), false );
    CHECK_EQUAL( testHashTable.find( 5 ), true );
    
    testHashTable.erase(1);
    CHECK_EQUAL( testHashTable.find( 1 ), false );
    CHECK_EQUAL( testHashTable.find( 2 ), false );
    CHECK_EQUAL( testHashTable.find( 3 ), true );
    CHECK_EQUAL( testHashTable.find( 4 ), false );
    CHECK_EQUAL( testHashTable.find( 5 ), true );
    
    testHashTable.erase(5);
    CHECK_EQUAL( testHashTable.find( 1 ), false );
    CHECK_EQUAL( testHashTable.find( 2 ), false );
    CHECK_EQUAL( testHashTable.find( 3 ), true );
    CHECK_EQUAL( testHashTable.find( 4 ), false );
    CHECK_EQUAL( testHashTable.find( 5 ), false );
    
    testHashTable.insert( std::make_pair( 5, 7 ) );
    CHECK_EQUAL( testHashTable.find( 1 ), false );
    CHECK_EQUAL( testHashTable.find( 2 ), false );
    CHECK_EQUAL( testHashTable.find( 3 ), true );
    CHECK_EQUAL( testHashTable.find( 4 ), false );
    CHECK_EQUAL( testHashTable.find( 5 ), true );
}

void openAddressingHashTest()
{
    OpenAddressingHashTable<int, std::string> h(8);
    
    h.insert( std::make_pair( 1, std::string("foo") ) );
    CHECK( h.find(1) );
    CHECK( !h.find(2) );
    CHECK( !h.find(3) );
    CHECK( !h.find(4) );
    
    h.insert( std::make_pair( 2, std::string("bar") ) );
    CHECK( h.find(1) );
    CHECK( h.find(2) );
    CHECK( !h.find(3) );
    CHECK( !h.find(4) );
    h.insert( std::make_pair( 3, std::string("bippy") ) );
    CHECK( h.find(1) );
    CHECK( h.find(2) );
    CHECK( h.find(3) );
    CHECK( !h.find(4) );
    h.insert( std::make_pair( 4, std::string("dingus") ) );
    CHECK( h.find(1) );
    CHECK( h.find(2) );
    CHECK( h.find(3) );
    CHECK( h.find(4) );
    CHECK_EQUAL( h.size(), 4U );
    
    h.insert( std::make_pair( 5, std::string("foo") ) );
    h.insert( std::make_pair( 6, std::string("bar") ) );
    h.insert( std::make_pair( 7, std::string("bippy") ) );
    h.insert( std::make_pair( 8, std::string("dingus") ) );
    CHECK_EQUAL( h.size(), 8U );
    
    CHECK( h.find(1) );
    CHECK( h.find(2) );
    CHECK( h.find(3) );
    CHECK( h.find(4) );
    CHECK( h.find(5) );
    CHECK( h.find(6) );
    CHECK( h.find(7) );
    CHECK( h.find(8) );
    CHECK( !h.find(9) );
    CHECK( !h.find(10) );
    
    CHECK( h.erase(1) );
    CHECK_EQUAL( h.size(), 7U );
    CHECK( !h.find(1) );
    CHECK( h.find(2) );
    CHECK( h.find(3) );
    CHECK( h.find(4) );
    CHECK( h.find(5) );
    CHECK( h.find(6) );
    CHECK( h.find(7) );
    CHECK( h.find(8) );
    CHECK( !h.find(9) );
    CHECK( !h.find(10) );
    
    CHECK( h.erase(5) );
    CHECK_EQUAL( h.size(), 6U );
    CHECK( !h.find(1) );
    CHECK( h.find(2) );
    CHECK( h.find(3) );
    CHECK( h.find(4) );
    CHECK( !h.find(5) );
    CHECK( h.find(6) );
    CHECK( h.find(7) );
    CHECK( h.find(8) );
    CHECK( !h.find(9) );
    CHECK( !h.find(10) );
    
    CHECK( h.erase(2) );
    CHECK( h.erase(3) );
    CHECK( h.erase(4) );
    CHECK_EQUAL( h.size(), 3U );
    CHECK( !h.find(1) );
    CHECK( !h.find(2) );
    CHECK( !h.find(3) );
    CHECK( !h.find(4) );
    CHECK( !h.find(5) );
    CHECK( h.find(6) );
    CHECK( h.find(7) );
    CHECK( h.find(8) );
    CHECK( !h.find(9) );
    CHECK( !h.find(10) );
    
}

void mergeSortTest()
{
    std::vector<int> input = { 4, 1, 2, 3, 6, 1, 5, 3, 7, 6 };
    std::vector<int> results = input;
    
    mergeSort( input );
    CHECK_EQUAL( input.size(), results.size() );
    std::sort( results.begin(), results.end() );
    
    for ( size_t i = 0; i < results.size(); ++i )
    {
        CHECK_EQUAL( input[i], results[i] );
    }
}

void quickSortTest()
{
    std::vector<int> input = { 4, 1, 2, 3, 6, 1, 5, 3, 7, 6 };
    std::vector<int> results = input;
    
    quickSort( input );
    CHECK_EQUAL( input.size(), results.size() );
    std::sort( results.begin(), results.end() );
    
    for ( size_t i = 0; i < results.size(); ++i )
    {
        CHECK_EQUAL( input[i], results[i] );
    }
}

void heapTest()
{  
    std::vector<int> input = { 4, 1, 2, 3, 6, 1, 5, 3, 7, 6, 0, 0, 0, 0, 100, 100, 100, 100, 100 };

    heap<int> h;
    for ( int el : input )
    {
        h.push( el );
    }
    
    std::vector<int> result( input.begin(), input.end() );
    std::sort( result.begin(), result.end() );
    
    for ( int el : result )
    {
        CHECK( !h.empty() );
        CHECK_EQUAL( h.pop(), el );
    }
    CHECK( h.empty() );
}

int main( int /*argc*/, char** /*argv*/ )
{
    std::cerr << "Running data structure tests" << std::endl;
    mergeSortTest();
    quickSortTest();
    hashTest();
    openAddressingHashTest();
    heapTest();
    std::cerr << "Complete" << std::endl;
}

