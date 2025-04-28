#include <sstream>
#include <vector>
#include <cstdint>
#include <algorithm>

#include <doctest.h>
#include <celengine/starname.h>
#include <celutil/binaryread.h>
#include <celengine/astroobj.h>

using namespace celestia;
using util::fromMemoryLE;

static std::string make_header(uint16_t version = 0x0100)
{
    // CrossIndexHeader { char magic[8]; uint16_t version; }
    const char magic[] = "CELINDEX";    // 8 bytes, no trailing NUL
    std::string buf(magic, magic + 8);
    // little‐endian version:
    buf.push_back(static_cast<char>(version & 0xFF));
    buf.push_back(static_cast<char>(version >> 8));
    return buf;
}

static std::string make_record(uint32_t catalogNum,
                               uint32_t celCatalogNum)
{
    // CrossIndexRecord { uint32_t catalogNumber, celCatalogNumber; }
    std::string buf;
    for (int i = 0; i < 4; ++i)
        buf.push_back(static_cast<char>((catalogNum >> (8*i)) & 0xFF));
    for (int i = 0; i < 4; ++i)
        buf.push_back(static_cast<char>((celCatalogNum >> (8*i)) & 0xFF));
    return buf;
}

TEST_SUITE("StarNameDatabase::loadCrossIndex")
{
    TEST_CASE("Loads a simple valid index and sorts entries")
    {
        std::ostringstream out;
        // 1) write valid header
        out << make_header();
        // 2) write three unsorted records
        out << make_record(100,  5000)
            << make_record( 42,  4242)
            << make_record(1234,  1111);

        std::istringstream in(out.str());
        StarNameDatabase db;
        // should succeed
        CHECK(db.loadCrossIndex(StarCatalog::HenryDraper, in));

        // now search: entries must be sorted by catalogNumber:
        //   42→4242, 100→5000, 1234→1111
        CHECK(db.searchCrossIndexForCatalogNumber(StarCatalog::HenryDraper,   42) == 4242);
        CHECK(db.searchCrossIndexForCatalogNumber(StarCatalog::HenryDraper,  100) == 5000);
        CHECK(db.searchCrossIndexForCatalogNumber(StarCatalog::HenryDraper, 1234) == 1111);

        // out‐of‐range lookups
        CHECK(db.searchCrossIndexForCatalogNumber(StarCatalog::HenryDraper, 9999)
              == AstroCatalog::InvalidIndex);
    }

    TEST_CASE("Rejects bad magic string")
    {
        std::ostringstream out;
        // wrong magic
        out << "BADSIG!!"         // not "CELINDEX"
            << "\x00\x01"         // version 0x0100
            ;
        std::istringstream in(out.str());
        StarNameDatabase db;
        CHECK_FALSE(db.loadCrossIndex(StarCatalog::HenryDraper, in));
    }

    TEST_CASE("Rejects unsupported version")
    {
        std::ostringstream out;
        // correct magic but wrong version
        out << make_header(0x0200);
        std::istringstream in(out.str());
        StarNameDatabase db;
        CHECK_FALSE(db.loadCrossIndex(StarCatalog::HenryDraper, in));
    }

    TEST_CASE("Rejects truncated record at EOF")
    {
        std::ostringstream out;
        // valid header
        out << make_header();
        // write one full record + half of a second
        out << make_record(1, 2);
        std::string half = make_record(3,4).substr(0, 3); // only 3/8 bytes
        out << half;

        std::istringstream in(out.str());
        StarNameDatabase db;
        CHECK_FALSE(db.loadCrossIndex(StarCatalog::HenryDraper, in));
    }

    TEST_CASE("Empty index (no records) yields true but empty crossIndices")
    {
        std::ostringstream out;
        out << make_header();    // header only, no records
        std::istringstream in(out.str());

        StarNameDatabase db;
        CHECK(db.loadCrossIndex(StarCatalog::HenryDraper, in));
        // lookup should always fail
        CHECK(db.searchCrossIndexForCatalogNumber(StarCatalog::HenryDraper, 1)
              == AstroCatalog::InvalidIndex);
    }
}
