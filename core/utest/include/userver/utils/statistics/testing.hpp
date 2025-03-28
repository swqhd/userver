#pragma once

/// @file userver/utils/statistics/testing.hpp
/// @brief Utilities for analyzing emitted metrics in unit tests

#include <iosfwd>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include <userver/utils/not_null.hpp>
#include <userver/utils/statistics/labels.hpp>
#include <userver/utils/statistics/metric_value.hpp>
#include <userver/utils/statistics/storage.hpp>

USERVER_NAMESPACE_BEGIN

namespace utils::statistics {

namespace impl {
struct SnapshotData;
}  // namespace impl

/// @brief Thrown by statistics::Snapshot queries on unexpected metrics states.
class MetricQueryError final : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

/// @brief A snapshot of metrics from utils::statistics::Storage.
class Snapshot final {
public:
    /// @brief Create a new snapshot of metrics with paths starting with @a prefix
    /// and labels containing @a require_labels.
    /// @throws std::exception if a metric writer throws.
    explicit Snapshot(const Storage& storage, std::string prefix = {}, std::vector<Label> require_labels = {});

    Snapshot(const Snapshot& other) = default;
    Snapshot(Snapshot&& other) noexcept = default;

    /// @brief Find a single metric by the given filter.
    /// @param path The path of the target metric. `prefix` specified in the
    /// constructor is prepended to the path.
    /// @param require_labels Labels that the target metric should have.
    /// @returns The value of the single found metric.
    /// @throws MetricQueryError if none or multiple metrics are found.
    MetricValue SingleMetric(std::string path, std::vector<Label> require_labels = {}) const;

    /// @overload
    std::optional<MetricValue> SingleMetricOptional(std::string path, std::vector<Label> require_labels = {}) const;

private:
    friend void PrintTo(const Snapshot& data, std::ostream*);

    Request request_;
    utils::SharedRef<const impl::SnapshotData> data_;
};

/// @brief Support for gtest diagnostics for utils::statistics::Snapshot.
///
/// @warning Never check the printed value programmatically! The string is not
/// stable and may change, depending on the version of stdlib and userver.
///
/// **Valid usage:**
/// @code
/// EXPECT_EQ(..., ...) << testing::PrintToString(snapshot);
/// @endcode
///
/// **Invalid usage:**
/// @code
/// std::ostringstream stream;
/// PrintTo(snapshot, &stream);
/// EXPECT_EQ(stream.str(), ...);
/// @endcode
void PrintTo(const Snapshot& data, std::ostream*);

/// @brief Support for gtest diagnostics for utils::statistics::MetricValue.
void PrintTo(MetricValue value, std::ostream*);

}  // namespace utils::statistics

USERVER_NAMESPACE_END
