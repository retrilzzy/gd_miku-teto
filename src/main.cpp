#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

enum class SpriteMode {
  Random,
  Alternating,
  MikuOnly,
  TetoOnly,
  LeftTeto,
  LeftMiku
};

class $modify(HypePlayLayer, PlayLayer) {
  bool init(GJGameLevel *level, bool useReplay, bool dontCreateObjects) {
    if (!PlayLayer::init(level, useReplay, dontCreateObjects)) {
      return false;
    }

    auto winSize = CCDirector::sharedDirector()->getWinSize();

    // Retrieve mod settings
    int64_t spacing = Mod::get()->getSettingValue<int64_t>("spacing");
    std::string modeStr =
        Mod::get()->getSettingValue<std::string>("sprite-mode");
    double hypeStrength =
        Mod::get()->getSettingValue<int64_t>("hype-strength") / 100.0;
    int64_t spriteSize = Mod::get()->getSettingValue<int64_t>("sprite-size");
    int64_t margin = Mod::get()->getSettingValue<int64_t>("margin");

    float marginF = static_cast<float>(margin);
    float sizeF = static_cast<float>(spriteSize);

    // Parse sprite mode setting once (optimizes out string comparison in loop)
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

    // Lambda to add a border sprite at a specific position with
    // a sequence index
    auto addBorderSprite = [&](CCPoint pos, int index) {
      bool isTeto = false;

      switch (mode) {
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
      default:
        isTeto = (std::rand() % 2 == 0);
        break;
      }

      auto spriteName = isTeto ? "teto.png"_spr : "miku.png"_spr;
      auto sprite = CCSprite::create(spriteName);
      if (!sprite)
        return;

      // Set basic scale and rotation based on index and parameters
      auto texSize = sprite->getContentSize();
      float baseScale = sizeF / std::max(texSize.width, texSize.height);
      float scaleInfo = baseScale + std::fmod(index * 0.05f, baseScale * 0.5f);

      sprite->setScale(scaleInfo);
      sprite->setRotation(std::fmod(index * 47.f, 360.f));
      sprite->setPosition(pos);
      sprite->setZOrder(95);

      // Periodically add blend effects for specific sprites to create variety
      if (index % 3 == 0) {
        sprite->setBlendFunc({GL_SRC_ALPHA, GL_ONE});
        sprite->setColor(isTeto ? ccColor3B{255, 100, 100}
                                : ccColor3B{100, 255, 220});
        sprite->setOpacity(200);
      }

      // Apply animations scaling with hype strength
      if (hypeStrength > 0.0) {
        float rotSpeed = (15.f + std::fmod(index * 4.f, 10.f)) * hypeStrength;
        // Alternate rotation direction
        if (index % 2 == 0)
          rotSpeed = -rotSpeed;

        float duration = 15.f / std::max(0.1f, std::abs(rotSpeed));
        sprite->runAction(CCRepeatForever::create(
            CCRotateBy::create(duration, rotSpeed > 0 ? 360.f : -360.f)));

        // Pulsing animation
        float beatDur = 0.43f;
        float pulseTarget = scaleInfo * (1.0f + 0.15f * hypeStrength);

        auto scaleUp = CCScaleTo::create(beatDur / 2.f, pulseTarget);
        auto scaleDown = CCScaleTo::create(beatDur / 2.f, scaleInfo);
        sprite->runAction(CCRepeatForever::create(
            CCSequence::create(scaleUp, scaleDown, nullptr)));
      }

      this->m_uiLayer->addChild(sprite);
    };

    float actualSpacing = std::max(10.f, static_cast<float>(spacing));

    // Generate top and bottom borders
    int countX = std::max(
        1, static_cast<int>((winSize.width - marginF * 2.f) / actualSpacing));
    float realSpacingX =
        countX > 0 ? (winSize.width - marginF * 2.f) / countX : 0.f;

    for (int i = 0; i <= countX; i++) {
      float x = marginF + i * realSpacingX;
      addBorderSprite({x, marginF}, i);                  // Bottom
      addBorderSprite({x, winSize.height - marginF}, i); // Top
    }

    // Generate left and right borders
    int countY = std::max(
        1, static_cast<int>((winSize.height - marginF * 2.f) / actualSpacing));
    float realSpacingY =
        countY > 0 ? (winSize.height - marginF * 2.f) / countY : 0.f;

    for (int i = 1; i < countY; i++) {
      float y = marginF + i * realSpacingY;
      addBorderSprite({marginF, y}, i);                 // Left
      addBorderSprite({winSize.width - marginF, y}, i); // Right
    }

    return true;
  }
};