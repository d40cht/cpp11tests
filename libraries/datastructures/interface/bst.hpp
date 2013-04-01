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
        
        // Returns true for an insert, false for an overwrite
        bool insert( self_t*& parentPointer, const elem_t& element )
        {
            if ( this == NULL )
            {
                parentPointer = new Node( element );
                return true;
            }
            else if ( element.first == m_value.first )
            {
                m_value = element;
                return false;
            }
            else if ( element.first < m_value.first ) return m_left->insert( m_left, element );
            else return m_right->insert( m_right, element );
        }
        
        // Returns true for an erase, false for key not found
        bool erase( self_t*& parentPointer, const K& key )
        {
            if ( this == NULL ) return false;
            else if ( key < m_value.first )
            {
                return m_left->erase( m_left, key );
            }
            else if ( key > m_value.first )
            {
                return m_right->erase( m_right, key );
            }
            else
            {
                if ( m_left == NULL && m_right == NULL )
                {
                    parentPointer = NULL;
                    delete this;
                }
                else if ( m_left == NULL )
                {
                    parentPointer = m_right;
                    delete this;
                }
                else if ( m_right == NULL )
                {
                    parentPointer = m_left;
                    delete this;
                }
                else
                {
                    // Swap with leftmost value in right subtree, then delete
                    // that leftmost node (which may have a right-pointer, so
                    // we need to keep that as appropriate).
                    self_t** parentPointer = &m_right;
                    self_t* iter = m_right;
                    while ( iter->m_left != NULL )
                    {
                        parentPointer = &iter->m_left;
                        iter = iter->m_left;
                    }
                    m_value = iter->m_value;
                    (*parentPointer) = iter->m_right;
                    delete iter;
                }
                
                return true;
            }
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
        
        const elem_t* find( const K& key ) const
        {
            node_t* node = m_root->find( key );
            if ( node == NULL ) return NULL;
            else return &(node->get());
        }
        
        void insert( const elem_t& elem )
        {
            if ( m_root->insert( m_root, elem ) ) m_size++;
            validate();
        }
        
        void erase( const K& key )
        {
            if ( m_root->erase( m_root, key ) ) m_size--;
            validate();
        }
        
        size_t size() const { return m_size; }
        
    private:
        void validate()
        {
            // Check invariants
            std::queue<node_t*> q;
            q.push( m_root );
            
            size_t count = 0;
            while ( !q.empty() )
            {
                auto head = q.front();
                q.pop();
                
                if ( head != NULL )
                {
                    count += 1;
                    if ( head->m_left != NULL )
                    {
                        q.push( head->m_left );
                        CHECK( head->m_left->m_value.first < head->m_value.first );
                        
                    }
                    if ( head->m_right != NULL )
                    {
                        q.push( head->m_right );
                        CHECK( head->m_value.first < head->m_right->m_value.first );
                    }
                }
            }
            
            CHECK_EQUAL( count, m_size );
        }
        
    private:
        node_t* m_root;
        size_t  m_size;
    };
}

