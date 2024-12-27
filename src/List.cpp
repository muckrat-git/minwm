#include <stddef.h>
#include <string>
#include <cstring>
#include <functional>

using namespace std;

template <typename type>
class List {
    public:
    type * data;        // Internal list data array
    size_t length = 0;  // List length

    // - Constructors -

    // Create a new list from a pointer array
    List(type * data, size_t length) {
        // Allocate data
        this->data = new type[length];
            
        // Copy memory from data to this->data
        memcpy(this->data, data, sizeof(type) * length);
        this->length = length;
    }
    // Create an empty list of size 'length'
    List(size_t length) {
        // Allocate data
        this->data = new type[length];
        this->length = length;
    }
    // Null constructor
    List() = default;
    
    // - Destructor -
    ~List() {
        if(this->length > 0) delete[] this->data;
    }

    // - Functions -
    
    // Check if 'index' is valid within the list
    bool HasIndex(size_t index) {
        return index >= 0 && index < this->length;
    }
    
    // Returns index of first occurrence of 'value' in the list
    size_t IndexOf(type value) {
        for(size_t i = 0; i < this->length; ++i) {
            if(data[i] == value) return i;
        }
        return this->length;
    }

    type * Search(function<bool(type*)> comparator) {
        for(size_t i = 0; i < this->length; ++i) {
            if(comparator(&data[i])) return &data[i];
        }
        return nullptr;
    }
    
    // Add item 'value' to the end of the list
    type & Append(type value) {
        // Check if empty
        if(this->length == 0) {
            this->data = new type[1];
            this->data[0] = value;
            this->length = 1;
            return this->data[0];
        }

        // Calculate new size
        size_t size = sizeof(type) * this->length;
            
        // Copy old data to buffer
        type * buffer = new type[this->length];
        memcpy(buffer, this->data, size);
    
        // Reallocate new data
        delete[] this->data;
        this->data = new type[this->length + 1];
            
        // Copy old data to new data
        memcpy(this->data, buffer, size);
            
        // Deallocate buffer
        delete[] buffer;
            
        // Add new value to end of list
        return data[this->length++] = value;
    }
    
    // Remove item at 'index' from list
    bool Remove(size_t index) {
        // Ensure index is valid
        if(!HasIndex(index)) return false;
            
        // Copy old data to buffer
        type * buffer = new type[this->length];
        memcpy(buffer, this->data, sizeof(type) * this->length);
    
        // Reallocate new data
        delete[] this->data;
        this->data = new type[this->length - 1];
    
        // Copy over data
        for(size_t i = 0; i < this->length; ++i)
            this->data[i] = buffer[i - (i>index ? 1 : 0)];
    
        // Deallocate buffer
        delete[] buffer;
        return true;
    }
    
    // Remove first occurance of 'value' from the list
    bool RemoveItem(type value) {
        // Locate item
        size_t index = IndexOf(value);
        if(index == this->length) return false;
    
        // Remove item
        return Remove(index);
    }
    
    // Replace item at index
    bool Replace(size_t index, type value) {
        // Ensure index is valid
        if(!HasIndex(index)) return false;
    
        // Replace item
        data[index] = value;
        return true;
    }
    
    // Convert list to string representation
    std::string ToString() {
        std::string result = "[";
        for(int i = 0; i < this->length; ++i) {
            result += std::to_string(data[i]);
            if(i != this->length - 1) result += ", ";
        }
        return result + "]";
    }
    
    // - Operator Overloads -
    
    // Get item at index    
    type & operator[](size_t index) {    
        return this->data[index];    
    }   

    // Append item to list    
    type & operator+=(type value) {    
        return Append(value);    
    } 

    // Remove first occurance of item    
    type & operator-=(type value) {    
        return RemoveItem(value);    
    }  

    // Remove last element    
    void operator--() const {    
        Remove(this->length - 1);    
    }    

    // Append one list to another
    List operator+(List rhs) {
        List ret = List(this->length + rhs.length);
        
        for(int i = 0; i < this->length; ++i) ret[i] = this->data[i];
        for(int i = 0; i < rhs.length; ++i) ret[i+this->length] = rhs.data[i];

        return ret;
    }

    // - Iterator functions -
    type * begin() {
        return data;
    }
    type * end() {
        return data + this->length;
    }
};