#ifndef AFINA_STORAGE_SIMPLE_LRU_H
#define AFINA_STORAGE_SIMPLE_LRU_H

#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <string>

#include <afina/Storage.h>

namespace Afina {
namespace Backend {

/**
 * # Map based implementation
 * That is NOT thread safe implementaiton!!
 */

class SimpleLRU : public Afina::Storage {
public:
    SimpleLRU(size_t max_size = 1024) : _max_size(max_size), _allocated_memory(0) {}

    ~SimpleLRU();

    // Implements Afina::Storage interface
    // add const
    bool Put(const std::string &key, const std::string &value) override;

    // Implements Afina::Storage interface
    bool PutIfAbsent(const std::string &key, const std::string &value) override;

    // Implements Afina::Storage interface
    bool Set(const std::string &key, const std::string &value) override;

    // Implements Afina::Storage interface
    bool Delete(const std::string &key) override;

    // Implements Afina::Storage interface
    bool Get(const std::string &key, std::string &value) override;

    // Print all items in Storage
    void PrintStorage();

private:
    // LRU cache node
    using lru_node = struct lru_node {
        const std::string key;
        std::string value;
        lru_node *prev;
        std::unique_ptr<lru_node> next;

        lru_node(const std::string &key, const std::string &value) : key(key), value(value){};
    };

    using string_wrapper = std::reference_wrapper<const std::string>;
    using node_wrapper = std::reference_wrapper<lru_node>;

    bool prepareLRU(const int record_size);
    // Function that free last elements;
    // Returns number realised bytes

    std::size_t freeTail(const int req_mem);
    // Add node at head of list
    void addNode(const std::string &key, const std::string &value);
    // move node to head
    void moveNode(const std::map<string_wrapper, node_wrapper>::iterator &it);
    std::size_t deleteNode(const std::map<string_wrapper, node_wrapper>::iterator &it);

    //--------------------------------------------------------------

    // Maximum number of bytes could be stored in this cache.
    // i.e all (keys+values) must be less the _max_size
    std::size_t _max_size;
    // Save number of busy bytes
    std::size_t _allocated_memory;

    // Main storage of lru_nodes, elements in this list ordered descending by "freshness": in the head
    // element that wasn't used for longest time.

    // List owns all nodes
    std::unique_ptr<lru_node> _lru_head;
    // Index of nodes from list above, allows fast random access to elements by lru_node#key
    std::map<string_wrapper, node_wrapper, std::less<std::string>> _lru_index;
};

} // namespace Backend
} // namespace Afina

#endif // AFINA_STORAGE_SIMPLE_LRU_H
