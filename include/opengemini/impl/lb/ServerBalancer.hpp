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

#ifndef OPENGEMINI_IMPL_LB_SERVERBALANCER_HPP
#define OPENGEMINI_IMPL_LB_SERVERBALANCER_HPP

#include <atomic>
#include <memory>
#include <vector>

#include "opengemini/Endpoint.hpp"
#include "opengemini/impl/comm/Context.hpp"
#include "opengemini/impl/http/IHttpClient.hpp"

namespace opengemini::impl::lb {

class ServerBalancer : public Context {
public:
    ServerBalancer(
        const std::vector<Endpoint>&       endpoints,
        std::shared_ptr<http::IHttpClient> http,
        std::chrono::seconds healthCheckPeriod = std::chrono::seconds(10));
    ~ServerBalancer() = default;

    const Endpoint& PickServer(std::size_t index) const;
    const Endpoint& PickAvailableServer();

private:
    void CheckEndpoints(const std::vector<Endpoint>& endpoints) const;
    void HealthCheck();

private:
    struct Server {
        Endpoint          endpoint;
        std::atomic<bool> good{ true };
    };

    std::shared_ptr<http::IHttpClient> http_;
    std::vector<Server>                servers_;
    std::atomic<std::size_t>           nextIdx_{ 0 };

    boost::asio::steady_timer timer_;

    const std::chrono::seconds healthCheckPeriod_;
};

} // namespace opengemini::impl::lb

#ifndef OPENGEMINI_SEPERATE_COMPILATION
#    include "opengemini/impl/lb/ServerBalancer.cpp"
#endif // !OPENGEMINI_SEPERATE_COMPILATION

#endif // !OPENGEMINI_IMPL_LB_SERVERBALANCER_HPP
