//
// Created by haridev on 10/23/23.
//

#include "File.h"
#include "Index.h"
#include "RegExpIndexer.h"
#include "ConsoleLog.h"
#include "FieldIndexer.h"
#include "IndexParser.h"

#include <tclap/CmdLine.h>

#include <iostream>
#include <stdexcept>
#include <limits.h>
#include "ExternalIndexer.h"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "RangeFetcher.h"
#include "LineSink.h"

using namespace std;
using namespace TCLAP;
struct MaxLineSink : LineSink {
    uint64_t line_number;

    explicit MaxLineSink() { }

    bool onLine(size_t l, size_t, const char *_line, size_t length) override {
        line_number = l;
        (void) _line;
        (void) length;
        return true;
    }
};
struct OneLineSink : LineSink {
    std::string line;

    explicit OneLineSink() { }

    bool onLine(size_t l, size_t, const char *_line, size_t length) override {
        (void) l;
        line = string(_line, length);
        return true;
    }
};

struct MaxLineHandler : RangeFetcher::Handler {
    Index &index;
    MaxLineSink &sink;
    uint64_t max_line;

    MaxLineHandler(Index &index, MaxLineSink &sink)
            : index(index), sink(sink), max_line(0) { }

    void onLine(uint64_t line) override {
        index.getLine(line, sink);
        if (max_line < sink.line_number) max_line = sink.line_number;
    }

    void onSeparator() override {
    }
};
struct ListHandler : RangeFetcher::Handler {
    Index &index;
    OneLineSink &sink;
    std::vector<std::string> lines;

    ListHandler(Index &index, OneLineSink &sink)
            : index(index), sink(sink), lines() { }

    void onLine(uint64_t line) override {
        index.getLine(line, sink);
        lines.push_back(sink.line);
    }

    void onSeparator() override {
    }
};
namespace {

string getRealPath(const string &relPath) {
    char realPathBuf[PATH_MAX];
    auto result = realpath(relPath.c_str(), realPathBuf);
    if (result == nullptr) return relPath;
    return string(relPath);
}

}

uint64_t toInt(const string &s) {
    char *endP;
    auto res = strtoull(&s[0], &endP, 10);
    if (*endP != '\0') throw runtime_error("Non-numeric value: '" + s + "'");
    return res;
}
namespace py = pybind11;
namespace zindex {


int create_index(std::string input_file, std::string index_file = "",
                 bool debug = false, bool verbose = false, int skipFirst=0,
                 bool numeric = true, bool unique = true, bool sparse = false,
                 int checkpointEvery = 0, std::string regex = nullptr, int capture = 0,
                 int field = 0, std::string configFile = "", std::string delimiterArg = "",
                 bool tabDelimiterArg=false, std::string externalIndexer = "") {
    ConsoleLog log(debug ? Log::Severity::Debug : verbose
                                                   ? Log::Severity::Info
                                                   : Log::Severity::Warning,
            false, true);
    try {
        auto realPath = getRealPath(input_file);
        File in(fopen(realPath.c_str(), "rb"));
        if (in.get() == nullptr) {
            log.debug("Unable to open ", input_file, " (as ",
                      realPath,
                      ")");
            log.error("Could not open ", input_file, " for reading");
            return -1;
        }

        auto outputFile = !index_file.empty() ? index_file :
                          "file:" + input_file + ".zindex";
        Index::Builder builder(log, std::move(in), realPath, outputFile);
        if (skipFirst)
            builder.skipFirst(skipFirst);

        Index::IndexConfig config{};
        config.numeric = numeric;
        config.unique = unique;
        config.sparse = sparse;
        //config.indexLineOffsets = // TODO - add command line flag if desired

        auto delimiter = delimiterArg;
        if (tabDelimiterArg && !delimiterArg.empty()) {
            log.error("Cannot set both --delimiter and --tab-delimiter");
            return 1;
        }
        if (tabDelimiterArg)
            delimiter = "\t";

        if (!configFile.empty()) {
            auto indexParser = IndexParser(configFile);
            indexParser.buildIndexes(&builder, log);
        } else {
            if (!regex.empty() && field != 0) {
                throw std::runtime_error(
                        "Sorry; multiple indices must be defined by an "
                        "indexes file - see '-i' option");
            }
            if (!regex.empty()) {
                auto regexIndexer = new RegExpIndexer(regex,
                                                      capture);
                builder.addIndexer("default", regex, config,
                                   std::unique_ptr<LineIndexer>(regexIndexer));
            }
            if (field != 0) {
                ostringstream name;
                name << "Field " << field << " delimited by '"
                     << delimiter << "'";
                builder.addIndexer("default", name.str(), config,
                                   std::unique_ptr<LineIndexer>(
                                           new FieldIndexer(
                                                   delimiter,
                                                   field)));
            }
            if (!externalIndexer.empty()) {
                auto indexer = std::unique_ptr<LineIndexer>(
                        new ExternalIndexer(log,
                                            externalIndexer,
                                            delimiter));
                builder.addIndexer("default", externalIndexer,
                                   config, std::move(indexer));
            }
        }
        if (checkpointEvery != 0)
            builder.indexEvery(checkpointEvery);
        builder.build();
    } catch (const exception &e) {
        log.error(e.what());
        return 1;
    }
    return 0;
}

size_t get_max_line(std::string inputFile, std::string index_file = "",  bool verbose = false, bool debug = false) {
    ConsoleLog log(
            debug ? Log::Severity::Debug : verbose
                                           ? Log::Severity::Info
                                           : Log::Severity::Warning,
            false, true);
    try {
        auto compressedFile = inputFile;
        File in(fopen(compressedFile.c_str(), "rb"));
        if (in.get() == nullptr) {
            log.error("Could not open ", compressedFile, " for reading");
            return {};
        }

        auto indexFile = !index_file.empty() ? index_file :
                         inputFile + ".zindex";
        log.debug("Using index file  ", indexFile, " for reading");
        auto index = Index::load(log, std::move(in), indexFile,
                                 true);
        return index.total_lines();
    } catch (const exception &e) {
        log.error(e.what());
        return 0;
    }
}

uint64_t get_total_size(std::string inputFile, std::string index_file = "",  bool verbose = false, bool debug = false) {
    ConsoleLog log(
            debug ? Log::Severity::Debug : verbose
                                           ? Log::Severity::Info
                                           : Log::Severity::Warning,
            false, true);
    try {
        auto compressedFile = inputFile;
        File in(fopen(compressedFile.c_str(), "rb"));
        if (in.get() == nullptr) {
            log.error("Could not open ", compressedFile, " for reading");
            return {};
        }

        auto indexFile = !index_file.empty() ? index_file :
                         inputFile + ".zindex";
        log.debug("Using index file  ", indexFile, " for reading");
        auto index = Index::load(log, std::move(in), indexFile,
                                 true);
        return index.total_size();
    } catch (const exception &e) {
        log.error(e.what());
        return 0;
    }
}

std::vector<std::string> zquery(std::string input_file, std::vector<std::string> query = {}, bool lineMode = false,
               bool verbose = false, bool debug = false, bool forceLoad = false, uint64_t after = 0, uint64_t before = 0,
               uint64_t contextArg = 0, std::string index_file = "", std::string queryIndexArg = "", std::string raw="") {

    ConsoleLog log(
            debug ? Log::Severity::Debug : verbose
                                                   ? Log::Severity::Info
                                                   : Log::Severity::Warning,
            false, true);

    try {
        auto compressedFile = input_file;
        File in(fopen(compressedFile.c_str(), "rb"));
        if (in.get() == nullptr) {
            log.error("Could not open ", compressedFile, " for reading");
            return {};
        }

        auto indexFile = !index_file.empty() ? index_file :
                         input_file + ".zindex";
        auto index = Index::load(log, std::move(in), indexFile,
                                 forceLoad);
        auto queryIndex = !queryIndexArg.empty() ? queryIndexArg : "default";

        if (contextArg != 0) before = after = contextArg;
        log.debug("Fetching context of ", before, " lines before and ", after,
                  " lines after");
        OneLineSink sink;
        ListHandler ph(index, sink);
        RangeFetcher rangeFetcher(ph, before, after);
        if (lineMode) {
            for(auto q :query)
                rangeFetcher(toInt(q));
        } else if (!raw.empty()) {
            index.queryCustom(raw, rangeFetcher);
        } else {
            index.queryIndexMulti(queryIndex, query, rangeFetcher);
        }
        return ph.lines;
    } catch (const exception &e) {
        log.error(e.what());
        return {};
    }

    return {};
}
} // zindex
PYBIND11_MODULE(zindex_py, m) {
    m.doc() = "Python module for zindex"; // optional module docstring
    m.def("get_max_line", &zindex::get_max_line, "Get Maximum number of lines from index",
        py::arg("input_file"), py::arg("index_file") = "",
        py::arg("debug") = false,  py::arg("verbose") = false
    );
    m.def("get_total_size", &zindex::get_total_size, "Get Maximum size of uncompressed file from index",
        py::arg("input_file"), py::arg("index_file") = "",
        py::arg("debug") = false,  py::arg("verbose") = false
    );
    m.def("create_index", &zindex::create_index, "create index for gzip file",
        py::arg("input_file"), py::arg("index_file") = "",
        py::arg("debug") = false,  py::arg("verbose") = false,
        py::arg("skipFirst") = 0, py::arg("numeric") = false,
        py::arg("unique") = true, py::arg("sparse") = false,
        py::arg("checkpointEvery") = 0, py::arg("regex") = "",
        py::arg("capture") = 0, py::arg("field") = 0,
        py::arg("configFile") = "", py::arg("delimiterArg") = "",
        py::arg("tabDelimiterArg") = false, py::arg("externalIndexer") = ""
    );
    m.def("zquery", &zindex::zquery, "query the gzip file and get lines",
        py::arg("input_file"), py::arg("query") = std::vector<std::string>(), py::arg("lineMode") = false,
        py::arg("debug") = false,  py::arg("verbose") = false,
        py::arg("forceLoad") = false,
        py::arg("after") = 0, py::arg("before") = 0,
        py::arg("contextArg") = 0, py::arg("index_file") = "",
        py::arg("queryIndexArg") = "", py::arg("raw") = ""
    );
}
