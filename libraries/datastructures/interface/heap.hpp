#include <algorithm>

template<typename T, typename Comparison=std::less<T>>
class heap
{
private:
    typedef size_t fromOneIndex_t;
    
public:
    void push( const T& val )
    {
        m_storage.push_back( val );
        
        bubble_up(m_storage.size());
    }
    
    T pop()
    {
        auto next = m_storage.front();
        m_storage.front() = m_storage.back();
        m_storage.pop_back();
        
        if ( !empty() )
        {
            bubble_down(1);
        }
        
        return next;
    }
    
    bool empty()
    {
        return m_storage.empty();
    }
    
    size_t size()
    {
        return m_storage.size();
    }

private:
    T& get( fromOneIndex_t index ) { return m_storage[index-1]; }
    
    void validate()
    {
        for ( fromOneIndex_t i = 2; i <= size(); ++i )
        {
            CHECK( !m_cmp( get(i), get(i/2) ) );
        }
    }
    
    // Note: both bubble_up and bubble_down accept indices starting from 1
    // in order to simplify the maths. But they require the
    // 1 to be subtracted before finally doing a vector access
    void bubble_up( fromOneIndex_t childIndex )
    {
        while ( childIndex != 1 )
        {
            auto parentIndex = childIndex / 2;
            T& parent = get(parentIndex);
            T& child = get(childIndex);
            
            if ( m_cmp( child, parent ) )
            {
                std::swap( child, parent );
                childIndex = parentIndex;
            }
            else break;
        }
    }
    
    void bubble_down( fromOneIndex_t parentIndex )
    {
        while ( parentIndex < size() )
        {
            auto childIndex1 = parentIndex * 2;
            
            if ( childIndex1 <= size() )
            {
                auto childIndex2 = childIndex1 + 1;
                
                // Quick hack to deal with there being only a child1 remaining
                if ( childIndex2 > size() ) childIndex2 = childIndex1;
                
                T& parent = get(parentIndex);
                
                T& child1 = get(childIndex1);
                T& child2 = get(childIndex2);
                
                typedef std::pair<fromOneIndex_t, T&> cmp_t;
                
                cmp_t cmp = m_cmp( child1, child2 ) ?
                    cmp_t( childIndex1, child1 ) :
                    cmp_t( childIndex2, child2 );
                
                
                if ( m_cmp( cmp.second, parent ) )
                {
                    std::swap( cmp.second, parent );
                    parentIndex = cmp.first;
                }
                else break;
            }
            else break;
        }
        
        validate();
    }
    
private:
    Comparison      m_cmp;
    std::vector<T>  m_storage;
};


