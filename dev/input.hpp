#ifndef _MASYS_DEV_INPUT_HPP_
#define _MASYS_DEV_INPUT_HPP_

#include "types.hpp"

#include "dev/device.hpp"

namespace masys {
namespace dev {

class CharacterInput : public Device {
public:
    virtual Status getch( u8 & data ) = 0;
};

} /* dev */
} /* masys */

#endif /* end of include guard: _MASYS_DEV_INPUT_HPP_ */
