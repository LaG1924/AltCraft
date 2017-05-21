#pragma once

class Widget {
    unsigned int x,y,w,h;
public:
    Widget(Widget *parent);
    ~Widget();
};
