//
// Created by matthew on 5/25/25.
//

#ifndef PYBIND_BUFFER_H
#define PYBIND_BUFFER_H

#include <pybind11/pybind11.h>
#include <streambuf>

namespace py = pybind11;

/**
 * A custom stream buffer that wraps a Python BytesIO object.
 */
class PyBytesIOBuf final : public std::streambuf {
    py::object bytesio_;
    std::string read_buffer_;

public:
    explicit PyBytesIOBuf(const py::object& bytesio) : bytesio_(bytesio) {}

protected:
    // Output operations (writing to BytesIO)

    /**
     * Writes a single character to the BytesIO object on overflow
     * @param c the character to write
     * @return the character written, or EOF on error
     */
    int_type overflow(const int_type c) override {
        if (c != EOF) {
            const char ch = static_cast<char>(c);
            py::bytes data(std::string(1, ch));
            bytesio_.attr("write")(data);
        }
        return c;
    }

    /**
     * Writes a block of characters to the BytesIO object
     * @param s the character buffer to write
     * @param count the number of characters to write
     * @return the number of characters written
     */
    std::streamsize xsputn(const char* s, const std::streamsize count) override {
        py::bytes data(s, count);
        bytesio_.attr("write")(data);
        return count;
    }

    // Input operations (reading from BytesIO)

    /**
     * On underflow, attempts to read more data from the BytesIO object.
     * @return the next character read, or EOF if no more data is available
     */
    int_type underflow() override {
        if (gptr() == egptr()) {
            // Buffer is empty, try to read more data
            if (!fill_buffer()) {
                return EOF;
            }
        }
        return traits_type::to_int_type(*gptr());
    }

    /**
     * Reads the next character from the buffer, advancing the get pointer.
     * @return the next character read, or EOF if no more data is available
     */
    int_type uflow() override {
        const int_type result = underflow();
        gbump(1);
        return result;
    }

    /**
     * Reads a block of characters from the buffer.
     * @param s the character buffer to fill
     * @param count the number of characters to read
     * @return the number of characters read
     */
    std::streamsize xsgetn(char* s, std::streamsize count) override {
        std::streamsize total_read = 0;
        while (count > 0) {
            if (gptr() == egptr()) {
                if (!fill_buffer()) {
                    break; // No more data available
                }
            }
            const std::streamsize available = egptr() - gptr();
            const std::streamsize to_copy = std::min(count, available);
            // Copy data from the buffer to the output
            std::memcpy(s, gptr(), to_copy);
            // Advance the get pointer
            gbump(static_cast<int>(to_copy));
            s += to_copy;
            count -= to_copy;
            total_read += to_copy;
        }
        return total_read;
    }

    // Seeking support

    /**
     * Seeks to a specific offset in the BytesIO object.
     * @param off the offset to seek to
     * @param way the direction to seek (beginning, current, end)
     * @param which the open mode (input, output, or both)
     * @return the new position in the stream, or -1 on error
     */
    pos_type seekoff(off_type off, const std::ios_base::seekdir way,
                     const std::ios_base::openmode which = std::ios_base::in |
                                                           std::ios_base::out) override {
        if (which & std::ios_base::out) {
            // For output, delegate to BytesIO's seek
            try {
                if (way == std::ios_base::beg) {
                    bytesio_.attr("seek")(off);
                } else if (way == std::ios_base::cur) {
                    bytesio_.attr("seek")(off, 1);
                } else if (way == std::ios_base::end) {
                    bytesio_.attr("seek")(off, 2);
                }
                return bytesio_.attr("tell")().cast<pos_type>();
            } catch (...) {
                return {-1};
            }
        }
        if (which & std::ios_base::in) {
            // For input, we need to manage our own position
            // This is simplified - full implementation would be more complex
            try {
                if (way == std::ios_base::beg) {
                    bytesio_.attr("seek")(off);
                } else if (way == std::ios_base::cur) {
                    // Account for buffered data
                    const off_type current_pos = bytesio_.attr("tell")().cast<off_type>();
                    const off_type buffered_offset = gptr() - eback();
                    bytesio_.attr("seek")(current_pos - (egptr() - eback()) + buffered_offset +
                                          off);
                } else if (way == std::ios_base::end) {
                    bytesio_.attr("seek")(off, 2);
                }
                // Clear the buffer since position changed
                setg(nullptr, nullptr, nullptr);
                return bytesio_.attr("tell")().cast<pos_type>();
            } catch (...) {
                return {-1};
            }
        }
        return {-1};
    }

    /**
     * Seeks to a specific position in the BytesIO object.
     * @param sp the position to seek to
     * @param which the open mode (input, output, or both)
     * @return the new position in the stream, or -1 on error
     */
    pos_type seekpos(const pos_type sp,
                     const std::ios_base::openmode which = std::ios_base::in |
                                                           std::ios_base::out) override {
        return seekoff(sp, std::ios_base::beg, which);
    }

private:
    /**
     * Fills the internal read buffer by reading from the BytesIO object.
     * @return true if data was read successfully, false if EOF or error occurred
     */
    bool fill_buffer() {
        try {
            // Read a chunk of data from BytesIO
            const py::object result = bytesio_.attr("read")(8192); // Read 8KB chunks

            if (py::isinstance<py::bytes>(result)) {
                std::string data = result.cast<std::string>();
                if (data.empty()) {
                    return false; // EOF
                }

                read_buffer_ = std::move(data);
                setg(&read_buffer_[0], &read_buffer_[0], &read_buffer_[0] + read_buffer_.size());
                return true;
            }
            return false;
        } catch (...) {
            return false;
        }
    }
};

#endif // PYBIND_BUFFER_H
