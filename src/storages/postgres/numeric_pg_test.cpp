#include <gtest/gtest.h>

#include <storages/postgres/io/boost_multiprecision.hpp>

#include <boost/math/special_functions.hpp>

namespace pg = storages::postgres;
namespace io = pg::io;

namespace static_test {

static_assert((pg::io::traits::HasFormatter<
                   pg::Numeric, pg::io::DataFormat::kTextDataFormat>::value ==
               true),
              "");
static_assert((pg::io::traits::HasParser<
                   pg::Numeric, pg::io::DataFormat::kTextDataFormat>::value ==
               true),
              "");

}  // namespace static_test

namespace storages {
namespace postgres {
namespace test {

// TODO Move to a common io_test header
using Buffer = std::vector<char>;

io::FieldBuffer MakeFieldBuffer(const Buffer& buffer, io::DataFormat format) {
  return {false, format, buffer.size(), buffer.data()};
}

}  // namespace test
}  // namespace postgres
}  // namespace storages

TEST(PostgreIO, Numeric) {
  {
    pg::Numeric src{"3.14"};
    pg::test::Buffer buffer;
    EXPECT_NO_THROW(
        io::WriteBuffer<io::DataFormat::kTextDataFormat>(buffer, src));
    auto fb =
        pg::test::MakeFieldBuffer(buffer, io::DataFormat::kTextDataFormat);
    pg::Numeric tgt{0};
    EXPECT_NO_THROW(io::ReadBuffer<io::DataFormat::kTextDataFormat>(fb, tgt));
    EXPECT_EQ(src, tgt);
  }
  {
    pg::Numeric src = boost::math::sin_pi(pg::Numeric{1});
    pg::test::Buffer buffer;
    EXPECT_NO_THROW(
        io::WriteBuffer<io::DataFormat::kTextDataFormat>(buffer, src));
    auto fb =
        pg::test::MakeFieldBuffer(buffer, io::DataFormat::kTextDataFormat);
    pg::Numeric tgt{0};
    EXPECT_NO_THROW(io::ReadBuffer<io::DataFormat::kTextDataFormat>(fb, tgt));
    EXPECT_EQ(0, src.compare(tgt))
        << "Number writter to the buffer "
        << std::setprecision(std::numeric_limits<pg::Numeric>::digits10) << src
        << " is expected to be equal to number read from buffer " << tgt;
  }
}
