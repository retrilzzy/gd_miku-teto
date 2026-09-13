#pragma once

#include <Geode/loader/Mod.hpp>
#include <string>

namespace zorder {
constexpr int behindHud = 1;
constexpr int aboveHud = 100;
constexpr int sprite = -100;
}

enum class SpriteMode {
    Random,
    Alternating,
    MikuOnly,
    TetoOnly,
    LeftTeto,
    LeftMiku,
};

struct HypeSettings {
    int64_t m_spacing;
    SpriteMode m_mode;
    int64_t m_spriteSize;
    int64_t m_margin;
    bool m_showTop;
    bool m_renderBehindHud;
    bool m_clickSync;
    float m_spinSpeed;
    float m_pulseSpeed;
    float m_pulseStrength;
    float m_clickSpinBoost;
    float m_clickPulseBoost;

    static HypeSettings load() {
        auto* mod = geode::Mod::get();
        std::string const modeStr =
            mod->getSettingValue<std::string>("sprite-mode");

        SpriteMode mode = SpriteMode::Random;
        if (modeStr == "Miku Only") {
            mode = SpriteMode::MikuOnly;
        } else if (modeStr == "Teto Only") {
            mode = SpriteMode::TetoOnly;
        } else if (modeStr == "Left Teto / Right Miku") {
            mode = SpriteMode::LeftTeto;
        } else if (modeStr == "Left Miku / Right Teto") {
            mode = SpriteMode::LeftMiku;
        } else if (modeStr == "Alternating") {
            mode = SpriteMode::Alternating;
        }

        return {
            mod->getSettingValue<int64_t>("spacing"),
            mode,
            mod->getSettingValue<int64_t>("sprite-size"),
            mod->getSettingValue<int64_t>("margin"),
            mod->getSettingValue<bool>("show-top"),
            mod->getSettingValue<bool>("render-behind-hud"),
            mod->getSettingValue<bool>("click-sync"),
            static_cast<float>(
                mod->getSettingValue<int64_t>("spin-speed") / 100.0f
            ),
            static_cast<float>(
                mod->getSettingValue<int64_t>("pulse-speed") / 100.0f
            ),
            static_cast<float>(
                mod->getSettingValue<int64_t>("pulse-strength") / 100.0f
            ),
            static_cast<float>(
                mod->getSettingValue<int64_t>("click-spin-boost") / 100.0f
            ),
            static_cast<float>(
                mod->getSettingValue<int64_t>("click-pulse-boost") / 100.0f
            ),
        };
    }
};
