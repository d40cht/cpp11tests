#pragma once

#include <functional>
#include <vector>
#include <limits>

template<typename K, typename V>
class OpenAddressingHashTable
{
private:
    size_t bi( const K& k ) const
    {
        return m_hashFn(k) % m_numBuckets;
    }
    
    void inc( size_t& index ) const { index = (index+1) % m_numBuckets; }
    
    size_t invalidIndex = std::numeric_limits<size_t>::max();

    size_t findByIndex( const K& k ) const
    {
        size_t oh = bi( k );
        size_t i = oh;
        while ( m_assigned[i] )
        {
            auto el = m_cellData[i];
            auto h = bi( el.first );
            
            // Either we've found the key, or we're no longer on this hash
            if ( el.first == k ) return i;
            else if ( h != oh ) return invalidIndex;
            
            inc(i);
        }
        return invalidIndex;
    }  
        
public:
    OpenAddressingHashTable( size_t initialCapacity ) :
        m_numBuckets(initialCapacity),
        m_size(0),
        m_cellData( initialCapacity, std::make_pair( K(), V() ) ),
        m_assigned( initialCapacity, false )
    {
    }
    
    size_t size() const { return m_size; }
    
    void insert( const std::pair<K, V>& v )
    {
        size_t i = bi( v.first );
        while ( m_assigned[i] )
        {
            inc(i);
        }
        m_cellData[i] = v;
        m_assigned[i] = true;
        m_size += 1;
    }
    
    bool find( const K& k ) const { return findByIndex(k) != invalidIndex; }
    
    V get( const K& k )
    {
        size_t index = findByIndex(k);
        throwing_assert( index != invalidIndex, "Key not found in get" );
        return m_cellData[index].second;
    }
    
    bool erase( const K& k )
    {
        auto kh = bi(k);
        size_t i = findByIndex(k);
        
        if ( i == invalidIndex ) return false;
        
        size_t lastDup = i;
        inc(i);
        
        // If this key is in the middle of a run, shift the run down
        while ( m_assigned[i] && bi( m_cellData[i].first ) == kh )
        {
            std::swap( m_cellData[lastDup], m_cellData[i] );
            lastDup = i;
            inc(i);
        }
        
        m_assigned[lastDup] = false;
        m_size--;
        
        return true;
    }
    
private:
    size_t                          m_numBuckets;
    size_t                          m_size;
    std::vector<std::pair<K, V>>    m_cellData;
    std::vector<bool>               m_assigned;
    std::hash<K>                    m_hashFn;
};

