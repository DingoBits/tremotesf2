/*
 * Tremotesf
 * Copyright (C) 2015-2021 Alexey Rochev <equeim@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "bencodeparser.h"

#include <charconv>
#include <cinttypes>
#include <limits>

#include <QDebug>
#include <QFile>
#include <QString>

namespace tremotesf::bencode
{
    namespace {
        constexpr char integerPrefix = 'i';
        constexpr char listPrefix = 'l';
        constexpr char dictionaryPrefix = 'd';
        constexpr char terminator = 'e';
        constexpr char byteArraySeparator = ':';

        // digits10 + 1 is maximum number of character needed to hold integer, +1 for minus sign, +1 for terminator character
        constexpr int integerBufferSize = std::numeric_limits<Value::Integer>::digits10 + 3;
    }

    std::optional<Value::Integer> Value::takeInteger()
    {
        return takeValue<Integer>();
    }

    std::optional<Value::ByteArray> Value::takeByteArray()
    {
        return takeValue<ByteArray>();
    }

    std::optional<QString> Value::takeString()
    {
        return std::visit([](auto&& value) -> std::optional<QString> {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, ByteArray>) {
                auto string = QString::fromStdString(value);
                value.clear();
                return string;
            }
            return std::nullopt;
        }, mValue);
    }

    std::optional<Value::List> Value::takeList()
    {
        return takeValue<List>();
    }

    std::optional<Value::Dictionary> Value::takeDictionary()
    {
        return takeValue<Dictionary>();
    }

    template<typename Expected>
    std::optional<Expected> Value::takeValue()
    {
        return std::visit([](auto&& value) -> std::optional<Expected> {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, Expected>) {
                return std::move(value);
            }
            return std::nullopt;
        }, mValue);
    }

    namespace
    {
        class Parser
        {
        public:
            explicit Parser(QIODevice& device) : mDevice{device} {};

            Result parse()
            {
                auto parseResult = parseValue();
                if (mError == NoError) {
                    return {std::move(parseResult), NoError};
                }
                return {{}, mError};
            }

        private:
            Value parseValue()
            {
                const char byte = peekByte();
                if (byte == integerPrefix) {
                    return parseInteger();
                }
                if (byte == listPrefix) {
                    return parseList();
                }
                if (byte == dictionaryPrefix) {
                    return parseDictionary();
                }
                return parseByteArray();
            }

            Value::Dictionary parseDictionary()
            {
                return parseContainer<Value::Dictionary>([&](auto& dict) {
                    Value::ByteArray key(parseByteArray());
                    if (mError != NoError) {
                        qWarning("parseDictionary: failed to read dictionary key");
                        return;
                    }
                    Value value(parseValue());
                    if (mError != NoError) {
                        qWarning("parseDictionary: failed to read dictionary value");
                        return;
                    }
                    dict.emplace(std::move(key), std::move(value));
                }, "parseDictionary", "dictionary");
            }

            Value::List parseList()
            {
                return parseContainer<Value::List>([&](auto& list) {
                    Value value = parseValue();
                    if (mError == NoError) {
                        list.push_back(std::move(value));
                    } else {
                        qWarning("parseList: failed to read list element");
                    }
                }, "parseList", "list");
            }

            template<typename Container, typename Append>
            Container parseContainer(Append&& append, const char* funcName, const char* containerName)
            {
                if (!skipByte()) {
                    qWarning("parseList: failed to skip prefix");
                    return {};
                }
                Container container{};
                while (mError == NoError) {
                    if (peekByte() == terminator) {
                        if (!skipByte()) {
                            qWarning("%s: failed to skip terminator", funcName);
                            return {};
                        }
                        return container;
                    }
                    if (mError != NoError) {
                        qWarning("%s: failed to peek next byte", funcName);
                        return {};
                    }
                    append(container);
                }
                qWarning("%s: failed to read %s", funcName, containerName);
                return {};
            }

            Value::ByteArray parseByteArray()
            {
                const auto size = readIntegerUntilTerminator(byteArraySeparator);
                if (mError != NoError) {
                    qWarning("parseByteArray: failed to read byte array size");
                    return {};
                }
                if (size < 0) {
                    qWarning("parseByteArray: incorrect byte array size %" PRId64, size);
                    mError = ParsingError;
                    return {};
                }
                Value::ByteArray byteArray(static_cast<size_t>(size), 0);
                auto read = mDevice.read(byteArray.data(), size);
                if (read != size) {
                    setErrorFromIODevice(QString::fromLatin1("parseByteArray: failed to read byte array with size %1").arg(size));
                    return {};
                }
                return byteArray;
            }

            Value::Integer parseInteger()
            {
                if (!skipByte()) {
                    qWarning("parseInteger: failed to skip prefix");
                    return {};
                }
                const auto integer = readIntegerUntilTerminator(terminator);
                if (mError != NoError) {
                    qWarning("parseInteger: failed to read integer");
                }
                return integer;
            }

            Value::Integer readIntegerUntilTerminator(char terminator) {
                const auto peeked = mDevice.peek(mIntegerBuffer.data(), integerBufferSize);
                if (peeked <= 0) {
                    setErrorFromIODevice("readIntegerUntilTerminator: failed to peek integer buffer");
                    return {};
                }
                Value::Integer integer{};
                const auto result = std::from_chars(mIntegerBuffer.begin(), mIntegerBuffer.end(), integer);
                if (result.ec != std::errc{}) {
                    qWarning("readIntegerUntilTerminator: error parsing integer, std::from_chars() error %s", std::make_error_condition(result.ec).message().data());
                    mError = ParsingError;
                    return {};
                }
                if (*result.ptr != terminator) {
                    qWarning("readIntegerUntilTerminator: terminator doesn't match, expected 0x%hhx but got 0x%hhx", terminator, *result.ptr);
                    mError = ParsingError;
                    return {};
                }
                const auto toSkip = result.ptr - mIntegerBuffer.begin() + 1;
                if (mDevice.skip(toSkip) != toSkip) {
                    setErrorFromIODevice("readIntegerUntilTerminator: failed to skip read integer");
                    return {};
                }
                return integer;
            }

            char peekByte()
            {
                char byte{};
                if (mDevice.peek(&byte, 1) != 1) {
                    setErrorFromIODevice("peekByte: peek() failure");
                }
                return byte;
            }

            bool skipByte()
            {
                if (mDevice.skip(1) != 1) {
                    setErrorFromIODevice("skipByte: skip() failure");
                    return false;
                }
                return true;
            }

            void setErrorFromIODevice(const QString& message)
            {
                auto debug = qWarning().nospace();
                {
                    QDebugStateSaver saver(debug);
                    debug.noquote() << message;
                }
                setErrorFromIODevice(debug);
            }

            void setErrorFromIODevice(const char* message)
            {
                setErrorFromIODevice(qWarning().nospace() << message);
            }

            void setErrorFromIODevice(QDebug& debug)
            {
                if (auto fileDevice = dynamic_cast<QFileDevice*>(&mDevice); fileDevice) {
                    debug << ", error = " << fileDevice->error();
                }
                debug << ", error string = " << mDevice.errorString() << ", at end = " << mDevice.atEnd();
                if (mDevice.atEnd()) {
                    mError = ParsingError;
                } else {
                    mError = ReadingError;
                }
            }

            QIODevice& mDevice;
            Error mError{};
            std::array<char, integerBufferSize> mIntegerBuffer{};
        };
    }

    Result parse(const QString& filePath)
    {
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly)) {
            return Parser(file).parse();
        }
        qWarning().nospace() << "Failed to open file, error = " << file.error() << ", error string = " << file.errorString();
        return Result{{}, ReadingError};
    }

    Result parse(QIODevice& device)
    {
        return Parser(device).parse();
    }
}
