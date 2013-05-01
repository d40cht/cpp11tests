#include "checks.hpp"

#include <thread>
#include <future>
#include <utility>
#include <vector>

/*
    Memory ordering:
    
    (afaik, these affect in-thread re-ordering as a result of optimisations - and how the consequent writes are visible to other threads).
    
    - memory_order_relaxed 	Relaxed ordering: there are no synchronization or ordering constraints, only atomicity is required of this operation.
    
    - memory_order_consume 	A load operation with this memory order performs a consume operation on the affected memory location: prior writes to data-dependent memory locations made by the thread that did a release operation become visible to this thread.
    
    - memory_order_acquire 	A load operation with this memory order performs the acquire operation on the affected memory location: prior writes made to other memory locations by the thread that did the release become visible in this thread.
    
    - memory_order_release 	A store operation with this memory order performs the release operation: prior writes to other memory locations become visible to the threads that do a consume or an acquire on the same location.
    
    - memory_order_acq_rel 	A load operation with this memory order performs the acquire operation on the affected memory location and a store operation with this memory order performs the release operation.
    
    - memory_order_seq_cst 	Same as memory_order_acq_rel, and a single total order exists in which all threads observe all modifications (see below) 
 
*/

void atomicTest()
{
    std::atomic<int> a1 = {0};
    
    std::vector<std::thread> threads;
    for ( int i = 0; i < 1000; ++i )
    {
        threads.push_back( std::thread( [&a1]()
        {
            // Sequentially consistent
            a1.fetch_add(1, std::memory_order_seq_cst);
        } ) );
    }
    
    for ( auto& t : threads ) t.join();
    
    CHECK_EQUAL( a1, 1000 );
}

void mutexTest()
{
    // From the standard (1.10 Multi-threaded executions and data races, para 5):
    // Informally, performing a release operation on [mutex] A forces prior side eﬀects on other
    // memory locations to become visible to other threads that later perform a consume or an acquire operation on A.
    // Fence - synchronises-with.
    
    std::mutex vlock;
    int v = 0;
    
    std::vector<std::thread> threads;
    for ( int i = 0; i < 1000; ++i )
    {
        threads.push_back( std::thread( [&v, &vlock]()
        {
            std::lock_guard<std::mutex> scopeLock( vlock );
            
            v += 1;
        } ) );
    }
    
    for ( auto& t : threads ) t.join();
    
    CHECK_EQUAL( v, 1000 );
}

void callOnceTest()
{
    std::once_flag onceFlag;
    int value = 2;
    
    std::vector<std::thread> threads;
    for ( int i = 0; i < 10; ++i )
    {
        threads.push_back( std::thread( [&onceFlag, &value]()
        {
            std::call_once( onceFlag, [&value]()
            {
                value = 13;
            } );
            
            CHECK_EQUAL( value, 13 );
        } ) );
    }
    
    for ( auto& t : threads )
    {
        t.join();
    }
}

void promiseFutureTest()
{
    std::promise<int> p;
    
    auto future = p.get_future();
    
    std::thread foo( [&p]()
    {
        std::cout << "Foo bar" << std::endl;
        p.set_value(12);
    } );
    
    CHECK_EQUAL( future.get(), 12 );
    
    foo.join();
}

void packagedTaskTest()
{
    std::packaged_task<int()> task( []
    {
        return 14;
    } );
    
    auto future = task.get_future();
    
    std::thread foo( std::ref( task ) );

    CHECK_EQUAL( future.get(), 14 );
    
    foo.join();
}

#define RUN_TEST( name ) std::cout << "Running: " << #name << std::endl; name();

int main( int /*argc*/, char** /*argv*/ )
{
    std::cerr << "Running concurrency tests" << std::endl;
    RUN_TEST( promiseFutureTest );
    RUN_TEST( packagedTaskTest );
    RUN_TEST( callOnceTest );
    RUN_TEST( atomicTest );
    RUN_TEST( mutexTest );
    std::cerr << "Complete" << std::endl;
}

