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

#ifndef OPENGEMINI_IMPL_CLIENTIMPL_HPP
#define OPENGEMINI_IMPL_CLIENTIMPL_HPP

#include <memory>

#include "opengemini/ClientConfig.hpp"
#include "opengemini/impl/http/IHttpClient.hpp"
#include "opengemini/impl/lb/ServerBalancer.hpp"

namespace opengemini::impl {

class ClientImpl : public Context {
public:
    explicit ClientImpl(const ClientConfig& config);
    ~ClientImpl() = default;

    template<typename COMPLETION_TOKEN>
    auto Ping(std::size_t index, COMPLETION_TOKEN&& token);

private:
    std::shared_ptr<http::IHttpClient>
    ConstructHttpClient(const ClientConfig& config) const;

    template<typename RETURN_TYPE,
             typename COMPLETION_TOKEN,
             typename INITIATION,
             typename... ARGS>
    void Initiate(INITIATION&& init, COMPLETION_TOKEN&& token, ARGS... args);

    static std::size_t ChooseThreadsNum(std::size_t concurrencyHint) noexcept;

private:
    std::shared_ptr<http::IHttpClient> http_;
    lb::ServerBalancer                 lb_;
};

} // namespace opengemini::impl

#include "opengemini/impl/ClientImpl.tpp"
#ifndef OPENGEMINI_SEPERATE_COMPILATION
#    include "opengemini/impl/ClientImpl.cpp"
#endif // !OPENGEMINI_SEPERATE_COMPILATION

#endif // !OPENGEMINI_IMPL_CLIENT_HPP
