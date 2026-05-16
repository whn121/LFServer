#ifndef NOCOPYABLE_HPP
#define NOCOPYABLE_HPP

class nocopyable {
public:
    nocopyable() = default;
    ~nocopyable() = default;
    nocopyable(const nocopyable&) = delete;
    nocopyable& operator=(const nocopyable&) = delete;
};

#endif
