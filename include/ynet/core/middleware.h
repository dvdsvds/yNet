#pragma once

#include "ynet/core/request.h"
#include "ynet/core/response.h"
#include <functional>

namespace ynet {
    using Next = std::function<void ()>;
    using Middleware = std::function<void(Request&, Response&, Next)>;
};
