#include "checks.hpp"

#include <queue>

namespace balanced
{
    template<typename K, typename V> class BST;

    template<typename K, typename V>
    class Node
    {
    public:
        typedef Node<K, V>      self_t;
        typedef std::pair<K, V> elem_t;
        
    public:
        Node( const elem_t& value ) : m_value(value), m_height(0), m_left(NULL), m_right(NULL)
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
        
        void rotl( self_t*& parentPointer )
        {
            self_t* oldRight = m_right;
            m_right = oldRight->m_left;
            oldRight->m_left = this;
            parentPointer = oldRight;
            updateHeight();
            oldRight->updateHeight();
        }
        
        void rotr( self_t*& parentPointer )
        {
            self_t* oldLeft = m_left;
            m_left = oldLeft->m_right;
            oldLeft->m_right = this;
            parentPointer = oldLeft;
            updateHeight();
            oldLeft->updateHeight();
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
            else if ( element.first < m_value.first )
            {
                bool insertion = m_left->insert( m_left, element );
                updateHeight();
                rebalance( parentPointer, false );
                return insertion;
            }
            else
            {
                bool insertion = m_right->insert( m_right, element );
                updateHeight();
                rebalance( parentPointer, false );
                return insertion;
            }
        }
        
        // Returns true for an erase, false for key not found
        bool erase( self_t*& parentPointer, const K& key )
        {
            if ( this == NULL ) return false;
            else if ( key < m_value.first )
            {
                bool erased = m_left->erase( m_left, key );
                updateHeight();
                rebalance( parentPointer, true );
                return erased;
            }
            else if ( key > m_value.first )
            {
                bool erased = m_right->erase( m_right, key );
                updateHeight();
                rebalance( parentPointer, true );
                return erased;
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
                    m_value = m_right->eraseSwap( m_right );
                    updateHeight();
                    rebalance( parentPointer, true );
                }
                
                return true;
            }
        }
    private:
        // Swap with leftmost value in right subtree, then delete
        // that leftmost node (which may have a right-pointer, so
        // we need to keep that as appropriate).
        elem_t eraseSwap( self_t*& parentPointer )
        {
            if ( m_left != NULL )
            {
                auto res = m_left->eraseSwap( m_left );
                updateHeight();
                rebalance( parentPointer, true );
                return res;
            }
            else
            {
                auto toSwap = m_value;
                parentPointer = m_right;
                delete this;
                
                return toSwap;
            }
        }
    
    public:
        
        const elem_t& get() { return m_value; }
        
        void debug( std::string indent ) const
        {
            std::cerr << indent << m_value.first << std::endl;
            std::string newIndent;
            for ( char c : indent ) newIndent.push_back(' ');
            if ( m_left != NULL ) m_left->debug( newIndent + "l->" );
            if ( m_right != NULL ) m_right->debug( newIndent + "r->" );
        }
        
    private:
        std::pair<int, int> childHeights()
        {
            int lh = m_left == NULL ? 0 : m_left->m_height + 1;
            int rh = m_right == NULL ? 0 : m_right->m_height + 1;
            return std::make_pair( lh, rh );
        }
        void updateHeight()
        {
            auto th = childHeights();
            auto lh = th.first;
            auto rh = th.second;
            m_height = std::max( lh, rh );
        }
        
        void rebalance( self_t*& inboundPointer, bool isDelete )
        {
            //isDelete = false;
            auto th = childHeights();
            auto lh = th.first;
            auto rh = th.second;
            int balanceFactor = (lh - rh);
            
            throwing_assert( std::abs(balanceFactor) <= 2, "Node height difference is greater than 2" );
            if ( balanceFactor == -2 )
            {
                auto rth = m_right->childHeights();
                int rbf = rth.first - rth.second;
                
                if ( rbf == -1 || (isDelete && rbf == 0) ) rotl( inboundPointer );
                else if ( rbf == 1 )
                {
                    m_right->rotr( m_right );
                    rotl( inboundPointer );
                }
                
                th = childHeights();
                throwing_assert( std::abs(th.first - th.second) <= 1, "Node height difference is greater than 1 after rebalancing" );
            }
            else if ( balanceFactor == 2 )
            {
                auto lth = m_left->childHeights();
                int lbf = lth.first - lth.second;
                
                if ( lbf == 1 || (isDelete && lbf == 0) ) rotr( inboundPointer );
                else if ( lbf == -1 )
                {
                    m_left->rotl( m_left );
                    rotr( inboundPointer );
                }
                th = childHeights();
                throwing_assert( std::abs(th.first - th.second) <= 1, "Node height difference is greater than 1 after rebalancing" );
            }
            
            
        }
                
    private:
        elem_t          m_value;
        size_t          m_height;
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
            //debug();
            if ( m_root->erase( m_root, key ) ) m_size--;
            validate();
        }
        
        size_t size() const { return m_size; }
        
        void debug() const { if ( m_root != NULL ) m_root->debug(std::string()); }
        
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
                    
                    auto th = head->childHeights();
                    auto lh = th.first;
                    auto rh = th.second;
                    CHECK_EQUAL( (int) head->m_height, std::max( lh, rh ) );
                    
                    auto heightDiff = lh - rh;
                    if ( std::abs( heightDiff ) >= 2 )
                    {
                        std::cerr << "Height difference violation: " << heightDiff << " - " << head->m_value.first << std::endl;
                    }
                    //CHECK( std::abs( lh - rh ) < 2 );
                }
            }
            
            CHECK_EQUAL( count, m_size );
        }
        
    private:
        node_t* m_root;
        size_t  m_size;
    };
}

