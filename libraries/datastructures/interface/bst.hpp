#include "checks.hpp"

#include <queue>

namespace unbalanced
{
    template<typename K, typename V> class BST;

    template<typename K, typename V>
    class Node
    {
    public:
        typedef Node<K, V>      self_t;
        typedef std::pair<K, V> elem_t;
        
    public:
        Node( const elem_t& value ) : m_value(value), m_left(NULL), m_right(NULL)
        {
        }
        
        self_t* find( const K& key )
        {
            // Search terminates on finding a NULL or on finding the key
            if ( this == NULL || m_value.first == key ) return this;
            else
            {
                if ( key < m_value.first  ) return m_left->find( key );
                else return m_right->find( key );
            }
        }
        
        void insert( self_t*& parentPointer, const elem_t& element )
        {
            if ( this == NULL ) parentPointer = new Node( element );
            else if ( element.first < m_value.first ) m_left->insert( m_left, element );
            else m_right->insert( m_right, element );
        }
        
        void erase( self_t*& parentPointer, const K& key )
        {
        }
        
        const elem_t& get() { return m_value; }
                
    private:
        elem_t          m_value;
        Node<K, V>*     m_left;
        Node<K, V>*     m_right;
        
        friend class BST<K, V>;
    };

    template<typename K, typename V>
    class BST
    {
    private:
        typedef Node<K, V> node_t;
        
    public:
        typedef typename node_t::elem_t elem_t;
        
    public:
        BST() : m_root(NULL), m_size(0)
        {
        }
        
        const elem_t* find( const K& key )
        {
            node_t* node = m_root->find( key );
            if ( node == NULL ) return NULL;
            else return &(node->get());
        }
        
        void insert( const elem_t& elem )
        {
            m_root->insert( m_root, elem );
            m_size++;
            validate();
        }
        
        void erase( const K& key )
        {
            m_root->erase( m_root, key );
            m_size--;
            validate();
        }
        
    private:
        void validate()
        {
            // Check invariants
            std::queue<node_t*> q;
            q.push( m_root );
            
            size_t count = 0;
            while ( !q.empty() )
            {
                auto first = q.front();
                q.pop();
                
                if ( first != NULL )
                {
                    count += 1;
                    q.push( first->m_left );
                    q.push( first->m_right );
                }
            }
            
            CHECK_EQUAL( count, m_size );
        }
        
    private:
        node_t* m_root;
        size_t  m_size;
    };
}

