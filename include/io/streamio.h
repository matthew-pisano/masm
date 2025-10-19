//
// Created by matthew on 6/8/25.
//

#ifndef STREAMIO_H
#define STREAMIO_H
#include <iostream>


/**
 * Seek reference points
 */
enum class Whence {
    SET, // Beginning of stream
    CUR, // Current position in stream
    END // End of stream
};


class StreamHandle {
protected:
    /**
     * The input stream for reading characters
     */
    std::istream& istream;

    /**
     * The output stream for writing characters
     */
    std::ostream& ostream;

    /**
     * Working input buffer before being flushed to the output stream
     */
    std::string buffer;

    /**
     * Cursor position in the input stream
     */
    int32_t cursor = -1;

public:
    StreamHandle(std::istream& istream, std::ostream& ostream) :
        istream(istream), ostream(ostream) {}
    virtual ~StreamHandle() = default;

    /**
     * Gets the current input buffer
     * @return The current input buffer as a string
     */
    virtual std::string getBuffer();

    /**
     * Checks if there are characters available to read from the input stream
     * @return True if there are characters available, false otherwise
     */
    virtual bool hasChar();

    /**
     * Gets a character from the input stream
     * @return The character read from the input stream
     */
    virtual char getChar();

    /**
     * Reads (blocking) a character from the input stream
     * @return The character read from the input stream
     */
    char getCharBlocking();

    /**
     * Reads (blocking) a line from the input stream, omitting control characters
     * @return The line read from the input stream
     */
    virtual std::string getLine();

    /**
     * Sends a character to the output stream
     */
    virtual void putChar(char c);

    /**
     * Writes a null-terminated string to the output stream
     * @param str The string to write to the output stream
     */
    void putStr(const std::string& str);

    /**
     * Clears the output buffer
     */
    virtual void clear();

    /**
     * Flushes the output buffer to the output stream
     */
    virtual void flush();

    /**
     * Seeks to a position in the input stream
     * @param offset The offset to seek to
     * @param whence The reference point for the seek
     */
    virtual void seek(int32_t offset, Whence whence);

    virtual void show();
};

#endif // STREAMIO_H
