#include "checks.hpp"

#include <thread>
#include <future>
#include <utility>
#include <vector>

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

int main( int /*argc*/, char** /*argv*/ )
{
    std::cerr << "Running concurrency tests" << std::endl;
    promiseFutureTest();
    packagedTaskTest();
    callOnceTest();
    atomicTest();
    mutexTest();
    std::cerr << "Complete" << std::endl;
}

