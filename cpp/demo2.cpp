#include <iostream>
#include <iomanip>
#include <memory_resource>

#include <atomic>
#include <memory>
#include <map>
#include <list>
#include <cstdint>

using namespace std;

#define WITH_SIZES 0

/**
 * A custom memory resouce that logs all allocate and deallocate calls
 * 
 * All call wil be logged and forward
 **/
class counting_memory_resource : public pmr::memory_resource
{
  public:
    /**
     * store the upstream resource
     * default: std::pmr::get_default_resource
     **/
    counting_memory_resource(pmr::memory_resource* upstream = pmr::get_default_resource()) 
        : m_upstream{upstream}
    {
    }

    void printStatus(std::ostream & out, char const* name, char const* end = "\n") const
    {
        auto numAlloc = m_numAllocs.load();
        auto numDealloc = m_numDeallocs.load();
        auto activeAlloc = numAlloc - numDealloc;

        auto numAllocBytes = m_numAllocBytes.load();
        auto numDeallocBytes = m_numDeallocBytes.load();
        auto activeBytes = numAllocBytes - numDeallocBytes;

        out << name
            << " active: " << std::setfill(' ') << std::setw(10) << activeAlloc 
            << ", bytes: " << std::setfill(' ') << std::setw(10) << activeBytes
            << ", num_alloc: " << std::setfill(' ') << std::setw(10) << numAlloc
            << ", num_dealloc: " << std::setfill(' ') << std::setw(10) << numDealloc
            << ", alloc bytes: " << std::setfill(' ') << std::setw(10) << numAllocBytes
            << ", dealloc bytes: " << std::setfill(' ') << std::setw(10) << numDeallocBytes;

            
#if WITH_SIZES
        out << end << "-- sizes: ";

        char const* sep = "";
        for (auto && [size, numAlloc] : m_allocSizes) {
            out << sep << size << ": " << numAlloc;
            sep = ", ";
        }
#endif
        out << end;
    }

  private:
    /**
     * call upstream and log
     **/
    void* do_allocate(std::size_t bytes, std::size_t alignment) override
    {
        auto p = m_upstream->allocate(bytes, alignment);
        if (p) {
            m_numAllocs += 1;
            m_numAllocBytes += bytes;
            #if WITH_SIZES
            m_allocSizes[bytes] += 1;
            #endif
        }
        return p;
    }

    /**
     * log and call upstream
     **/
    void do_deallocate(void* p, std::size_t bytes, std::size_t alignment) override
    {
        if (p) {
            m_numDeallocs += 1;
            m_numDeallocBytes += bytes;
        }
        m_upstream->deallocate(p, bytes, alignment);
    }

    /**
     * debug_memorys are equal if both have the same upstream
     **/
    bool do_is_equal(const pmr::memory_resource& other) const noexcept override
    {
        auto other_p = dynamic_cast<counting_memory_resource const*>(&other);
        if (other_p) {
            return m_upstream == other_p->m_upstream;
        } else {
            return false;
        }
    }

  private:
    pmr::memory_resource* m_upstream;
    std::atomic<std::uint64_t> m_numAllocs{0};
    std::atomic<std::uint64_t> m_numDeallocs{0};
    std::atomic<std::uint64_t> m_numAllocBytes{0};
    std::atomic<std::uint64_t> m_numDeallocBytes{0};

#if WITH_SIZES
    std::map<std::size_t, std::size_t> m_allocSizes;
#endif    
};


/**
 * a class using a custom allocator
 **/
class ExampleData {
  public:
    using allocator_type = std::pmr::polymorphic_allocator<std::byte>;


    ExampleData(int i) 
        : ExampleData{i, allocator_type{pmr::get_default_resource()}}
    {
    }

    ExampleData(int i, allocator_type alloc) 
        : m_data{ size_t(i * 200),  alloc}
    {
    }

    ExampleData(ExampleData const& other, allocator_type alloc)
    {
        m_data = other.m_data;
    }

    ExampleData(ExampleData && other, allocator_type alloc)
    {
        m_data = std::move(other.m_data);
    }
    
  private:
    std::pmr::list<int> m_data;
};

int main(int argc, char const *argv[])
{
    // create a debug resource
    counting_memory_resource counting_mem{};
    std::pmr::memory_resource* mem = &counting_mem;

    // std::pmr::unsynchronized_pool_resource pool{&counting_mem};
    // counting_memory_resource counting_pool{&pool};
    // mem = &counting_pool;

    auto print_status = [&] (std::string name) {
        counting_mem.printStatus(std::cout, (name + " ").c_str());
        //counting_pool.printStatus(std::cout, ("*" + name).c_str());
    };

    print_status("empty    ");
    
    
    // create a scope for some_vector
    {
        std::pmr::map<int, ExampleData> some_data(mem);

        print_status("some_data");

        // create some data
        for (int i = 0; i < 100; ++i)
        {
            some_data.emplace(i, i);
        }

        print_status("with_data");

        for (int i = 0; i < 200; i+=2)
        {
            some_data.erase(i);
            some_data.emplace(i, i);
        }

        print_status("more_data");
    }    

    print_status("done     ");

    return 0;
}
