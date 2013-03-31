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
        auto next = m_storage.first;
        m_storage.first = m_storage.last;
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
    T& foiGet( fromOneIndex_t index ) { return m_storage[index-1]; }
    
    // Note: both bubble_up and bubble_down accept indices starting from 1
    // in order to simplify the maths. But they require the
    // 1 to be subtracted before finally doing a vector access
    void bubble_up( fromOneIndex_t childIndex )
    {
        while ( childIndex != 1 )
        {
            auto parentIndex = index / 2;
            T& parent = get(parentIndex);
            T& child = get(childIndex);
            
            if ( Comparison( child, parent ) )
            {
                std::swap( child, parent );
                childIndex = parentIndex;
            }
            else break;
        }
    }
    
    void bubble_down( fromOneIndex_t parentIndex )
    {
        while ( parentIndex <= m_storage.size() )
        {
            auto childIndex1 = index * 2;
            auto childIndex2 = childIndex1 + 1;
            
            T& parent = get(parentIndex);
            T& child1 = get(childIndex1);
            T& child2 = get(childIndex2)
            
            std::pair<fromOneIndex_t, T&> cmp = Comparison( child1, child2 ) ?
                std::make_pair( childIndex1, child1 ) :
                std::make_pair( childIndex2, child2 );
            
            
            if ( Comparison( cmp.second, parent ) )
            {
                std::swap( cmp.second, parent );
                parentIndex = cmp.second;
            }
            else break;
        }
    }
    
private:
    std::vector<T> m_storage;
};


