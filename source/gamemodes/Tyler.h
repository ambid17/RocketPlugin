#pragma once
#include "../RocketPlugin.h"


class Tyler final : public RocketGameMode
{
public:
    explicit Tyler(RocketPlugin* rp) : RocketGameMode(rp) { _typeid = std::make_shared<std::type_index>(typeid(*this)); }

    void RenderOptions() override;
    bool IsActive() override;
    void Activate(bool active) override;
    std::string GetGameModeName() override;

private:
    void tagRandomPlayer();
    void highlightTaggedPlayer() const;
    void addHighlight(PriWrapper player) const;
    void removeHighlightsTaggedPlayer() const;
    void removeHighlights(PriWrapper player) const;
    void onTick(ServerWrapper server, void* params);
    void onCarImpact(CarWrapper, void*) const;
    void onRumbleItemActivated(ActorWrapper, void*) const;

    const unsigned long long emptyPlayer = static_cast<unsigned long long>(-1);

    bool enableRumbleTouches = false;
    unsigned long long tagged = emptyPlayer;

    enum class TylerTaggedOption
    {
        NONE,
        UNLIMITED_BOOST,
        DIFFERENT_COLOR
    };

    TylerTaggedOption taggedOption = TylerTaggedOption::NONE;
};
