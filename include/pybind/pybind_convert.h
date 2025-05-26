//
// Created by matthew on 5/25/25.
//

#ifndef PYBIND_CONVERT_H
#define PYBIND_CONVERT_H


#include <pybind11/cast.h>
#include <vector>


template<>
class pybind11::detail::type_caster<std::vector<std::byte>> {
public:
    PYBIND11_TYPE_CASTER(std::vector<std::byte>, _("bytes"));

    bool load(handle src, bool) {
        if (!src)
            return false;

        // Handle Python bytes object
        if (PyBytes_Check(src.ptr())) {
            const char* data = PyBytes_AsString(src.ptr());
            const Py_ssize_t size = PyBytes_Size(src.ptr());

            value.clear();
            value.reserve(size);
            for (Py_ssize_t i = 0; i < size; ++i) {
                value.push_back(static_cast<std::byte>(static_cast<unsigned char>(data[i])));
            }
            return true;
        }

        // Handle list of ints
        if (PyList_Check(src.ptr())) {
            value.clear();
            const Py_ssize_t size = PyList_Size(src.ptr());
            value.reserve(size);

            for (Py_ssize_t i = 0; i < size; ++i) {
                PyObject* item = PyList_GetItem(src.ptr(), i);
                if (!PyLong_Check(item))
                    return false;

                long val = PyLong_AsLong(item);
                if (val < 0 || val > 255)
                    return false;

                value.push_back(static_cast<std::byte>(val));
            }
            return true;
        }

        return false;
    }

    static handle cast(const std::vector<std::byte>& src, return_value_policy /* policy */,
                       handle /* parent */) {
        return PyBytes_FromStringAndSize(reinterpret_cast<const char*>(src.data()),
                                         static_cast<Py_ssize_t>(src.size()));
    }
};

#endif // PYBIND_CONVERT_H
