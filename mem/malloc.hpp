#ifndef _MASYS_MEM_MALLOC_HPP_
#define _MASYS_MEM_MALLOC_HPP_

#include <types.hpp>

namespace masys {
namespace mem {

class PageAllocator;

class SubpageAllocator {
    PageAllocator *pal;
    struct ChunkHeader {
        u32 szfl;

        u32 size() const { return szfl & ~0x7; }
        void size( u32 s ) { szfl = s | ( szfl & 0x7 ); }
        bool free() const { return szfl & 0x1; }
        void free( bool f ) { szfl = f ? szfl | 0x1 : szfl & ~0x1; }
    };
    struct alignas( 8 ) Chunk {
        ChunkHeader pre_footer;
        ChunkHeader header;

        Chunk * following() {
            return reinterpret_cast< Chunk * >( header.size() + 8 +
                    reinterpret_cast< char * >( this ) );
        }
        Chunk * preceding() {
            return reinterpret_cast< Chunk * >( - pre_footer.size() - 8 +
                    reinterpret_cast< char * >( this ) );
        }
        void update_footer() { following()->pre_footer = header; }
        char * data() { return reinterpret_cast< char * >( this + 1 ); }
        Chunk *& next() { return *reinterpret_cast< Chunk ** >( this + 1 ); }
        Chunk *& prev() { return *(reinterpret_cast< Chunk ** >( this + 1 ) + 1 ); }
        u32 size() const { return header.size(); }
        void size( u32 s ) { header.size( s ); update_footer(); }
        bool free() const { return header.free(); }
        void free( bool f ) { header.free( f ); update_footer(); }
    };

    static_assert( sizeof( Chunk ) == 8,
                   "SubpageAllocator::Chunk has wrong size" );

    Chunk *freelist = nullptr;

public:
    SubpageAllocator( PageAllocator *pa )
        : pal( pa ) {}
    void * alloc( u32 size );
    void * realloc( void *ptr, u32 size );
    void free( void *ptr );
};

extern SubpageAllocator *allocator;

} /* mem */
} /* masys */


extern "C" {

inline void * kmalloc( unsigned long size )
{
    return masys::mem::allocator->alloc( size );
}
inline void kfree( void *ptr )
{
    masys::mem::allocator->free( ptr );
}
inline void * krealloc( void *ptr, unsigned long size )
{
    return masys::mem::allocator->realloc( ptr, size );
}

}

#endif /* end of include guard: _MASYS_MEM_MALLOC_HPP_ */
