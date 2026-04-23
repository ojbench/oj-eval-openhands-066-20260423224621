#ifndef SRC_HPP
#define SRC_HPP

#include <stdexcept>
#include <initializer_list>
#include <typeinfo>
#include <utility>

namespace sjtu {

class any_ptr {
 private:
  struct ControlBlock {
    int ref_count;
    ControlBlock() : ref_count(1) {}
    virtual ~ControlBlock() {}
    virtual const std::type_info &type() const = 0;
    virtual void *get_data() = 0;
    virtual const void *get_data() const = 0;
  };

  template <typename T> struct DataBlock : ControlBlock {
    T *ptr;
    DataBlock(T *p) : ptr(p) {}
    ~DataBlock() { delete ptr; }
    const std::type_info &type() const override { return typeid(T); }
    void *get_data() override { return ptr; }
    const void *get_data() const override { return ptr; }
  };

  ControlBlock *cb;

 public:
  any_ptr() : cb(nullptr) {}

  any_ptr(const any_ptr &other) : cb(other.cb) {
    if (cb) cb->ref_count++;
  }

  template <class T> any_ptr(T *ptr) : cb(ptr ? new DataBlock<T>(ptr) : nullptr) {}

  ~any_ptr() {
    if (cb && --cb->ref_count == 0) delete cb;
  }

  any_ptr &operator=(const any_ptr &other) {
    if (this == &other) return *this;
    if (cb && --cb->ref_count == 0) delete cb;
    cb = other.cb;
    if (cb) cb->ref_count++;
    return *this;
  }

  template <class T> any_ptr &operator=(T *ptr) {
    if (cb && --cb->ref_count == 0) delete cb;
    cb = ptr ? new DataBlock<T>(ptr) : nullptr;
    return *this;
  }

  template <class T> T &unwrap() {
    if (!cb || cb->type() != typeid(T)) throw std::bad_cast();
    return *static_cast<T *>(cb->get_data());
  }

  template <class T> const T &unwrap() const {
    if (!cb || cb->type() != typeid(T)) throw std::bad_cast();
    return *static_cast<const T *>(cb->get_data());
  }
};

template <class T> any_ptr make_any_ptr(const T &t) { return any_ptr(new T(t)); }

template <class T, class... Args> any_ptr make_any_ptr(Args &&...args) {
  return any_ptr(new T{std::forward<Args>(args)...});
}

template <class T, class U> any_ptr make_any_ptr(std::initializer_list<U> il) {
  return any_ptr(new T(il));
}

}  // namespace sjtu

#endif
