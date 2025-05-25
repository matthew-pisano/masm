//
// Created by matthew on 5/25/25.
//

#ifndef PYBIND_BUFFER_H
#define PYBIND_BUFFER_H

#include <iostream>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <streambuf>

namespace py = pybind11;

class PyBytesIOBuf final : public std::streambuf {
    py::object bytesio_;

public:
    explicit PyBytesIOBuf(const py::object& bytesio) : bytesio_(bytesio) {}

protected:
    int_type overflow(const int_type c) override {
        if (c != EOF) {
            const char ch = static_cast<char>(c);
            py::bytes data(std::string(1, ch));
            bytesio_.attr("write")(data);
        }
        return c;
    }

    std::streamsize xsputn(const char* s, const std::streamsize count) override {
        py::bytes data(s, count);
        bytesio_.attr("write")(data);
        return count;
    }
};


#endif // PYBIND_BUFFER_H
