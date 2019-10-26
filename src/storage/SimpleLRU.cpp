#include "SimpleLRU.h"

namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h
SimpleLRU::~SimpleLRU() {

    _lru_index.clear();

    lru_node *last = _lru_head.get() ? _lru_head.get()->prev : nullptr;
    while (last != _lru_head.get()) {
        last = last->prev;
        last->next.reset();
    }

    _lru_head.reset();
}

bool SimpleLRU::Put(const std::string &key, const std::string &value) {

    //if not have enough memory
    if (!prepareLRU(key.size() + value.size())){
        return false;
    }

    auto it = _lru_index.find(std::ref(key));
    if (it != _lru_index.end()) {
        deleteNode(it);
    }
    addNode(key, value);

    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) {

    auto it = _lru_index.find(std::ref(key));
    //key exists or not enought memory
    if (it != _lru_index.end() || !prepareLRU(key.size() + value.size())) {
        return false;

    }
    addNode(key, value);
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value) {

    auto it = _lru_index.find(std::ref(key));
    //key not exists or not enought memory
    if (it == _lru_index.end() || !prepareLRU(value.size())) {
        return false;

    }
    deleteNode(it);
    addNode(key, value);

    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key) {

    auto it = _lru_index.find(std::ref(key));
    if (it == _lru_index.end()) {
        return false;

    } else {
        deleteNode(it);
        return true;
    }
}
// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value) {

    auto it = _lru_index.find(std::ref(key));
    if (it == _lru_index.end()) {
        return false;
    } else {
        lru_node &finded_node = it->second.get();
        value.assign(finded_node.value);
        return true;
    }
}

void SimpleLRU::PrintStorage() {
    lru_node *tmp = _lru_head.get();
    while (tmp != nullptr) {
        std::cout << tmp->key << " : " << tmp->value << std::endl;
        tmp = tmp->next.get();
    }
}

//----------------------------------PRIVATE-------------------------------------
//Check that record fit in LRU and free memory if it need
bool SimpleLRU::prepareLRU(const size_t record_size){

    if (record_size > _max_size) {
        return false;

    }

    if (record_size + _allocated_memory >= _max_size) {
        freeTail(record_size + _allocated_memory - _max_size);

    }
    _allocated_memory += record_size;

    return true;
}

size_t SimpleLRU::deleteNode(const std::map<string_wrapper, node_wrapper>::iterator &it) {

    lru_node &finded_node = it->second.get();
    size_t node_size = finded_node.key.size() + finded_node.value.size();

    // head deletion
    if (_lru_head.get() == &finded_node) {
        if (_lru_head.get()->next) {
            _lru_head.get()->next.get()->prev = _lru_head.get()->prev;
        }

        _lru_head.reset(_lru_head.get()->next.release());

    }
    // tail deletion
    else if (_lru_head.get()->prev == &finded_node) {
        _lru_head.get()->prev = finded_node.prev;
        finded_node.prev->next.reset(finded_node.next.release());

    }
    // middle node deletion
    else {
        finded_node.next.get()->prev = finded_node.prev;
        finded_node.prev->next.reset(finded_node.next.release());
    }

    _allocated_memory -= node_size;
    _lru_index.erase(it);

    return node_size;
}

void SimpleLRU::addNode(const std::string &key, const std::string &value) {

    std::unique_ptr<lru_node> new_head = std::unique_ptr<lru_node>(
                                                    new lru_node(key, value)
                                                );


    if (_lru_head.get()) {
        new_head.get()->prev = _lru_head.get()->prev;
        _lru_head.get()->prev = new_head.get();

    } else {
        new_head.get()->prev = new_head.get();
    }

    new_head.get()->next.reset(_lru_head.release());

    _lru_head.reset(new_head.release());

    _lru_index.insert(
        std::pair<string_wrapper, node_wrapper>(
            std::cref(_lru_head.get()->key),
            std::ref(*_lru_head.get())
        )
    );
}

size_t SimpleLRU::freeTail(const size_t requared_size) {

    size_t allocated_before = _allocated_memory;
    while (_max_size - _allocated_memory < requared_size) {
        lru_node *last = _lru_head.get()->prev;
        size_t node_size = last->key.size() + last->value.size();
        _lru_index.erase(std::cref(last->key));

        if (last == _lru_head.get()) {
            _lru_head.reset();

        } else {
            _lru_head.get()->prev = last->prev;
            last->prev->next.reset();
        }
        _allocated_memory -= node_size;
    }

    return allocated_before - _allocated_memory;
}
//------------------------------------------------------------------------------
} // namespace Backend
} // namespace Afina
