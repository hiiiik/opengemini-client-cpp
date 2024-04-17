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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "opengemini/ClientConfigBuilder.hpp"
#include "opengemini/CompletionToken.hpp"
#include "opengemini/impl/ClientImpl.hpp"
#include "test/HackMember.hpp"
#include "test/MockIHttpClient.hpp"

namespace opengemini::test {

using namespace opengemini::impl;

OPENGEMINI_TEST_MEMBER_HACKER(ClientImpl, &ClientImpl::http_)

TEST(ClientImplTest, PingSuccess)
{
    auto mockHttp = std::make_shared<MockIHttpClient>();
    EXPECT_CALL(*mockHttp, SendRequest(testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(
            http::HttpResponse{ http::HttpStatus::no_content, 11 }));

    ClientImpl cli{ { { { "0.0.0.0", 1234 } } } };

    auto& hackHttp_ = cli.*(std::get<0>(HackingMember(cli)));
    hackHttp_       = mockHttp;
    EXPECT_NO_THROW(cli.Ping(0, token::sync));
}

TEST(ClientImplTest, PingFailedOutOfRange)
{
    ClientImpl cli{ ClientConfigBuilder()
                        .AppendAddresses({ { "dummy_host1", 1234 },
                                           { "dummy_host2", 4321 } })
                        .Finalize() };

    EXPECT_THROW(cli.Ping(2, token::sync), InvalidArgument);
}

TEST(ClientImplTest, PingFailedUnexpectedResponse)
{
    auto mockHttp = std::make_shared<MockIHttpClient>();
    EXPECT_CALL(*mockHttp, SendRequest(testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(
            http::HttpResponse{ http::HttpStatus::internal_server_error, 11 }));

    ClientImpl cli{ { { { "0.0.0.0", 1234 } } } };

    auto& hackHttp_ = cli.*(std::get<0>(HackingMember(cli)));
    hackHttp_       = mockHttp;
    EXPECT_THROW(cli.Ping(0, token::sync), ServerError);
}

TEST(ClientImplTest, PingFailedNetworkError)
{
    ClientImpl cli{ { { { "0.0.0.0", 1234 } } } };

    EXPECT_THROW(cli.Ping(0, token::sync), NetworkError);
}

} // namespace opengemini::test
