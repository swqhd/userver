#pragma once

#include <storages/postgres/io/nullable_traits.hpp>
#include <storages/postgres/io/traits.hpp>
#include <storages/postgres/io/type_mapping.hpp>

#include <storages/postgres/io/floating_point_types.hpp>
#include <storages/postgres/io/integral_types.hpp>
#include <storages/postgres/io/optional.hpp>
#include <storages/postgres/io/string_types.hpp>

#include <storages/postgres/io/stream_text_formatter.hpp>

#include <vector>

namespace storages {
namespace postgres {
namespace detail {

/// @brief Helper to write query parameters to buffers
class QueryParameters {
 public:
  bool Empty() const { return param_types.empty(); }
  std::size_t Size() const { return param_types.size(); }
  const char* const* ParamBuffers() const { return param_buffers.data(); }
  const Oid* ParamTypesBuffer() const { return param_types.data(); }
  const int* ParamLengthsBuffer() const { return param_lengths.data(); }
  const int* ParamFormatsBuffer() const { return param_formats.data(); }

  template <typename T>
  void Write(const T& arg) {
    static_assert(io::traits::HasPgMapping<T>::value,
                  "Type doesn't have mapping to Postgres type");
    WriteParamType(arg);
    WriteNullable(arg, io::traits::IsNullable<T>{});
  }

  template <typename... T>
  void Write(const T&... args) {
    (Write(args), ...);
  }

 private:
  template <typename T>
  void WriteParamType(const T&) {
    // C++ to pg oid mapping
    param_types.push_back(io::CppToPg<T>::GetOid());
  }

  template <typename T>
  void WriteNullable(const T& arg, std::true_type) {
    using NullDetector = io::traits::GetSetNull<T>;
    if (NullDetector::IsNull(arg)) {
      param_formats.push_back(0);
      param_lengths.push_back(0);
      param_buffers.push_back(nullptr);
    } else {
      WriteNullable(arg, std::false_type{});
    }
  }

  template <typename T>
  void WriteNullable(const T& arg, std::false_type) {
    using FormatterTraits = io::traits::BestFormatter<T>;

    param_formats.push_back(FormatterTraits::value);
    parameters.push_back({});
    auto& buffer = parameters.back();
    io::WriteBuffer<FormatterTraits::value>(buffer, arg);
    param_lengths.push_back(buffer.size());
    param_buffers.push_back(buffer.data());
  }

 private:
  using OidList = std::vector<Oid>;
  using BufferType = std::vector<char>;
  using ParameterList = std::vector<BufferType>;
  using IntList = std::vector<int>;

  ParameterList parameters;
  OidList param_types;
  std::vector<const char*> param_buffers;
  IntList param_lengths;
  IntList param_formats;
};

}  // namespace detail
}  // namespace postgres
}  // namespace storages
