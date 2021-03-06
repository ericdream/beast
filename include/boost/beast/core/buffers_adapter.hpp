//
// Copyright (c) 2016-2017 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BOOST_BEAST_BUFFERS_ADAPTER_HPP
#define BOOST_BEAST_BUFFERS_ADAPTER_HPP

#include <boost/beast/core/detail/config.hpp>
#include <boost/beast/core/type_traits.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/optional.hpp>
#include <type_traits>

namespace boost {
namespace beast {

/** Adapts a <em>MutableBufferSequence</em> into a <em>DynamicBuffer</em>.

    This class wraps a <em>MutableBufferSequence</em> to meet the requirements
    of <em>DynamicBuffer</em>. Upon construction the input and output sequences
    are empty. A copy of the mutable buffer sequence object is stored; however,
    ownership of the underlying memory is not transferred. The caller is
    responsible for making sure that referenced memory remains valid
    for the duration of any operations.

    The size of the mutable buffer sequence determines the maximum
    number of bytes which may be prepared and committed.

    @tparam MutableBufferSequence The type of mutable buffer sequence to wrap.
*/
template<class MutableBufferSequence>
class buffers_adapter
{
    static_assert(net::is_mutable_buffer_sequence<
            MutableBufferSequence>::value,
        "MutableBufferSequence requirements not met");

    using iter_type = typename
        detail::buffer_sequence_iterator<
            MutableBufferSequence>::type;

    MutableBufferSequence bs_;
    iter_type begin_;
    iter_type out_;
    iter_type end_;
    std::size_t max_size_;
    std::size_t in_pos_ = 0;    // offset in *begin_
    std::size_t in_size_ = 0;   // size of input sequence
    std::size_t out_pos_ = 0;   // offset in *out_
    std::size_t out_end_ = 0;   // output end offset

    template<class Deduced>
    buffers_adapter(Deduced&& other,
        std::size_t nbegin, std::size_t nout,
            std::size_t nend)
        : bs_(std::forward<Deduced>(other).bs_)
        , begin_(std::next(bs_.begin(), nbegin))
        , out_(std::next(bs_.begin(), nout))
        , end_(std::next(bs_.begin(), nend))
        , max_size_(other.max_size_)
        , in_pos_(other.in_pos_)
        , in_size_(other.in_size_)
        , out_pos_(other.out_pos_)
        , out_end_(other.out_end_)
    {
    }

    iter_type end_impl() const;

public:
    /// The type of the underlying mutable buffer sequence
    using value_type = MutableBufferSequence;

    /// Move constructor.
    buffers_adapter(buffers_adapter&& other);

    /// Copy constructor.
    buffers_adapter(buffers_adapter const& other);

    /// Move assignment.
    buffers_adapter& operator=(buffers_adapter&& other);

    /// Copy assignment.
    buffers_adapter& operator=(buffers_adapter const&);

    /** Construct a buffers adapter.

        @param buffers The mutable buffer sequence to wrap. A copy of
        the object will be made, but ownership of the memory is not
        transferred.
    */
    explicit
    buffers_adapter(MutableBufferSequence const& buffers);

    /** Constructor

        This constructs the buffer adapter in-place from
        a list of arguments.

        @param args Arguments forwarded to the buffers constructor.
    */
    template<class... Args>
    explicit
    buffers_adapter(boost::in_place_init_t, Args&&... args);

    /// Returns the original mutable buffer sequence
    value_type const&
    value() const
    {
        return bs_;
    }

    //--------------------------------------------------------------------------

#if BOOST_BEAST_DOXYGEN
    /// The ConstBufferSequence used to represent the readable bytes.
    using const_buffers_type = __implementation_defined__;

    /// The MutableBufferSequence used to represent the writable bytes.
    using mutable_buffers_type = __implementation_defined__;

#else
    class const_buffers_type;
    class mutable_buffers_type;
#endif

    /// Returns the number of readable bytes.
    std::size_t
    size() const noexcept
    {
        return in_size_;
    }

    /// Return the maximum number of bytes, both readable and writable, that can ever be held.
    std::size_t
    max_size() const noexcept
    {
        return max_size_;
    }
    
    /// Return the maximum number of bytes, both readable and writable, that can be held without requiring an allocation.
    std::size_t
    capacity() const noexcept
    {
        return max_size_;
    }

    /// Returns a constant buffer sequence representing the readable bytes
    const_buffers_type
    data() const noexcept;

    /** Returns a mutable buffer sequence representing writable bytes.
    
        Returns a mutable buffer sequence representing the writable
        bytes containing exactly `n` bytes of storage. This function
        does not allocate memory. Instead, the storage comes from
        the underlying mutable buffer sequence.

        All buffer sequences previously obtained using @ref prepare are
        invalidated. Buffer sequences previously obtained using @ref data
        remain valid.

        @param n The desired number of bytes in the returned buffer
        sequence.

        @throws std::length_error if `size() + n` exceeds `max_size()`.

        @par Exception Safety

        Strong guarantee.
    */
    mutable_buffers_type
    prepare(std::size_t n);

    /** Append writable bytes to the readable bytes.

        Appends n bytes from the start of the writable bytes to the
        end of the readable bytes. The remainder of the writable bytes
        are discarded. If n is greater than the number of writable
        bytes, all writable bytes are appended to the readable bytes.

        All buffer sequences previously obtained using @ref prepare are
        invalidated. Buffer sequences previously obtained using @ref data
        remain valid.

        @param n The number of bytes to append. If this number
        is greater than the number of writable bytes, all
        writable bytes are appended.

        @par Exception Safety

        No-throw guarantee.
    */
    void
    commit(std::size_t n) noexcept;

    /** Remove bytes from beginning of the readable bytes.

        Removes n bytes from the beginning of the readable bytes.

        All buffers sequences previously obtained using
        @ref data or @ref prepare are invalidated.

        @param n The number of bytes to remove. If this number
        is greater than the number of readable bytes, all
        readable bytes are removed.

        @par Exception Safety

        No-throw guarantee.
    */
    void
    consume(std::size_t n) noexcept;
};

} // beast
} // boost

#include <boost/beast/core/impl/buffers_adapter.hpp>

#endif
