#include<iostream>
#include "include/com_alipay_kepler_fasterkv_FasterKv.h"

#include "core/faster.h"
#include "device/null_disk.h"

using namespace FASTER;
using FASTER::core::FasterKv;
using FASTER::device::NullDisk;

class alignas(1) Key {
 public:
  Key(uint8_t len, int8_t* fill)
    : len_{ len } {
    for(uint8_t idx = 0; idx < len_; ++idx) {
      buffer()[idx] = fill[idx];
    }
  }

  /// Copy constructor.
  Key(const Key& other)
    : len_{ other.len_ } {
    std::memcpy(buffer(), other.buffer(), len_ * sizeof(int8_t));
  }

  inline uint32_t size() const {
    return sizeof(*this) + (len_ * sizeof(int8_t));
  }
 private:
  inline int8_t* buffer() {
    return reinterpret_cast<int8_t*>(this + 1);
  }
 public:
  inline const int8_t* buffer() const {
    return reinterpret_cast<const int8_t*>(this + 1);
  }
  inline KeyHash GetHash() const {
    return KeyHash{ Utility::HashBytes(
                      reinterpret_cast<const uint16_t*>(buffer()), len_ / 2) }; // todo
  }

  /// Comparison operators.
  inline bool operator==(const Key& other) const {
    return len_ == other.len_ &&
           std::memcmp(buffer(), other.buffer(), len_ * sizeof(int8_t)) == 0;
  }
  inline bool operator!=(const Key& other) const {
    return len_ != other.len_ ||
           std::memcmp(buffer(), other.buffer(), len_ * sizeof(int8_t)) != 0;
  }

 private:
  uint8_t len_;
};

typedef uint16_t real_value_t;

class Value {
 public:
  friend class UpsertContext;
  friend class ReadContext;

  public:
  inline uint32_t size() const {
    return size_;
  }

 private:
  uint16_t size_;
  union {
    std::atomic<real_value_t> atomic_val_;
    real_value_t val_;
  };
};

class UpsertContext : public IAsyncContext {
 public:
  typedef Key key_t;
  typedef Value value_t;

  UpsertContext(int8_t* key_array, uint32_t key_len, real_value_t val)
    : val_{ val } {
    key_ = alloc_context<key_t>(sizeof(key_t) + (key_len * sizeof(int8_t)));
    new(key_.get()) key_t{ key_len, key_array };
  }

  /// Deep-copy constructor.
  UpsertContext(UpsertContext& other)
    : key_{ std::move(other.key_) }
    , val_{ other.val_ } {
  }

  /// The implicit and explicit interfaces require a key() accessor.
  inline const Key& key() const {
    return *key_.get();
  }
  inline static constexpr uint32_t value_size() {
    return sizeof(value_t);
  }
  /// Non-atomic and atomic Put() methods.
  inline void Put(Value& value) {
    value.size_ = sizeof(value);
    value.val_ = val_;
  }
  inline bool PutAtomic(Value& value) {
    value.atomic_val_.store(val_);
    return true;
  }

 protected:
  /// The explicit interface requires a DeepCopy_Internal() implementation.
  Status DeepCopy_Internal(IAsyncContext*& context_copy) {
    return IAsyncContext::DeepCopy_Internal(*this, context_copy);
  }

 private:
  context_unique_ptr_t<key_t> key_;
  real_value_t val_;
};  

class ReadContext: public IAsyncContext {
 public:
  typedef Key key_t;
  typedef Value value_t;

  ReadContext(int8_t* key_array, uint32_t key_len)
    : val_{ 0 } {
    key_ = alloc_context<key_t>(sizeof(key_t) + (key_len * sizeof(int8_t)));
    new(key_.get()) key_t{ key_len, key_array };
  }

  /// Deep-copy constructor.
  ReadContext(ReadContext& other)
    : key_{ std::move(other.key_) }
    , val_{ other.val_ } {
  }

  /// The implicit and explicit interfaces require a key() accessor.
  inline const Key& key() const {
    return *key_.get();
  }

  inline void Get(const Value& value) {
    val_ = value.val_;
  }
  inline void GetAtomic(const Value& value) {
    val_ = value.atomic_val_.load();
  }

  real_value_t val() const {
    return val_;
  }

  protected:
   /// The explicit interface requires a DeepCopy_Internal() implementation.
   Status DeepCopy_Internal(IAsyncContext*& context_copy) {
     return IAsyncContext::DeepCopy_Internal(*this, context_copy);
   }
 
  private:
   context_unique_ptr_t<key_t> key_;
   real_value_t val_;
};


jlong Java_com_alipay_kepler_fasterkv_FasterKv_open(JNIEnv* env, jclass jcls, jlong table_size, jlong log_size, jstring filename, jdouble log_mutable_fraction) {
  const char* db_path = env->GetStringUTFChars(filename, nullptr);
  FasterKv<Key, Value, NullDisk>* store = new FasterKv<Key, Value, NullDisk>(table_size, log_size, std::string(db_path));
  jlong handler = reinterpret_cast<jlong>(store);
  std::cout << "open:" <<handler << "\n";
  std::cout << store->StartSession().ToString() << "\n";

  std::cout << sizeof(Key) << "\n";
  std::cout << alignof(Key) << "\n";
  return handler;
}

static auto callback = [](IAsyncContext* context, Status result) {
  // Upserts don't go to disk.
};

jlong Java_com_alipay_kepler_fasterkv_FasterKv_get(JNIEnv* env, jclass jcls, jlong handler, jbyteArray jkey, jint jkey_len) {
  jbyte* key = new jbyte[jkey_len];
  env->GetByteArrayRegion(jkey, 0, jkey_len, key);

  FasterKv<Key, Value, NullDisk>* store = reinterpret_cast<FasterKv<Key, Value, NullDisk>*>(handler);

  ReadContext context{ key, jkey_len };
  Status result = store->Read(context, callback, 1);
  if(result == Status::Ok) {
    // printf("get: key: %s value: %i\n", key, context.val());
    return context.val();
  } else {
    printf("%i \n", result);
    return -1;
  }
}

jlong Java_com_alipay_kepler_fasterkv_FasterKv_put(JNIEnv * env, jclass jcls, jlong handler, jbyteArray jkey, jint jkey_len, jint v) {
  jbyte* key = new jbyte[jkey_len];
  env->GetByteArrayRegion(jkey, 0, jkey_len, key);

  FasterKv<Key, Value, NullDisk>* store = reinterpret_cast<FasterKv<Key, Value, NullDisk>*>(handler);
  UpsertContext context{key, jkey_len, v};
  Status result = store->Upsert(context, callback, 1);
  if (result == Status::Ok) {
    return handler;
  } else {
    return -1;
  }
}