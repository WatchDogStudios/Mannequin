#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Memory/Allocator.h>
#include <Foundation/Logging/Log.h>

#include <stdexcept>

template <typename T>
class nsObjectPtr {
private:
    T* ptr;

public:

    NS_DISALLOW_COPY_AND_ASSIGN(nsObjectPtr);
    // Constructors
    nsObjectPtr() : ptr(nullptr) {}
    explicit nsObjectPtr(T* p) : ptr(p) {}

    // Destructor
    ~nsObjectPtr() {
        delete ptr;
    }

    // Move Constructor
    nsObjectPtr(nsObjectPtr&& other) noexcept : ptr(other.ptr) {
        other.ptr = nullptr;
    }

    // Move Assignment
    nsObjectPtr& operator=(nsObjectPtr&& other) noexcept {
        if (this != &other) {
            delete ptr;
            ptr = other.ptr;
            other.ptr = nullptr;
        }
        return *this;
    }

    // Access Operators with Verification
    T& operator*() const {
        if (!ptr) {
            nsLog::Debug("Attempting to dereference a null pointer.");
        }
        return *ptr;
    }

    T* operator->() const {
        if (!ptr) {
            nsLog::Debug("Attempting to access a null pointer.");
        }
        return ptr;
    }

    // Utility Functions
    bool isValid() const {
        return ptr != nullptr;
    }

    T* get() const {
        return ptr;
    }

    void reset(T* p = nullptr) {
        delete ptr;
        ptr = p;
    }

    // Convenience accessor matching usage in existing code
    T* rawPtr() const { return ptr; }

    // Safe explicit bool conversion for conditionals
    explicit operator bool() const { return ptr != nullptr; }
};
