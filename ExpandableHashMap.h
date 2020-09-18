// ExpandableHashMap.h

//  Implementation of an expandable hash map
//  utilizes dynamic memory allocation

#ifndef EXPANDABLEHASHMAP_H
#define EXPANDABLEHASHMAP_H

#include <list>
#include <utility>
#include <iostream>

// associate a key with a value with a boolean variable to store if the bucket has been used.
// a linked list of associations helps us deal with collisions
#define MapType std::pair<std::list<std::pair<KeyType, ValueType> >, bool>

// the type of the associations stored at every bucket, to enable easy declaration of iterators
#define AssociationType std::list<std::pair<KeyType, ValueType> >


template<typename KeyType, typename ValueType>
class ExpandableHashMap
{
public:
    ExpandableHashMap(double maximumLoadFactor = 0.5);
    ~ExpandableHashMap();
    void reset();
    int size() const;
    void associate(const KeyType& key, const ValueType& value);

    // for a map that can't be modified, return a pointer to const ValueType
    const ValueType* find(const KeyType& key) const;

    // for a modifiable map, return a pointer to modifiable ValueType
    ValueType* find(const KeyType& key)
    {
        return const_cast<ValueType*>(const_cast<const ExpandableHashMap*>(this)->find(key));
    }

    // C++11 syntax for preventing copying and assignment
    ExpandableHashMap(const ExpandableHashMap&) = delete;
    ExpandableHashMap& operator=(const ExpandableHashMap&) = delete;

private:
    double m_loadFactor;
    MapType *m_hashmap;
    unsigned int m_size;
    unsigned int m_buckets;
};


template <typename KeyType, typename ValueType>
ExpandableHashMap<KeyType, ValueType>::ExpandableHashMap(double maximumLoadFactor)
        : m_loadFactor(maximumLoadFactor)
{
    m_hashmap = new MapType[8]; // initially 8 buckets
    m_buckets = 8;
    m_size = 0; // initially no associations
}

template <typename KeyType, typename ValueType>
ExpandableHashMap<KeyType, ValueType>::~ExpandableHashMap()
{
    delete [] m_hashmap; // delete the dynamically allocated hashmap
}

template <typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::reset()
{
    // save the pointer to the current hashmap, to be deleted soon
    MapType* toBeDeleted = m_hashmap;
    
    // allocate and initialize a fresh hashmap
    m_hashmap = new MapType[8];
    m_buckets = 8;
    m_size = 0;
    
    // delete the old map
    delete [] toBeDeleted;
}

template <typename KeyType, typename ValueType>
int ExpandableHashMap<KeyType, ValueType>::size() const
{
    return m_size;
}

template <typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::associate(const KeyType& key, const ValueType& value)
{
    unsigned int hasher(const KeyType& k);
    ValueType* foundValue = find(key); // used to check if key already exists in map

    // key is not already in hashmap, so we have to insert a new association
    if (foundValue == nullptr) {
        
        // if we were to insert a key, what would be the loadfactor of the resulting hashmap?
        double newLoadFactor = ((double) (m_size + 1)) / ((double)m_buckets);
        
        // if adding a new association would push the size over the loadFactor, double the number of buckets
        if (newLoadFactor > m_loadFactor) {
            
            // assign new hash_map
            MapType *temp = m_hashmap; // temp variable to store old hashmap which has to be deleted
            m_hashmap = new MapType[m_buckets * 2]; // allocate a new hashmap with double the number of buckets
            
            // rehash every key and add it to the new map
            for (unsigned int i = 0; i < m_buckets; i++) {
                
                // if a particular bucket in the old map is in use, rehash and copy into new map
                if (temp[i].second) {
                    
                    // iterator to iterate through the linked list
                    typename AssociationType::iterator it;
                    
                    
                    for (it = temp[i].first.begin(); it != temp[i].first.end(); it++) {
                        
                        // rehash the KeyType field of the old map
                        unsigned int h = hasher((*it).first) % (m_buckets * 2);
                        
                        // copy over the associated value into the new hash map
                        m_hashmap[h].first.push_back(std::make_pair((*it).first, (*it).second));
                        
                        // mark the new bucket as used
                        m_hashmap[h].second = true;
                    }
                }
            }
            
            m_buckets *= 2; // update the value of m_buckets to reflect that the map has been expanded
            delete [] temp; // delete the old map after the new map has been created
        }
        
        
        // after we are done creating a new hashmap (if needed), insert the new key and value
        unsigned int newPos = hasher(key) % m_buckets; // find a position in the map to insert the new association
        m_hashmap[newPos].first.push_back(std::make_pair(key, value)); // add the key-value pair to linked list of associations
        m_hashmap[newPos].second = true; // mark the bucket as used
        m_size++; // increment size by 1
    }
    
    else  // key is already present in hashmap, so we update the value associated with it
        *foundValue = value;
}

template <typename KeyType, typename  ValueType>
const ValueType* ExpandableHashMap<KeyType, ValueType>::find(const KeyType& key) const
{
    // prototype for hashing function
    unsigned int hasher (const KeyType& k);
    
    // if there are no associations in the map, simply return nullptr
    if (!m_size)
        return nullptr;
    
    // hash the key to get the bucket we are looking for
    int pos = hasher(key) % m_buckets;
    
    // we only iterate through the linked list of associations if the bucket has been used
    if (m_hashmap[pos].second) {
        typename AssociationType::iterator it;
        for (it = m_hashmap[pos].first.begin(); it != m_hashmap[pos].first.end(); it++) {
            if ((*it).first == key)
                return &((*it).second);
        }
    }

    return nullptr;  // return nullptr for value not found
}


#endif // EXPANDABLEHASHMAP_H
