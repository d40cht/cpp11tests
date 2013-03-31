#include "hashtable.hpp"
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
    heapTest();
    std::cerr << "Complete" << std::endl;
}

