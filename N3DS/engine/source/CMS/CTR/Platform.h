#ifndef PLATFORM_H
#define PLATFORM_H

#include <string>

struct ScreenInfo {
    int x;
    int y;
    bool primary;
    bool touch;
    std::string name;

    ScreenInfo(
        int x1,
        int y1,
        bool prim,
        bool t,
        std::string n
    ) :
    x(x1), y(y1), primary(prim), touch(t), name(n)
    {}
};

class Platform {
public:
    static int PrimaryScreen;
    static int TouchScreen;
    static int ScreenCount;
    static int AllScreenIndex;
    static ScreenInfo Screens[2];
    static std::string name;
	static ScreenInfo GetScreen(int si);
};

#endif // PLATFORM_H
