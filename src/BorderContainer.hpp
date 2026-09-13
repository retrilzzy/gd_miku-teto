#pragma once

#include "Settings.hpp"

#include <Geode/DefaultInclude.hpp>
#include <cocos2d.h>
#include <vector>

struct BorderSpriteInfo {
    cocos2d::CCSprite* sprite = nullptr;
    float baseScale = 1.0f;
    float rotSpeed = 0.0f;
    int index = 0;
};

class BorderContainer final : public cocos2d::CCNode {
public:
    static BorderContainer* create(HypeSettings const& settings);

    bool init(HypeSettings const& settings);
    void update(float dt) override;

    void onPlayerPush();
    void onPlayerRelease();

private:
    void createBorders(HypeSettings const& settings);
    void addBorderSprite(
        cocos2d::CCPoint const& pos,
        int index,
        HypeSettings const& settings
    );

    bool m_clickSync = true;
    float m_spinSpeedMult = 1.0f;
    float m_pulseSpeedMult = 1.0f;
    float m_pulseStrengthMult = 1.0f;
    float m_clickSpinBoost = 3.5f;
    float m_clickPulseBoost = 1.75f;

    bool m_isHolding = false;
    float m_pressImpulse = 0.0f;
    float m_currentSpinMult = 1.0f;
    float m_currentPulseBoost = 0.0f;
    float m_time = 0.0f;
    std::vector<BorderSpriteInfo> m_sprites;
};
