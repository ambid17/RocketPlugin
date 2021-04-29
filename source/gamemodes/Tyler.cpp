// GameModes/Tag.cpp
// Tag others by bumping other and try to survive until the very end.
//
// Author:        Stanbroek
// Version:       0.1.1 24/12/20
// BMSDK version: 95

#include "Tyler.h"

#include <random>


/// <summary>Renders the available options for the game mode.</summary>
void Tyler::RenderOptions()
{
    ImGui::Checkbox("Enable Rumble Touches", &enableRumbleTouches);
    ImGui::Separator();

    ImGui::TextWrapped("Highlight Tagged Player:");
    bool tagOptionChanged = false;
    if (ImGui::RadioButton("None", taggedOption == TylerTaggedOption::NONE)) {
        tagOptionChanged = true;
        taggedOption = TylerTaggedOption::NONE;
    }
    if (ImGui::RadioButton("Unlimited Boost", taggedOption == TylerTaggedOption::UNLIMITED_BOOST)) {
        tagOptionChanged = true;
        taggedOption = TylerTaggedOption::UNLIMITED_BOOST;
    }
    if (tagOptionChanged) {
        removeHighlightsTaggedPlayer();
        highlightTaggedPlayer();
    }
    ImGui::Separator();

    ImGui::TextWrapped("Game mode suggested by: SimpleAOB");
}


/// <summary>Gets if the game mode is active.</summary>
/// <returns>Bool with if the game mode is active</returns>
bool Tyler::IsActive()
{
    return isActive;
}


/// <summary>Activates the game mode.</summary>
void Tyler::Activate(const bool active)
{
    if (active && !isActive) {
        tagRandomPlayer();
        HookEventWithCaller<ServerWrapper>("Function GameEvent_Soccar_TA.Active.Tick",
            [this](const ServerWrapper& caller, void* params, const std::string&) {
            onTick(caller, params);
        });
        HookEventWithCaller<CarWrapper>("Function TAGame.Car_TA.ApplyCarImpactForces",
            [this](const CarWrapper& caller, void* params, const std::string&) {
            onCarImpact(caller, params);
        });
        HookEventWithCaller<ActorWrapper>("Function TAGame.SpecialPickup_Targeted_TA.TryActivate",
            [this](const ActorWrapper& caller, void* params, const std::string&) {
            onRumbleItemActivated(caller, params);
        });
        HookEvent("Function TAGame.GameEvent_Soccar_TA.InitGame", [this](const std::string&) {
            tagRandomPlayer();
        });
    }
    else if (!active && isActive) {
        removeHighlightsTaggedPlayer();
        UnhookEvent("Function GameEvent_Soccar_TA.Active.Tick");
        UnhookEvent("Function TAGame.Car_TA.ApplyCarImpactForces");
        UnhookEvent("Function TAGame.SpecialPickup_Targeted_TA.TryActivate");
        UnhookEvent("Function TAGame.GameEvent_Soccar_TA.InitGame");
    }

    isActive = active;
}


/// <summary>Gets the game modes name.</summary>
/// <returns>The game modes name</returns>
std::string Tyler::GetGameModeName()
{
    return "Tyler";
}


/// <summary>Gets a random player from the players in the match.</summary>
void Tyler::tagRandomPlayer()
{
    LOG("tagging a player");

    std::vector<PriWrapper> players = rocketPlugin->getPlayers(false, true);

    if (players.size() <= 0) {
        tagged = emptyPlayer;
        ERROR_LOG("no players to tag");
        return;
    }

    static std::random_device rd;
    static std::default_random_engine generator(rd());
    const std::uniform_int_distribution<size_t> distribution(0, players.size() - 1);
    PriWrapper randomPlayer = players[distribution(generator)];
    tagged = randomPlayer.GetUniqueIdWrapper().GetUID();
    INFO_LOG(quote(randomPlayer.GetPlayerName().ToString()) + " is now tagged");
    addHighlight(randomPlayer);
}


/// <summary>Highlights the tagged player.</summary>
void Tyler::highlightTaggedPlayer() const
{
    std::vector<PriWrapper> players = rocketPlugin->getPlayers(true);
    for (PriWrapper player : players) {
        if (player.GetUniqueIdWrapper().GetUID() == tagged) {
            addHighlight(player);
        }
    }
}


/// <summary>Highlights the given player.</summary>
/// <param name="player">player to give the highlight to</param>
void Tyler::addHighlight(PriWrapper player) const
{
    CarWrapper car = player.GetCar();

    switch (taggedOption) {
    case TylerTaggedOption::NONE:
        break;
    case TylerTaggedOption::UNLIMITED_BOOST:
        if (!car.IsNull() && !car.GetBoostComponent().IsNull()) {
            car.GetBoostComponent().SetUnlimitedBoost2(true);
            car.SetbOverrideBoostOn(true);
        }
        break;
    case TylerTaggedOption::DIFFERENT_COLOR:
        break;
    }
}


/// <summary>Removes the highlights from the tagged player.</summary>
void Tyler::removeHighlightsTaggedPlayer() const
{
    std::vector<PriWrapper> players = rocketPlugin->getPlayers(true);
    for (PriWrapper player : players) {
        if (player.GetUniqueIdWrapper().GetUID() == tagged) {
            removeHighlights(player);
        }
    }
}


/// <summary>Removes the highlight from the given player.</summary>
/// <param name="player">player to remove the highlight from</param>
void Tyler::removeHighlights(PriWrapper player) const
{
    if (player.IsNull()) {
        ERROR_LOG("could not get the player");
        return;
    }

    // Unlimited Boost
    CarWrapper car = player.GetCar();
    if (car.IsNull()) {
        ERROR_LOG("could not get the car");
        return;
    }
    car.GetBoostComponent().SetUnlimitedBoost2(true);
    car.SetbOverrideBoostOn(false);
    // Different Color
    // TBD
}


/// <summary>Updates the game every game tick.</summary>
/// <remarks>Gets called on 'Function GameEvent_Soccar_TA.Active.Tick'.</remarks>
/// <param name="server"><see cref="ServerWrapper"/> instance of the server</param>
/// <param name="params">Delay since last update</param>
void Tyler::onTick(ServerWrapper server, void* params)
{
    if (server.IsNull()) {
        ERROR_LOG("could not get the server");
        return;
    }

    // Pick the first infected player at random
    if (tagged == emptyPlayer) {
        tagRandomPlayer();
        return;
    }

    // This is used because we have params that need used by the compiler
    const float dt = *static_cast<float*>(params);

    // Give all players infinite boost
    std::vector<PriWrapper> players = rocketPlugin->getPlayers(false, true);
    for (PriWrapper player : players) {
        // only do this if you're on the infected team
        CarWrapper car = player.GetCar();
        if (car.IsNull()) {
            ERROR_LOG("could not get the car");
            return;
        }
        car.GetBoostComponent().SetCurrentBoostAmount(100);
    }


    //std::vector<PriWrapper> players = rocketPlugin->getPlayers(true);
    //for (PriWrapper player : players) {
    //    if (player.GetUniqueIdWrapper().GetUID() == tagged) {
    //        //rocketPlugin->demolish(player);
    //        player.ServerChangeTeam(0);
    //    }
    //}
}


//void Tyler::onCarImpact(CarWrapper car, void* params)
//{
//
//}