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

#ifdef OPENGEMINI_ENABLE_SSL_SUPPORT

// clang-format off
#include <gtest/gtest.h>

#include "opengemini/CompletionToken.hpp"
#include "opengemini/Exceptions.hpp"
#include "opengemini/impl/http/HttpsClient.hpp"
#include "test/SelfRootCA.hpp"
// clang-format on

namespace opengemini::test {

using namespace std::chrono_literals;
using namespace impl::http;

TEST(HttpsClientTest, WithInvalidRootCA)
{
    EXPECT_THROW(HttpsClient cli({ false, {}, "dummy ca" }, 5s, 5s, 4),
                 InvalidArgument);
}

TEST(HttpsClientTest, WithInvalidCertificate)
{
    EXPECT_THROW(HttpsClient cli({ false, "dummy", selfRootCA }, 5s, 5s, 4),
                 InvalidArgument);
}

TEST(HttpsClientTest, WithSelfSignedRootCA)
{
    HttpsClient client{ { false, {}, selfRootCA }, 5s, 5s, 4 };

    EXPECT_THROW(
        std::ignore =
            client.Get({ "httpbin.org", 443 }, "/range/26", token::sync),
        NetworkError);
}

TEST(HttpsClientTest, SkipVerifyPeer)
{
    HttpsClient client{ { true, {}, selfRootCA }, 5s, 5s, 4 };
    auto rsp = client.Get({ "httpbin.org", 443 }, "/range/26", token::sync);
    EXPECT_EQ(rsp.body(), "abcdefghijklmnopqrstuvwxyz");
}

} // namespace opengemini::test

#endif // OPENGEMINI_ENABLE_SSL_SUPPORT
