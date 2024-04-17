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

#include <fmt/format.h>

#include "opengemini/impl/comm/UrlTargets.hpp"

namespace opengemini::impl {

template<typename COMPLETION_TOKEN>
auto ClientImpl::Ping(std::size_t index, COMPLETION_TOKEN&& token)
{
    auto ping = [this](auto&& token, std::size_t index) mutable {
        auto server = lb_.PickServer(index);

        auto done = [_token = std::forward<decltype(token)>(
                         token)](auto error, auto rsp) mutable {
            if (!error && rsp.result() != http::HttpStatus::no_content) {
                error = std::make_exception_ptr(
                    ServerError(fmt::format("unexpected status code: {}",
                                            rsp.result_int())));
            }
            _token(error);
        };
        http_->Get(std::move(server), url::PING, done);
    };

    auto init = [this](auto&& token, auto&& ping, std::size_t index) mutable {
        Initiate<void>(std::forward<decltype(ping)>(ping),
                       std::forward<decltype(token)>(token),
                       index);
    };

    return boost::asio::async_initiate<COMPLETION_TOKEN,
                                       void(std::exception_ptr)>(
        std::move(init),
        token,
        std::move(ping),
        index);
}

} // namespace opengemini::impl
