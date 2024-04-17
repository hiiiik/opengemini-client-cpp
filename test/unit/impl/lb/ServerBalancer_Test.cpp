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

#include <future>
#include <unordered_set>

#include <gtest/gtest.h>

#include "opengemini/Exceptions.hpp"
#include "opengemini/impl/lb/ServerBalancer.hpp"
#include "test/MockIHttpClient.hpp"
#include "test/Random.hpp"

using namespace std::chrono_literals;

namespace opengemini::test {

using namespace opengemini::impl;

class ServerBalancerTestFixture : public testing::Test {
protected:
    ServerBalancerTestFixture() :
        endpoints([] {
            std::vector<Endpoint> endpoints(GenerateRandomNumber(2, 50));
            std::generate(endpoints.begin(), endpoints.end(), [] {
                return Endpoint{ GenerateRandomString(10),
                                 static_cast<uint16_t>(
                                     GenerateRandomNumber(1, 65535)) };
            });
            return endpoints;
        }())
    { }

protected:
    std::vector<Endpoint> endpoints;
};

TEST(ServerBalancerTest, ConstructWithInvalidEndpoint)
{
    EXPECT_THROW(std::ignore = lb::ServerBalancer({}, nullptr),
                 InvalidArgument);

    EXPECT_THROW(
        lb::ServerBalancer lb({ { "host1", 1 }, { "", 2 }, { "host3", 3 } },
                              nullptr),
        InvalidArgument);

    EXPECT_THROW(lb::ServerBalancer lb({ { "host1", 1 },
                                         { "host2", 2 },
                                         { "host3", 3 },
                                         { "host2", 4 },
                                         { "host2", 2 } },
                                       nullptr),
                 InvalidArgument);
}

TEST_F(ServerBalancerTestFixture, SimplePickAllServer)
{
    lb::ServerBalancer lb{ endpoints, nullptr, 99999s };
    for (std::size_t idx = 0; idx < endpoints.size(); ++idx) {
        EXPECT_EQ(lb.PickAvailableServer(), endpoints[idx]);
    }
}

TEST_F(ServerBalancerTestFixture, HealthCheckAllAvailable)
{
    auto mockHttp = std::make_shared<MockIHttpClient>();
    EXPECT_CALL(*mockHttp, SendRequest(testing::_, testing::_, testing::_))
        .Times(testing::AtLeast(endpoints.size()))
        .WillRepeatedly(testing::Return(
            http::HttpResponse{ http::HttpStatus::no_content, 11 }));

    lb::ServerBalancer lb{ endpoints, mockHttp, 1s };
    std::this_thread::sleep_for(2s);

    for (const auto& endpoint : endpoints) {
        EXPECT_EQ(lb.PickAvailableServer(), endpoint);
    }
}

TEST_F(ServerBalancerTestFixture, HealthCheckAllUnavailable)
{
    auto mockHttp = std::make_shared<MockIHttpClient>();
    EXPECT_CALL(*mockHttp, SendRequest(testing::_, testing::_, testing::_))
        .Times(testing::AtLeast(endpoints.size()))
        .WillRepeatedly(testing::Throw(NetworkError("dummy network error")));

    lb::ServerBalancer lb{ endpoints, mockHttp, 1s };
    std::this_thread::sleep_for(2s);

    EXPECT_THROW(lb.PickAvailableServer(), ServerError);
}

TEST_F(ServerBalancerTestFixture, HealthCheckPartiallyAvailable)
{
    std::unordered_set<Endpoint, Endpoint::Hasher> unavailableEndpoints;
    std::generate_n(
        std::inserter(unavailableEndpoints, unavailableEndpoints.begin()),
        GenerateRandomNumber(1, endpoints.size() / 2),
        [this] {
            return endpoints[GenerateRandomNumber(0, endpoints.size() - 1)];
        });

    auto mockHttp = std::make_shared<MockIHttpClient>();
    EXPECT_CALL(
        *mockHttp,
        SendRequest(testing::Not(testing::AnyOfArray(unavailableEndpoints)),
                    testing::_,
                    testing::_))
        .Times(testing::AtLeast(endpoints.size() - unavailableEndpoints.size()))
        .WillRepeatedly(testing::Return(
            http::HttpResponse{ http::HttpStatus::no_content, 11 }));
    EXPECT_CALL(*mockHttp,
                SendRequest(testing::AnyOfArray(unavailableEndpoints),
                            testing::_,
                            testing::_))
        .Times(testing::AtLeast(unavailableEndpoints.size()))
        .WillRepeatedly(testing::Throw(NetworkError("dummy network error")));

    lb::ServerBalancer lb{ endpoints, mockHttp, 1s };
    std::this_thread::sleep_for(2s);

    std::vector<std::future<Endpoint>> futures;
    for (std::size_t cnt = 0; cnt < endpoints.size() * 2; ++cnt) {
        futures.emplace_back(
            std::async([&lb] { return lb.PickAvailableServer(); }));
    }
    for (auto& future : futures) {
        EXPECT_THAT(future.get(),
                    testing::Not(testing::AnyOfArray(unavailableEndpoints)));
    }
}

} // namespace opengemini::test
