// The sketch is auto-generated with XOD (https://xod.io).
//
// You can compile and upload it to an Arduino-compatible board with
// Arduino IDE.
//
// Rough code overview:
//
// - Intrusive pointer (a smart pointer with ref counter)
// - Immutable dynamic list data structure
// - XOD runtime environment
// - Native node implementation
// - Program graph definition
//
// Search for comments fenced with '====' and '----' to navigate through
// the major code blocks.

#include <Arduino.h>
#include <inttypes.h>


// ============================================================================
//
// Intrusive pointer
//
// ============================================================================

// This is a stripped down version of Boost v1.63 intrusive pointer.
//
//  Copyright (c) 2001, 2002 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//  See http://www.boost.org/libs/smart_ptr/intrusive_ptr.html for
//  documentation.

#ifndef XOD_INTRUSIVE_PTR_H
#define XOD_INTRUSIVE_PTR_H

namespace boost {
//
//  intrusive_ptr
//
//  A smart pointer that uses intrusive reference counting.
//
//  Relies on unqualified calls to
//
//      void intrusive_ptr_add_ref(T * p);
//      void intrusive_ptr_release(T * p);
//
//          (p != 0)
//
//  The object is responsible for destroying itself.
//

template <class T> class intrusive_ptr {
  private:
    typedef intrusive_ptr this_type;

  public:
    typedef T element_type;

    constexpr intrusive_ptr() : px(0) {}

    intrusive_ptr(T *p, bool add_ref = true) : px(p) {
        if (px != 0 && add_ref)
            intrusive_ptr_add_ref(px);
    }

    template <class U>
    intrusive_ptr(intrusive_ptr<U> const &rhs) : px(rhs.get()) {
        if (px != 0)
            intrusive_ptr_add_ref(px);
    }

    intrusive_ptr(intrusive_ptr const &rhs) : px(rhs.px) {
        if (px != 0)
            intrusive_ptr_add_ref(px);
    }

    ~intrusive_ptr() {
        if (px != 0)
            intrusive_ptr_release(px);
    }

    template <class U> intrusive_ptr &operator=(intrusive_ptr<U> const &rhs) {
        this_type(rhs).swap(*this);
        return *this;
    }

    intrusive_ptr(intrusive_ptr &&rhs) : px(rhs.px) { rhs.px = 0; }

    intrusive_ptr &operator=(intrusive_ptr &&rhs) {
        this_type(static_cast<intrusive_ptr &&>(rhs)).swap(*this);
        return *this;
    }

    template <class U> friend class intrusive_ptr;

    template <class U> intrusive_ptr(intrusive_ptr<U> &&rhs) : px(rhs.px) {
        rhs.px = 0;
    }

    template <class U> intrusive_ptr &operator=(intrusive_ptr<U> &&rhs) {
        this_type(static_cast<intrusive_ptr<U> &&>(rhs)).swap(*this);
        return *this;
    }

    intrusive_ptr &operator=(intrusive_ptr const &rhs) {
        this_type(rhs).swap(*this);
        return *this;
    }

    intrusive_ptr &operator=(T *rhs) {
        this_type(rhs).swap(*this);
        return *this;
    }

    void reset() { this_type().swap(*this); }

    void reset(T *rhs) { this_type(rhs).swap(*this); }

    void reset(T *rhs, bool add_ref) { this_type(rhs, add_ref).swap(*this); }

    T *get() const { return px; }

    T *detach() {
        T *ret = px;
        px = 0;
        return ret;
    }

    T &operator*() const { return *px; }

    T *operator->() const { return px; }

    operator bool() const { return px != 0; }

    void swap(intrusive_ptr &rhs) {
        T *tmp = px;
        px = rhs.px;
        rhs.px = tmp;
    }

  private:
    T *px;
};

template <class T, class U>
inline bool operator==(intrusive_ptr<T> const &a, intrusive_ptr<U> const &b) {
    return a.get() == b.get();
}

template <class T, class U>
inline bool operator!=(intrusive_ptr<T> const &a, intrusive_ptr<U> const &b) {
    return a.get() != b.get();
}

template <class T, class U>
inline bool operator==(intrusive_ptr<T> const &a, U *b) {
    return a.get() == b;
}

template <class T, class U>
inline bool operator!=(intrusive_ptr<T> const &a, U *b) {
    return a.get() != b;
}

template <class T, class U>
inline bool operator==(T *a, intrusive_ptr<U> const &b) {
    return a == b.get();
}

template <class T, class U>
inline bool operator!=(T *a, intrusive_ptr<U> const &b) {
    return a != b.get();
}

#if __GNUC__ == 2 && __GNUC_MINOR__ <= 96

// Resolve the ambiguity between our op!= and the one in rel_ops

template <class T>
inline bool operator!=(intrusive_ptr<T> const &a, intrusive_ptr<T> const &b) {
    return a.get() != b.get();
}

#endif

template <class T>
inline bool operator==(intrusive_ptr<T> const &p, nullptr_t) {
    return p.get() == 0;
}

template <class T>
inline bool operator==(nullptr_t, intrusive_ptr<T> const &p) {
    return p.get() == 0;
}

template <class T>
inline bool operator!=(intrusive_ptr<T> const &p, nullptr_t) {
    return p.get() != 0;
}

template <class T>
inline bool operator!=(nullptr_t, intrusive_ptr<T> const &p) {
    return p.get() != 0;
}

template <class T>
inline bool operator<(intrusive_ptr<T> const &a, intrusive_ptr<T> const &b) {
    return a.get() < b.get();
}

template <class T> void swap(intrusive_ptr<T> &lhs, intrusive_ptr<T> &rhs) {
    lhs.swap(rhs);
}

} // namespace boost

#endif // #ifndef XOD_INTRUSIVE_PTR_H

// ============================================================================
//
// Immutable dynamic list
//
// ============================================================================

#ifndef XOD_LIST_H
#define XOD_LIST_H

#ifndef XOD_LIST_CHUNK_SIZE
#define XOD_LIST_CHUNK_SIZE 15
#endif

namespace xod {
// forward declaration
template <typename T> class List;
}

namespace xod {
namespace detail {

#if XOD_LIST_CHUNK_SIZE < 256
typedef uint8_t index_t;
#else
typedef size_t index_t;
#endif

typedef uint8_t refcount_t;
typedef uint8_t depth_t;

/*
 * Bounds define a used range of data within Chunk’s ring buffer.  `first` is
 * an index (not byte offset) of the first element in range.  `last` is an
 * index (not byte offset) of the last filled element.  I.e. `last` points to
 * an existing element, *not* a slot past end.
 *
 * Value of `first` can be greater than `last`. It means that the range is
 * wrapped arround buffer’s origin.
 *
 * Examples:
 *
 * - `first == 0 && last == 0`: chunk have 1 element and it is at buffer[0]
 * - `first == 0 && last == 15`: chunk have 16 elements spanning from buffer[0]
 *   to buffer[15] inclusive
 * - `first == 14 && last == 2`: given the chunk size == 16 it has 5 elements:
 *   buffer[14], buffer[15], buffer[0], buffer[1], buffer[2].
 */
struct Bounds {
#if XOD_LIST_CHUNK_SIZE < 16
    index_t first : 4;
    index_t last : 4;
#else
    index_t first;
    index_t last;
#endif
};

template <typename T> struct Traits {
    enum { N = XOD_LIST_CHUNK_SIZE / sizeof(T) };
};

/*
 * Ring buffer
 */
struct Chunk {
    char buffer[XOD_LIST_CHUNK_SIZE];
    Bounds bounds;
    refcount_t _refcount;

    Chunk() { memset(this, 0, sizeof(Chunk)); }

    /*
     * Returns number of elements occupied
     */
    template <typename T> index_t usage() {
        return (bounds.last - bounds.first + Traits<T>::N) % Traits<T>::N + 1;
    }

    template <typename T> bool isFull() { return usage<T>() == Traits<T>::N; }

    template <typename T> bool append(T val) {
        if (isFull<T>())
            return false;

        appendUnsafe(val);
        return true;
    }

    template <typename T> void appendUnsafe(T val) {
        auto idx = ++bounds.last;
        *((T *)buffer + idx) = val;
    }

    template <typename T> bool concat(T *val, index_t len) {
        if (usage<T>() > Traits<T>::N - len)
            return false;

        while (len--)
            appendUnsafe(*val++);

        return true;
    }
};

void intrusive_ptr_add_ref(Chunk *chunk) {
    // TODO: deal with possible overflow
    ++chunk->_refcount;
}

void intrusive_ptr_release(Chunk *chunk) {
    if (--chunk->_refcount == 0) {
        delete chunk;
    }
}

template <typename T> class ListIterator {
    typedef List<T> ListT;
    typedef const ListT *ListRawPtr;

  public:
    ListIterator(ListRawPtr root) {
        _stackSize = 0;
        if (root->isEmpty()) {
            _stack = 0;
        } else {
            _stack = new ListRawPtr[root->maxDepth()];
            push(root);
            drillDownToLeftmost();
        }
    }

    ~ListIterator() {
        if (_stack)
            delete[] _stack;
    }

    /*
     * Returns false if iteration is done
     */
    operator bool() const { return _stackSize > 0; }

    const T &operator*() const { return chunk()->buffer[_indexInChunk]; }

    ListIterator &operator++() {
        if (!_stackSize)
            return *this;

        ++_indexInChunk;

        if (_indexInChunk > top()->_rightBounds.last) {
            // we’ve runned over whole chunk, move to next one
            while (true) {
                auto branch = pop();

                if (!_stackSize)
                    break;

                auto parent = top();
                if (parent->_left == branch) {
                    // switch to right branch if we just completed with left one
                    push(parent->_right.get());
                    drillDownToLeftmost();
                    break;
                }
            }
        }

        return *this;
    }

  private:
    ListRawPtr top() const { return _stack[_stackSize - 1]; }

    void push(ListRawPtr list) { _stack[_stackSize++] = list; }

    ListRawPtr pop() { return _stack[_stackSize-- - 1]; }

    void drillDownToLeftmost() {
        ListRawPtr left;
        while ((left = top()->_left.get()))
            push(left);
        _indexInChunk = top()->_rightBounds.first;
    }

    Chunk *chunk() const { return top()->_chunk.get(); }

  private:
    ListRawPtr *_stack;
    depth_t _stackSize;
    index_t _indexInChunk;
};
}
} // namespace xod::detail

namespace xod {

template <typename T> void intrusive_ptr_add_ref(List<T> *list) {
    // TODO: deal with possible overflow
    ++list->_refcount;
}

template <typename T> void intrusive_ptr_release(List<T> *list) {
    if (--list->_refcount == 0) {
        delete list;
    }
}

template <typename T> class List {
    typedef boost::intrusive_ptr<detail::Chunk> ChunkPtr;

  public:
    typedef boost::intrusive_ptr<List> ListPtr;
    typedef detail::ListIterator<T> Iterator;

    static ListPtr empty() { return ListPtr(new List()); }

    static ListPtr of(T val) {
        auto list = empty();
        auto chunk = new detail::Chunk();
        chunk->buffer[0] = val;
        list->_chunk = chunk;
        return list;
    }

    static ListPtr fromPlainArray(const T *buf, size_t len) {
        auto list = empty();
        if (!len)
            return list;

        if (len <= detail::Traits<T>::N) {
            // whole buf can be contained within a single chunk
            auto chunk = new detail::Chunk();
            memcpy(chunk->buffer, buf, len);
            list->_chunk = chunk;
            list->_rightBounds.last = chunk->bounds.last = len - 1;
        } else {
            // split the buffer into two portions
            auto leftLen = len / 2;
            list->_left = fromPlainArray(buf, leftLen);
            list->_right = fromPlainArray(buf + leftLen, len - leftLen);
        }

        return list;
    }

    bool isEmpty() const { return length() == 0; }

    size_t length() const {
        if (_left == nullptr && _right == nullptr) {
            return 0;
        } else if (chunk()) {
            return _rightBounds.last - _rightBounds.first + 1;
        } else {
            return _left->length() + _right->length();
        }
    }

    size_t chunkCount() const {
        if (_left) {
            return _left->chunkCount() + _right->chunkCount();
        } else if (_chunk) {
            return 1;
        } else {
            return 0;
        }
    }

    detail::depth_t maxDepth() const {
        if (_left) {
            auto leftDepth = _left->maxDepth();
            auto rightDepth = _right->maxDepth();
            return 1 + (leftDepth > rightDepth ? leftDepth : rightDepth);
        } else {
            return 1;
        }
    }

    ListPtr append(T val) const {
        if (length() == 0) {
            return of(val);
        }

        auto chunk = this->chunk();

        if (chunk && isChunkTailFree()) {
            bool amend = chunk->append(val);
            if (amend) {
                auto list = empty();
                list->_chunk = chunk;
                list->_rightBounds.last = _rightBounds.last + 1;
                return list;
            }
        }

        auto list = empty();
        list->_left = const_cast<List *>(this);
        list->_right = of(val);
        return list;
    }

    ListPtr concat(ListPtr other) const {
        if (isEmpty()) {
            return other;
        }

        if (other->isEmpty()) {
            return ListPtr(const_cast<List *>(this));
        }

        auto thisChunk = this->chunk();
        auto otherChunk = other->chunk();
        auto otherLen = other->length();
        if (thisChunk && isChunkTailFree() && otherChunk) {
            bool amend = thisChunk->concat(otherChunk->buffer, otherLen);
            if (amend) {
                auto list = empty();
                list->_chunk = thisChunk;
                list->_rightBounds.first = _rightBounds.first;
                list->_rightBounds.last = thisChunk->bounds.last;
                return list;
            }
        }

        auto list = empty();
        list->_left = const_cast<List *>(this);
        list->_right = other;
        return list;
    }

    void toPlainArrayUnsafe(T *buf) const {
        auto chunk = this->chunk();
        if (chunk) {
            memcpy(buf, chunk->buffer, length() * sizeof(T));
        } else if (_left) {
            _left->toPlainArrayUnsafe(buf);
            _right->toPlainArrayUnsafe(buf + _left->length());
        }
    }

    Iterator iterate() const { return Iterator(this); }

  protected:
    ChunkPtr chunk() const {
        if (_left == nullptr) {
            return _chunk;
        } else {
            return nullptr;
        }
    }

    bool isChunkTailFree() const {
      return _chunk->bounds.last == _rightBounds.last;
    }

  private:
    List() { memset(this, 0, sizeof(List)); }

    ~List() {
        // _right branch is in union with _chunk. Call a proper destructor
        // explicitly
        if (_left) {
            _right.~ListPtr();
        } else {
            _chunk.~ChunkPtr();
        }
    }

    friend Iterator;

    // There are two possible conditions of a list. It either:
    //
    // - concatenation of two children, in that case `_left` and `_right` point
    //   to branches
    // - a leaf with data, in that case `_left == nullptr` and `_chunk` contains
    //   pointer to the data
    //
    // Use union to save one pointer size, consider `_left` nullness to
    // understand the condition.
    ListPtr _left;
    union {
        ListPtr _right;
        ChunkPtr _chunk;
    };

    // Branch bounds inside chunks. In case if this is a leaf, only _rightBounds
    // is used.
    //
    // Note that the bounds will not match bounds in chunks themselves. It’s
    // because the chunks are reused across many List’s.
    detail::Bounds _leftBounds;
    detail::Bounds _rightBounds;

    friend void intrusive_ptr_add_ref<T>(List *list);
    friend void intrusive_ptr_release<T>(List *list);
    detail::refcount_t _refcount;
}; // class List<T>

} // namespace xod

#endif // #ifndef XOD_LIST_H


/*=============================================================================
 *
 *
 * Configuration
 *
 *
 =============================================================================*/

#define NODE_COUNT          11
#define MAX_OUTPUT_COUNT    1

// Uncomment to trace the program in the Serial Monitor
//#define XOD_DEBUG


/*=============================================================================
 *
 *
 * Runtime
 *
 *
 =============================================================================*/

//----------------------------------------------------------------------------
// Debug routines
//----------------------------------------------------------------------------
#ifndef DEBUG_SERIAL
#  define DEBUG_SERIAL Serial
#endif

#ifdef XOD_DEBUG
#  define XOD_TRACE(x)      { DEBUG_SERIAL.print(x); DEBUG_SERIAL.flush(); }
#  define XOD_TRACE_LN(x)   { DEBUG_SERIAL.println(x); DEBUG_SERIAL.flush(); }
#  define XOD_TRACE_F(x)    XOD_TRACE(F(x))
#  define XOD_TRACE_FLN(x)  XOD_TRACE_LN(F(x))
#else
#  define XOD_TRACE(x)
#  define XOD_TRACE_LN(x)
#  define XOD_TRACE_F(x)
#  define XOD_TRACE_FLN(x)
#endif

//----------------------------------------------------------------------------
// Type definitions
//----------------------------------------------------------------------------
#define PIN_KEY_OFFSET_BITS     (16 - MAX_OUTPUT_COUNT)
#define NO_NODE                 ((NodeId)-1)

namespace _program {
    typedef double Number;
    typedef bool Logic;

    // TODO: optimize, we should choose uint8_t if there are less than 255 nodes in total
    // and uint32_t if there are more than 65535
    typedef uint16_t NodeId;

    typedef NodeId Context;

    /*
     * PinKey is an address value used to find input’s or output’s data within
     * node’s Storage.
     *
     * For inputs its value is simply an offset in bytes from the beginning of
     * Storage structure instance. There will be a PinRef pointing to an upstream
     * output at this address.
     *
     * For outputs the pin key consists of two parts ORed bitwise. Least
     * significant bits (count defined by `PIN_KEY_OFFSET_BITS`) define an offset
     * from the beginning of node’s Storage where output data could be found. It
     * would be an OutputPin structure. Most significant bits define an index
     * number of that output among all outputs of the node. The index is used to
     * work with dirty flags bit-value.
     */
    // TODO: optimize, we should choose a proper type with a minimal enough capacity
    typedef uint16_t PinKey;

    // TODO: optimize, we should choose a proper type with a minimal enough capacity
    typedef uint16_t DirtyFlags;

    typedef unsigned long TimeMs;
    typedef void (*EvalFuncPtr)(NodeId nid, void* state);

    typedef xod::List<char>::ListPtr XString;
}

//----------------------------------------------------------------------------
// Engine
//----------------------------------------------------------------------------
namespace _program {
    extern void* storages[NODE_COUNT];
    extern EvalFuncPtr evaluationFuncs[NODE_COUNT];
    extern DirtyFlags dirtyFlags[NODE_COUNT];
    extern NodeId topology[NODE_COUNT];

    // TODO: replace with a compact list
    extern TimeMs schedule[NODE_COUNT];

    template<typename T>
    struct OutputPin {
        T value;
        // Keep outgoing link list with terminating `NO_NODE`
        NodeId* links;
    };

    struct PinRef {
        NodeId nodeId;
        PinKey pinKey;
    };

    /*
     * Input descriptor is a metaprogramming structure used to enforce an
     * input’s type and store its PinKey as a zero-memory constant.
     *
     * A specialized descriptor is required by `getValue` function. Every
     * input of every type node gets its own descriptor in generated code that
     * can be accessed as Inputs::FOO. Where FOO is a pin identifier.
     */
    template<typename ValueT_, size_t offsetInStorage>
    struct InputDescriptor {
        typedef ValueT_ ValueT;
        enum Offset : PinKey {
            KEY = offsetInStorage
        };
    };

    /*
     * Output descriptor serve the same purpose as InputDescriptor but for
     * ouputs.
     */
    template<typename ValueT_, size_t offsetInStorage, int index>
    struct OutputDescriptor {
        typedef ValueT_ ValueT;
        enum Offset : PinKey {
            KEY = offsetInStorage | (index << PIN_KEY_OFFSET_BITS)
        };
    };

    void* pinPtr(void* storage, PinKey key) {
        const size_t offset = key & ~(PinKey(-1) << PIN_KEY_OFFSET_BITS);
        return (uint8_t*)storage + offset;
    }

    DirtyFlags dirtyPinBit(PinKey key) {
        const PinKey nbit = (key >> PIN_KEY_OFFSET_BITS) + 1;
        return 1 << nbit;
    }

    bool isOutputDirty(NodeId nid, PinKey key) {
        return dirtyFlags[nid] & dirtyPinBit(key);
    }

    bool isInputDirtyImpl(NodeId nid, PinKey key) {
        PinRef* ref = (PinRef*)pinPtr(storages[nid], key);
        if (ref->nodeId == NO_NODE)
            return false;

        return isOutputDirty(ref->nodeId, ref->pinKey);
    }

    template<typename InputT>
    bool isInputDirty(NodeId nid) {
        return isInputDirtyImpl(nid, InputT::KEY);
    }

    void markPinDirty(NodeId nid, PinKey key) {
        dirtyFlags[nid] |= dirtyPinBit(key);
    }

    void markNodeDirty(NodeId nid) {
        dirtyFlags[nid] |= 0x1;
    }

    bool isNodeDirty(NodeId nid) {
        return dirtyFlags[nid] & 0x1;
    }

    TimeMs transactionTime() {
        return millis();
    }

    void setTimeout(NodeId nid, TimeMs timeout) {
        schedule[nid] = transactionTime() + timeout;
    }

    void clearTimeout(NodeId nid) {
        schedule[nid] = 0;
    }

    template<typename T>
    T getValueImpl(NodeId nid, PinKey key) {
        PinRef* ref = (PinRef*)pinPtr(storages[nid], key);
        if (ref->nodeId == NO_NODE)
            return (T)0;

        return *(T*)pinPtr(storages[ref->nodeId], ref->pinKey);
    }

    template<typename InputT>
    typename InputT::ValueT getValue(NodeId nid) {
        return getValueImpl<typename InputT::ValueT>(nid, InputT::KEY);
    }

    template<typename T>
    void emitValueImpl(NodeId nid, PinKey key, T value) {
        OutputPin<T>* outputPin = (OutputPin<T>*)pinPtr(storages[nid], key);

        outputPin->value = value;
        markPinDirty(nid, key);

        NodeId* linkedNode = outputPin->links;
        while (*linkedNode != NO_NODE) {
            markNodeDirty(*linkedNode++);
        }
    }

    template<typename OutputT>
    void emitValue(NodeId nid, typename OutputT::ValueT value) {
        emitValueImpl(nid, OutputT::KEY, value);
    }

    template<typename T>
    void reemitValueImpl(NodeId nid, PinKey key) {
        OutputPin<T>* outputPin = (OutputPin<T>*)pinPtr(storages[nid], key);
        emitValueImpl<T>(nid, key, outputPin->value);
    }

    template<typename OutputT>
    void reemitValue(NodeId nid) {
        reemitValueImpl<typename OutputT::ValueT>(nid, OutputT::KEY);
    }

    void evaluateNode(NodeId nid) {
        XOD_TRACE_F("eval #");
        XOD_TRACE_LN(nid);
        EvalFuncPtr eval = evaluationFuncs[nid];
        eval(nid, storages[nid]);
    }

    void runTransaction() {
        XOD_TRACE_F("Transaction started, t=");
        XOD_TRACE_LN(millis());
        for (NodeId nid : topology) {
            if (isNodeDirty(nid))
                evaluateNode(nid);
        }

        memset(dirtyFlags, 0, sizeof(dirtyFlags));
        XOD_TRACE_F("Transaction completed, t=");
        XOD_TRACE_LN(millis());
    }

    void idle() {
        TimeMs now = millis();
        for (NodeId nid = 0; nid < NODE_COUNT; ++nid) {
            TimeMs t = schedule[nid];
            if (t && t <= now) {
                markNodeDirty(nid);
                clearTimeout(nid);
                return;
            }
        }
    }
}

//----------------------------------------------------------------------------
// Entry point
//----------------------------------------------------------------------------
void setup() {
    // FIXME: looks like there is a rounding bug. Waiting for 1 second fights it
    delay(1000);
#ifdef XOD_DEBUG
    DEBUG_SERIAL.begin(9600);
#endif
    XOD_TRACE_FLN("Program started");
}

void loop() {
    _program::idle();
    _program::runTransaction();
}

/*=============================================================================
 *
 *
 * Native node implementations
 *
 *
 =============================================================================*/

namespace _program {

//-----------------------------------------------------------------------------
// xod/core/system_time implementation
//-----------------------------------------------------------------------------
namespace xod__core__system_time {

struct State {
};

struct Storage {
    State state;
    PinRef input_UPD;
    OutputPin<Number> output_TIME;
};

State* getState(NodeId nid) {
    return reinterpret_cast<State*>(storages[nid]);
}

using input_UPD = InputDescriptor<Logic, offsetof(Storage, input_UPD)>;

using output_TIME = OutputDescriptor<Number, offsetof(Storage, output_TIME), 0>;

void evaluate(Context ctx) {
    emitValue<output_TIME>(ctx, millis() / 1000.f);
}

} // namespace xod__core__system_time

//-----------------------------------------------------------------------------
// xod/common_hardware/text_lcd_16x2 implementation
//-----------------------------------------------------------------------------
namespace xod__common_hardware__text_lcd_16x2 {

// --- Enter global namespace ---
}}
#include <LiquidCrystal.h>

namespace _program {
namespace xod__common_hardware__text_lcd_16x2 {
// --- Back to local namespace ---
struct State {
    LiquidCrystal* lcd;
};

struct Storage {
    State state;
    PinRef input_RS;
    PinRef input_EN;
    PinRef input_D4;
    PinRef input_D5;
    PinRef input_D6;
    PinRef input_D7;
    PinRef input_L1;
    PinRef input_L2;
};

State* getState(NodeId nid) {
    return reinterpret_cast<State*>(storages[nid]);
}

using input_RS = InputDescriptor<Number, offsetof(Storage, input_RS)>;
using input_EN = InputDescriptor<Number, offsetof(Storage, input_EN)>;
using input_D4 = InputDescriptor<Number, offsetof(Storage, input_D4)>;
using input_D5 = InputDescriptor<Number, offsetof(Storage, input_D5)>;
using input_D6 = InputDescriptor<Number, offsetof(Storage, input_D6)>;
using input_D7 = InputDescriptor<Number, offsetof(Storage, input_D7)>;
using input_L1 = InputDescriptor<XString, offsetof(Storage, input_L1)>;
using input_L2 = InputDescriptor<XString, offsetof(Storage, input_L2)>;

void printLine(LiquidCrystal* lcd, uint8_t lineIndex, XString str) {
    if (!str)
        return;

    lcd->setCursor(0, lineIndex);
    for (auto it = str->iterate(); it; ++it)
        lcd->write(*it);
}

void evaluate(Context ctx) {
    State* state = getState(ctx);
    auto lcd = state->lcd;
    if (!state->lcd) {
        state->lcd = lcd = new LiquidCrystal(
            (int)getValue<input_RS>(ctx),
            (int)getValue<input_EN>(ctx),
            (int)getValue<input_D4>(ctx),
            (int)getValue<input_D5>(ctx),
            (int)getValue<input_D6>(ctx),
            (int)getValue<input_D7>(ctx));

        lcd->begin(16, 2);
    }

    printLine(lcd, 0, getValue<input_L1>(ctx));
    printLine(lcd, 1, getValue<input_L2>(ctx));
}

} // namespace xod__common_hardware__text_lcd_16x2

//-----------------------------------------------------------------------------
// xod/core/cast_number_to_string implementation
//-----------------------------------------------------------------------------
namespace xod__core__cast_number_to_string {

struct State {
};

struct Storage {
    State state;
    PinRef input_IN;
    OutputPin<XString> output_OUT;
};

State* getState(NodeId nid) {
    return reinterpret_cast<State*>(storages[nid]);
}

using input_IN = InputDescriptor<Number, offsetof(Storage, input_IN)>;

using output_OUT = OutputDescriptor<XString, offsetof(Storage, output_OUT), 0>;

void evaluate(Context ctx) {
    char str[16];
    auto num = getValue<input_IN>(ctx);
    dtostrf(num, 0, 2, str);
    auto xstr = ::xod::List<char>::fromPlainArray(str, strlen(str));
    emitValue<output_OUT>(ctx, xstr);
}

} // namespace xod__core__cast_number_to_string

//-----------------------------------------------------------------------------
// xod/core/continuously implementation
//-----------------------------------------------------------------------------
namespace xod__core__continuously {

struct State {
};

struct Storage {
    State state;
    OutputPin<Logic> output_TICK;
};

State* getState(NodeId nid) {
    return reinterpret_cast<State*>(storages[nid]);
}

using output_TICK = OutputDescriptor<Logic, offsetof(Storage, output_TICK), 0>;

void evaluate(Context ctx) {
    emitValue<output_TICK>(ctx, 1);
    setTimeout(ctx, 0);
}

} // namespace xod__core__continuously

//-----------------------------------------------------------------------------
// xod/core/constant_number implementation
//-----------------------------------------------------------------------------
namespace xod__core__constant_number {

struct State {};

struct Storage {
    State state;
    OutputPin<Number> output_VAL;
};

State* getState(NodeId nid) {
    return reinterpret_cast<State*>(storages[nid]);
}

using output_VAL = OutputDescriptor<Number, offsetof(Storage, output_VAL), 0>;

void evaluate(Context ctx) {
  reemitValue<output_VAL>(ctx);
}

} // namespace xod__core__constant_number

//-----------------------------------------------------------------------------
// xod/core/constant_string implementation
//-----------------------------------------------------------------------------
namespace xod__core__constant_string {

struct State {};

struct Storage {
    State state;
    OutputPin<XString> output_VAL;
};

State* getState(NodeId nid) {
    return reinterpret_cast<State*>(storages[nid]);
}

using output_VAL = OutputDescriptor<XString, offsetof(Storage, output_VAL), 0>;

void evaluate(Context ctx) {
  reemitValue<output_VAL>(ctx);
}

} // namespace xod__core__constant_string

} // namespace _program

/*=============================================================================
 *
 *
 * Program graph
 *
 *
 =============================================================================*/

namespace _program {

    NodeId links_0_TIME[] = { 2, NO_NODE };
    xod__core__system_time::Storage storage_0 = {
        { }, // state
        { NodeId(3), xod__core__continuously::output_TICK::KEY }, // input_UPD
        { 0, links_0_TIME } // output_TIME
    };

    xod__common_hardware__text_lcd_16x2::Storage storage_1 = {
        { }, // state
        { NodeId(9), xod__core__constant_number::output_VAL::KEY }, // input_RS
        { NodeId(8), xod__core__constant_number::output_VAL::KEY }, // input_EN
        { NodeId(4), xod__core__constant_number::output_VAL::KEY }, // input_D4
        { NodeId(7), xod__core__constant_number::output_VAL::KEY }, // input_D5
        { NodeId(6), xod__core__constant_number::output_VAL::KEY }, // input_D6
        { NodeId(10), xod__core__constant_number::output_VAL::KEY }, // input_D7
        { NodeId(2), xod__core__cast_number_to_string::output_OUT::KEY }, // input_L1
        { NodeId(5), xod__core__constant_string::output_VAL::KEY }, // input_L2
    };

    NodeId links_2_OUT[] = { 1, NO_NODE };
    xod__core__cast_number_to_string::Storage storage_2 = {
        { }, // state
        { NodeId(0), xod__core__system_time::output_TIME::KEY }, // input_IN
        { ::xod::List<char>::empty(), links_2_OUT } // output_OUT
    };

    NodeId links_3_TICK[] = { 0, NO_NODE };
    xod__core__continuously::Storage storage_3 = {
        { }, // state
        { ::xod::List<char>::fromPlainArray("CONTINUOUSLY", 12), links_3_TICK } // output_TICK
    };

    NodeId links_4_VAL[] = { 1, NO_NODE };
    xod__core__constant_number::Storage storage_4 = {
        { }, // state
        { 10, links_4_VAL } // output_VAL
    };

    NodeId links_5_VAL[] = { 1, NO_NODE };
    xod__core__constant_string::Storage storage_5 = {
        { }, // state
        { ::xod::List<char>::empty(), links_5_VAL } // output_VAL
    };

    NodeId links_6_VAL[] = { 1, NO_NODE };
    xod__core__constant_number::Storage storage_6 = {
        { }, // state
        { 12, links_6_VAL } // output_VAL
    };

    NodeId links_7_VAL[] = { 1, NO_NODE };
    xod__core__constant_number::Storage storage_7 = {
        { }, // state
        { 11, links_7_VAL } // output_VAL
    };

    NodeId links_8_VAL[] = { 1, NO_NODE };
    xod__core__constant_number::Storage storage_8 = {
        { }, // state
        { 9, links_8_VAL } // output_VAL
    };

    NodeId links_9_VAL[] = { 1, NO_NODE };
    xod__core__constant_number::Storage storage_9 = {
        { }, // state
        { 8, links_9_VAL } // output_VAL
    };

    NodeId links_10_VAL[] = { 1, NO_NODE };
    xod__core__constant_number::Storage storage_10 = {
        { }, // state
        { 13, links_10_VAL } // output_VAL
    };

    void* storages[NODE_COUNT] = {
        &storage_0,
        &storage_1,
        &storage_2,
        &storage_3,
        &storage_4,
        &storage_5,
        &storage_6,
        &storage_7,
        &storage_8,
        &storage_9,
        &storage_10
    };

    EvalFuncPtr evaluationFuncs[NODE_COUNT] = {
        (EvalFuncPtr)&xod__core__system_time::evaluate,
        (EvalFuncPtr)&xod__common_hardware__text_lcd_16x2::evaluate,
        (EvalFuncPtr)&xod__core__cast_number_to_string::evaluate,
        (EvalFuncPtr)&xod__core__continuously::evaluate,
        (EvalFuncPtr)&xod__core__constant_number::evaluate,
        (EvalFuncPtr)&xod__core__constant_string::evaluate,
        (EvalFuncPtr)&xod__core__constant_number::evaluate,
        (EvalFuncPtr)&xod__core__constant_number::evaluate,
        (EvalFuncPtr)&xod__core__constant_number::evaluate,
        (EvalFuncPtr)&xod__core__constant_number::evaluate,
        (EvalFuncPtr)&xod__core__constant_number::evaluate
    };

    DirtyFlags dirtyFlags[NODE_COUNT] = {
        DirtyFlags(-1),
        DirtyFlags(-1),
        DirtyFlags(-1),
        DirtyFlags(-1),
        DirtyFlags(-1),
        DirtyFlags(-1),
        DirtyFlags(-1),
        DirtyFlags(-1),
        DirtyFlags(-1),
        DirtyFlags(-1),
        DirtyFlags(-1)
    };

    NodeId topology[NODE_COUNT] = {
        3, 4, 5, 6, 7, 8, 9, 10, 0, 2, 1
    };

    TimeMs schedule[NODE_COUNT] = { 0 };
}
