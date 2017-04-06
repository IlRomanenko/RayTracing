//
// Created by ilya on 13.03.17.
//
#pragma once

#include "../base_headers.h"
#include "../geometry/Primitives.h"

#include <exception>
#include <sstream>

using namespace geometry;

static string ltrim(const string &s) {
    size_t beg_pos = 0;
    while (beg_pos < s.length() && isspace(s[beg_pos])) {
        beg_pos++;
    }
    return s.substr(beg_pos);
}

static string rtrim(const string &s) {
    string res = s;
    while (res.size() > 0 && isspace(res.back())) {
        res.pop_back();
    }
    return res;
}

static string trim(const string &s) {

    return ltrim(rtrim(s));
}

class FileScanner {
    ifstream file;

public:
    FileScanner(const string &filename) {
        FILE *c_file;
        if ((c_file = fopen(filename.c_str(), "r")) == nullptr) {
            fprintf(stderr, "Error with file open");
            throw exception();
        }
        fclose(c_file);
        file.open(filename, ios_base::in);
    }

    bool eof() const {
        return file.eof();
    }

    string nextLine() {
        string line;
        while (!file.eof()) {
            getline(file, line);
            line = trim(line);

            if (line[0] == '#' || line == "") {
                line = "";
                continue;
            } else {
                break;
            }
        }

        return line;
    }

    void close() {
        file.close();
    }

    ~FileScanner() {
        close();
    }
};

class StringScanner {
    stringstream stream;
public:
    StringScanner() {}

    StringScanner(const string &s) : stream(s) {}

    void setBuffer(const string &s) {
        stream = stringstream(s);
    }

    string nextString() {
        string res;
        stream >> res;
        return res;
    }

    int nextInt() {
        int res = 0;
        stream >> res;
        return res;
    }

    ldb nextDouble() {
        ldb res;
        stream >> res;
        return res;
    }

    Vector nextVector() {
        ldb x, y, z;
        x = nextDouble();
        y = nextDouble();
        z = nextDouble();
        return std::move(Vector(x, y, z));
    }
};