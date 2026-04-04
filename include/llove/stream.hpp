#pragma once

#include <iosfwd>
#include <type_traits>
#include <utility>

#include <llvm/Support/raw_ostream.h>

namespace llove
{
    template<typename T, typename B>
    concept base_of = std::is_base_of_v<B, T>;

    template<base_of<std::ios_base> T>
    class stream_ref
    {
    public:
        stream_ref()
            : m_Stream(nullptr),
              m_Cleanup(false)
        {
        }

        stream_ref(
            T *stream,
            const bool cleanup)
            : m_Stream(stream),
              m_Cleanup(cleanup)
        {
        }

        template<typename... Args>
        explicit stream_ref(Args &&... args)
            : m_Stream(new T(args...)),
              m_Cleanup(true)
        {
        }

        stream_ref(const stream_ref &) = delete;

        stream_ref(stream_ref &&other) noexcept
        {
            m_Stream = other.m_Stream;
            m_Cleanup = other.m_Cleanup;

            other.m_Stream = nullptr;
            other.m_Cleanup = false;
        }

        template<base_of<T> S>
        explicit stream_ref(stream_ref<S> &&other) noexcept
        {
            auto ref = other.release();
            m_Stream = dynamic_cast<T *>(ref.first);
            m_Cleanup = ref.second;
        }

        stream_ref &operator=(const stream_ref &) = delete;

        stream_ref &operator=(stream_ref &&other) noexcept
        {
            std::swap(m_Stream, other.m_Stream);
            std::swap(m_Cleanup, other.m_Cleanup);
            return *this;
        }

        template<base_of<T> S>
        stream_ref &operator=(stream_ref<S> &&other) noexcept
        {
            auto ref = other.swap(dynamic_cast<S *>(m_Stream), m_Cleanup);
            m_Stream = dynamic_cast<T *>(ref.first);
            m_Cleanup = ref.second;
            return *this;
        }

        ~stream_ref()
        {
            if (m_Cleanup && m_Stream)
                delete m_Stream;
            m_Stream = nullptr;
        }

        T &operator*() const
        {
            return *m_Stream;
        }

        T *operator->() const
        {
            return m_Stream;
        }

        T *get() const
        {
            return m_Stream;
        }

        std::pair<T *, bool> release()
        {
            std::pair ref(m_Stream, m_Cleanup);
            m_Stream = nullptr;
            m_Cleanup = false;
            return ref;
        }

        std::pair<T *, bool> swap(T *stream, const bool cleanup)
        {
            std::pair ref(m_Stream, m_Cleanup);
            m_Stream = stream;
            m_Cleanup = cleanup;
            return ref;
        }

    private:
        T *m_Stream;
        bool m_Cleanup;
    };

    template<typename C, typename T>
    class raw_pwrite_stream_adapter final : public llvm::raw_pwrite_stream
    {
    public:
        explicit raw_pwrite_stream_adapter(std::basic_ostream<C, T> &stream)
            : raw_pwrite_stream(true),
              m_Stream(stream)
        {
        }

    private:
        void write_impl(const char *ptr, const size_t size) override
        {
            m_Stream.write(ptr, static_cast<std::streamsize>(size));
        }

        uint64_t current_pos() const override
        {
            return m_Stream.tellp();
        }

        void pwrite_impl(const char *ptr, const size_t size, const uint64_t offset) override
        {
            const auto current = m_Stream.tellp();
            m_Stream.seekp(static_cast<std::streamsize>(offset));
            m_Stream.write(ptr, static_cast<std::streamsize>(size));
            m_Stream.seekp(current);
        }

    protected:
        std::basic_ostream<C, T> &m_Stream;
    };
}
