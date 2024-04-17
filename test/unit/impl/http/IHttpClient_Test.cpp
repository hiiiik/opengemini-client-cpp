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
#include <memory>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "opengemini/CompletionToken.hpp"
#include "opengemini/impl/http/HttpClient.hpp"
#ifdef OPENGEMINI_ENABLE_SSL_SUPPORT
#    include "opengemini/impl/http/HttpsClient.hpp"
#endif // OPENGEMINI_ENABLE_SSL_SUPPORT
#include "opengemini/Exceptions.hpp"
#include "test/Random.hpp"

namespace opengemini::test {

using namespace std::chrono_literals;
using namespace impl::http;

class IHttpClientTestFixture : public testing::Test {
protected:
    IHttpClientTestFixture()
    {
        clients.push_back(std::make_unique<HttpClient>(5s, 5s, 4));
        endpoints.emplace_back("httpbin.org", 80);
#ifdef OPENGEMINI_ENABLE_SSL_SUPPORT
        clients.push_back(
            std::make_unique<HttpsClient>(TLSConfig{}, 5s, 5s, 4));
        endpoints.emplace_back("httpbin.org", 443);
#endif // OPENGEMINI_ENABLE_SSL_SUPPORT
    }

protected:
    std::vector<std::unique_ptr<IHttpClient>> clients;
    std::vector<Endpoint>                     endpoints;
};

TEST_F(IHttpClientTestFixture, SyncGetRequest)
{
    for (auto idx = 0; idx < clients.size(); ++idx) {
        auto rsp = clients[idx]->Get(endpoints[idx], "/get", token::sync);
        EXPECT_EQ(rsp.result_int(), 200);
        EXPECT_TRUE(!rsp.body().empty());
    }
}

TEST_F(IHttpClientTestFixture, AsyncGetRequest)
{
    std::vector<std::future<void>> futures;
    for (auto idx = 0; idx < clients.size(); ++idx) {
        std::promise<void> promise;
        futures.push_back(promise.get_future());

        clients[idx]->Get(
            endpoints[idx],
            "/get",
            [promise = std::move(promise)](auto error, auto rsp) mutable {
                EXPECT_EQ(error, nullptr);
                EXPECT_EQ(rsp.result_int(), 200);
                EXPECT_TRUE(!rsp.body().empty());
                promise.set_value();
            });
    }

    for (auto& future : futures) { future.get(); }
}

TEST_F(IHttpClientTestFixture, SyncPostRequest)
{
    constexpr auto body = "test content";
    for (auto idx = 0; idx < clients.size(); ++idx) {
        auto rsp =
            clients[idx]->Post(endpoints[idx], "/anything", body, token::sync);
        EXPECT_EQ(rsp.result_int(), 200);
        EXPECT_THAT(rsp.body(), testing::HasSubstr(body));
    }
}

TEST_F(IHttpClientTestFixture, AsyncPostRequest)
{
    constexpr auto                 body = "test content";
    std::vector<std::future<void>> futures;
    for (auto idx = 0; idx < clients.size(); ++idx) {
        std::promise<void> promise;
        futures.push_back(promise.get_future());

        clients[idx]->Post(endpoints[idx],
                           "/anything",
                           body,
                           [promise = std::move(promise),
                            &body](auto error, auto rsp) mutable {
                               EXPECT_EQ(error, nullptr);
                               EXPECT_EQ(rsp.result_int(), 200);
                               EXPECT_THAT(rsp.body(),
                                           testing::HasSubstr(body));
                               promise.set_value();
                           });
    }

    for (auto& future : futures) { future.get(); }
}

TEST_F(IHttpClientTestFixture, CallSyncRequestFromMultiThreads)
{
    std::vector<std::future<void>> futures;
    for (auto idx = 0; idx < clients.size(); ++idx) {
        for (auto cnt = 0; cnt < 10; ++cnt) {
            futures.push_back(std::async(std::launch::async, [this, idx] {
                auto rsp =
                    clients[idx]->Get(endpoints[idx], "/range/32", token::sync);
                EXPECT_EQ(rsp.result_int(), 200);
                EXPECT_EQ(rsp.body(), "abcdefghijklmnopqrstuvwxyzabcdef");
            }));
        }
    }

    for (auto& future : futures) { future.get(); }
}

TEST_F(IHttpClientTestFixture, CallAsyncRequestFromMultiThreads)
{
    std::vector<std::future<void>> futures, ignores;
    for (auto idx = 0; idx < clients.size(); ++idx) {
        for (auto cnt = 0; cnt < 10; ++cnt) {
            std::promise<void> promise;
            futures.push_back(promise.get_future());

            auto body = GenerateRandomString(GenerateRandomNumber(1, 100));
            auto sendReq =
                [this, idx, body, promise = std::move(promise)]() mutable {
                    clients[idx]->Post(
                        endpoints[idx],
                        "/anything",
                        body,
                        [promise = std::move(promise), body](auto error,
                                                             auto rsp) mutable {
                            EXPECT_EQ(error, nullptr);
                            EXPECT_EQ(rsp.result_int(), 200);
                            EXPECT_THAT(rsp.body(), testing::HasSubstr(body));
                            promise.set_value();
                        });
                };
            ignores.push_back(
                std::async(std::launch::async, std::move(sendReq)));
        }
    }

    for (auto& future : futures) { future.get(); }
}

TEST_F(IHttpClientTestFixture, InvalidHost)
{
    for (auto idx = 0; idx < clients.size(); ++idx) {
        EXPECT_THROW(std::ignore = clients[idx]->Get({ "123.456.789.10", 123 },
                                                     "/get",
                                                     token::sync),
                     NetworkError);
    }
}

TEST_F(IHttpClientTestFixture, InvalidPort)
{
    for (auto idx = 0; idx < clients.size(); ++idx) {
        EXPECT_THROW(
            std::ignore =
                clients[idx]->Get({ "httpbin.org", 12345 }, "/", token::sync),
            NetworkError);
    }
}

TEST_F(IHttpClientTestFixture, WithoutSetDefaultHeaders)
{
    using boost::beast::http::field;

    for (auto idx = 0; idx < clients.size(); ++idx) {
        auto rsp = clients[idx]->Get(endpoints[idx], "/headers", token::sync);
        EXPECT_THAT(rsp.body(), testing::HasSubstr(R"("Host": "httpbin.org")"));
        EXPECT_THAT(rsp.body(),
                    testing::ContainsRegex(
                        R"(opengemini-client-cxx/[0-9]+\.[0-9]+\.[0-9]+)"));
    }
}

TEST_F(IHttpClientTestFixture, SetDefaultHeaders)
{
    using boost::beast::http::field;

    for (auto idx = 0; idx < clients.size(); ++idx) {
        clients[idx]->DefaultHeaders() = { { "Content-Type", "text/plain" },
                                           { "Authorization", "dummy" } };
        auto rsp = clients[idx]->Get(endpoints[idx], "/headers", token::sync);
        EXPECT_THAT(rsp.body(), testing::HasSubstr(R"("Host": "httpbin.org")"));
        EXPECT_THAT(
            rsp.body(),
            testing::ContainsRegex(
                R"("User-Agent": "opengemini-client-cxx/[0-9]+\.[0-9]+\.[0-9]+")"));
        EXPECT_THAT(rsp.body(),
                    testing::HasSubstr(R"("Content-Type": "text/plain")"));
        EXPECT_THAT(rsp.body(),
                    testing::HasSubstr(R"("Authorization": "dummy")"));
    }
}

TEST_F(IHttpClientTestFixture, SetDefaultHeadersWhichWillBeCoverd)
{
    using boost::beast::http::field;

    for (auto idx = 0; idx < clients.size(); ++idx) {
        clients[idx]->DefaultHeaders() = { { "User-Agent", "dummy" },
                                           { "Host", "dummy" } };
        auto rsp = clients[idx]->Get(endpoints[idx], "/headers", token::sync);
        EXPECT_THAT(rsp.body(), testing::HasSubstr(R"("Host": "httpbin.org")"));
        EXPECT_THAT(
            rsp.body(),
            testing::ContainsRegex(
                R"("User-Agent": "opengemini-client-cxx/[0-9]+\.[0-9]+\.[0-9]+")"));
    }
}

TEST_F(IHttpClientTestFixture, ReadWriteTimeout)
{
    for (auto idx = 0; idx < clients.size(); ++idx) {
        EXPECT_THROW(
            std::ignore =
                clients[idx]->Get(endpoints[idx], "/delay/7", token::sync),
            NetworkError);
    }
}

} // namespace opengemini::test
