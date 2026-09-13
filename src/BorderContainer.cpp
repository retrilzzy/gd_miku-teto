#include "BorderContainer.hpp"

#include <Geode/Geode.hpp>
#include <algorithm>
#include <cmath>
#include <cstdlib>

using namespace geode::prelude;

BorderContainer* BorderContainer::create(HypeSettings const& settings) {
    auto* ret = new BorderContainer();
    if (ret && ret->init(settings)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool BorderContainer::init(HypeSettings const& settings) {
    if (!CCNode::init()) return false;

    m_clickSync = settings.m_clickSync;
    m_spinSpeedMult = settings.m_spinSpeed;
    m_pulseSpeedMult = settings.m_pulseSpeed;
    m_pulseStrengthMult = settings.m_pulseStrength;
    m_clickSpinBoost = settings.m_clickSpinBoost;
    m_clickPulseBoost = settings.m_clickPulseBoost;

    this->createBorders(settings);
    this->scheduleUpdate();

    return true;
}

void BorderContainer::onPlayerPush() {
    m_isHolding = true;
    m_pressImpulse = 1.0f;
}

void BorderContainer::onPlayerRelease() {
    m_isHolding = false;
}

void BorderContainer::update(float dt) {
    CCNode::update(dt);
    if (m_sprites.empty()) return;

    m_time += dt * m_pulseSpeedMult;

    if (m_pressImpulse > 0.001f) {
        m_pressImpulse -= dt * 4.0f;
        if (m_pressImpulse < 0.0f) m_pressImpulse = 0.0f;
    }

    float targetSpinMult = 1.0f;
    float targetPulseBoost = 0.0f;

    if (m_clickSync) {
        if (m_isHolding) {
            targetSpinMult = m_clickSpinBoost;
            targetPulseBoost = 0.20f * m_clickPulseBoost;
        } else if (m_pressImpulse > 0.001f) {
            targetSpinMult =
                1.0f + m_pressImpulse * (m_clickSpinBoost - 1.0f);
            targetPulseBoost =
                m_pressImpulse * 0.25f * m_clickPulseBoost;
        }
    }

    float lerpRate = std::min(1.0f, dt * 14.0f);
    m_currentSpinMult += (targetSpinMult - m_currentSpinMult) * lerpRate;
    m_currentPulseBoost +=
        (targetPulseBoost - m_currentPulseBoost) * lerpRate;

    float pulseSine = std::sin(m_time * 4.0f);
    float idlePulse = 0.08f * m_pulseStrengthMult * (pulseSine + 1.0f);

    for (auto& info : m_sprites) {
        if (!info.sprite) continue;

        float currentRot = info.sprite->getRotation();
        float speed = info.rotSpeed * m_spinSpeedMult * m_currentSpinMult;
        info.sprite->setRotation(currentRot + speed * dt);

        float currentScale =
            info.baseScale * (1.0f + idlePulse + m_currentPulseBoost);
        info.sprite->setScale(currentScale);
    }
}

void BorderContainer::createBorders(HypeSettings const& settings) {
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    float margin = static_cast<float>(settings.m_margin);
    float spacing = std::max(10.0f, static_cast<float>(settings.m_spacing));

    int countX = std::max(
        1, static_cast<int>((winSize.width - margin * 2.0f) / spacing)
    );
    float spacingX = (winSize.width - margin * 2.0f) / countX;

    for (int i = 0; i <= countX; ++i) {
        float x = margin + i * spacingX;
        this->addBorderSprite({x, margin}, i, settings);
        if (settings.m_showTop) {
            this->addBorderSprite({x, winSize.height - margin}, i, settings);
        }
    }

    int countY = std::max(
        1, static_cast<int>((winSize.height - margin * 2.0f) / spacing)
    );
    float spacingY = (winSize.height - margin * 2.0f) / countY;

    for (int i = 1; i < countY; ++i) {
        float y = margin + i * spacingY;
        this->addBorderSprite({margin, y}, i, settings);
        this->addBorderSprite({winSize.width - margin, y}, i, settings);
    }
}

void BorderContainer::addBorderSprite(
    CCPoint const& pos,
    int index,
    HypeSettings const& settings
) {
    auto winSize = CCDirector::sharedDirector()->getWinSize();

    bool isTeto = false;
    switch (settings.m_mode) {
        case SpriteMode::MikuOnly:
            isTeto = false;
            break;
        case SpriteMode::TetoOnly:
            isTeto = true;
            break;
        case SpriteMode::LeftTeto:
            isTeto = (pos.x < winSize.width / 2.0f);
            break;
        case SpriteMode::LeftMiku:
            isTeto = (pos.x >= winSize.width / 2.0f);
            break;
        case SpriteMode::Alternating:
            isTeto = (index % 2 == 0);
            break;
        case SpriteMode::Random:
            isTeto = (std::rand() % 2 == 0);
            break;
    }

    auto* sprite = CCSprite::create(
        isTeto ? "teto.png"_spr : "miku.png"_spr
    );
    if (!sprite) return;

    auto texSize = sprite->getContentSize();
    float sizeF = static_cast<float>(settings.m_spriteSize);

    float baseScale = sizeF / std::max(texSize.width, texSize.height);
    float scaleVar = 0.75f + std::fmod(index * 0.17f, 0.40f);
    float scale = baseScale * scaleVar;

    sprite->setScale(scale);
    sprite->setRotation(std::fmod(index * 47.0f, 360.0f));
    sprite->setPosition(pos);

    if (index % 3 == 0) {
        sprite->setBlendFunc({GL_SRC_ALPHA, GL_ONE});
        sprite->setColor(
            isTeto ? ccColor3B{255, 100, 100} : ccColor3B{100, 255, 220}
        );
        sprite->setOpacity(200);
    }

    float rotSpeed = 30.0f + std::fmod(index * 9.0f, 25.0f);
    if (index % 2 == 0) rotSpeed = -rotSpeed;

    this->addChild(sprite, zorder::sprite);
    m_sprites.push_back({sprite, scale, rotSpeed, index});
}
