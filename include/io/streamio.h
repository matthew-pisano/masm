//
// Created by matthew on 6/8/25.
//

#ifndef STREAMIO_H
#define STREAMIO_H
#include <iostream>


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

public:
    StreamHandle(std::istream& istream, std::ostream& ostream) :
        istream(istream), ostream(ostream) {}
    virtual ~StreamHandle() = default;

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
     * Sends a character to the output stream
     */
    virtual void putChar(char c);

    /**
     * Writes a null-terminated string to the output stream
     * @param str The string to write to the output stream
     */
    void putStr(const std::string& str);
};

#endif // STREAMIO_H
