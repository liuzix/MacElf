//
//  debug.hpp
//  MacElf
//
//  Created by Zixiong Liu on 7/19/18.
//  Copyright © 2018 Zixiong Liu. All rights reserved.
//

#ifndef debug_h
#define debug_h

#include <cstdlib>
#include <cassert>
#include <Hypervisor/hv.h>
#include <Hypervisor/hv_vmx.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>


inline void hvCheck(hv_return_t val) {
    if (HV_SUCCESS != val)
        std::abort();
}




#endif /* debug_h */
