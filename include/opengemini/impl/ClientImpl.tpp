//
// Copyright 2024 Huawei Cloud Computing Technologies Co., Ltd.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include "opengemini/impl/ClientImpl.hpp"

#include <exception>
#include <type_traits>

#include "opengemini/Exceptions.hpp"

namespace opengemini::impl {

template<typename RETURN_TYPE,
         typename COMPLETION_TOKEN,
         typename INITIATION,
         typename... ARGS>
void ClientImpl::Initiate(INITIATION&&       init,
                          COMPLETION_TOKEN&& token,
                          ARGS... args)
try {
    init(std::forward<COMPLETION_TOKEN>(token), std::forward<ARGS>(args)...);
}
catch (const Exception& e) {
    auto handler = [_token    = std::forward<COMPLETION_TOKEN>(token),
                    exception = std::current_exception()]() mutable {
        if constexpr (std::is_void_v<RETURN_TYPE>) { _token(exception); }
        else {
            _token(exception, RETURN_TYPE{});
        }
    };
    boost::asio::post(ctx_, std::move(handler));
}
catch (...) {
    auto handler = [_token    = std::forward<COMPLETION_TOKEN>(token),
                    exception = std::current_exception()]() mutable {
        auto error = std::make_exception_ptr(
            RuntimeError("initiating async task failed", exception));

        if constexpr (std::is_void_v<RETURN_TYPE>) { _token(std::move(error)); }
        else {
            _token(std::move(error), RETURN_TYPE{});
        }
    };
    boost::asio::post(ctx_, std::move(handler));
}

} // namespace opengemini::impl

#include "opengemini/impl/cli/Ping.tpp"
