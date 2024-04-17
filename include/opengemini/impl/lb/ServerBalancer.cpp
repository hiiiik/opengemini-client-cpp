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

#include "opengemini/impl/lb/ServerBalancer.hpp"

#include <unordered_set>

#include <fmt/format.h>

#include "opengemini/Exceptions.hpp"
#include "opengemini/impl/comm/UrlTargets.hpp"
#include "opengemini/impl/util/Preprocessor.hpp"

namespace opengemini::impl::lb {

OPENGEMINI_INLINE_SPECIFIER
ServerBalancer::ServerBalancer(const std::vector<Endpoint>&       endpoints,
                               std::shared_ptr<http::IHttpClient> http,
                               std::chrono::seconds healthCheckPeriod) :
    http_(http),
    servers_(endpoints.size()),
    timer_(ctx_, healthCheckPeriod),
    healthCheckPeriod_(healthCheckPeriod)
{
    CheckEndpoints(endpoints);
    for (std::size_t idx = 0; idx < endpoints.size(); ++idx) {
        servers_[idx].endpoint = endpoints[idx];
    }

    timer_.async_wait([this](auto error) {
        if (error == boost::asio::error::operation_aborted) { return; }
        HealthCheck();
    });
}

OPENGEMINI_INLINE_SPECIFIER
const Endpoint& ServerBalancer::PickServer(std::size_t index) const
{
    if (index >= servers_.size()) {
        throw InvalidArgument("index out of range");
    }

    return servers_[index].endpoint;
}

OPENGEMINI_INLINE_SPECIFIER
const Endpoint& ServerBalancer::PickAvailableServer()
{
    for (std::size_t cnt = 0, size = servers_.size(); cnt < size; ++cnt) {
        auto idx = nextIdx_.fetch_add(1, std::memory_order_relaxed);
        const auto& [endpoint, good] = servers_[idx % size];
        if (!good.load(std::memory_order_relaxed)) { continue; }

        std::size_t newVal{ 0 };
        do {
            newVal = idx % size;
        } while (!nextIdx_.compare_exchange_weak(idx,
                                                 newVal,
                                                 std::memory_order_relaxed));

        return endpoint;
    }

    throw ServerError("No available server");
}

OPENGEMINI_INLINE_SPECIFIER
void ServerBalancer::HealthCheck()
{
    for (auto& server : servers_) {
        http_->Get(server.endpoint, url::PING, [&server](auto error, auto rsp) {
            server.good.store(!error &&
                                  rsp.result() == http::HttpStatus::no_content,
                              std::memory_order_relaxed);
        });
    }

    timer_.expires_after(healthCheckPeriod_);
    timer_.async_wait([this](auto error) {
        if (error == boost::asio::error::operation_aborted) { return; }
        HealthCheck();
    });
}

OPENGEMINI_INLINE_SPECIFIER
void ServerBalancer::CheckEndpoints(
    const std::vector<Endpoint>& endpoints) const
{
    if (endpoints.empty()) {
        throw InvalidArgument("At least one endpoint needed");
    }

    std::unordered_set<Endpoint, Endpoint::Hasher> dup;
    for (auto& endpoint : endpoints) {
        if (endpoint.host.empty()) {
            throw InvalidArgument("Host field should not be empty");
        }

        auto [it, success] = dup.insert(endpoint);
        if (!success) {
            throw InvalidArgument(
                fmt::format("Duplicate endpoint: {}:{}", it->host, it->port));
        }
    }
}

} // namespace opengemini::impl::lb
