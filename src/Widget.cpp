#include "Widget.hpp"

void RootWidget::AttachWidget(std::unique_ptr<Widget> widget, Widget * parent)
{
    parent->childs.push_back(widget.get());
    this->allWidgets.push_back(std::move(widget));
}

void RootWidget::AttachWidget(std::unique_ptr<Widget> widget) {
    widget->parent = nullptr;
    this->childs.push_back(widget.get());
    this->allWidgets.push_back(std::move(widget));
}

std::vector<Widget*> RootWidget::GetRenderList()
{
    std::vector<Widget*> renderList;    

    std::function<void(Widget*)> treeWalker = [&](Widget* node) {
        for (auto it : node->childs)
            treeWalker(it);
        renderList.push_back(node);
    };

    for (auto& it : this->childs)
        treeWalker(it);

    return renderList;
}

void RootWidget::UpdateEvents(double mouseX, double mouseY, bool mouseButton) {

    LOG(INFO) << mouseX << "x" << mouseY;

    auto testIsHover = [&](double x, double y, Widget* widget) {
        bool isOnX = widget->x > x && widget->x + widget->w < x;
        bool isOnY = widget->y > y && widget->y + widget->h < y;
        if (mouseButton)
            LOG(INFO) << "X: " << isOnX << " Y: " << isOnY;
        return isOnX && isOnY;
    };

    std::function<void(Widget*)> treeWalker = [&](Widget* node) {
        for (auto it : node->childs)
            treeWalker(it);
        
        if (testIsHover(mouseX,mouseY,node)) {
            if (node->onHover)
                node->onHover(node);
            if (mouseButton && !prevBut)
                if (node->onPress)
                    node->onPress(node);
            else if (!mouseButton && prevBut)
                if (node->onRelease)
                    node->onRelease(node);
        }
        else {
            if (testIsHover(prevX, prevY, node))
                if (node->onUnhover)
                    node->onUnhover(node);
        }
        
        if (node->onUpdate)
            node->onUpdate(node);
    };

    for (auto it : childs)
        treeWalker(it);

    prevX = mouseX;
    prevY = mouseY;
    prevBut = mouseButton;
}

WidgetButton::WidgetButton()
{
    this->state = WidgetState::Idle;

    onHover = [](Widget* widget) {
        WidgetButton* w = dynamic_cast<WidgetButton*>(widget);
        if (w->state != WidgetState::Pressed)
            w->state = WidgetState::Hovering;
        LOG(INFO) << "Hover";
    };

    onPress = [](Widget* widget) {
        WidgetButton* w = dynamic_cast<WidgetButton*>(widget);
        w->state = WidgetState::Pressed;
        LOG(INFO) << "Press";
    };

    onRelease = [](Widget* widget) {
        WidgetButton* w = dynamic_cast<WidgetButton*>(widget);
        w->state = WidgetState::Idle;
        w->onClick(w);
        LOG(INFO) << "Release";
    };

    onUnhover = [](Widget *widget) {
        WidgetButton* w = dynamic_cast<WidgetButton*>(widget);
        if (w->state!=WidgetState::Pressed)
            w->state = WidgetState::Idle;
        LOG(INFO) << "Unhover";
    };

}

std::tuple<double, double, double, double> WidgetButton::GetTexture()
{
    double yOffset;
    switch (this->state) {
    case WidgetState::Idle:
        yOffset = 0.2578;
        break;
    case WidgetState::Hovering:
        yOffset = 0.3359;
        break;
    case WidgetState::Pressed:
        yOffset = 0.1796;
    }

    TextureCoordinates texture = AssetManager::Instance().GetTextureByAssetName("minecraft/textures/gui/widgets");
    return { texture.x,texture.y + texture.h * yOffset,texture.w * 0.7812,texture.h * 0.07812 };
}
