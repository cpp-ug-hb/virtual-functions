#include <iostream>
#include <memory_resource>

#include <vector>
#include <memory>

using namespace std;


/**
 * A custom memory resource that logs all allocate and deallocate calls
 * 
 * All calls will be logged and forwarded
 **/
class debug_memory final : public pmr::memory_resource
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
            return m_upstream->is_equal(*other_p->m_upstream);
        } else {
            return false;
        }
    }

  private:
    pmr::memory_resource* m_upstream;
};


int main(int argc, char const *argv[])
{
    // create a debug resource
    debug_memory debug_mem{};

    // create a pmr allocator for vector using debug memory resource
    pmr::polymorphic_allocator<std::vector<int>> alloc(&debug_mem);

    std::cout << "allocate storage for 1 vector \n";
    auto some_vec = alloc.allocate(1);

    std::cout << "construct that vector\n";
    alloc.construct(some_vec);
    
    std::cout << "resize to 100\n";
    some_vec->reserve(100);
    
    std::cout << "resize to 100'000\n";
    some_vec->reserve(100'000);

    std::cout << "destroy vector (call destructor)\n";
    alloc.destroy(some_vec);

    std::cout << "deallocate vector memory\n";
    alloc.deallocate(some_vec, 1);

    std::cout << "done\n";
    return 0;
}
