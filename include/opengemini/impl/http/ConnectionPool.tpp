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

#include "opengemini/impl/http/ConnectionPool.hpp"

namespace opengemini::impl::http {

template<typename CRTP, typename STREAM>
ConnectionPool<CRTP, STREAM>::ConnectionPool(
    boost::asio::io_context& ctx,
    std::chrono::seconds     connectTimeout,
    std::size_t              maxSizeForEachEndpoint) :
    ctx_(ctx),
    connectTimeout_(connectTimeout),
    maxSizeForEachEndpoint_(maxSizeForEachEndpoint)
{ }

template<typename CRTP, typename STREAM>
typename ConnectionPool<CRTP, STREAM>::ConnectionPtr
ConnectionPool<CRTP, STREAM>::Retrieve(const Endpoint&            endpoint,
                                       boost::asio::yield_context yield)
{
    {
        std::lock_guard lock(mutex_);
        if (auto& connections = pool_[endpoint]; !connections.empty()) {
            auto connection = std::move(connections.front());
            connections.pop();
            return connection;
        }
    }

    return static_cast<CRTP*>(this)->CreateConnection(endpoint, yield);
}

template<typename CRTP, typename STREAM>
void ConnectionPool<CRTP, STREAM>::Push(const Endpoint& endpoint,
                                        ConnectionPtr   connection)
{
    if (!connection->used) { connection->used = true; }

    std::lock_guard lock(mutex_);
    auto&           connections = pool_[endpoint];
    if (connections.size() >= maxSizeForEachEndpoint_) { return; }
    pool_[endpoint].push(std::move(connection));
}

} // namespace opengemini::impl::http
