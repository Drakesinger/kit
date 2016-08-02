#ifndef KIT_TIMER_HEADER
#define KIT_TIMER_HEADER

#include "Kit/Export.hpp"
#include "Kit/Types.hpp"

#include <chrono>

//#define CLOCK_TYPE steady_clock
#define CLOCK_TYPE high_resolution_clock

using namespace std::chrono;

namespace kit{

    struct KITAPI Time{
        public:
            Time(nanoseconds t);

            uint64_t asHours();
            uint64_t asMinutes();
            uint64_t asSeconds();
            uint64_t asMilliseconds();
            uint64_t asMicroseconds();
            uint64_t asNanoseconds();
        private:
             nanoseconds m_point;
    };

    class KITAPI Timer{

        public:
            Timer();
            ~Timer();

            kit::Time restart();
            kit::Time timeSinceStart();

        private:
            CLOCK_TYPE::time_point m_origin;

    };

}

#endif // KIT_TIMER_HEADER
