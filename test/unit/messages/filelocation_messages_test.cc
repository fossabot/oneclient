/**
 * @file filelocation_messages_test.cc
 * @author Bartek Kryza
 * @copyright (C) 2018 ACK CYFRONET AGH
 * @copyright This software is released under the MIT license cited in
 * 'LICENSE.txt'
 */

#include "messages/fuse/fileBlock.h"
#include "messages/fuse/fileLocation.h"

#include <gtest/gtest.h>

using namespace one::messages::fuse;

/**
 * The purpose of this test suite is to test the file location message
 * and in particular the file blocks map rendering.
 */
struct FuseFileLocationMessagesTest : public ::testing::Test {
};

TEST_F(FuseFileLocationMessagesTest, emptyFileLocationShouldRenderEmptyProgress)
{
    auto fileLocation = FileLocation{};

    std::string expected(10, ' ');

    EXPECT_EQ(fileLocation.progressString(1024, 10), expected);
    EXPECT_EQ(fileLocation.progressString(10, 10), expected);
    EXPECT_EQ(fileLocation.progressString(5, 10), expected);
    EXPECT_EQ(fileLocation.progressString(1, 10), expected);
    EXPECT_EQ(fileLocation.progressString(0, 10), expected);
}

TEST_F(FuseFileLocationMessagesTest,
    completeFileLocationShouldRenderCompleteProgress)
{
    std::string expected(10, '#');

    auto fileLocationLarge = FileLocation{};
    fileLocationLarge.putBlock(0, 1024, FileBlock{"", ""});
    EXPECT_EQ(fileLocationLarge.progressString(1024, 10), expected);

    auto fileLocationEqual = FileLocation{};
    fileLocationEqual.putBlock(0, 10, FileBlock{"", ""});
    EXPECT_EQ(fileLocationEqual.progressString(10, 10), expected);

    auto fileLocationSmall = FileLocation{};
    fileLocationSmall.putBlock(0, 2, FileBlock{"", ""});
    EXPECT_EQ(fileLocationSmall.progressString(2, 10), expected);
}

TEST_F(FuseFileLocationMessagesTest,
    partialFileLocationShouldRenderPartialProgress)
{
    auto fileLocation = FileLocation{};
    fileLocation.putBlock(0, 1, FileBlock{"", ""});
    EXPECT_EQ(fileLocation.progressString(100, 10), ".         ");

    fileLocation = FileLocation{};
    fileLocation.putBlock(0, 511 - 1, FileBlock{"", ""});
    EXPECT_EQ(fileLocation.progressString(1024, 10), "#####     ");

    fileLocation = FileLocation{};
    fileLocation.putBlock(0, 512, FileBlock{"", ""});
    EXPECT_EQ(fileLocation.progressString(1024, 10), "#####.    ");

    fileLocation = FileLocation{};
    fileLocation.putBlock(0, 600, FileBlock{"", ""});
    EXPECT_EQ(fileLocation.progressString(1024, 10), "#####o    ");

    fileLocation = FileLocation{};
    fileLocation.putBlock(511 - 1, 1024, FileBlock{"", ""});
    EXPECT_EQ(fileLocation.progressString(1024, 10), "     #####");

    fileLocation = FileLocation{};
    fileLocation.putBlock(950, 1024, FileBlock{"", ""});
    EXPECT_EQ(fileLocation.progressString(1024, 10), "         o");
}

TEST_F(FuseFileLocationMessagesTest,
    partialFileLocationShouldRenderPartialProgressForSmallFiles)
{
    auto fileLocation = FileLocation{};
    fileLocation.putBlock(0, 1, FileBlock{"", ""});
    EXPECT_EQ(fileLocation.progressString(15, 10), "..........");

    fileLocation = FileLocation{};
    fileLocation.putBlock(0, 10, FileBlock{"", ""});
    EXPECT_EQ(fileLocation.progressString(15, 10), "oooooooooo");

    fileLocation = FileLocation{};
    fileLocation.putBlock(0, 15, FileBlock{"", ""});
    EXPECT_EQ(fileLocation.progressString(15, 10), "##########");
}

TEST_F(
    FuseFileLocationMessagesTest, replicationProgressShouldReportProperValues)
{
    auto fileLocation = FileLocation{};
    fileLocation.putBlock(0, 1, FileBlock{"", ""});
    EXPECT_EQ(fileLocation.replicationProgress(100), 0.01);

    fileLocation = FileLocation{};
    fileLocation.putBlock(0, 512, FileBlock{"", ""});
    EXPECT_EQ(fileLocation.replicationProgress(1024), 0.5);

    fileLocation = FileLocation{};
    fileLocation.putBlock(0, 100, FileBlock{"", ""});
    EXPECT_EQ(fileLocation.replicationProgress(0), 0);

    fileLocation = FileLocation{};
    EXPECT_EQ(fileLocation.replicationProgress(1024), 0);

    fileLocation = FileLocation{};
    fileLocation.putBlock(0, 100, FileBlock{"", ""});
    EXPECT_EQ(fileLocation.replicationProgress(100), 1.0);

    fileLocation = FileLocation{};
    fileLocation.putBlock(0, 100, FileBlock{"", ""});
    EXPECT_EQ(fileLocation.replicationProgress(50), 1.0);
}
