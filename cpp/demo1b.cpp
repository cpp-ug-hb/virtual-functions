#include <iostream>
#include <memory_resource>

#include <vector>
#include <memory>

using namespace std;


/**
 * A custom memory resouce that logs all allocate and deallocate calls
 * 
 * All call wil be logged and forward
 **/
class debug_memory : public pmr::memory_resource
{
  public:
    /**
     * store the upstream resource
     * default: std::pmr::get_default_resource
     **/
    debug_memory(pmr::memory_resource* upstream = pmr::get_default_resource()) 
        : m_upstream{upstream}
    {
    }

  private:
    /**
     * call upstream and log
     **/
    void* do_allocate(std::size_t bytes, std::size_t alignment) override
    {
        auto p = m_upstream->allocate(bytes, alignment);
        std::cout << "alloc   " << p << ", size: " << bytes << ", alignment: " << alignment << std::endl;
        return p;
    }

    /**
     * log and call upstream
     **/
    void do_deallocate(void* p, std::size_t bytes, std::size_t alignment) override
    {
        std::cout << "dealloc   " << p << ", size: " << bytes << ", alignment: " << alignment << std::endl;
        m_upstream->deallocate(p, bytes, alignment);
    }

    /**
     * debug_memorys are equal if both have the same upstream
     **/
    bool do_is_equal(const pmr::memory_resource& other) const noexcept override
    {
        auto other_p = dynamic_cast<debug_memory const*>(&other);
        if (other_p) {
            return m_upstream == other_p->m_upstream;
        } else {
            return false;
        }
    }

  private:
    pmr::memory_resource* m_upstream;
};


/**
 * a class using a custom allocator
 **/
class ExampleData {
  public:
    using allocator_type = std::pmr::polymorphic_allocator<std::byte>;


    ExampleData(int i) 
    {
        std::cout << "ExampleData constructed without allocator: " << i << '\n';
    }

    ExampleData(int i, allocator_type alloc) 
    {
        std::cout << "ExampleData constructed with allocator: " << i << '\n';
    }

    ExampleData(ExampleData const&, allocator_type alloc)
    {
        std::cout << "ExampleData copy-constructed with allocator"  << '\n';
    }

    ExampleData(ExampleData &&, allocator_type alloc)
    {
        std::cout << "ExampleData move-constructed with allocator"  << '\n';
    }
};

int main(int argc, char const *argv[])
{
    // create a debug resource
    debug_memory debug_mem{};

    // create a pmr allocator for vector using debug memory resource
    std::cout << "create some pmr vector\n";
    std::pmr::vector<ExampleData> some_vec(&debug_mem);

    std::cout << "resize to 5\n";
    for (int i = 0; i < 5; ++i)
    {
        std::cout << "\ncreate example data " << i << '\n';
        some_vec.emplace_back(i);
    }
    
    std::cout << "done first part\n\n\n";


    // allocate_shared requires pmr::polymorphic_allocator
    std::pmr::polymorphic_allocator<std::byte> alloc(&debug_mem);

    // allocate_shared will propagate the allocator to the object if the constructor
    // supports it
    auto other_vec = std::allocate_shared<std::pmr::vector<ExampleData>>(alloc);
    other_vec->emplace_back(6);

    other_vec.reset();

    return 0;
}
