#include "oneofdiscriminator.hpp"

#include <userver/chaotic/type_bundle_cpp.hpp>

#include "oneofdiscriminator_parsers.ipp"

namespace ns {

bool operator==(const ::ns::A& lhs, const ::ns::A& rhs) {
    return lhs.type == rhs.type && lhs.a_prop == rhs.a_prop && lhs.extra == rhs.extra &&

           true;
}

USERVER_NAMESPACE::logging::LogHelper& operator<<(USERVER_NAMESPACE::logging::LogHelper& lh, const ::ns::A& value) {
    return lh << ToString(USERVER_NAMESPACE::formats::json::ValueBuilder(value).ExtractValue());
}

A Parse(USERVER_NAMESPACE::formats::json::Value json, USERVER_NAMESPACE::formats::parse::To<::ns::A> to) {
    return Parse<USERVER_NAMESPACE::formats::json::Value>(json, to);
}

/* Parse(USERVER_NAMESPACE::formats::yaml::Value, To<::ns::A>) was not generated: ::ns::A has JSON-specific field
 * "extra" */

/* Parse(USERVER_NAMESPACE::yaml_config::Value, To<::ns::A>) was not generated: ::ns::A has JSON-specific field "extra"
 */

USERVER_NAMESPACE::formats::json::Value
Serialize([[maybe_unused]] const ::ns::A& value, USERVER_NAMESPACE::formats::serialize::To<USERVER_NAMESPACE::formats::json::Value>) {
    USERVER_NAMESPACE::formats::json::ValueBuilder vb = value.extra;

    if (value.type) {
        vb["type"] = USERVER_NAMESPACE::chaotic::Primitive<std::string>{*value.type};
    }

    if (value.a_prop) {
        vb["a_prop"] = USERVER_NAMESPACE::chaotic::Primitive<int>{*value.a_prop};
    }

    return vb.ExtractValue();
}

bool operator==(const ::ns::B& lhs, const ::ns::B& rhs) {
    return lhs.type == rhs.type && lhs.b_prop == rhs.b_prop && lhs.extra == rhs.extra &&

           true;
}

USERVER_NAMESPACE::logging::LogHelper& operator<<(USERVER_NAMESPACE::logging::LogHelper& lh, const ::ns::B& value) {
    return lh << ToString(USERVER_NAMESPACE::formats::json::ValueBuilder(value).ExtractValue());
}

B Parse(USERVER_NAMESPACE::formats::json::Value json, USERVER_NAMESPACE::formats::parse::To<::ns::B> to) {
    return Parse<USERVER_NAMESPACE::formats::json::Value>(json, to);
}

/* Parse(USERVER_NAMESPACE::formats::yaml::Value, To<::ns::B>) was not generated: ::ns::B has JSON-specific field
 * "extra" */

/* Parse(USERVER_NAMESPACE::yaml_config::Value, To<::ns::B>) was not generated: ::ns::B has JSON-specific field "extra"
 */

USERVER_NAMESPACE::formats::json::Value
Serialize([[maybe_unused]] const ::ns::B& value, USERVER_NAMESPACE::formats::serialize::To<USERVER_NAMESPACE::formats::json::Value>) {
    USERVER_NAMESPACE::formats::json::ValueBuilder vb = value.extra;

    if (value.type) {
        vb["type"] = USERVER_NAMESPACE::chaotic::Primitive<std::string>{*value.type};
    }

    if (value.b_prop) {
        vb["b_prop"] = USERVER_NAMESPACE::chaotic::Primitive<int>{*value.b_prop};
    }

    return vb.ExtractValue();
}

bool operator==(const ::ns::C& lhs, const ::ns::C& rhs) { return lhs.version == rhs.version && true; }

USERVER_NAMESPACE::logging::LogHelper& operator<<(USERVER_NAMESPACE::logging::LogHelper& lh, const ::ns::C& value) {
    return lh << ToString(USERVER_NAMESPACE::formats::json::ValueBuilder(value).ExtractValue());
}

C Parse(USERVER_NAMESPACE::formats::json::Value json, USERVER_NAMESPACE::formats::parse::To<::ns::C> to) {
    return Parse<USERVER_NAMESPACE::formats::json::Value>(json, to);
}

C Parse(USERVER_NAMESPACE::formats::yaml::Value json, USERVER_NAMESPACE::formats::parse::To<::ns::C> to) {
    return Parse<USERVER_NAMESPACE::formats::yaml::Value>(json, to);
}

C Parse(USERVER_NAMESPACE::yaml_config::Value json, USERVER_NAMESPACE::formats::parse::To<::ns::C> to) {
    return Parse<USERVER_NAMESPACE::yaml_config::Value>(json, to);
}

USERVER_NAMESPACE::formats::json::Value
Serialize([[maybe_unused]] const ::ns::C& value, USERVER_NAMESPACE::formats::serialize::To<USERVER_NAMESPACE::formats::json::Value>) {
    USERVER_NAMESPACE::formats::json::ValueBuilder vb = USERVER_NAMESPACE::formats::common::Type::kObject;

    if (value.version) {
        vb["version"] = USERVER_NAMESPACE::chaotic::Primitive<int>{*value.version};
    }

    return vb.ExtractValue();
}

bool operator==(const ::ns::D& lhs, const ::ns::D& rhs) { return lhs.version == rhs.version && true; }

USERVER_NAMESPACE::logging::LogHelper& operator<<(USERVER_NAMESPACE::logging::LogHelper& lh, const ::ns::D& value) {
    return lh << ToString(USERVER_NAMESPACE::formats::json::ValueBuilder(value).ExtractValue());
}

D Parse(USERVER_NAMESPACE::formats::json::Value json, USERVER_NAMESPACE::formats::parse::To<::ns::D> to) {
    return Parse<USERVER_NAMESPACE::formats::json::Value>(json, to);
}

D Parse(USERVER_NAMESPACE::formats::yaml::Value json, USERVER_NAMESPACE::formats::parse::To<::ns::D> to) {
    return Parse<USERVER_NAMESPACE::formats::yaml::Value>(json, to);
}

D Parse(USERVER_NAMESPACE::yaml_config::Value json, USERVER_NAMESPACE::formats::parse::To<::ns::D> to) {
    return Parse<USERVER_NAMESPACE::yaml_config::Value>(json, to);
}

USERVER_NAMESPACE::formats::json::Value
Serialize([[maybe_unused]] const ::ns::D& value, USERVER_NAMESPACE::formats::serialize::To<USERVER_NAMESPACE::formats::json::Value>) {
    USERVER_NAMESPACE::formats::json::ValueBuilder vb = USERVER_NAMESPACE::formats::common::Type::kObject;

    if (value.version) {
        vb["version"] = USERVER_NAMESPACE::chaotic::Primitive<int>{*value.version};
    }

    return vb.ExtractValue();
}

bool operator==(const ::ns::IntegerOneOfDiscriminator& lhs, const ::ns::IntegerOneOfDiscriminator& rhs) {
    return lhs.foo == rhs.foo && true;
}

USERVER_NAMESPACE::logging::LogHelper&
operator<<(USERVER_NAMESPACE::logging::LogHelper& lh, const ::ns::IntegerOneOfDiscriminator& value) {
    return lh << ToString(USERVER_NAMESPACE::formats::json::ValueBuilder(value).ExtractValue());
}

IntegerOneOfDiscriminator Parse(
    USERVER_NAMESPACE::formats::json::Value json,
    USERVER_NAMESPACE::formats::parse::To<::ns::IntegerOneOfDiscriminator> to
) {
    return Parse<USERVER_NAMESPACE::formats::json::Value>(json, to);
}

/* Parse(USERVER_NAMESPACE::formats::yaml::Value, To<::ns::IntegerOneOfDiscriminator>) was not generated:
 * ::ns::IntegerOneOfDiscriminator::Foo has JSON-specific field "extra" */

/* Parse(USERVER_NAMESPACE::yaml_config::Value, To<::ns::IntegerOneOfDiscriminator>) was not generated:
 * ::ns::IntegerOneOfDiscriminator::Foo has JSON-specific field "extra" */

USERVER_NAMESPACE::formats::json::Value
Serialize([[maybe_unused]] const ::ns::IntegerOneOfDiscriminator& value, USERVER_NAMESPACE::formats::serialize::To<USERVER_NAMESPACE::formats::json::Value>) {
    USERVER_NAMESPACE::formats::json::ValueBuilder vb = USERVER_NAMESPACE::formats::common::Type::kObject;

    if (value.foo) {
        vb["foo"] = USERVER_NAMESPACE::chaotic::OneOfWithDiscriminator<
            &::ns::IntegerOneOfDiscriminator::kFoo_Settings,
            USERVER_NAMESPACE::chaotic::Primitive<::ns::C>,
            USERVER_NAMESPACE::chaotic::Primitive<::ns::D>>{*value.foo};
    }

    return vb.ExtractValue();
}

bool operator==(const ::ns::OneOfDiscriminator& lhs, const ::ns::OneOfDiscriminator& rhs) {
    return lhs.foo == rhs.foo && true;
}

USERVER_NAMESPACE::logging::LogHelper&
operator<<(USERVER_NAMESPACE::logging::LogHelper& lh, const ::ns::OneOfDiscriminator& value) {
    return lh << ToString(USERVER_NAMESPACE::formats::json::ValueBuilder(value).ExtractValue());
}

OneOfDiscriminator Parse(
    USERVER_NAMESPACE::formats::json::Value json,
    USERVER_NAMESPACE::formats::parse::To<::ns::OneOfDiscriminator> to
) {
    return Parse<USERVER_NAMESPACE::formats::json::Value>(json, to);
}

/* Parse(USERVER_NAMESPACE::formats::yaml::Value, To<::ns::OneOfDiscriminator>) was not generated:
 * ::ns::OneOfDiscriminator::Foo has JSON-specific field "extra" */

/* Parse(USERVER_NAMESPACE::yaml_config::Value, To<::ns::OneOfDiscriminator>) was not generated:
 * ::ns::OneOfDiscriminator::Foo has JSON-specific field "extra" */

USERVER_NAMESPACE::formats::json::Value
Serialize([[maybe_unused]] const ::ns::OneOfDiscriminator& value, USERVER_NAMESPACE::formats::serialize::To<USERVER_NAMESPACE::formats::json::Value>) {
    USERVER_NAMESPACE::formats::json::ValueBuilder vb = USERVER_NAMESPACE::formats::common::Type::kObject;

    if (value.foo) {
        vb["foo"] = USERVER_NAMESPACE::chaotic::OneOfWithDiscriminator<
            &::ns::OneOfDiscriminator::kFoo_Settings,
            USERVER_NAMESPACE::chaotic::Primitive<::ns::A>,
            USERVER_NAMESPACE::chaotic::Primitive<::ns::B>>{*value.foo};
    }

    return vb.ExtractValue();
}

}  // namespace ns
