#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/CCDirector.hpp>
#include <Geode/binding/CCScheduler.hpp>

using namespace geode::prelude;

// ---------------------------------------------
// Holds the attempt-bucket counts for one level session.
// Buckets mirror the screenshot: 1, 2, 3, 4, 5-6, 7-8, 9-12, 13-15
// ---------------------------------------------
struct FEBuckets {
    int b1 = 0;
    int b2 = 0;
    int b3 = 0;
    int b4 = 0;
    int b5_6 = 0;
    int b7_8 = 0;
    int b9_12 = 0;
    int b13_15 = 0;

    void reset() {
        b1 = b2 = b3 = b4 = b5_6 = b7_8 = b9_12 = b13_15 = 0;
    }

    // Buckets an integer percent (0-100) into the matching range.
    // Percents above 15 are intentionally ignored (matches the
    // screenshot, which only tracks early-attempt death ranges).
    void add(int percent) {
        if (percent <= 0) return;
        if (percent == 1) b1++;
        else if (percent == 2) b2++;
        else if (percent == 3) b3++;
        else if (percent == 4) b4++;
        else if (percent >= 5 && percent <= 6) b5_6++;
        else if (percent >= 7 && percent <= 8) b7_8++;
        else if (percent >= 9 && percent <= 12) b9_12++;
        else if (percent >= 13 && percent <= 15) b13_15++;
    }
};

static FEBuckets g_buckets;

// ---------------------------------------------
// The on-screen overlay node. One instance is created per
// PlayLayer and destroyed when the layer is destroyed.
// ---------------------------------------------
class FEOverlay : public CCNode {
public:
    CCLabelBMFont* m_msLabel = nullptr;
    CCLabelBMFont* m_hzLabel = nullptr;
    CCLabelBMFont* m_bucketLabel = nullptr;
    CCLabelBMFont* m_frameLabel = nullptr;

    double m_lastFrameTime = 0.0;
    double m_smoothedDelta = 0.0;

    // The live frame counter: increments once per call to update(),
    // i.e. once per game frame/tick, since the level was (re)entered.
    long long m_frameCount = 0;

    static FEOverlay* create() {
        auto ret = new FEOverlay();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    bool init() override {
        if (!CCNode::init()) return false;

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        // Frametime readout, top-right (mirrors screenshot position)
        m_msLabel = CCLabelBMFont::create("0.000 MS", "bigFont.fnt");
        m_msLabel->setScale(0.45f);
        m_msLabel->setAnchorPoint({1.f, 1.f});
        m_msLabel->setPosition({winSize.width - 10.f, winSize.height - 10.f});
        m_msLabel->setColor({120, 255, 150});
        this->addChild(m_msLabel);

        m_hzLabel = CCLabelBMFont::create("0 Hz", "chatFont.fnt");
        m_hzLabel->setScale(0.5f);
        m_hzLabel->setAnchorPoint({1.f, 1.f});
        m_hzLabel->setPosition({winSize.width - 10.f, winSize.height - 30.f});
        this->addChild(m_hzLabel);

        // Attempt-bucket readout, top-left (mirrors screenshot position)
        m_bucketLabel = CCLabelBMFont::create("", "chatFont.fnt");
        m_bucketLabel->setScale(0.5f);
        m_bucketLabel->setAnchorPoint({0.f, 1.f});
        m_bucketLabel->setPosition({10.f, winSize.height - 10.f});
        m_bucketLabel->setAlignment(kCCTextAlignmentLeft);
        this->addChild(m_bucketLabel);

        // Live frame counter, bottom-left (mirrors the "48.84" timer
        // position in the screenshot, but counts frames instead of seconds)
        m_frameLabel = CCLabelBMFont::create("0", "bigFont.fnt");
        m_frameLabel->setScale(0.6f);
        m_frameLabel->setAnchorPoint({0.f, 0.f});
        m_frameLabel->setPosition({10.f, 10.f});
        this->addChild(m_frameLabel);

        this->refreshBuckets();
        this->scheduleUpdate();

        return true;
    }

    void update(float dt) override {
        // Smooth the delta slightly so the MS readout isn't jittery
        m_smoothedDelta = m_smoothedDelta * 0.9 + dt * 0.1;

        double ms = m_smoothedDelta * 1000.0;
        int hz = m_smoothedDelta > 0.0 ? static_cast<int>(round(1.0 / m_smoothedDelta)) : 0;

        m_msLabel->setString(fmt::format("{:.3f} MS", ms).c_str());
        m_hzLabel->setString(fmt::format("{} Hz", hz).c_str());

        // Tick the frame counter up by exactly 1 per update() call.
        // update() fires once per game frame, so this is a true
        // frame count, not a time-based estimate.
        m_frameCount++;
        m_frameLabel->setString(std::to_string(m_frameCount).c_str());
    }

    // Call this to zero the counter back to 0 (used on retry).
    void resetFrameCount() {
        m_frameCount = 0;
        if (m_frameLabel) {
            m_frameLabel->setString("0");
        }
    }

    void refreshBuckets() {
        if (!m_bucketLabel) return;
        std::string text = fmt::format(
            "13-15: {}\n9-12: {}\n7-8: {}\n5-6: {}\n4: {}\n3: {}\n2: {}\n1: {}",
            g_buckets.b13_15, g_buckets.b9_12, g_buckets.b7_8, g_buckets.b5_6,
            g_buckets.b4, g_buckets.b3, g_buckets.b2, g_buckets.b1
        );
        m_bucketLabel->setString(text.c_str());
    }
};

static FEOverlay* g_overlay = nullptr;

// ---------------------------------------------
// Hook PlayLayer to create/destroy the overlay and to
// bucket each death by the percent reached.
// ---------------------------------------------
class $modify(FEPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        // Fresh bucket counts each time a level is (re)entered,
        // matching Mega Hack's per-session behavior.
        g_buckets.reset();

        if (Mod::get()->getSettingValue<bool>("show-fps") ||
            Mod::get()->getSettingValue<bool>("show-buckets") ||
            Mod::get()->getSettingValue<bool>("show-framecount")) {
            g_overlay = FEOverlay::create();
            this->addChild(g_overlay, 1000);

            if (!Mod::get()->getSettingValue<bool>("show-framecount") && g_overlay->m_frameLabel) {
                g_overlay->m_frameLabel->setVisible(false);
            }
            if (!Mod::get()->getSettingValue<bool>("show-fps")) {
                if (g_overlay->m_msLabel) g_overlay->m_msLabel->setVisible(false);
                if (g_overlay->m_hzLabel) g_overlay->m_hzLabel->setVisible(false);
            }
            if (!Mod::get()->getSettingValue<bool>("show-buckets") && g_overlay->m_bucketLabel) {
                g_overlay->m_bucketLabel->setVisible(false);
            }
        }

        return true;
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        PlayLayer::destroyPlayer(player, object);

        // getCurrentPercent() returns a float 0-100; truncate to int
        // to match the whole-number bucket ranges in the screenshot.
        int percent = static_cast<int>(this->getCurrentPercent());
        g_buckets.add(percent);

        if (g_overlay) {
            g_overlay->refreshBuckets();
        }
    }

    void resetLevel() {
        PlayLayer::resetLevel();
        // Note: buckets are NOT reset here, intentionally — the
        // screenshot's counts persist across retries within one
        // session, only clearing on fresh level entry (see init above).

        // The frame counter DOES reset here — each new attempt starts
        // counting from frame 0, like a stopwatch restarting.
        if (g_overlay) {
            g_overlay->resetFrameCount();
        }
    }

    void onQuit() {
        g_overlay = nullptr;
        PlayLayer::onQuit();
    }
};
