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

#ifndef OPENGEMINI_IMPL_COMM_CONTEXT_HPP
#define OPENGEMINI_IMPL_COMM_CONTEXT_HPP

#include <thread>

#include <boost/asio.hpp>

namespace opengemini::impl {

class Context {
public:
    Context() :
        ctxGuard_(boost::asio::make_work_guard(ctx_)),
        thread_([this] {
            for (;;) {
                try {
                    ctx_.run();
                    break;
                }
                catch (...) {
                }
            }
        })
    { }

    ~Context()
    {
        ctxGuard_.reset();
        ctx_.stop();
        if (thread_.joinable()) { thread_.join(); }
    }

protected:
    boost::asio::io_context ctx_;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type>
                ctxGuard_;
    std::thread thread_;

private:
    Context(const Context&)            = delete;
    Context(Context&& impl)            = delete;
    Context& operator=(const Context&) = delete;
    Context& operator=(Context&& impl) = delete;
};

} // namespace opengemini::impl

#endif // !OPENGEMINI_IMPL_COMM_CONTEXT_HPP
