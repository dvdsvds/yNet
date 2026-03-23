#pragma once

#include <ynet/request.h>
#include <ynet/response.h>
#include <functional>

namespace ynet {
    using Next = std::function<void ()>;
    using Middleware = std::function<void(const Request&, Response&, Next)>;
};
