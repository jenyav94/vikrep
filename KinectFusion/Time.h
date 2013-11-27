#pragma once

#include <Windows.h>


namespace Timing1
{
    /// <summary>
    /// Timer is a minimal class to get the system time in seconds
    /// </summary>
    class Time
    {
    private:
        /// <summary>
        ///  Whether the timer is initialized and can be used.
        /// </summary>
        bool initialized;

        /// <summary>
        ///  The clock frequency in ticks per second.
        /// </summary>
        LARGE_INTEGER frequency;

    public:
        /// <summary>
        ///  Initializes a new instance of the <see cref="Timer"/> class.
        /// </summary>
        Time();

        /// <summary>
        ///  Gets the absolute time.
        /// </summary>
        /// <returns>Returns the absolute time in s.</returns>
        double AbsoluteTime();
    };
};

