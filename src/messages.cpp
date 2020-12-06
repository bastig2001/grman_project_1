#include "messages.h"

#ifdef UNIT_TEST
#include "catch2/catch.hpp"

TEST_CASE("Message Objects are copied", "[messages]") {
    auto msg1 = LogMessage("Message");

    REQUIRE(msg1.content == "Message");

    auto msg2 = msg1;
    msg1.content = "New Message";

    CHECK(msg2.content == "Message");
    CHECK(msg1.content == "New Message");
}
#endif
