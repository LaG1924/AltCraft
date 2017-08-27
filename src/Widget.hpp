#pragma once

#include <vector>
#include <memory>
#include <functional>

#include "AssetManager.hpp"

class Widget;
class RootWidget {
    std::vector<std::unique_ptr<Widget>> allWidgets;

    std::vector<Widget*> childs;

    double prevX, prevY;
    bool prevBut;
public:
    RootWidget() = default;

    ~RootWidget() = default;

    void AttachWidget(std::unique_ptr<Widget> widget, Widget* parent);

    void AttachWidget(std::unique_ptr<Widget> widget);

    std::vector<Widget*> GetRenderList();

    void UpdateEvents(double mouseX, double mouseY, bool mouseButton);
};

struct Widget {
    Widget() = default;

    virtual ~Widget() = default;

    Widget *parent;

    std::vector<Widget*> childs;

    double x, y, w, h; //In OGL screen-coordinates

    virtual std::tuple<double, double, double, double> GetTexture() = 0;

    
    using Handler = std::function<void(Widget*)>;

    Handler onPress;

    Handler onRelease;

    Handler onHover;

    Handler onUnhover;

    Handler onUpdate;
};

struct WidgetButton : Widget {
    WidgetButton();

    ~WidgetButton() override = default;

    std::string Text;

    Handler onClick;

    std::tuple<double, double, double, double> GetTexture() override;

    enum class WidgetState {
        Idle,
        Hovering,
        Pressed,
    } state;
};