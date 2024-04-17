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

#include "opengemini/CompletionToken.hpp"

namespace boost::asio {

template<typename Signature>
class async_result<opengemini::token::Sync, Signature> {
public:
    template<typename Initiation, typename RawCompletionToken, typename... Args>
    static auto
    initiate(Initiation&& initiation, RawCompletionToken&&, Args&&... args)
    {
        return async_initiate<const use_future_t<>&, Signature>(
                   [init = std::forward<Initiation>(
                        initiation)](auto&& handler, auto&&... _args) mutable {
                       std::move(init)(std::forward<decltype(handler)>(handler),
                                       std::forward<decltype(_args)>(_args)...);
                   },
                   use_future,
                   std::forward<Args>(args)...)
            .get();
    }
};

} // namespace boost::asio
