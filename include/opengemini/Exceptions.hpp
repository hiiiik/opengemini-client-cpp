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

#ifndef OPENGEMINI_EXCEPTIONS_HPP
#define OPENGEMINI_EXCEPTIONS_HPP

#include <exception>
#include <string>
#include <system_error>

namespace opengemini {

///
/// \~Chinese
/// @brief 库通用异常类。
///
/// \~English
/// @brief General exception of the library.
///
class Exception : public std::exception {
public:
    explicit Exception(std::string what) : what_(std::move(what)) { }

    Exception(std::string what, std::error_code nestedError) :
        what_(std::move(what)),
        nestedError_(std::move(nestedError))
    { }

    Exception(std::string what, std::exception_ptr nestedException) :
        what_(std::move(what)),
        nestedException_(nestedException)
    { }

    ~Exception() = default;

    Exception(const Exception& ex) :
        what_(ex.what_),
        nestedError_(ex.nestedError_)
    { }

    Exception(Exception&& ex) noexcept :
        what_(std::move(ex.what_)),
        nestedError_(std::move(ex.nestedError_))
    { }

    Exception& operator=(Exception other) noexcept
    {
        swap(*this, other);
        return *this;
    }

    friend void swap(Exception& lhs, Exception& rhs) noexcept
    {
        using std::swap;
        swap(lhs.what_, rhs.what_);
        swap(lhs.nestedError_, rhs.nestedError_);
    }

    ///
    /// \~Chinese
    /// @brief 返回描述异常信息的字符串。与 @ref What() 相同，仅为满足
    /// std::exception 约束而实现。
    ///
    /// \~English
    /// @brief Return a static string describing the exception。Same as @ref
    /// What(), just for compatibility with std::exception.
    ///
    const char* what() const noexcept override { return what_.c_str(); }

    ///
    /// \~Chinese
    /// @brief 返回描述异常信息的字符串。
    ///
    /// \~English
    /// @brief Return a string ojbect describing the exception.
    ///
    const std::string& What() const noexcept { return what_; }

    ///
    /// \~Chinese
    /// @brief 内嵌错误码，可能承载来自外部库或系统组件的错误。
    /// @return 错误码对象，使用前需先检查错误码的值是否有效：
    /// @code if (e.NestedError()) { /* handle */ } @endcode
    ///
    /// \~English
    /// @brief Nested error_code object, may hold an error originating from the
    /// other libraries or the OS.
    /// @return Error code object, user should check if the error code is valid
    /// beforing using: @code if (e.NestedError()) { /* handle */ } @endcode
    ///
    const std::error_code& NestedError() const noexcept { return nestedError_; }

    ///
    /// \~Chinese
    /// @brief 内嵌异常，可能承载来自外部库或系统组件的错误。
    /// @return 异常指针，使用前需先检查指针是否有效：
    /// @code if (e.NestedException()) { /* handle */ } @endcode
    ///
    /// \~English
    /// @brief Nested exception pointer, may hold an error originating from the
    /// other libraries or the OS.
    /// @return Exception pointer, user should check if the exception is valid
    /// beforing using: @code if (e.NestedException()) { /* handle */ } @endcode
    ///
    const std::exception_ptr& NestedException() const noexcept
    {
        return nestedException_;
    }

protected:
    std::string        what_;
    std::error_code    nestedError_;
    std::exception_ptr nestedException_;
};

#define OPENGEMINI_DEFINE_EXCEPTION(TYPENAME)                            \
    class TYPENAME : public Exception {                                  \
    public:                                                              \
        explicit TYPENAME(std::string what) : Exception(what) { }        \
                                                                         \
        TYPENAME(std::string what, std::error_code nestedError) :        \
            Exception(what, nestedError)                                 \
        { }                                                              \
                                                                         \
        TYPENAME(std::string what, std::exception_ptr nestedException) : \
            Exception(what, nestedException)                             \
        { }                                                              \
    };

///
/// @class NetworkError
/// @inherit opengemini::Exception
///
/// \~Chinese
/// @brief 网络错误。
///
/// \~English
/// @brief Exception indicating a network error.
///
OPENGEMINI_DEFINE_EXCEPTION(NetworkError)

///
/// @class InvalidArgument
/// @inherit opengemini::Exception
///
/// \~Chinese
/// @brief 非法参数。
///
/// \~English
/// @brief Exception indicating an invalid argument.
///
OPENGEMINI_DEFINE_EXCEPTION(InvalidArgument)

///
/// @class RuntimeError
/// @inherit opengemini::Exception
///
/// \~Chinese
/// @brief 运行时异常，通常是来自标准库或依赖库的错误。
///
/// \~English
/// @brief Exception indicating a runtime error, usually from the standard
/// library or external libraries.
///
OPENGEMINI_DEFINE_EXCEPTION(RuntimeError)

///
/// @class NotImplemented
/// @inherit opengemini::Exception
///
/// \~Chinese
/// @brief 操作未实现。
///
/// \~English
/// @brief Exception indicating the operation is not implemented.
///
OPENGEMINI_DEFINE_EXCEPTION(NotImplemented)

///
/// @class ServerError
/// @inherit opengemini::Exception
///
/// \~Chinese
/// @brief 服务端错误。
///
/// \~English
/// @brief Exception indicating the error return from the server.
///
OPENGEMINI_DEFINE_EXCEPTION(ServerError)

///
/// @class JsonError
/// @inherit opengemini::Exception
///
/// \~Chinese
/// @brief JSON解析错误。
///
/// \~English
/// @brief Exception indicating a json parsing error.
///
OPENGEMINI_DEFINE_EXCEPTION(JsonError)

} // namespace opengemini

#endif // !OPENGEMINI_EXCEPTIONS_HPP
