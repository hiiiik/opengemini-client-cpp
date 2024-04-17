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

#ifndef OPENGEMINI_IMPL_HTTP_CONNECTIONPOOL_HPP
#define OPENGEMINI_IMPL_HTTP_CONNECTIONPOOL_HPP

#include <memory>
#include <mutex>
#include <queue>
#include <unordered_map>

#include <boost/asio/spawn.hpp>
#include <boost/beast.hpp>

#include "opengemini/Endpoint.hpp"
#include "opengemini/Exceptions.hpp"

namespace opengemini::impl::http {

template<typename STREAM>
struct Connection {
    using Stream = STREAM;

    Stream stream;
    bool   used;

#if __cplusplus < 202002L
    Connection(Stream _stream, bool _used) :
        stream(std::move(_stream)),
        used(_used)
    { }
#endif // (__cplusplus < 202002L)
};

template<typename STREAM>
inline bool ShouldRetry(std::string&&             what,
                        boost::beast::error_code& error,
                        const Connection<STREAM>& connection)
{
    if (!error) { return false; }
    if (connection.used) { return true; }
    throw NetworkError(std::move(what), std::move(error));
}

template<typename CRTP, typename STREAM>
class ConnectionPool {
public:
    using ConnectionPtr = std::unique_ptr<Connection<STREAM>>;
    using Stream        = STREAM;

public:
    ConnectionPool(boost::asio::io_context& ctx,
                   std::chrono::seconds     connectTimeout,
                   std::size_t              maxSizeForEachEndpoint = 3);

    ConnectionPtr Retrieve(const Endpoint&            endpoint,
                           boost::asio::yield_context yield);

    void Push(const Endpoint& endpoint, ConnectionPtr connection);

protected:
    boost::asio::io_context& ctx_;

    const std::chrono::seconds connectTimeout_;

private:
    std::unordered_map<Endpoint, std::queue<ConnectionPtr>, Endpoint::Hasher>
               pool_;
    std::mutex mutex_;

    const std::size_t maxSizeForEachEndpoint_;
};

} // namespace opengemini::impl::http

#include "opengemini/impl/http/ConnectionPool.tpp"

#endif // !OPENGEMINI_IMPL_HTTP_CONNECTIONPOOL_HPP
