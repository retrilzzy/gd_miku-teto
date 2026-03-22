#include <cmath>
#include <cstdlib>
#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(MyPlayLayer, PlayLayer) {
  bool init(GJGameLevel *level, bool useReplay, bool dontCreateObjects) {
    if (!PlayLayer::init(level, useReplay, dontCreateObjects)) {
      return false;
    }

    auto winSize = CCDirector::sharedDirector()->getWinSize();

    int64_t spacing = Mod::get()->getSettingValue<int64_t>("spacing");
    std::string charMode =
        Mod::get()->getSettingValue<std::string>("char-mode");
    double hypeStrength =
        Mod::get()->getSettingValue<int64_t>("hype-strength") / 100.0;
    int64_t spriteSize = Mod::get()->getSettingValue<int64_t>("sprite-size");
    int64_t margin = Mod::get()->getSettingValue<int64_t>("margin");

    float marginF = static_cast<float>(margin);
    float sizeF = static_cast<float>(spriteSize);

    auto addBorderSprite = [&](CCPoint pos, float index) {
      bool isTeto = false;

      if (charMode == "Miku Only") {
        isTeto = false;
      } else if (charMode == "Teto Only") {
        isTeto = true;
      } else if (charMode == "Left Teto / Right Miku") {
        isTeto = (pos.x < winSize.width / 2.f);
      } else if (charMode == "Left Miku / Right Teto") {
        isTeto = (pos.x >= winSize.width / 2.f);
      } else if (charMode == "Fully Random") {
        isTeto = (std::rand() % 2 == 0);
      } else {
        isTeto = (static_cast<int>(index) % 2 == 0);
      }

      auto spriteName = isTeto ? "teto.png"_spr : "miku.png"_spr;
      auto sprite = CCSprite::create(spriteName);
      if (!sprite)
        return;

      auto texSize = sprite->getContentSize();
      float baseScale = sizeF / std::max(texSize.width, texSize.height);
      float scaleInfo = baseScale + std::fmod(index * 0.05f, baseScale * 0.5f);
      sprite->setScale(scaleInfo);
      sprite->setRotation(std::fmod(index * 47.f, 360.f));
      sprite->setPosition(pos);
      sprite->setZOrder(95);

      if (static_cast<int>(index) % 3 == 0) {
        sprite->setBlendFunc({GL_SRC_ALPHA, GL_ONE});
        sprite->setColor(isTeto ? ccColor3B{255, 100, 100}
                                : ccColor3B{100, 255, 220});
        sprite->setOpacity(200);
      }

      if (hypeStrength > 0.0) {
        float rotSpeed = (15.f + std::fmod(index * 4.f, 10.f)) * hypeStrength;
        if (static_cast<int>(index) % 2 == 0)
          rotSpeed = -rotSpeed;
        float duration = 15.f / std::max(0.1f, std::abs(rotSpeed));
        sprite->runAction(CCRepeatForever::create(
            CCRotateBy::create(duration, rotSpeed > 0 ? 360.f : -360.f)));

        float beatDur = 0.43f;
        float pulseTarget = scaleInfo * (1.0f + 0.15f * hypeStrength);
        auto scaleUp = CCScaleTo::create(beatDur / 2.f, pulseTarget);
        auto scaleDown = CCScaleTo::create(beatDur / 2.f, scaleInfo);
        sprite->runAction(CCRepeatForever::create(
            CCSequence::create(scaleUp, scaleDown, nullptr)));
      }

      this->m_uiLayer->addChild(sprite);
    };

    float index = 0.f;
    float actualSpacing = std::max(10.f, static_cast<float>(spacing));

    int countX = std::max(
        1, static_cast<int>((winSize.width - marginF * 2.f) / actualSpacing));
    float realSpacingX =
        countX > 0 ? (winSize.width - marginF * 2.f) / countX : 0.f;

    for (int i = 0; i <= countX; i++) {
      float x = marginF + i * realSpacingX;
      addBorderSprite({x, marginF}, index++);
      addBorderSprite({x, winSize.height - marginF}, index++);
    }

    int countY = std::max(
        1, static_cast<int>((winSize.height - marginF * 2.f) / actualSpacing));
    float realSpacingY =
        countY > 0 ? (winSize.height - marginF * 2.f) / countY : 0.f;

    for (int i = 1; i < countY; i++) {
      float y = marginF + i * realSpacingY;
      addBorderSprite({marginF, y}, index++);
      addBorderSprite({winSize.width - marginF, y}, index++);
    }

    return true;
  }
};