/**
 * @file DADIFileChannelTests.cc
 * @brief This file implements the libdadi tests for file channel logger
 * @author Benjamin DEPARDON (benjamin.depardon@sysfera.com)
 * @section License
 *  |LICENSE|
 *
 */

#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>
#include "dadi/Logging/FileChannel.hh"
#include "dadi/Logging/Logger.hh"
#include "dadi/Logging/Message.hh"
#include "dadi/Config.hh"
#include "dadi/Options.hh"

namespace bfs = boost::filesystem;  // an alias for boost filesystem namespace

static const std::string SRCSTR = "Bridgekeeper";
static const std::string MSGSTR =
  "What... is the air-speed velocity of an unladen swallow?";


namespace {
typedef boost::scoped_ptr<dadi::FileChannel> FChannelPtr;
}

BOOST_AUTO_TEST_SUITE(FileChannelTests)

BOOST_AUTO_TEST_CASE(default_constructor_test) {
  BOOST_TEST_MESSAGE("#Default constructor test#");
  FChannelPtr myFileC(new dadi::FileChannel);
  // Nothing to test?
}


BOOST_AUTO_TEST_CASE(constructor_file_does_not_exist_test) {
  BOOST_TEST_MESSAGE("#Constructor test where file does not exist#");

  // Create working file
  bfs::path tmpFile = bfs::temp_directory_path().native();
  tmpFile /= "%%%%-%%%%-%%%%-%%%%";
  tmpFile = bfs::unique_path(tmpFile).native();
  BOOST_TEST_MESSAGE("tmp file = " + tmpFile.native());

  FChannelPtr myFileC(new dadi::FileChannel(tmpFile.native()));

  // Check correct path
  BOOST_REQUIRE_EQUAL(myFileC->getPath(), tmpFile.native());
  // Check that size is 0
  BOOST_REQUIRE_EQUAL(myFileC->getSize(), 0);
  // Check that last write time is -1 (file does not exist)
  BOOST_REQUIRE_EQUAL(myFileC->getLastWriteTime(), -1);
  // Delete working file
  bfs::remove_all(tmpFile);
}

BOOST_AUTO_TEST_CASE(constructor_file_exits_test) {
  BOOST_TEST_MESSAGE("#Constructor file exists test#");

  std::string source(SRCSTR);
  std::string msgToLog(MSGSTR);
  dadi::Message myMsg =
    dadi::Message(source, msgToLog, dadi::Message::PRIO_DEBUG);

  // Create working file
  bfs::path tmpFile = bfs::temp_directory_path();
  tmpFile /= "%%%%-%%%%-%%%%-%%%%";
  tmpFile = bfs::unique_path(tmpFile);
  BOOST_TEST_MESSAGE("tmp file = " + tmpFile.native());

  // Create file
  bfs::fstream ofs(tmpFile, std::ios_base::out);
  ofs << msgToLog;
  ofs.close();

  FChannelPtr myFileC(new dadi::FileChannel(tmpFile.native()));

  // To check that the file with message logged is created
  BOOST_REQUIRE(bfs::exists(tmpFile));
  // Check correct path
  BOOST_REQUIRE_EQUAL(myFileC->getPath(), tmpFile);
  // Check that size is different from 0
  BOOST_REQUIRE(myFileC->getSize() != 0);
  // Check that last write time is different from -1 (file exists)
  BOOST_REQUIRE(myFileC->getLastWriteTime() != -1);
  // Delete working file
  bfs::remove_all(tmpFile);
}


BOOST_AUTO_TEST_CASE(path_to_directory_test) {
  BOOST_TEST_MESSAGE("#Path to directory test#");

  // Create working directory
  bfs::path tmpDir = bfs::temp_directory_path();
  tmpDir /= "%%%%-%%%%-%%%%-%%%%";
  tmpDir = bfs::unique_path(tmpDir);
  bfs::create_directory(tmpDir);

  BOOST_TEST_MESSAGE("tmp dir = " + tmpDir.native());

  FChannelPtr myFileC(new dadi::FileChannel(tmpDir.native()));

  // FIXME: when dadi::FileChannel::open will throw an exception
  // re-enable this test (with the proper exception type
  // Check that open throws an exception
  // BOOST_REQUIRE_THROW( myFileC->open(), std::exception)
  // Delete working file
  bfs::remove_all(tmpDir);
}


BOOST_AUTO_TEST_CASE(compression_methods_test) {
  BOOST_TEST_MESSAGE("#Compression methods test#");

  std::string source(SRCSTR);
  std::string msgToLog(MSGSTR);
  dadi::Message myMsg =
    dadi::Message(source, msgToLog, dadi::Message::PRIO_DEBUG);

  // Create working directory
  bfs::path tmpDir = bfs::temp_directory_path();
  tmpDir /= "%%%%-%%%%-%%%%-%%%%";
  tmpDir = bfs::unique_path(tmpDir);
  bfs::create_directory(tmpDir);
  BOOST_TEST_MESSAGE("tmp directory = " + tmpDir.native());

  // Create file name
  bfs::path tmpFile = tmpDir;
  tmpFile /= "tmpFile.log";
  BOOST_TEST_MESSAGE("tmp file = " + tmpFile.native());

  /////////////////////////////////////
  // Create Channel with no compression
  BOOST_TEST_MESSAGE("# Testing compression methods: no compression #");
  {
    FChannelPtr myFileC(new dadi::FileChannel(tmpFile.native()));

    // Check correct path
    BOOST_REQUIRE_EQUAL(myFileC->getPath(), tmpFile);

    // Log a message
    BOOST_REQUIRE_NO_THROW(myFileC->log(myMsg));

    // To check that the file with message logged is created
    BOOST_REQUIRE(bfs::exists(tmpFile));
    // Close Channel
    BOOST_REQUIRE_NO_THROW(myFileC->close());
  }

  // load this file
  {
    boost::iostreams::filtering_streambuf<boost::iostreams::input> tmp;
    tmp.push(boost::iostreams::file_source(tmpFile.native()));
    std::stringstream strstream;
    BOOST_REQUIRE_NO_THROW(boost::iostreams::copy(tmp, strstream));
    BOOST_REQUIRE(std::string(myMsg.getText() + "\n") == strstream.str());
  }
  bfs::remove_all(tmpFile);
  // end no compression test
  //////////////////////////

  ///////////////////////////
  // Create Channel with gzip
  BOOST_TEST_MESSAGE("# Testing compression methods gzip #");
  {
    FChannelPtr myFileC(new dadi::FileChannel(tmpFile.native()));

    // Check correct path
    BOOST_REQUIRE_EQUAL(myFileC->getPath(), tmpFile);

    myFileC->putAttr("compression_mode", "gzip");
    // Log a message
    BOOST_REQUIRE_NO_THROW(myFileC->log(myMsg));

    // To check that the file with message logged is created
    BOOST_REQUIRE(bfs::exists(tmpFile));
    // Close Channel
    BOOST_REQUIRE_NO_THROW(myFileC->close());
  }

  // load this file
  {
    boost::iostreams::filtering_streambuf<boost::iostreams::input> tmp;
    tmp.push(boost::iostreams::gzip_decompressor());
    tmp.push(boost::iostreams::file_source(tmpFile.native()));
    std::stringstream strstream;
    BOOST_REQUIRE_NO_THROW(boost::iostreams::copy(tmp, strstream));
    BOOST_REQUIRE(std::string(myMsg.getText() + "\n") == strstream.str());
  }
  bfs::remove_all(tmpFile);
  // end gzip test
  ///////////////

  ////////////////////////////
  // Create Channel with bzip2
  BOOST_TEST_MESSAGE("# Testing compression methods bzip2 #");
  {
    FChannelPtr myFileC(new dadi::FileChannel(tmpFile.native()));

    // Check correct path
    BOOST_REQUIRE_EQUAL(myFileC->getPath(), tmpFile);

    myFileC->putAttr("compression_mode", "bzip2");
    // Log a message
    BOOST_REQUIRE_NO_THROW(myFileC->log(myMsg));

    // To check that the file with message logged is created
    BOOST_REQUIRE(bfs::exists(tmpFile));
    // Close Channel
    BOOST_REQUIRE_NO_THROW(myFileC->close());
  }

  // load this file
  {
    boost::iostreams::filtering_streambuf<boost::iostreams::input> tmp;
    tmp.push(boost::iostreams::bzip2_decompressor());
    tmp.push(boost::iostreams::file_source(tmpFile.native()));
    std::stringstream strstream;
    BOOST_REQUIRE_NO_THROW(boost::iostreams::copy(tmp, strstream));
    BOOST_REQUIRE(std::string(myMsg.getText() + "\n") == strstream.str());
  }
  bfs::remove_all(tmpFile);
  // end bzip2 test
  ////////////////

  ////////////////////////////
  // Create Channel with zlib
  BOOST_TEST_MESSAGE("# Testing compression methods zlib #");
  {
    FChannelPtr myFileC(new dadi::FileChannel(tmpFile.native()));

    // Check correct path
    BOOST_REQUIRE_EQUAL(myFileC->getPath(), tmpFile);

    myFileC->putAttr("compression_mode", "zlib");
    // Log a message
    BOOST_REQUIRE_NO_THROW(myFileC->log(myMsg));

    // To check that the file with message logged is created
    BOOST_REQUIRE(bfs::exists(tmpFile));
    // Close Channel
    BOOST_REQUIRE_NO_THROW(myFileC->close());
  }

  // load this file
  {
    boost::iostreams::filtering_streambuf<boost::iostreams::input> tmp;
    tmp.push(boost::iostreams::zlib_decompressor());
    tmp.push(boost::iostreams::file_source(tmpFile.native()));
    std::stringstream strstream;
    BOOST_REQUIRE_NO_THROW(boost::iostreams::copy(tmp, strstream));
    BOOST_REQUIRE(std::string(myMsg.getText() + "\n") == strstream.str());
  }
  bfs::remove_all(tmpFile);
  // end zlib test
  ////////////////

  bfs::remove_all(tmpDir);
}

// FIXME: should be broken into multiple test case (needs a fixture)
BOOST_AUTO_TEST_CASE(rotate_methods_test) {
  BOOST_TEST_MESSAGE("#Rotate methods test#");

  namespace fs = boost::filesystem;
  std::string source(SRCSTR);
  std::string msgToLog(MSGSTR);
  dadi::Message myMsg =
    dadi::Message(source, msgToLog, dadi::Message::PRIO_DEBUG);

  // Create working directory
  bfs::path tmpDir = bfs::temp_directory_path();
  tmpDir /= "%%%%-%%%%-%%%%-%%%%";
  tmpDir = bfs::unique_path(tmpDir);
  BOOST_TEST_MESSAGE("tmp directory = " + tmpDir.native());

  // Create file name
  bfs::path tmpFile = tmpDir;
  tmpFile /= "tmpFile.log";
  BOOST_TEST_MESSAGE("tmp file = " + tmpFile.native());

  ///////////////////////////////////
  // Create Channel with no rotation
  BOOST_TEST_MESSAGE("# Testing no rotation methods #");
  {
    bfs::create_directory(tmpDir);
    FChannelPtr myFileC(new dadi::FileChannel(tmpFile.native()));

    // Check correct path
    BOOST_REQUIRE_EQUAL(myFileC->getPath(), tmpFile);

    // set archive mode
    myFileC->putAttr("archive", "timestamp");

    // Set rotate mode to 1 second
    myFileC->putAttr("rotate", "none");

    // Log a message
    BOOST_REQUIRE_NO_THROW(myFileC->log(myMsg));

    // wait 1s
    sleep(1);

    // Log another message
    BOOST_REQUIRE_NO_THROW(myFileC->log(myMsg));
    BOOST_REQUIRE_NO_THROW(myFileC->log(myMsg));

    // wait 1s
    sleep(1);

    // Log another message
    BOOST_REQUIRE_NO_THROW(myFileC->log(myMsg));
    BOOST_REQUIRE_NO_THROW(myFileC->log(myMsg));

    // To check that the file with message logged is created
    BOOST_REQUIRE(bfs::exists(tmpFile));
    // Close Channel
    BOOST_REQUIRE_NO_THROW(myFileC->close());
  }

  // load this file
  {
    fs::directory_iterator end_iter;
    std::vector<fs::path> files;
    if (fs::exists(tmpDir) && fs::is_directory(tmpDir)) {
      unsigned int i = 0;
      for (fs::directory_iterator dir_iter(tmpDir);
           dir_iter != end_iter; ++dir_iter, ++i) {
        if (fs::is_regular_file(dir_iter->status())) {
          files.push_back(*dir_iter);
        }
      }
    }
    BOOST_REQUIRE_EQUAL(files.size(), 1);
    bfs::remove_all(tmpDir);
  }
  // end  no rotation test
  //////////////////////////

  ///////////////////////////////////
  // Create Channel with size rotation and archive based on number
  BOOST_TEST_MESSAGE("# Testing rotation based on size methods,"
                     << " archive number #");
  {
    bfs::create_directory(tmpDir);
    FChannelPtr myFileC(new dadi::FileChannel(tmpFile.native()));

    // Check correct path
    BOOST_REQUIRE_EQUAL(myFileC->getPath(), tmpFile);

    // set archive mode
    myFileC->putAttr("archive", "number");
    myFileC->putAttr("purge", "none");

    // Set rotate mode to 1 second
    myFileC->putAttr("rotate", "size");
    myFileC->putAttr("rotate.size", "57");

    // Log 5 messages
    BOOST_REQUIRE_NO_THROW(myFileC->log(myMsg));
    BOOST_REQUIRE_NO_THROW(myFileC->log(myMsg));
    BOOST_REQUIRE_NO_THROW(myFileC->log(myMsg));
    BOOST_REQUIRE_NO_THROW(myFileC->log(myMsg));
    BOOST_REQUIRE_NO_THROW(myFileC->log(myMsg));

    // To check that the file with message logged is created
    BOOST_REQUIRE(bfs::exists(tmpFile));
    // Close Channel
    BOOST_REQUIRE_NO_THROW(myFileC->close());
  }

  // load this file
  {
    fs::directory_iterator end_iter;
    std::vector<fs::path> files;
    if (fs::exists(tmpDir) && fs::is_directory(tmpDir)) {
      unsigned int i = 0;
      for (fs::directory_iterator dir_iter(tmpDir);
           dir_iter != end_iter; ++dir_iter, ++i) {
        if (fs::is_regular_file(dir_iter->status())) {
          files.push_back(*dir_iter);
        }
      }
    }
    // 6 files are generated: 5 full, and 1 empty
    BOOST_REQUIRE_EQUAL(files.size(), 6);
    bfs::remove_all(tmpDir);
  }
  // end  size rotation and archive number test
  //////////////////////////

  ///////////////////////////////////
  // Create Channel with size (in bytes) rotation
  // and archive based on utc timestamp
  BOOST_TEST_MESSAGE("# Testing rotation based on size (in bytes) methods,"
                     << " archive utc timestamp #");
  {
    bfs::create_directory(tmpDir);
    FChannelPtr myFileC(new dadi::FileChannel(tmpFile.native()));

    // Check correct path
    BOOST_REQUIRE_EQUAL(myFileC->getPath(), tmpFile);

    // set archive mode
    myFileC->putAttr("archive", "timestamp");
    myFileC->putAttr("purge", "none");

    // Set rotate mode to 1 second
    myFileC->putAttr("rotate", "size");
    myFileC->putAttr("rotate.size", "57");

    // Log 5 messages
    BOOST_REQUIRE_NO_THROW(myFileC->log(myMsg));
    BOOST_REQUIRE_NO_THROW(myFileC->log(myMsg));
    BOOST_REQUIRE_NO_THROW(myFileC->log(myMsg));
    BOOST_REQUIRE_NO_THROW(myFileC->log(myMsg));
    BOOST_REQUIRE_NO_THROW(myFileC->log(myMsg));

    // To check that the file with message logged is created
    BOOST_REQUIRE(bfs::exists(tmpFile));
    // Close Channel
    BOOST_REQUIRE_NO_THROW(myFileC->close());
  }

  // load this file
  {
    fs::directory_iterator end_iter;
    std::vector<fs::path> files;
    if (fs::exists(tmpDir) && fs::is_directory(tmpDir)) {
      unsigned int i = 0;
      for (fs::directory_iterator dir_iter(tmpDir);
           dir_iter != end_iter; ++dir_iter, ++i) {
        if (fs::is_regular_file(dir_iter->status())) {
          files.push_back(*dir_iter);
        }
      }
    }
    // 6 files are generated: 5 full, and 1 empty
    BOOST_REQUIRE_EQUAL(files.size(), 6);
    bfs::remove_all(tmpDir);
  }
  // end  size (in bytes) rotation and utc timestamp archive test
  //////////////////////////

  unsigned int size;
  ///////////////////////////////////
  // Create Channel with size (in kbytes) rotation
  // and archive based on utc timestamp
  BOOST_TEST_MESSAGE("# Testing rotation based on size (in kbytes) methods,"
                     << " archive number #");
  {
    bfs::create_directory(tmpDir);
    FChannelPtr myFileC(new dadi::FileChannel(tmpFile.native()));
    size = 1024;

    // Check correct path
    BOOST_REQUIRE_EQUAL(myFileC->getPath(), tmpFile);

    // set archive mode
    myFileC->putAttr("archive", "number");
    myFileC->putAttr("purge", "none");

    // Set rotate mode to 1 second
    myFileC->putAttr("rotate", "size");
    myFileC->putAttr("rotate.size", "1k");

    // Log 3kB messages
    for (unsigned int i = 0; i * MSGSTR.size() < size * 3; ++i) {
      myFileC->log(myMsg);
    }

    // To check that the file with message logged is created
    BOOST_REQUIRE(bfs::exists(tmpFile));
    // Close Channel
    BOOST_REQUIRE_NO_THROW(myFileC->close());
  }

  // load this file
  {
    fs::directory_iterator end_iter;
    std::vector<fs::path> files;
    if (fs::exists(tmpDir) && fs::is_directory(tmpDir)) {
      unsigned int i = 0;
      for (fs::directory_iterator dir_iter(tmpDir);
           dir_iter != end_iter; ++dir_iter, ++i) {
        if (fs::is_regular_file(dir_iter->status())) {
          files.push_back(*dir_iter);
        }
      }
    }
    // 3 files are generated: 2 full, and 1 empty
    BOOST_REQUIRE_EQUAL(files.size(), 4);
    // size is not necessarily a multiple of the message size
    double tolerance = 100 * (
      ceil(static_cast<double>(size) / (MSGSTR.size() + 1))
      * (MSGSTR.size() + 1) / static_cast<double>(size) - 1);
    BOOST_REQUIRE_CLOSE(bfs::file_size(tmpFile.native() + ".0"),
                        static_cast<double>(size), tolerance);
    bfs::remove_all(tmpDir);
  }
  // end  size (in kbytes) rotation and utc timestamp archive test
  //////////////////////////


  ///////////////////////////////////
  // Create Channel with size (in mbytes) rotation
  // and archive based on utc timestamp
  BOOST_TEST_MESSAGE("# Testing rotation based on size (in mbytes) methods,"
                     << " archive number #");
  {
    bfs::create_directory(tmpDir);
    FChannelPtr myFileC(new dadi::FileChannel(tmpFile.native()));
    size = 1024 * 1024;

    // Check correct path
    BOOST_REQUIRE_EQUAL(myFileC->getPath(), tmpFile);

    // set archive mode
    myFileC->putAttr("archive", "number");
    myFileC->putAttr("purge", "none");

    // Set rotate mode to 1 second
    myFileC->putAttr("rotate", "size");
    myFileC->putAttr("rotate.size", "1m");

    // Log 3MB messages
    for (unsigned int i = 0; i * MSGSTR.size() < size * 3; ++i) {
      myFileC->log(myMsg);
    }

    // To check that the file with message logged is created
    BOOST_REQUIRE(bfs::exists(tmpFile));
    // Close Channel
    BOOST_REQUIRE_NO_THROW(myFileC->close());
  }

  // load this file
  {
    fs::directory_iterator end_iter;
    std::vector<fs::path> files;
    if (fs::exists(tmpDir) && fs::is_directory(tmpDir)) {
      unsigned int i = 0;
      for (fs::directory_iterator dir_iter(tmpDir);
           dir_iter != end_iter; ++dir_iter, ++i) {
        if (fs::is_regular_file(dir_iter->status())) {
          files.push_back(*dir_iter);
        }
      }
    }
    // 4 files are generated: 3 full, and 1 empty
    BOOST_REQUIRE_EQUAL(files.size(), 4);
    // size is not necessarily a multiple of the message size
    double tolerance = 100 * (
      ceil(static_cast<double>(size) / (MSGSTR.size() + 1))
      * (MSGSTR.size() + 1) / static_cast<double>(size) - 1);
    BOOST_REQUIRE_CLOSE(bfs::file_size(tmpFile.native() + ".0"),
                        static_cast<double>(size), tolerance);
    bfs::remove_all(tmpDir);
  }
  // end  size (in mbytes) rotation and utc timestamp archive test
  //////////////////////////
}





BOOST_AUTO_TEST_CASE(archive_methods_test) {
  BOOST_TEST_MESSAGE("#Archive methods test#");

  namespace fs = boost::filesystem;
  std::string source(SRCSTR);
  std::string msgToLog(MSGSTR);
  dadi::Message myMsg =
    dadi::Message(source, msgToLog, dadi::Message::PRIO_DEBUG);

  // Create working directory
  bfs::path tmpDir = bfs::temp_directory_path();
  tmpDir /= "%%%%-%%%%-%%%%-%%%%";
  tmpDir = bfs::unique_path(tmpDir);
  bfs::create_directory(tmpDir);
  BOOST_TEST_MESSAGE("tmp directory = " + tmpDir.native());

  // Create file name
  bfs::path tmpFile = tmpDir;
  tmpFile /= "tmpFile.log";
  BOOST_TEST_MESSAGE("tmp file = " + tmpFile.native());

  /////////////////////////////////////
  // Create Channel with no archive
  BOOST_TEST_MESSAGE("# Testing archive methods: no archive #");
  {
    FChannelPtr myFileC(new dadi::FileChannel(tmpFile.native()));

    // Check correct path
    BOOST_REQUIRE_EQUAL(myFileC->getPath(), tmpFile);

    // Set rotate mode to 1 second
    myFileC->putAttr("rotate", "interval");
    myFileC->putAttr("rotate.interval", "00:00:01");

    // Log a message
    BOOST_REQUIRE_NO_THROW(myFileC->log(myMsg));

    // wait 1s
    sleep(1);

    // Log another message
    BOOST_REQUIRE_NO_THROW(myFileC->log(myMsg));
    BOOST_REQUIRE_NO_THROW(myFileC->log(myMsg));

    // wait 1s
    sleep(1);

    // Log another message
    BOOST_REQUIRE_NO_THROW(myFileC->log(myMsg));
    BOOST_REQUIRE_NO_THROW(myFileC->log(myMsg));

    // To check that the file with message logged is created
    BOOST_REQUIRE(bfs::exists(tmpFile));
    // Close Channel
    BOOST_REQUIRE_NO_THROW(myFileC->close());
  }

  // load this file
  {
    fs::directory_iterator end_iter;
    std::vector<fs::path> files;
    if (fs::exists(tmpDir) && fs::is_directory(tmpDir)) {
      for (fs::directory_iterator dir_iter(tmpDir);
           dir_iter != end_iter; ++dir_iter) {
        if (fs::is_regular_file(dir_iter->status())) {
          files.push_back(*dir_iter);
        }
      }
    }
    BOOST_REQUIRE_EQUAL(files.size(), 1);

    boost::iostreams::filtering_streambuf<boost::iostreams::input> tmp;
    tmp.push(boost::iostreams::file_source(tmpFile.native()));
    std::stringstream strstream;
    BOOST_REQUIRE_NO_THROW(boost::iostreams::copy(tmp, strstream));
    // message is different, as the file contains twice the message
    BOOST_REQUIRE(std::string(myMsg.getText() + "\n") != strstream.str());
  }
  bfs::remove_all(tmpFile);
  // end no archive test
  //////////////////////////


  /////////////////////////////////////
  // Create Channel with number archive
  BOOST_TEST_MESSAGE("# Testing archive methods: number #");
  {
    FChannelPtr myFileC(new dadi::FileChannel(tmpFile.native()));

    // Check correct path
    BOOST_REQUIRE_EQUAL(myFileC->getPath(), tmpFile);

    // set archive mode
    myFileC->putAttr("archive", "number");

    // Set rotate mode to 1 second
    myFileC->putAttr("rotate", "interval");
    myFileC->putAttr("rotate.interval", "00:00:01");

    // Log a message
    BOOST_REQUIRE_NO_THROW(myFileC->log(myMsg));

    // wait 1s
    sleep(1);

    // Log another message
    BOOST_REQUIRE_NO_THROW(myFileC->log(myMsg));
    BOOST_REQUIRE_NO_THROW(myFileC->log(myMsg));

    // wait 1s
    sleep(1);

    // Log another message
    BOOST_REQUIRE_NO_THROW(myFileC->log(myMsg));
    BOOST_REQUIRE_NO_THROW(myFileC->log(myMsg));

    // To check that the file with message logged is created
    BOOST_REQUIRE(bfs::exists(tmpFile));
    // Close Channel
    BOOST_REQUIRE_NO_THROW(myFileC->close());
  }

  // load this file
  {
    fs::directory_iterator end_iter;
    std::vector<fs::path> files;
    if (fs::exists(tmpDir) && fs::is_directory(tmpDir)) {
      unsigned int i = 0;
      for (fs::directory_iterator dir_iter(tmpDir);
           dir_iter != end_iter; ++dir_iter, ++i) {
        if (fs::is_regular_file(dir_iter->status())) {
          files.push_back(*dir_iter);
        }
      }
    }
    BOOST_REQUIRE_EQUAL(files.size(), 3);
  }
  bfs::remove_all(tmpFile);
  bfs::remove_all(tmpFile.native() + ".0");
  bfs::remove_all(tmpFile.native() + ".1");
  // end number archive test
  //////////////////////////

  /////////////////////////////////////
  // Create Channel with timestamp archive
  BOOST_TEST_MESSAGE("# Testing archive methods: timestamp #");
  {
    FChannelPtr myFileC(new dadi::FileChannel(tmpFile.native()));

    // Check correct path
    BOOST_REQUIRE_EQUAL(myFileC->getPath(), tmpFile);

    // set archive mode
    myFileC->putAttr("archive", "timestamp");

    // Set rotate mode to 1 second
    myFileC->putAttr("rotate", "interval");
    myFileC->putAttr("rotate.interval", "00:00:01");

    // Log a message
    BOOST_REQUIRE_NO_THROW(myFileC->log(myMsg));

    // wait 1s
    sleep(1);

    // Log another message
    BOOST_REQUIRE_NO_THROW(myFileC->log(myMsg));
    BOOST_REQUIRE_NO_THROW(myFileC->log(myMsg));

    // wait 1s
    sleep(1);

    // Log another message
    BOOST_REQUIRE_NO_THROW(myFileC->log(myMsg));
    BOOST_REQUIRE_NO_THROW(myFileC->log(myMsg));

    // To check that the file with message logged is created
    BOOST_REQUIRE(bfs::exists(tmpFile));
    // Close Channel
    BOOST_REQUIRE_NO_THROW(myFileC->close());
  }

  // load this file
  {
    fs::directory_iterator end_iter;
    std::vector<fs::path> files;
    if (fs::exists(tmpDir) && fs::is_directory(tmpDir)) {
      unsigned int i = 0;
      for (fs::directory_iterator dir_iter(tmpDir);
           dir_iter != end_iter; ++dir_iter, ++i) {
        if (fs::is_regular_file(dir_iter->status())) {
          files.push_back(*dir_iter);
        }
      }
    }
    BOOST_REQUIRE_EQUAL(files.size(), 3);
  }
  bfs::remove_all(tmpDir);
  // end  timestamp test
  //////////////////////////
}



BOOST_AUTO_TEST_SUITE_END()

// THE END
