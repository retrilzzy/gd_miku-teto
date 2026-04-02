#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

namespace ZOrder {
constexpr int BehindHud = 1;
constexpr int AboveHud = 100;
constexpr int Sprite = -100;
}

enum class SpriteMode {
  Random,
  Alternating,
  MikuOnly,
  TetoOnly,
  LeftTeto,
  LeftMiku
};

struct HypeSettings {
  int64_t m_spacing;
  SpriteMode m_mode;
  double m_hypeStrength;
  int64_t m_spriteSize;
  int64_t m_margin;
  bool m_showTop;
  bool m_renderBehindHud;

  static HypeSettings load() {
    auto mod = geode::Mod::get();

    const std::string modeStr =
        mod->getSettingValue<std::string>("sprite-mode");

    SpriteMode mode = SpriteMode::Random;
    if (modeStr == "Miku Only")
      mode = SpriteMode::MikuOnly;
    else if (modeStr == "Teto Only")
      mode = SpriteMode::TetoOnly;
    else if (modeStr == "Left Teto / Right Miku")
      mode = SpriteMode::LeftTeto;
    else if (modeStr == "Left Miku / Right Teto")
      mode = SpriteMode::LeftMiku;
    else if (modeStr == "Alternating")
      mode = SpriteMode::Alternating;

    return {mod->getSettingValue<int64_t>("spacing"),
            mode,
            mod->getSettingValue<int64_t>("hype-strength") / 100.0,
            mod->getSettingValue<int64_t>("sprite-size"),
            mod->getSettingValue<int64_t>("margin"),
            mod->getSettingValue<bool>("show-top"),
            mod->getSettingValue<bool>("render-behind-hud")};
  }
};

class $modify(HypePlayLayer, PlayLayer) {
  struct Fields {
    CCNode *m_borderContainer = nullptr;
    bool m_renderBehindHud = true;
  };

  bool init(GJGameLevel *level, bool useReplay, bool dontCreateObjects) {
    if (!PlayLayer::init(level, useReplay, dontCreateObjects))
      return false;

    auto settings = HypeSettings::load();
    m_fields->m_renderBehindHud = settings.m_renderBehindHud;

    auto *container = CCNode::create();

    if (settings.m_renderBehindHud) {
      this->addChild(container, ZOrder::BehindHud);
    } else if (m_uiLayer) {
      m_uiLayer->addChild(container, ZOrder::AboveHud);
    }

    m_fields->m_borderContainer = container;
    this->createBorders(settings);

    return true;
  }

  void update(float dt) override {
    PlayLayer::update(dt);

    // Compensate for camera movement to keep the borders static on the screen
    if (m_fields->m_borderContainer && m_objectLayer &&
        m_fields->m_renderBehindHud) {
      m_fields->m_borderContainer->setPosition(-m_objectLayer->getPosition());
    }
  }

  void createBorders(const HypeSettings &settings) {
    auto winSize = geode::prelude::CCDirector::sharedDirector()->getWinSize();
    float margin = static_cast<float>(settings.m_margin);
    float spacing = std::max(10.f, static_cast<float>(settings.m_spacing));

    // Calculate the number of sprites and exact spacing
    int countX =
        std::max(1, static_cast<int>((winSize.width - margin * 2.f) / spacing));
    float spacingX = (winSize.width - margin * 2.f) / countX;

    for (int i = 0; i <= countX; ++i) {
      float x = margin + i * spacingX;
      addBorderSprite({x, margin}, i, settings);
      if (settings.m_showTop)
        addBorderSprite({x, winSize.height - margin}, i, settings);
    }

    int countY = std::max(
        1, static_cast<int>((winSize.height - margin * 2.f) / spacing));
    float spacingY = (winSize.height - margin * 2.f) / countY;

    for (int i = 1; i < countY; ++i) {
      float y = margin + i * spacingY;
      addBorderSprite({margin, y}, i, settings);
      addBorderSprite({winSize.width - margin, y}, i, settings);
    }
  }

  void addBorderSprite(geode::prelude::CCPoint pos, int index,
                       const HypeSettings &settings) {
    if (!m_fields->m_borderContainer)
      return;

    auto winSize = geode::prelude::CCDirector::sharedDirector()->getWinSize();

    bool isTeto = false;
    switch (settings.m_mode) {
    case SpriteMode::MikuOnly:
      isTeto = false;
      break;
    case SpriteMode::TetoOnly:
      isTeto = true;
      break;
    case SpriteMode::LeftTeto:
      isTeto = (pos.x < winSize.width / 2.f);
      break;
    case SpriteMode::LeftMiku:
      isTeto = (pos.x >= winSize.width / 2.f);
      break;
    case SpriteMode::Alternating:
      isTeto = (index % 2 == 0);
      break;
    case SpriteMode::Random:
      isTeto = (std::rand() % 2 == 0);
      break;
    }

    auto *sprite = geode::prelude::CCSprite::create(isTeto ? "teto.png"_spr
                                                           : "miku.png"_spr);
    if (!sprite)
      return;

    auto texSize = sprite->getContentSize();
    float sizeF = static_cast<float>(settings.m_spriteSize);

    // Base scale and random variance using fmod
    float baseScale = sizeF / std::max(texSize.width, texSize.height);
    float scale = baseScale + std::fmod(index * 0.05f, baseScale * 0.5f);

    sprite->setScale(scale);
    sprite->setRotation(std::fmod(index * 47.f, 360.f));
    sprite->setPosition(pos);

    // Make every 3rd sprite semi-transparent with additive blending
    if (index % 3 == 0) {
      sprite->setBlendFunc({GL_SRC_ALPHA, GL_ONE});
      sprite->setColor(isTeto ? geode::prelude::ccColor3B{255, 100, 100}
                              : geode::prelude::ccColor3B{100, 255, 220});
      sprite->setOpacity(200);
    }

    if (settings.m_hypeStrength > 0.0) {
      // Calculate rotation speed; even/odd indices rotate in opposite
      // directions
      float rotSpeed = (15.f + std::fmod(index * 4.f, 10.f)) *
                       static_cast<float>(settings.m_hypeStrength);
      if (index % 2 == 0)
        rotSpeed = -rotSpeed;

      // Time taken for the sprite to complete a full rotation
      float spinDuration = 15.f / std::max(0.1f, std::abs(rotSpeed));
      float spinAngle = rotSpeed > 0 ? 360.f : -360.f;
      sprite->runAction(geode::prelude::CCRepeatForever::create(
          geode::prelude::CCRotateBy::create(spinDuration, spinAngle)));

      // Pulsation animation
      float beatHalf = 0.43f / 2.f;
      float pulseScale =
          scale * (1.0f + 0.15f * static_cast<float>(settings.m_hypeStrength));
      sprite->runAction(geode::prelude::CCRepeatForever::create(
          geode::prelude::CCSequence::create(
              geode::prelude::CCScaleTo::create(beatHalf, pulseScale),
              geode::prelude::CCScaleTo::create(beatHalf, scale), nullptr)));
    }
    m_fields->m_borderContainer->addChild(sprite, ZOrder::Sprite);
  }
};
