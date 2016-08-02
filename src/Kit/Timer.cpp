#include <Kit/Timer.hpp>

using namespace std::chrono;

kit::Time::Time(nanoseconds t){
    this->m_point = t;
}

uint64_t kit::Time::asHours(){
    return (uint64_t)duration_cast<hours>(this->m_point).count();
}

uint64_t kit::Time::asMinutes(){
    return (uint64_t)duration_cast<minutes>(this->m_point).count();
}

uint64_t kit::Time::asSeconds(){
    return (uint64_t)duration_cast<seconds>(this->m_point).count();
}

uint64_t kit::Time::asMilliseconds(){
    return (uint64_t)duration_cast<milliseconds>(this->m_point).count();
}

uint64_t kit::Time::asMicroseconds(){
    return (uint64_t)duration_cast<microseconds>(this->m_point).count();
}

uint64_t kit::Time::asNanoseconds(){
    return this->m_point.count();
}

kit::Timer::Timer(){
    this->restart();
}

kit::Timer::~Timer(){

}

kit::Time kit::Timer::restart(){
    auto now = CLOCK_TYPE::now();
    auto old = kit::Time(duration_cast<nanoseconds>(now - this->m_origin));
    this->m_origin = CLOCK_TYPE::now();
    return old;
}

kit::Time kit::Timer::timeSinceStart(){
    auto now = CLOCK_TYPE::now();
    return kit::Time(duration_cast<nanoseconds>(now - this->m_origin));
}
