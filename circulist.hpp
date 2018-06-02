#ifndef _MASYS_CIRCULIST_HPP_
#define _MASYS_CIRCULIST_HPP_

template< typename T >
struct Circulist : public T {
    T* prev = this;
    T* next = this;

    Circulist & merge( Circulist & o )
    {
        next->prev = o.prev;
        o.prev->next = next;
        next = &o;
        o.prev = this;
        return *this;
    }
};

#endif /* end of include guard: _MASYS_CIRCULIST_HPP_ */
