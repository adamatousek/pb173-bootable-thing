#ifndef _MASYS_DEV_DEVICE_HPP_
#define _MASYS_DEV_DEVICE_HPP_

#include <status.hpp>

namespace masys {
namespace dev {

class Device {
public:
    virtual Status reinit() { return Status::SUCCESS; } ;
};

} /* dev */
} /* masys */

#endif /* end of include guard: _MASYS_DEV_DEVICE_HPP_ */
