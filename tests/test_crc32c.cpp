#include <catch2/catch_test_macros.hpp>
#include <crc32c/crc32c.hpp>

TEST_CASE("Hello, world!", "[main]") {
  auto crc32c = CRC32C();
  auto result = crc32c.calc("Hello, world!", 13, 0);

  REQUIRE(result == 0xC8A106E5);
}

TEST_CASE("test", "[main]") {
  auto crc32c = CRC32C();
  auto result = crc32c.calc("test", 4, 0);

  REQUIRE(result ==  0x86A072C0);
}
