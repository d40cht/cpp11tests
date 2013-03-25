#pragma once

#include <functional>
#include <vector>
#include <cstddef>

#include "checks.hpp"
#include "hashtable.hpp"


template<typename K, typename V>
class HashTable
{
private:
    class HashRow
    {
    public:
        HashRow( size_t defaultCapacity )
        {
            m_slots.reserve( defaultCapacity );
        }
        
        void insert( const std::pair<K, V>& kv )
        {
            m_slots.push_back( kv );
        }
        
        bool erase( const K& key )
        {
            for ( auto it = m_slots.begin(); it != m_slots.end(); ++it )
            {
                if ( it->first == key )
                {
                    m_slots.erase(it);
                    return true;
                }
            }
            
            return false;
        }
        
        bool find( const K& key )
        {
            for ( size_t i = 0; i < m_slots.size(); ++i )
            {
                if ( m_slots[i].first == key )
                {
                    return true;
                }
            }
            return false;
        }
        
    private:    
        std::vector<std::pair<K, V>> m_slots;
    };
    
private:
    size_t bi( const K& k )
    {
        return m_hashFn(k) % m_numBuckets;
    }
    
public:
    HashTable( size_t numBuckets, size_t bucketCapacity ) : m_size(0), m_numBuckets(numBuckets), m_buckets(numBuckets, HashRow(bucketCapacity))
    {
    }
    
    void insert( const std::pair<K, V>& kv )
    {
        m_buckets[bi(kv.first)].insert( kv );
        m_size++;
    }
    
    bool erase( const K& key )
    {
        bool found = m_buckets[bi(key)].erase(key);
        if ( found ) m_size--;
        throwing_assert( m_size >= 0, "Number of bucket elements is negative" );
        
        return found;
    }
    
    bool find( const K& key )
    {
        return m_buckets[bi(key)].find(key);
    }
    
    int size() { return m_size; }
    
private:
    int                     m_size;
    size_t                  m_numBuckets;
    std::vector<HashRow>    m_buckets;
    std::hash<K>            m_hashFn;
};


