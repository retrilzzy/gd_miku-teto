#include "BorderContainer.hpp"
#include "Settings.hpp"

#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>

using namespace geode::prelude;

class $modify(HypePlayLayer, PlayLayer) {
    struct Fields {
        BorderContainer* m_borderContainer = nullptr;
        bool m_renderBehindHud = true;
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) {
            return false;
        }

        auto settings = HypeSettings::load();
        m_fields->m_renderBehindHud = settings.m_renderBehindHud;

        auto* container = BorderContainer::create(settings);
        if (settings.m_renderBehindHud) {
            this->addChild(container, zorder::behindHud);
        } else if (m_uiLayer) {
            m_uiLayer->addChild(container, zorder::aboveHud);
        }

        m_fields->m_borderContainer = container;
        return true;
    }

    void update(float dt) override {
        PlayLayer::update(dt);

        if (m_fields->m_borderContainer && m_objectLayer &&
            m_fields->m_renderBehindHud) {
            m_fields->m_borderContainer->setPosition(
                -m_objectLayer->getPosition()
            );
        }
    }
};

class $modify(HypeBaseGameLayer, GJBaseGameLayer) {
    void handleButton(bool down, int button, bool isPlayer1) {
        GJBaseGameLayer::handleButton(down, button, isPlayer1);

        if (auto* playLayer = typeinfo_cast<PlayLayer*>(this)) {
            auto* hypeLayer = static_cast<HypePlayLayer*>(playLayer);
            if (auto* container = hypeLayer->m_fields->m_borderContainer) {
                if (down) {
                    container->onPlayerPush();
                } else {
                    container->onPlayerRelease();
                }
            }
        }
    }
};

class $modify(HypePlayerObject, PlayerObject) {
    bool pushButton(PlayerButton button) {
        bool res = PlayerObject::pushButton(button);

        if (auto* playLayer = PlayLayer::get()) {
            auto* hypeLayer = static_cast<HypePlayLayer*>(playLayer);
            if (auto* container = hypeLayer->m_fields->m_borderContainer) {
                container->onPlayerPush();
            }
        }
        return res;
    }

    bool releaseButton(PlayerButton button) {
        bool res = PlayerObject::releaseButton(button);

        if (auto* playLayer = PlayLayer::get()) {
            auto* hypeLayer = static_cast<HypePlayLayer*>(playLayer);
            if (auto* container = hypeLayer->m_fields->m_borderContainer) {
                container->onPlayerRelease();
            }
        }
        return res;
    }
};
