#include <deque>

template<typename T>
void quickSort( std::vector<T>& data )
{
    int length = data.size();
    
    std::deque<std::pair<int, int>> splits = { {0, length-1} };
    
    while( !splits.empty() )
    {
        auto next = splits.front();
        splits.pop_front();
        
        auto storeIndex = next.first;
        auto pivotValue = data[next.second];
        for ( int i = next.first; i < next.second-1; ++i )
        {
            if ( data[i] < pivotValue )
            {
                std::swap( data[i], data[storeIndex] );
                storeIndex += 1;
            }
        }
        std::swap( data[next.second], data[storeIndex] );
        
        auto left = std::make_pair( next.first, storeIndex-1 );
        auto right = std::make_pair( storeIndex+1, next.second );
        
        if ( left.second > left.first ) splits.push_back( left );
        if ( right.second > right.first ) splits.push_back( right );
    }
}


