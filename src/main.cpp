#include "BorderContainer.hpp"
#include "Settings.hpp"

#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(HypePlayLayer, PlayLayer) {
    struct Fields {
        BorderContainer* m_borderContainer = nullptr;
        bool m_renderBehindHud = true;
    };

    bool init(GJGameLevel* level, bool dontCreateObjects) {
        if (!PlayLayer::init(level, dontCreateObjects)) {
            return false;
        }

        auto settings = HypeSettings::load();
        m_fields->m_renderBehindHud = settings.m_renderBehindHud;

        auto* container = BorderContainer::create(settings);
        if (container) {
            if (settings.m_renderBehindHud) {
                this->addChild(container, zorder::behindHud);
                m_fields->m_borderContainer = container;
            } else if (m_uiLayer) {
                m_uiLayer->addChild(container, zorder::aboveHud);
                m_fields->m_borderContainer = container;
            }
        }

        return true;
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) override {
        PlayLayer::destroyPlayer(player, object);

        if (m_fields->m_borderContainer && player && player->m_isDead) {
            m_fields->m_borderContainer->resetInputState();
        }
    }

    void levelComplete() {
        PlayLayer::levelComplete();

        if (m_fields->m_borderContainer) {
            m_fields->m_borderContainer->resetInputState();
        }
    }

    void resetLevel() override {
        PlayLayer::resetLevel();

        if (m_fields->m_borderContainer) {
            m_fields->m_borderContainer->resetInputState();
        }
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
                bool isAttemptEnded =
                    (playLayer->m_player1 && playLayer->m_player1->m_isDead) ||
                    playLayer->m_hasCompletedLevel ||
                    playLayer->m_levelEndAnimationStarted;
                if (down) {
                    if (!isAttemptEnded) {
                        container->onPlayerPush();
                    }
                } else {
                    container->onPlayerRelease();
                }
            }
        }
    }
};
