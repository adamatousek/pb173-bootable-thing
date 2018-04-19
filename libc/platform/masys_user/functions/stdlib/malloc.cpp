#include<stdlib.h>
#include<_PDCLIB_glue.h>
#include<_PDCLIB_config.h>

#ifndef __cplusplus
#error "C++ is needed for malloc"
#else

class SubpageAllocator {
    struct ChunkHeader {
        unsigned szfl;

        unsigned size() const { return szfl & ~0x7; }
        void size( unsigned s ) { szfl = s | ( szfl & 0x7 ); }
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
        unsigned size() const { return header.size(); }
        void size( unsigned s ) { header.size( s ); update_footer(); }
        bool free() const { return header.free(); }
        void free( bool f ) { header.free( f ); update_footer(); }
    };

    static_assert( sizeof( Chunk ) == 8,
                   "SubpageAllocator::Chunk has wrong size" );

    Chunk *freelist = nullptr;

public:
    void * alloc( unsigned size );
    void * realloc( void *ptr, unsigned size );
    void free( void *ptr );
} allocator;


void * SubpageAllocator::alloc( unsigned size )
{
    if ( size == 0 )
        return nullptr;

    // Size is a multiple of 8
    size += 7;
    size -= size % 8;

    Chunk *chunk = freelist,
          *prev = nullptr;
    while ( chunk && chunk->size() < size ) {
        prev = chunk;
        chunk = chunk->next();
    }

    if ( ! chunk ) {
        unsigned newpages = ( size + 16 + _PDCLIB_MALLOC_PAGESIZE - 1 ) / _PDCLIB_MALLOC_PAGESIZE;

        void *newmem = _PDCLIB_allocpages( newpages );
        if ( ! newmem )
            return nullptr;

        chunk = reinterpret_cast< Chunk * >( newmem );
        chunk->size( newpages * _PDCLIB_MALLOC_PAGESIZE - 16 );
        chunk->free( true );
        chunk->next() = nullptr;
        chunk->prev() = prev;
        chunk->following()->header.szfl = 0;
        if ( prev )
            prev->next() = chunk;
    }


    Chunk *newfree;
    if ( chunk->size() > size + 8 ) {
        newfree = chunk + ( size / 8 ) + 1;
        newfree->size( chunk->size() - size - 8 );
        newfree->free( true );
        newfree->next() = chunk->next();
        newfree->prev() = chunk;
        chunk->next() = newfree;
        chunk->size( size );
    } else {
        newfree = chunk->next();
    }

    if ( chunk->prev() )
        chunk->prev()->next() = chunk->next();
    else
        freelist = chunk->next();
    if ( chunk->next() )
        chunk->next()->prev() = chunk->prev();

    chunk->free( false );
    return chunk->data();
}

void * SubpageAllocator::realloc( void *ptr, unsigned size )
{
    if ( ptr == nullptr )
        return alloc( size );

    auto chunk = reinterpret_cast< Chunk * >( ptr ) - 1;
    if ( size <= chunk->size() )
        return ptr;

    if ( chunk->following()->free() &&
         size < chunk->following()->size() + chunk->size() + 8 )
    {
        Chunk *foll = chunk->following();
        unsigned residual_size = foll->size() + chunk->size() + 8 - size;
        if ( residual_size > 8 ) { // shrink and move
            foll += ( foll->size() - residual_size ) / 8;
            foll->size( residual_size );
            foll->free( true );
            if ( foll->prev() )
                foll->prev()->next() = foll;
            else
                freelist = foll;
            if ( foll->next() )
                foll->next()->prev() = foll;
        } else { // obliterate
            if ( foll->prev() )
                foll->prev()->next() = foll->next();
            else
                freelist = foll->next();
            if ( foll->next() )
                foll->next()->prev() = foll->prev();
        }
        return ptr;
    }

    auto newmem = reinterpret_cast< char * >( alloc( size ) );
    if ( ! newmem )
        return nullptr;
    for ( unsigned i = 0; i < chunk->size(); ++i ) {
        newmem[ i ] = chunk->data()[ i ];
    }
    free( ptr );
    return newmem;
}

void SubpageAllocator::free( void *ptr )
{
    if ( ptr == nullptr )
        return;

    auto chunk = reinterpret_cast< Chunk * >( ptr ) - 1;
    chunk->free( true );

    Chunk *prec = chunk->preceding(), // Only valid if chunk->pre_footer.free
          *foll = chunk->following();

    if ( ! chunk->pre_footer.free() && ! foll->free() ) {
        freelist->prev() = chunk;
        chunk->next() = freelist;
        chunk->prev() = nullptr;
        freelist = chunk;
    } else if ( chunk->pre_footer.free() && ! foll->free() ) {
        prec->size( prec->size() + chunk->size() + 8 );
    } else if ( ! chunk->pre_footer.free() && foll->free() ) {
        chunk->next() = foll->next();
        chunk->prev() = foll->prev();
        if ( chunk->prev() )
            chunk->prev()->next() = chunk;
        else
            freelist = chunk;
        if ( chunk->next() )
            chunk->next()->prev() = chunk;
        chunk->size( chunk->size() + foll->size() + 8 );
    } else /* both free */ {
        prec->size( prec->size() + chunk->size() + 8 );
        chunk = prec;
        if ( foll->prev() )
            foll->prev()->next() = foll->next();
        else
            freelist = foll->next();
        chunk->size( chunk->size() + foll->size() + 8 );
    }
}


extern "C" {

void * malloc( unsigned long size ) noexcept
{
    return allocator.alloc( size );
}
void free( void *ptr ) noexcept
{
    allocator.free( ptr );
}
void * realloc( void *ptr, unsigned long size ) noexcept
{
    return allocator.realloc( ptr, size );
}

}

#endif

