/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
 
/* ScriptData
SDName: Scourge Invasion
SD%Complete: 90
SDComment:
SDCategory: Scourge Invasion
EndScriptData */

#include "../game/AI/ScriptDevAI/scripts/world/scourge_invasion/scourge_invasion_mgr.h"
#include "../game/AI/ScriptDevAI/scripts/world/scourge_invasion/scourge_invasion.h"
#include "../game/AI/ScriptDevAI/PreCompiledHeader.h"
#include "../game/Grids/GridNotifiers.h"
#include "../game/Grids/CellImpl.h"
#include "../game/Grids/GridNotifiersImpl.h"

/* ContentData
 npc_damaged_shard
 npc_cultist_engineer
 npc_necropolis
 npc_necropolis_controller
 npc_necrotic_shard
 npc_scourge_minion
 npc_scourge_rewards
 npc_scourge_emissary
 npc_scourge_messenger
EndContentData */

/*######
## npc_scourge_messenger
## Entry: 16359
######*/

struct npc_scourge_messengerAI : public ScriptedAI
{
    uint32 m_uiAnnounceTimer;
    std::vector<uint8> m_invadedZonesAnnounced;

    npc_scourge_messengerAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_invadedZonesAnnounced = { 0, 0, 0, 0, 0, 0 };

        Reset();
    }

    void Reset() override { ScheduleEvents(); }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (m_uiAnnounceTimer < uiDiff)
        {
            for (uint8 zone = 0; zone < 6; ++zone)
            {
                uint8 newVal = sScourgeInvasionMgr.GetAliveNecropolisCount(m_invadedZoneIds[zone]) > 0 ? 1 : 0;

                if (newVal != m_invadedZonesAnnounced[zone])
                {
                    if (m_invadedZonesAnnounced[zone] == 0)
                        DoScriptText(m_invadedZoneTextIds[zone], m_creature);

                    m_invadedZonesAnnounced[zone] = newVal;

                    m_uiAnnounceTimer = 2000;
                    return;
                }
            }

            m_uiAnnounceTimer = 10000;
        }
        else
            m_uiAnnounceTimer -= uiDiff;
    }

    void ScheduleEvents()
    {
        for (auto currentMessengerPosX : m_messengersNotAnnouncingPosX)
        {
            float x, y, z;
            m_creature->GetRespawnCoord(x, y, z);

            if (currentMessengerPosX == int32(x))
                return;
        }

        m_uiAnnounceTimer = 10000;
    }
};

static bool GossipHello_messenger(Player* player, Creature* creature)
{
    SendGossipHello_messenger(player, creature);
    return true;
}

static bool GossipSelect_messenger(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
{
    switch (action)
    {
    case GOSSIP_ACTION_INFO_DEF + 1:
    case GOSSIP_ACTION_INFO_DEF + 2:
    case GOSSIP_ACTION_INFO_DEF + 3:
    case GOSSIP_ACTION_INFO_DEF + 4:
    case GOSSIP_ACTION_INFO_DEF + 5:
    case GOSSIP_ACTION_INFO_DEF + 6:
    {
        SendGossipInvadedZones_messenger(player, creature, action - GOSSIP_ACTION_INFO_DEF);
        break;
    }
    case GOSSIP_ACTION_INFO_DEF + 7:
    {
        SendGossipBattlesWon_messenger(player, creature);
        break;
    }
    case GOSSIP_ACTION_INFO_DEF + 8:
    {
        SendGossipOnBackAction_messenger(player, creature);
        break;
    }
    }

    return true;
}

static void AddGoBackMenuItem_messenger(Player* player)
{
    player->ADD_GOSSIP_ITEM_ID(0, GOSSIP_GO_BACK, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 8);
}

static void SendGossipHello_messenger(Player* player, Creature* creature)
{
    player->PlayerTalkClass->GetGossipMenu().ClearMenu();
    player->ADD_GOSSIP_ITEM_ID(0, GOSSIP_OPTION_AZSHARA, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
    player->ADD_GOSSIP_ITEM_ID(0, GOSSIP_OPTION_BLASTED_LANDS, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
    player->ADD_GOSSIP_ITEM_ID(0, GOSSIP_OPTION_BURNING_STEPPES, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
    player->ADD_GOSSIP_ITEM_ID(0, GOSSIP_OPTION_TANARIS, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
    player->ADD_GOSSIP_ITEM_ID(0, GOSSIP_OPTION_EASTERN_PLAGUELANDS, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5);
    player->ADD_GOSSIP_ITEM_ID(0, GOSSIP_OPTION_WINTERSPRING, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 6);
    player->ADD_GOSSIP_ITEM_ID(0, GOSSIP_BATTLES_WON, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 7);
    player->PlayerTalkClass->SendGossipMenu(GOSSIP_TEXT_WHERE_BATTLING, creature->GetObjectGuid());
    player->PlayerTalkClass->GetGossipMenu().SetMenuId(GOSSIP_TEXT_WHERE_BATTLING);
}

static void SendGossipBattlesWon_messenger(Player* player, Creature* creature)
{
    player->PlayerTalkClass->GetGossipMenu().ClearMenu();
    AddGoBackMenuItem_messenger(player);
    UpdateGossipText_messenger(player, GOSSIP_TEXT_BATTLES_WON);
    player->PlayerTalkClass->SendGossipMenu(GOSSIP_TEXT_BATTLES_WON, creature->GetObjectGuid());
    player->PlayerTalkClass->GetGossipMenu().SetMenuId(GOSSIP_TEXT_BATTLES_WON);
}

static void SendGossipInvadedZones_messenger(Player* player, Creature* creature, uint32 action)
{
    player->PlayerTalkClass->GetGossipMenu().ClearMenu();

    if (sScourgeInvasionMgr.GetAliveNecropolisCount(m_invadedZoneIds[action - 1]) > 0)
    {
        uint32 textId = m_invadedZonesGossips[action - 1];

        AddGoBackMenuItem_messenger(player);
        UpdateGossipText_messenger(player, textId);

        player->PlayerTalkClass->SendGossipMenu(textId, creature->GetObjectGuid());
        player->PlayerTalkClass->GetGossipMenu().SetMenuId(textId);
    }
    else
    {
        AddGoBackMenuItem_messenger(player);

        player->PlayerTalkClass->SendGossipMenu(GOSSIP_TEXT_NOT_INVADED, creature->GetObjectGuid());
        player->PlayerTalkClass->GetGossipMenu().SetMenuId(GOSSIP_TEXT_NOT_INVADED);
    }
}

static void UpdateGossipText_messenger(Player* player, uint32 textId)
{
//    if (GossipText const* pGossip = sObjectMgr.GetGossipText(textId))
//    {
//        player->UpdateWorldStatesForGossipText(player, pGossip->Options[0].Text_0);
//        player->UpdateWorldStatesForGossipText(player, pGossip->Options[0].Text_1);
//    }
}

static void SendGossipOnBackAction_messenger(Player* player, Creature* creature)
{
    uint32 menuId = player->PlayerTalkClass->GetGossipMenu().GetMenuId();

    switch (menuId)
    {
        case GOSSIP_TEXT_BATTLES_WON:
        case GOSSIP_TEXT_AZSHARA_INVADED:
        case GOSSIP_TEXT_BLASTED_LANDS_INVADED:
        case GOSSIP_TEXT_BURNING_STEPPES_INVADED:
        case GOSSIP_TEXT_TANARIS_INVADED:
        case GOSSIP_TEXT_EASTERN_PLAGUELANDS_INVADED:
        case GOSSIP_TEXT_WINTERSPRING_INVADED:
        case GOSSIP_TEXT_NOT_INVADED:
        {
            SendGossipHello_messenger(player, creature);
            break;
        }
    }
}

/*######
## npc_scourge_emissary
## Entry: 16285
######*/

struct npc_scourge_emissaryAI : public ScriptedAI
{
    npc_scourge_emissaryAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    uint32 m_uiReactionTimer;
    uint32 m_uiTalkTimer;

    void Reset() override
    {
        m_uiReactionTimer = urand(10000, 90000);
        m_uiTalkTimer = urand(10000, 30000);
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (m_uiReactionTimer < uiDiff)
        {
            m_creature->HandleEmote(urand(0,1) ? EMOTE_CRY : EMOTE_LAUGH);
            m_uiReactionTimer = urand(10000, 90000);
        }
        else
            m_uiReactionTimer -= uiDiff;

        if (m_uiTalkTimer < uiDiff)
        {
            m_creature->HandleEmote(EMOTE_TALK);
            m_uiTalkTimer = urand(10000, 30000);
        }
        else
            m_uiTalkTimer -= uiDiff;
    }
};

static bool GossipHello_emissary(Player* player, Creature* creature)
{
    SendGossipHello_emissary(player, creature);
    return true;
}

static bool GossipSelect_emissary(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
{
    switch (action)
    {
    case GOSSIP_ACTION_INFO_DEF + 1:
    {
        SendGossipWhatsHappening_emissary(player, creature);
        break;
    }
    case GOSSIP_ACTION_INFO_DEF + 2:
    {
        SendGossipWhatCanIDo_emissary(player, creature);
        break;
    }
    case GOSSIP_ACTION_INFO_DEF + 3:
    {
        SendGossipWhereAreWeBattling_emissary(player, creature);
        break;
    }
    case GOSSIP_ACTION_INFO_DEF + 4:
    {
        SendGossipBattlesWon_emissary(player, creature);
        break;
    }
    case GOSSIP_ACTION_INFO_DEF + 5:
    case GOSSIP_ACTION_INFO_DEF + 6:
    case GOSSIP_ACTION_INFO_DEF + 7:
    case GOSSIP_ACTION_INFO_DEF + 8:
    case GOSSIP_ACTION_INFO_DEF + 9:
    case GOSSIP_ACTION_INFO_DEF + 10:
    {
        SendGossipInvadedZones_emissary(player, creature, action - GOSSIP_ACTION_INFO_DEF);
        break;
    }
    case GOSSIP_ACTION_INFO_DEF + 11:
    {
        SendGossipOnBackAction_emissary(player, creature);
        break;
    }
    }

    return true;
}

static void AddGoBackMenuItem_emissary(Player* player)
{
    player->ADD_GOSSIP_ITEM_ID(0, GOSSIP_GO_BACK, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 11);
}

static void SendGossipHello_emissary(Player* player, Creature* creature)
{
    player->PlayerTalkClass->GetGossipMenu().ClearMenu();
    player->ADD_GOSSIP_ITEM_ID(0, GOSSIP_WHATS_HAPPENING, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
    player->PlayerTalkClass->SendGossipMenu(GOSSIP_TEXT_EMISSARY_HELLO, creature->GetObjectGuid());
    player->PlayerTalkClass->GetGossipMenu().SetMenuId(GOSSIP_TEXT_EMISSARY_HELLO);
}

static void SendGossipWhatsHappening_emissary(Player* player, Creature* creature)
{
    player->PlayerTalkClass->GetGossipMenu().ClearMenu();
    player->ADD_GOSSIP_ITEM_ID(0, GOSSIP_WHAT_CAN_I_DO, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
    AddGoBackMenuItem_emissary(player);
    player->PlayerTalkClass->SendGossipMenu(GOSSIP_TEXT_WHAT_IS_HAPPENING, creature->GetObjectGuid());
    player->PlayerTalkClass->GetGossipMenu().SetMenuId(GOSSIP_TEXT_WHAT_IS_HAPPENING);
}

static void SendGossipWhatCanIDo_emissary(Player* player, Creature* creature)
{
    player->PlayerTalkClass->GetGossipMenu().ClearMenu();
    player->ADD_GOSSIP_ITEM_ID(0, GOSSIP_WHERE_BATTLING, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
    player->ADD_GOSSIP_ITEM_ID(0, GOSSIP_BATTLES_WON, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
    AddGoBackMenuItem_emissary(player);
    player->PlayerTalkClass->SendGossipMenu(GOSSIP_TEXT_WHAT_CAN_I_DO, creature->GetObjectGuid());
    player->PlayerTalkClass->GetGossipMenu().SetMenuId(GOSSIP_TEXT_WHAT_CAN_I_DO);
}

static void SendGossipWhereAreWeBattling_emissary(Player* player, Creature* creature)
{
    player->PlayerTalkClass->GetGossipMenu().ClearMenu();
    player->ADD_GOSSIP_ITEM_ID(0, GOSSIP_OPTION_AZSHARA, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5);
    player->ADD_GOSSIP_ITEM_ID(0, GOSSIP_OPTION_BLASTED_LANDS, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 6);
    player->ADD_GOSSIP_ITEM_ID(0, GOSSIP_OPTION_BURNING_STEPPES, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 7);
    player->ADD_GOSSIP_ITEM_ID(0, GOSSIP_OPTION_TANARIS, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 8);
    player->ADD_GOSSIP_ITEM_ID(0, GOSSIP_OPTION_EASTERN_PLAGUELANDS, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9);
    player->ADD_GOSSIP_ITEM_ID(0, GOSSIP_OPTION_WINTERSPRING, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 10);
    AddGoBackMenuItem_emissary(player);
    player->PlayerTalkClass->SendGossipMenu(GOSSIP_TEXT_WHERE_BATTLING, creature->GetObjectGuid());
    player->PlayerTalkClass->GetGossipMenu().SetMenuId(GOSSIP_TEXT_WHERE_BATTLING);
}

static void SendGossipBattlesWon_emissary(Player* player, Creature* creature)
{
    player->PlayerTalkClass->GetGossipMenu().ClearMenu();
    AddGoBackMenuItem_emissary(player);
    UpdateGossipText_emissary(player, GOSSIP_TEXT_BATTLES_WON);
    player->PlayerTalkClass->SendGossipMenu(GOSSIP_TEXT_BATTLES_WON, creature->GetObjectGuid());
    player->PlayerTalkClass->GetGossipMenu().SetMenuId(GOSSIP_TEXT_BATTLES_WON);
}

static void SendGossipInvadedZones_emissary(Player* player, Creature* creature, uint32 action)
{
    player->PlayerTalkClass->GetGossipMenu().ClearMenu();

    if (sScourgeInvasionMgr.GetAliveNecropolisCount(m_invadedZoneIds[action - 5]) > 0)
    {
        uint32 textId = m_invadedZonesGossips[action - 5];

        player->ADD_GOSSIP_ITEM_ID(0, GOSSIP_GO_BACK, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 11);
        UpdateGossipText_emissary(player, textId);

        player->PlayerTalkClass->SendGossipMenu(textId, creature->GetObjectGuid());
        player->PlayerTalkClass->GetGossipMenu().SetMenuId(textId);
    }
    else
    {
        player->ADD_GOSSIP_ITEM_ID(0, GOSSIP_GO_BACK, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 11);

        player->PlayerTalkClass->SendGossipMenu(GOSSIP_TEXT_NOT_INVADED, creature->GetObjectGuid());
        player->PlayerTalkClass->GetGossipMenu().SetMenuId(GOSSIP_TEXT_NOT_INVADED);
    }
}

static void SendGossipOnBackAction_emissary(Player* player, Creature* creature)
{
    uint32 menuId = player->PlayerTalkClass->GetGossipMenu().GetMenuId();

    switch (menuId)
    {
    case GOSSIP_TEXT_WHAT_IS_HAPPENING:
    {
        SendGossipHello_emissary(player, creature);
        break;
    }
    case GOSSIP_TEXT_WHAT_CAN_I_DO:
    {
        SendGossipWhatsHappening_emissary(player, creature);
        break;
    }
    case GOSSIP_TEXT_WHERE_BATTLING:
    case GOSSIP_TEXT_BATTLES_WON:
    {
        SendGossipWhatCanIDo_emissary(player, creature);
        break;
    }
    case GOSSIP_TEXT_AZSHARA_INVADED:
    case GOSSIP_TEXT_BLASTED_LANDS_INVADED:
    case GOSSIP_TEXT_BURNING_STEPPES_INVADED:
    case GOSSIP_TEXT_TANARIS_INVADED:
    case GOSSIP_TEXT_EASTERN_PLAGUELANDS_INVADED:
    case GOSSIP_TEXT_WINTERSPRING_INVADED:
    case GOSSIP_TEXT_NOT_INVADED:
    {
        SendGossipWhereAreWeBattling_emissary(player, creature);
        break;
    }
    }
}

static void UpdateGossipText_emissary(Player* player, uint32 textId)
{
//    if (GossipText const* pGossip = sObjectMgr.GetGossipText(textId))
//    {
//        player->UpdateWorldStatesForGossipText(player, pGossip->Options[0].Text_0);
//        player->UpdateWorldStatesForGossipText(player, pGossip->Options[0].Text_1);
//    }
}

/*######
## npc_scourge_rewards
## Entry: 16384 16435 16436 16395 16433 16434
######*/

struct npc_scourge_rewardsAI : public ScriptedAI
{
    uint32 m_visibilityTimer;

    npc_scourge_rewardsAI(Creature* pCreature) : ScriptedAI(pCreature), m_visibilityTimer(60 * IN_MILLISECONDS) { Reset(); }

    void Reset() override { SetMounts(); }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (m_creature->GetVisibility() != VISIBILITY_ON)
        {
            if (m_visibilityTimer <= uiDiff)
            {
                uint32 deadNecros = sScourgeInvasionMgr.GetDeadNecropolisCount();
                uint32 requiredAmount = 0;
                switch (m_creature->GetEntry())
                {
                    case NPC_AD_PALADIN:
                    case NPC_AD_INITIATE:
                    {
                        requiredAmount = REWARD_STAGE_1;
                        break;
                    }
                    case NPC_AD_CRUSADER:
                    case NPC_AD_CLERIC:
                    {
                        requiredAmount = REWARD_STAGE_2;
                        break;
                    }
                    case NPC_AD_CHAMPION:
                    case NPC_AD_PRIEST:
                    {
                        requiredAmount = REWARD_STAGE_3;
                        break;
                    }
                }
    
                if (deadNecros >= requiredAmount)
                    m_creature->SetVisibility(VISIBILITY_ON);
            }
            else
                m_visibilityTimer -= uiDiff;
        }

        if (!m_creature->getVictim())
            return;

        DoMeleeAttackIfReady();
    }

    void SetMounts()
    {
        uint32 spellId;

        switch (m_creature->GetEntry())
        {
            case NPC_AD_PALADIN:
                spellId = SPELL_SUMMON_AD_PALADIN_MOUNT;
                break;
            case NPC_AD_INITIATE:
                spellId = SPELL_SUMMON_AD_INITIATE_MOUNT;
                break;
            case NPC_AD_CRUSADER:
                spellId = SPELL_SUMMON_AD_CRUSADER_MOUNT;
                break;
            case NPC_AD_CLERIC:
                spellId = SPELL_SUMMON_AD_CLERIC_MOUNT;
                break;
            case NPC_AD_CHAMPION:
                spellId = SPELL_SUMMON_AD_CHAMPION_MOUNT;
                break;
            case NPC_AD_PRIEST:
                spellId = SPELL_SUMMON_AD_PRIEST_MOUNT;
                break;
            default:
                return;
        }

        DoCastSpellIfCan(m_creature, spellId, TRIGGERED_FULL_MASK);
    }
};

static bool GossipHello_rewards(Player* player, Creature* creature)
{
    bool isRewardNpcHorde;

    switch (creature->GetEntry())
    {
    case NPC_AD_INITIATE:
    case NPC_AD_CLERIC:
    case NPC_AD_PRIEST:
        isRewardNpcHorde = true;
        break;
    default:
        isRewardNpcHorde = false;
    }

    uint32 itemId = 0;
    switch (creature->GetEntry())
    {
    case NPC_AD_PALADIN:
    case NPC_AD_INITIATE:
    {
        itemId = ITEM_LESSER_MARK_OF_THE_DAWN;
        break;
    }
    case NPC_AD_CRUSADER:
    case NPC_AD_CLERIC:
    {
        itemId = ITEM_MARK_OF_THE_DAWN;
        break;
    }
    case NPC_AD_CHAMPION:
    case NPC_AD_PRIEST:
    {
        itemId = ITEM_GREATER_MARK_OF_THE_DAWN;
        break;
    }
    default:
        return true;
    }

    SendGossip_rewards(player, creature, !player->HasItemCount(itemId, 3, true), isRewardNpcHorde);

    return true;
}

static void SendGossip_rewards(Player* player, Creature* creature, bool playerCanReceiveMarks, bool isRewardNpcHorde)
{
    player->PlayerTalkClass->GetQuestMenu().ClearMenu();
    player->PlayerTalkClass->GetGossipMenu().ClearMenu();

    if (isRewardNpcHorde && player->GetTeam() == HORDE && playerCanReceiveMarks)
        player->ADD_GOSSIP_ITEM_ID(0, GOSSIP_NPC_REWARDS, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    if (!isRewardNpcHorde && player->GetTeam() == ALLIANCE && playerCanReceiveMarks)
        player->ADD_GOSSIP_ITEM_ID(0, GOSSIP_NPC_REWARDS, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    if (isRewardNpcHorde && player->GetTeam() == ALLIANCE)
        player->PlayerTalkClass->SendGossipMenu(GOSSIP_TEXT_IF_ALLIANCE, creature->GetObjectGuid());
    else if (!isRewardNpcHorde && player->GetTeam() == HORDE)
        player->PlayerTalkClass->SendGossipMenu(GOSSIP_TEXT_IF_HORDE, creature->GetObjectGuid());
    else
        player->PlayerTalkClass->SendGossipMenu(GOSSIP_TEXT_IF_SAME_FACTION, creature->GetObjectGuid());
}

static bool GossipSelect_rewards(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
{
    switch (action)
    {
        case GOSSIP_ACTION_INFO_DEF + 1:
        {
            uint32 rewardSpell;

            switch (creature->GetEntry())
            {
            case NPC_AD_PALADIN:
            case NPC_AD_INITIATE:
            {
                rewardSpell = SPELL_CREATE_LESSER_MARK_OF_THE_DAWN;
                break;
            }
            case NPC_AD_CRUSADER:
            case NPC_AD_CLERIC:
            {
                rewardSpell = SPELL_CREATE_MARK_OF_THE_DAWN;
                break;
            }
            case NPC_AD_CHAMPION:
            case NPC_AD_PRIEST:
            {
                rewardSpell = SPELL_CREATE_GREATER_MARK_OF_THE_DAWN;
                break;
            }
            default:
                return true;
            }

            player->PlayerTalkClass->CloseGossip();
            creature->AI->DoCastSpellIfCan(player, rewardSpell);
            break;
        }
    }

    return true;
}

/*######
## npc_scourge_minion
## Entry: 16141 16299 16298 14697 16379 16380 16143
######*/

struct npc_scourge_minionAI : public ScriptedAI
{
    uint32 m_entry;
    bool m_enraged;
    ObjectGuid m_damagedShardObjectGuid;

    npc_scourge_minionAI(Creature* pCreature) : ScriptedAI(pCreature), m_uiBoneShardsTimer(15000)
    {
        m_entry = m_creature->GetEntry();
        m_enraged = false;
        m_creature->CastSpell(m_creature, SPELL_MINIONS_SPAWN_VISUAL, TRIGGERED_FULL_MASK);

        if (m_entry == NPC_SHADOW_OF_DOOM)
            if (Creature* shard = GetClosestCreatureWithEntry(m_creature, NPC_DAMAGED_NECROTIC_SHARD, 100.0f))
                m_damagedShardObjectGuid = shard->GetObjectGuid();

        Reset();
    }

    uint32 m_uiBoneShardsTimer;
    uint32 m_uiInfectedBiteTimer;
    uint32 m_uiSunderArmorTimer;
    uint32 m_uiDemoralizingShoutTimer;
    uint32 m_uiStrikeTimer;
    uint32 m_uiRibbonOfSoulsTimer;
    uint32 m_uiStrongCleaveTimer;
    uint32 m_uiHamstringTimer;
    uint32 m_uiArcaneBoltTimer;
    uint32 m_uiFearTimer;
    uint32 m_uiMindFlayTimer;
    uint32 m_uiDarkStrikeTimer;

    void Reset() override
    {
        switch (m_entry)
        {
            case NPC_GHOUL_BERSERKER:
            {
                m_uiInfectedBiteTimer = urand(10000, 20000);
                m_enraged = false;
                break;
            }
            case NPC_SKELETAL_SHOCKTROOPER:
            {
                m_uiSunderArmorTimer = urand(15000, 25000);
                m_uiBoneShardsTimer = 15000;
                break;
            }
            case NPC_SPECTRAL_SOLDIER:
            {
                m_uiDemoralizingShoutTimer = urand(1000, 25000);
                m_uiStrikeTimer = urand(8000, 15000);
                break;
            }
            case NPC_RARE_SPIRIT_OF_THE_DAMNED:
            {
                m_uiRibbonOfSoulsTimer = 1000;
                break;
            }
            case NPC_RARE_LUMBERING_HORROR:
            {
                m_uiStrongCleaveTimer = urand(10000, 15000);
                m_uiHamstringTimer = urand(8000, 15000);
                break;
            }
            case NPC_RARE_BONE_WITCH:
            {
                // SetRangedMovement(5.0f, 30.0f, RANGED_NONE);
                m_uiArcaneBoltTimer = 3000;
                m_uiBoneShardsTimer = 15000;
                break;
            }
            case NPC_SHADOW_OF_DOOM:
            {
                m_uiFearTimer = urand(7000, 15000);
                m_uiMindFlayTimer = urand(9000, 18000);
                m_uiDarkStrikeTimer = urand(8000, 14000);
                break;
            }
        }
    }

    void JustRespawned() override
    {
        
        DoCastSpellIfCan(m_creature, SPELL_MINIONS_SPAWN_VISUAL, TRIGGERED_FULL_MASK);

        if (m_entry == NPC_SKELETAL_SHOCKTROOPER || m_entry == NPC_RARE_BONE_WITCH)
            DoCastSpellIfCan(m_creature, SPELL_BONE_SHARDS, TRIGGERED_NONE);

        Reset();
    }

    void JustDied(Unit* /*killer*/) override
    {
        switch (m_entry)
        {
            case NPC_SHADOW_OF_DOOM:
            {
                if (Creature* shard = m_creature->GetMap()->GetCreature(m_damagedShardObjectGuid))
                    DoCastSpellIfCan(shard, SPELL_MINIONS_SPAWN_VISUAL, TRIGGERED_FULL_MASK);

                break;
            }
            case NPC_RARE_BONE_WITCH:
            case NPC_RARE_LUMBERING_HORROR:
            case NPC_RARE_SPIRIT_OF_THE_DAMNED:
            {
                if (Creature* necroticShard = GetClosestCreatureWithEntry(m_creature, NPC_NECROTIC_SHARD, 100.0f, true))
                {
                    necroticShard->AI->DoCastSpellIfCan(necroticShard, SPELL_DISRUPT_CULTIST_TRANCE_VISUAL, TRIGGERED_FULL_MASK);
                    necroticShard->AI->DoCastSpellIfCan(necroticShard, SPELL_DAMAGE_CRYSTAL, TRIGGERED_FULL_MASK);
                }
                break;
            }
            case NPC_GHOUL_BERSERKER:
            case NPC_SKELETAL_SHOCKTROOPER:
            case NPC_SPECTRAL_SOLDIER:
            {
                if (Creature* necroticShard = GetClosestCreatureWithEntry(m_creature, NPC_NECROTIC_SHARD, 100.0f, true))
                    DoCastSpellIfCan(necroticShard, SPELL_ZAP_CRYSTAL, TRIGGERED_FULL_MASK);

                break;
            }
        }
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (m_entry == NPC_RARE_BONE_WITCH || m_entry == NPC_SKELETAL_SHOCKTROOPER)
        {
            if (m_uiBoneShardsTimer < uiDiff)
            {
                DoCastSpellIfCan(m_creature, SPELL_BONE_SHARDS);
                m_uiBoneShardsTimer = 10000;
            }
            else
                m_uiBoneShardsTimer -= uiDiff;
        }

        // Check if we have a valid target, otherwise do nothing
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Check if we are casting / channelling, return to not interrupt process and pause CDs
        if (m_creature->IsNonMeleeSpellCasted(false))
            return;

        switch (m_entry)
        {
            case NPC_GHOUL_BERSERKER:
            {
                if (!m_enraged && m_creature->GetHealthPercent() <= 50.0f)
                {
                    if (DoCastSpellIfCan(m_creature, SPELL_GHOUL_ENRAGE, TRIGGERED_FULL_MASK) == CAST_OK)
                        m_enraged = true;
                }

                if (m_uiInfectedBiteTimer < uiDiff)
                {
                    if (!m_creature->getVictim()->HasAura(SPELL_INFECTED_BITE))
                        if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_INFECTED_BITE) == CAST_OK)
                            m_uiInfectedBiteTimer = urand(10000, 20000);
                }
                else
                    m_uiInfectedBiteTimer -= uiDiff;

                break;
            }
            case NPC_SKELETAL_SHOCKTROOPER:
            {
                if (m_uiSunderArmorTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_SUNDER_ARMOR) == CAST_OK)
                        m_uiSunderArmorTimer = urand(15000, 25000);
                }
                else
                    m_uiSunderArmorTimer -= uiDiff;
                
                break;
            }
            case NPC_SPECTRAL_SOLDIER:
            {
                if (m_uiDemoralizingShoutTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature, SPELL_DEMORALIZING_SHOUT, TRIGGERED_NONE) == CAST_OK)
                        m_uiDemoralizingShoutTimer = urand(10000, 25000);
                }
                else
                    m_uiDemoralizingShoutTimer -= uiDiff;

                if (m_uiStrikeTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_STRIKE) == CAST_OK)
                        m_uiStrikeTimer = urand(8000, 15000);
                }
                else
                    m_uiStrikeTimer -= uiDiff;

                break;
            }
            case NPC_RARE_SPIRIT_OF_THE_DAMNED:
            {
                if (m_uiRibbonOfSoulsTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_RIBBON_OF_SOULS) == CAST_OK)
                        m_uiRibbonOfSoulsTimer = 1000;
                }
                else
                    m_uiRibbonOfSoulsTimer -= uiDiff;

                break;
            }
            case NPC_RARE_BONE_WITCH:
            {
                if (m_uiArcaneBoltTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_ARCANE_BOLT) == CAST_OK)
                        m_uiArcaneBoltTimer = 3000;
                }
                else
                    m_uiArcaneBoltTimer -= uiDiff;

                break;
            }
            case NPC_RARE_LUMBERING_HORROR:
            {
                if (m_uiStrongCleaveTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_STRONG_CLEAVE) == CAST_OK)
                        m_uiStrongCleaveTimer = urand(10000, 15000);
                }
                else
                    m_uiStrongCleaveTimer -= uiDiff;

                if (m_uiHamstringTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_HAMSTRING) == CAST_OK)
                        m_uiStrongCleaveTimer = urand(8000, 15000);
                }
                else
                    m_uiHamstringTimer -= uiDiff;

                break;
            }
            case NPC_SHADOW_OF_DOOM:
            {
                if (m_uiFearTimer < uiDiff)
                {
                    if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, SPELL_FEAR, SELECT_FLAG_PLAYER))
                        if (DoCastSpellIfCan(target, SPELL_FEAR) == CAST_OK)
                            m_uiFearTimer = urand(7000, 15000);
                }
                else
                    m_uiFearTimer -= uiDiff;

                if (m_uiMindFlayTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_MINDFLAY) == CAST_OK)
                        m_uiMindFlayTimer = urand(9000, 18000);
                }
                else
                    m_uiMindFlayTimer -= uiDiff;

                if (m_uiDarkStrikeTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_DARK_STRIKE) == CAST_OK)
                        m_uiDarkStrikeTimer = urand(8000, 14000);
                }
                else
                    m_uiDarkStrikeTimer -= uiDiff;

                break;
            }
        }

        // Check if we are casting / channelling, return to not interrupt process and pause CDs
        if (m_creature->IsNonMeleeSpellCasted(false))
            return;

        DoMeleeAttackIfReady();
    }
};

/*######
## npc_necrotic_shard
## Entry: 16136
######*/

struct npc_necrotic_shardAI : public ScriptedAI
{
    npc_necrotic_shardAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    uint32 m_uiCampToNecroVisualTimer;
    uint32 m_uiScourgeStrikeTimer;

    void Reset() override
    {
        m_uiCampToNecroVisualTimer = 35000;
        m_uiScourgeStrikeTimer = 7000;
    }

    void JustDied(Unit* /*killer*/) override
    {
        DoCastSpellIfCan(m_creature, SPELL_SUMMON_CRYSTAL_CORPSE, TRIGGERED_FULL_MASK);
        SummonCultists();
    }

    void SpellHit(Unit* /*caster*/, const SpellEntry* spell) override
    {
        switch (spell->Id)
        {
            case SPELL_NECROPOLIS_TO_CAMPS_VISUAL:
                DoCastSpellIfCan(m_creature, SPELL_COM_CAMP_RECEIVE_VISUAL, TRIGGERED_FULL_MASK);
                break;
        }
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (m_uiCampToNecroVisualTimer < uiDiff)
        {
            if (Creature* target = GetClosestCreatureWithEntry(m_creature, NPC_NECROPOLIS, 300))
                if (DoCastSpellIfCan(target, SPELL_CAMP_TO_NECROPOLIS_VISUAL, TRIGGERED_FULL_MASK) == CAST_OK)
                    m_uiCampToNecroVisualTimer = 35000;
        }
        else
            m_uiCampToNecroVisualTimer -= uiDiff;

        if (m_uiScourgeStrikeTimer < uiDiff)
        {
            if (Unit* target = SelectScourgeStrikeTarget())
                if (DoCastSpellIfCan(target, SPELL_SCOURGE_STRIKE, TRIGGERED_FULL_MASK) == CAST_OK)
                    m_uiScourgeStrikeTimer = 7000;
        }
        else
            m_uiScourgeStrikeTimer -= uiDiff;
    }

    Unit* SelectScourgeStrikeTarget()
    {
        std::list<Unit*> targets;

        MaNGOS::AnyUnfriendlyUnitInObjectRangeCheck check(m_creature, 100.0f);
        MaNGOS::UnitListSearcher<MaNGOS::AnyUnfriendlyUnitInObjectRangeCheck>  searcher(targets, check);
        Cell::VisitGridObjects(m_creature, searcher, 100.0f);

        targets.remove_if([](Unit* unit)
        {
            // Ignore other Scourge creatures
            if (unit->getFaction() == FACTION_UNDEAD_SCOURGE)
                return true;

            // Ignore player and his companions
            if (unit->GetBeneficiaryPlayer())
                return true;

            // Ignore spirit healers
            if (unit->isSpiritHealer())
                return true;

            return false;
        });

        if (targets.empty())
            return nullptr;

        return *(targets.begin());;
    }

    void SummonCultists()
    {
        float currentAngle = 0;

        for (uint32 i = 0; i < 4; ++i)
        {
            currentAngle += 2 * M_PI_F / 4.0f;

            float fX, fY, fZ;
            m_creature->GetNearPoint(m_creature, fX, fY, fZ, 0, 6.9f, currentAngle);
            
            if (Creature* cultist = m_creature->SummonCreature(NPC_CULTIST_ENGINEER, fX, fY, fZ, 0, TEMPSPAWN_DEAD_DESPAWN, 60000))
                cultist->SetFacingToObject(m_creature);
        }
    }
};

/*######
## npc_necropolis_controller
## Entry: 16214
######*/

struct npc_necropolis_controllerAI : public ScriptedAI
{
    uint32 m_killedMinionsCounter;
    uint32 m_requiredMinionsForRare;
    uint32 m_zoneId;

    uint8 m_campType;
    uint8 m_minionsAmount;

    float m_minionsMinSpawnDistance;
    float m_minionsMaxSpawnDistance;

    bool m_isCampDestroyed;

    std::vector<MinionToRespawn> m_minionsRespawnQueue;
    std::vector<MinionToSpawn> m_minionsSpawnQueue;

    npc_necropolis_controllerAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_killedMinionsCounter = 0;
        m_requiredMinionsForRare = urand(REQUIRED_DEAD_MINIONS_MIN, REQUIRED_DEAD_MINIONS_MAX);
        m_zoneId = m_creature->GetZoneId();

        m_campType = 0;
        m_minionsAmount = 50;

        m_minionsMinSpawnDistance = 10.0f;
        m_minionsMaxSpawnDistance = 40.0f;

        m_isCampDestroyed = true;

        switch (m_zoneId)
        {
            case ZONE_ELWYNN:
            case ZONE_MULGORE:
            case ZONE_DUN_MOROGH:
            case ZONE_TIRISFAL_GLADES:
            case ZONE_DUROTAR:
            case ZONE_TELDRASSIL:
                m_minionsAmount = 21;
                m_minionsMinSpawnDistance = 6.0f;
                m_minionsMaxSpawnDistance = 30.0f;
                m_isCampDestroyed = false;
                SetMinionsTypes();
                break;
        }

        Reset();
    }

    void Reset() override {}

    void SpellHit(Unit* /*caster*/, const SpellEntry* spell) override
    {
        if (spell->Id != SPELL_ACTIVATE_CAMP_VISUAL)
            return;

        m_minionsSpawnQueue.clear();
        m_minionsRespawnQueue.clear();

        SetCampGOsVisibility(true);
        m_isCampDestroyed = false;

        SpawnNecroticShard();
        SetMinionsTypes();
    }

    void SummonedCreatureJustDied(Creature* summoned) override
    {
        if (m_isCampDestroyed)
            return;

        switch (summoned->GetEntry())
        {
            case NPC_GHOUL_BERSERKER:
            case NPC_SKELETAL_SHOCKTROOPER:
            case NPC_SPECTRAL_SOLDIER:
            {
                time_t respawnTime = WorldTimer::getMSTime() + urand(MINIONS_RESPAWN_TIME_MIN, MINIONS_RESPAWN_TIME_MAX);
                m_killedMinionsCounter++;

                if (m_killedMinionsCounter == m_requiredMinionsForRare)
                {
                    SummonRareCreature();
                    m_requiredMinionsForRare = urand(REQUIRED_DEAD_MINIONS_MIN, REQUIRED_DEAD_MINIONS_MAX);
                    m_killedMinionsCounter = 0;
                }

                MinionToRespawn m_killedMinion(summoned->GetEntry(), respawnTime);
                m_minionsRespawnQueue.push_back(m_killedMinion);
                break;
            }
            case NPC_SKELETAL_SOLDIER:
            case NPC_SKELETAL_TROOPER:
            case NPC_SPECTRAL_SPIRIT:
            case NPC_SPECTRAL_APPARITION:
            {
                time_t respawnTime = WorldTimer::getMSTime() + urand(MINIONS_RESPAWN_TIME_MIN, MINIONS_RESPAWN_TIME_MAX);
                MinionToRespawn m_killedMinion(summoned->GetEntry(), respawnTime);
                m_minionsRespawnQueue.push_back(m_killedMinion);
                break;
            }
        }
    }

    void UpdateAI(const uint32 /*diff*/) override
    {
        if (m_isCampDestroyed)
            return;

        if (!m_minionsSpawnQueue.empty())
        {
            MinionToSpawn& minionToSpawn = m_minionsSpawnQueue.back();
            m_creature->SummonCreature(minionToSpawn.m_entry, minionToSpawn.m_position.x, minionToSpawn.m_position.y, minionToSpawn.m_position.z, minionToSpawn.m_position.o, TEMPSPAWN_DEAD_DESPAWN, 0, true);
            m_minionsSpawnQueue.pop_back();
        }

        if (!m_minionsRespawnQueue.empty())
        {
            time_t now = WorldTimer::getMSTime();

            for (auto minion = m_minionsRespawnQueue.begin(); minion != m_minionsRespawnQueue.end();)
            {
                if (minion->m_when > now)
                {
                    ++minion;
                    continue;
                }

                float fX, fY, fZ;
                m_creature->GetNearPoint(m_creature, fX, fY, fZ, 0, urand(8, 40), 0);

                m_creature->SummonCreature(minion->m_entry, fX, fY, fZ, 0, TEMPSPAWN_DEAD_DESPAWN, 1200000);
                minion = m_minionsRespawnQueue.erase(minion);
                break;
            }
        }
    }

    void SetCampType(uint8 id) {  m_campType = id; }

    void CampDestroyed()
    {
        m_isCampDestroyed = true;
        SetCampGOsVisibility(false);
    }

    void SpawnNecroticShard()
    {
        if (!GetClosestCreatureWithEntry(m_creature, NPC_DAMAGED_NECROTIC_SHARD, 10.0f))
            DoCastSpellIfCan(m_creature, SPELL_CREATE_CRYSTAL, TRIGGERED_FULL_MASK);
    }

    void SetMinionsTypes()
    {
        uint32 minionType1 = 0;
        uint32 minionType2 = 0;

        switch (m_zoneId)
        {
            case ZONE_ELWYNN:
            case ZONE_MULGORE:
            {
                minionType1 = NPC_SPECTRAL_APPARITION;
                minionType2 = NPC_SKELETAL_SOLDIER;
                break;
            }
            case ZONE_DUN_MOROGH:
            case ZONE_TIRISFAL_GLADES:
            case ZONE_DUROTAR:
            case ZONE_TELDRASSIL:
            {
                minionType1 = NPC_SPECTRAL_SPIRIT;
                minionType2 = NPC_SKELETAL_TROOPER;
                break;
            }
            default:
                switch (m_campType)
                {
                    case CAMP_TYPE_GHOUL_SKELETON:
                    {
                        minionType1 = NPC_GHOUL_BERSERKER;
                        minionType2 = NPC_SKELETAL_SHOCKTROOPER;
                        break;
                    }
                    case CAMP_TYPE_GHOST_GHOUL:
                    {
                        minionType1 = NPC_SPECTRAL_SOLDIER;
                        minionType2 = NPC_GHOUL_BERSERKER;
                        break;
                    }
                    case CAMP_TYPE_GHOST_SKELETON:
                    {
                        minionType1 = NPC_SPECTRAL_SOLDIER;
                        minionType2 = NPC_SKELETAL_SHOCKTROOPER;
                        break;
                    }
                }
                break;
        }

        if (minionType1 && minionType2)
        {
            FillMinionsSpawnQueue(minionType1, m_minionsMinSpawnDistance, m_minionsMaxSpawnDistance, m_minionsAmount);
            FillMinionsSpawnQueue(minionType2, m_minionsMinSpawnDistance, m_minionsMaxSpawnDistance, m_minionsAmount);
            std::random_shuffle(m_minionsSpawnQueue.begin(), m_minionsSpawnQueue.end());
        }
    }

    void DespawnCultistEngineers()
    {
        std::list<Creature*> cultistEngineers;
        MaNGOS::AllCreaturesOfEntryInRangeCheck check(m_creature, NPC_CULTIST_ENGINEER, 10.0f);
        MaNGOS::CreatureListSearcher<MaNGOS::AllCreaturesOfEntryInRangeCheck> searcher(cultistEngineers, check);
        Cell::VisitGridObjects(m_creature, searcher, 10.0f);

        for (auto& cultist : cultistEngineers)
            cultist->ForcedDespawn();
    }

    void SummonRareCreature()
    {
        float fX, fY, fZ;
        m_creature->GetNearPoint(m_creature, fX, fY, fZ, 0, urand(8, 40), 0);

        uint32 rareMinionType = 0;

        switch (urand(1, 3))
        {
            case 1: rareMinionType = NPC_RARE_BONE_WITCH; break;
            case 2: rareMinionType = NPC_RARE_LUMBERING_HORROR; break;
            case 3: rareMinionType = NPC_RARE_SPIRIT_OF_THE_DAMNED; break;
        }

        if (rareMinionType)
            m_creature->SummonCreature(rareMinionType, fX, fY, fZ, 0, TEMPSPAWN_DEAD_DESPAWN, 120000);
    }

    void FillMinionsSpawnQueue(uint32 creatureId, float minRadius = 5, float maxRadius = 5, uint32 amount = 1)
    {
        float radius = 0;
        float angle = 0;
        float currentAngle = 0;

        if (amount != 1)
            angle += 2 * M_PI_F / amount;
        else
            angle += urand(0, 2 * M_PI_F);

        if (minRadius == maxRadius)
            radius = minRadius;

        for (uint32 i = 0; i < amount; ++i)
        {
            if (minRadius != maxRadius)
                radius = urand(minRadius, maxRadius);

            currentAngle += angle;

            float fX, fY, fZ;
            m_creature->GetNearPoint(m_creature, fX, fY, fZ, 0, radius, currentAngle);

            Position spawnPosition = Position();
            spawnPosition.x = fX;
            spawnPosition.y = fY;
            spawnPosition.z = fZ;

            MinionToSpawn minionToSpawn(creatureId, spawnPosition);
           
            m_minionsSpawnQueue.push_back(minionToSpawn);
        }
    }

    void SetCampGOsVisibility(bool visible)
    {
        /*std::list<GameObject*> undeadFires;
        std::list<GameObject*> undeadFireAuras;

        GetGameObjectListWithEntryInGrid(undeadFires, m_creature, GO_UNDEAD_FIRE, 30);
        GetGameObjectListWithEntryInGrid(undeadFireAuras, m_creature, GO_UNDEAD_FIRE_AURA, 30);

        if (GameObject* circle = GetClosestGameObjectWithEntry(m_creature, GO_CIRCLE, 5))
        {
            circle->SetVisibility(visible);
            circle->UpdateObjectVisibility();
        }

        for (auto& fire : undeadFires)
        {
            fire->SetVisibility(visible);
            fire->UpdateObjectVisibility();
        }

        for (auto& fireAura : undeadFireAuras)
        {
            fireAura->SetVisibility(visible);
            fireAura->UpdateObjectVisibility();
        }*/
    }
};

/*######
## npc_cultist_engineer
## Entry: 16230
######*/

struct npc_cultist_engineerAI : public ScriptedAI
{
    npc_cultist_engineerAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        DoCastSpellIfCan(m_creature, SPELL_CULTIST_ENGINEER_SPAWN_VISUAL, TRIGGERED_FULL_MASK);

        Reset();
    }

    uint32 m_uiButtressChannelTimer;
    uint32 m_uiNecroticShardTimer;
    uint32 m_uiShadowOfDoomTimer;
    bool m_bDoSummonShadow;

    ObjectGuid m_playerGuid;

    void Reset() override 
    {
        m_uiButtressChannelTimer = 3000;
        m_uiNecroticShardTimer = urand(600000, 900000);
        m_bDoSummonShadow = false;
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (m_uiButtressChannelTimer < uiDiff)
        {
            if (DoCastSpellIfCan(GetClosestCreatureWithEntry(m_creature, NPC_DAMAGED_NECROTIC_SHARD, 20.0f, true), SPELL_BUTTRESS_CHANNEL, TRIGGERED_FULL_MASK) == CAST_OK)
                m_uiButtressChannelTimer = 3000;
        }
        else
            m_uiButtressChannelTimer -= uiDiff;

        if (m_bDoSummonShadow)
        {
            if (m_uiShadowOfDoomTimer < uiDiff)
            {
                if (DoCastSpellIfCan(m_creature, SPELL_SUMMON_SHADOW_OF_DOOM) == SPELL_CAST_OK)
                {
                    m_creature->ForcedDespawn();
                    m_bDoSummonShadow = false;
                }
            }
            else
                m_uiShadowOfDoomTimer -= uiDiff;
        }

        if (m_uiNecroticShardTimer < uiDiff)
        {
            if (Creature* shard = GetClosestCreatureWithEntry(m_creature, NPC_DAMAGED_NECROTIC_SHARD, 10.0f))
                shard->ForcedDespawn();

            if (Creature* controller = GetClosestCreatureWithEntry(m_creature, NPC_NECROPOLIS_CONTROLLER, 10.0f))
            {
                if (npc_necropolis_controllerAI* ai = dynamic_cast<npc_necropolis_controllerAI*>(controller->AI()))
                {
                    ai->DespawnCultistEngineers();
                    ai->SpawnNecroticShard();
                }
            }

            m_uiNecroticShardTimer = urand(600000, 900000);
        }
        else
            m_uiNecroticShardTimer -= uiDiff;
    }

    void SummonBoss(Player const* player)
    {
        m_playerGuid = player->GetObjectGuid();
        if (m_creature->AI->DoCastSpellifCan(m_creature, SPELL_DISRUPT_CULTIST_TRANCE_VISUAL, TRIGGERED_FULL_MASK) == CAST_OK)
        {
            m_bDoSummonShadow = true;
            m_uiShadowOfDoomTimer = 2000;
        }
    }
};

static bool GossipHello_engineer(Player* player, Creature* creature)
{
    if (player->HasItemCount(ITEM_NECROTIC_RUNE, 8))
        player->ADD_GOSSIP_ITEM_ID(0, GOSSIP_DISRUPT_CULTIST, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    player->PlayerTalkClass->SendGossipMenu(3, creature->GetObjectGuid());

    return true;
}

static bool GossipSelect_engineer(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
{
    switch (action)
    {
        case GOSSIP_ACTION_INFO_DEF + 1:
        {
            player->PlayerTalkClass->CloseGossip();

            if (npc_cultist_engineerAI* cultistAi = dynamic_cast<npc_cultist_engineerAI*>(creature->AI()))
                cultistAi->SummonBoss(player);

            break;
        }
    }

    return true;
}

/*######
## npc_necropolis
## Entry: 16401
######*/

struct npc_necropolisAI : public ScriptedAI
{
    uint8 m_campsDestroyed;
    uint32 m_necropolisWs;
    uint32 m_necropolisGoGuid;

    npc_necropolisAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_necropolisWs = m_creature->GetGUIDLow();

        m_necropolisGoGuid = 0;

        NotifyNecropolisSpawned();
        Reset();
    }

    uint32 m_uiInvasionCheckTimer;
    uint32 m_uiActivateCampsTimer;
    bool m_bDoActivateCamps;
    uint32 m_uiNecroToCampVisualTimer;
    bool m_bDoNecroToCamp;

    void Reset() override
    {
        m_uiInvasionCheckTimer = 5000;
        m_bDoActivateCamps = false;
        m_bDoNecroToCamp = false;
    }

    void SpellHit(Unit* /*caster*/, const SpellEntry* spell) override
    {
        if (spell->Id == SPELL_CAMP_TO_NECROPOLIS_VISUAL)
            DoCastSpellIfCan(m_creature, SPELL_CAMP_TO_NECROPOLIS_VISUAL);
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (m_uiInvasionCheckTimer < uiDiff)
        {
            if (!m_necropolisGoGuid)
                if (GameObject* necropolis = GetClosestGameObjectWithEntry(m_creature, GO_NECROPOLIS, 100))
                    m_necropolisGoGuid = necropolis->GetGUIDLow();

            if (sScourgeInvasionMgr.IsNecropolisReady(m_creature))
                StartEvents();
            else
                m_uiInvasionCheckTimer = 5000;
        }
        else
            m_uiInvasionCheckTimer -= uiDiff;

        if (m_bDoActivateCamps)
        {
            if (m_uiActivateCampsTimer < uiDiff)
            {
                std::list<Creature*> invasionCamps;
                GetInvasionCamps(invasionCamps, NPC_NECROPOLIS_CONTROLLER);

                uint8 campSize = invasionCamps.size();
                if (campSize < 3)
                {
                    invasionCamps.clear();
                    m_uiNecroToCampVisualTimer = 2000;
                }
                else if (campSize == 3)
                {
                    std::vector<uint8> campTypes = { 1, 2, 3 };
                    std::random_shuffle(campTypes.begin(), campTypes.end());

                    for (auto& camp : invasionCamps)
                    {
                        if (npc_necropolis_controllerAI* ai = dynamic_cast<npc_necropolis_controllerAI*>(camp->AI()))
                        {
                            ai->SetCampType(campTypes.back());
                            campTypes.pop_back();
                        }

                        DoCastSpellIfCan(camp, SPELL_ACTIVATE_CAMP_VISUAL, TRIGGERED_FULL_MASK);
                    }

                    m_bDoNecroToCamp = true;
                    m_uiNecroToCampVisualTimer = 15000;
                }
            }
            else
                m_uiActivateCampsTimer -= uiDiff;
        }

        if (m_bDoNecroToCamp)
        {
            if (m_uiNecroToCampVisualTimer < uiDiff)
            {
                std::list<Creature*> necroticShards;
                std::list<Creature*> damagedCrystals;
                GetInvasionCamps(necroticShards, NPC_NECROTIC_SHARD);
                GetInvasionCamps(damagedCrystals, NPC_DAMAGED_NECROTIC_SHARD);

                for (auto& shard : necroticShards)
                {
                    if (shard->isAlive())
                        DoCastSpellIfCan(shard, SPELL_NECROPOLIS_TO_CAMPS_VISUAL, TRIGGERED_FULL_MASK);
                }

                for (auto& crystal : damagedCrystals)
                {
                    if (crystal->isAlive())
                        DoCastSpellIfCan(crystal, SPELL_NECROPOLIS_TO_CAMPS_VISUAL, TRIGGERED_FULL_MASK);
                }

                m_uiNecroToCampVisualTimer = 15000;
            }
            else
                m_uiNecroToCampVisualTimer -= uiDiff;
        }
    }

    void NotifyNecropolisSpawned() { sScourgeInvasionMgr.OnNecropolisSpawn(m_creature); }

    void NotifyNecropolisDied() { sScourgeInvasionMgr.OnNecropolisDespawn(m_creature); }

    void GetInvasionCamps(std::list<Creature*>& camps, uint32 entry)
    {
        MaNGOS::AllCreaturesOfEntryInRangeCheck check(m_creature, entry, 300.0f);
        MaNGOS::CreatureListSearcher<MaNGOS::AllCreaturesOfEntryInRangeCheck> searcher(camps, check);
        Cell::VisitGridObjects(m_creature, searcher, 300.0f);
    }

    void CampDestroyed()
    {
        ++m_campsDestroyed;

        if (m_campsDestroyed == 3)
        {
            Reset();
            NotifyNecropolisDied();
            DespawnNecropolisGO();
            m_uiInvasionCheckTimer = 5000;
        }
    }

    void DespawnNecropolisGO()
    {
        if (GameObject* necropolis = GetClosestGameObjectWithEntry(m_creature, GO_NECROPOLIS, 100.0f))
            necropolis->Delete();
    }

    void RespawnNecropolisGO()
    {
        if (GameObject* necropolis = GetClosestGameObjectWithEntry(m_creature, GO_NECROPOLIS, 100.0f))
        {
            //necropolis->SetVisibility(true);
            necropolis->UpdateObjectVisibility();
            return;
        }

        Map* map = m_creature->GetMap();
        GameObject* necropolis = new GameObject();

        if (necropolis->LoadFromDB(m_necropolisGoGuid, map))
        {
            map->Add(necropolis);
            //necropolis->SetVisibility(true);
            //necropolis->UpdateObjectVisibility();
        }
        else
            delete necropolis;
    }

    void StartEvents()
    {
        m_uiActivateCampsTimer = 20000;
        m_bDoActivateCamps = true;
        m_campsDestroyed = 0;
        RespawnNecropolisGO();
    }
};

/*######
## npc_damaged_shard
## Entry: 16172
######*/

struct npc_damaged_shardAI : public ScriptedAI
{
    npc_damaged_shardAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    void Reset() override {}

    void SpellHit(Unit* /*caster*/, const SpellEntry* spell) override
    {
        if (!m_creature->isAlive())
            return;

        if (spell->Id == SPELL_NECROPOLIS_TO_CAMPS_VISUAL)
            DoCastSpellIfCan(m_creature, SPELL_COM_CAMP_RECEIVE_VISUAL);
    }

    void JustDied(Unit* /*killer*/) override
    {
        DoCastSpellIfCan(m_creature, SPELL_SOUL_REVIVAL, TRIGGERED_FULL_MASK);

        if (Creature* necropolis = GetClosestCreatureWithEntry(m_creature, NPC_NECROPOLIS, 300))
            if (npc_necropolisAI* ai = dynamic_cast<npc_necropolisAI*>(necropolis->AI()))
                ai->CampDestroyed();

        if (Creature* necropolisController = GetClosestCreatureWithEntry(m_creature, NPC_NECROPOLIS_CONTROLLER, 5))
            if (npc_necropolis_controllerAI* ai = dynamic_cast<npc_necropolis_controllerAI*>(necropolisController->AI()))
                ai->CampDestroyed();
    }
};

CreatureAI* GetAI_npc_scourge_messenger(Creature* pCreature)
{
    return new npc_scourge_messengerAI(pCreature);
}

CreatureAI* GetAI_npc_scourge_emissary(Creature* pCreature)
{
    return new npc_scourge_emissaryAI(pCreature);
}

CreatureAI* GetAI_npc_scourge_rewards(Creature* pCreature)
{
    return new npc_scourge_rewardsAI(pCreature);
}

CreatureAI* GetAI_npc_scourge_minion(Creature* pCreature)
{
    return new npc_scourge_minionAI(pCreature);
}

CreatureAI* GetAI_npc_necrotic_shard(Creature* pCreature)
{
    return new npc_necrotic_shardAI(pCreature);
}

CreatureAI* GetAI_npc_necropolis_controller(Creature* pCreature)
{
    return new npc_necropolis_controllerAI(pCreature);
}

CreatureAI* GetAI_npc_cultist_engineer(Creature* pCreature)
{
    return new npc_cultist_engineerAI(pCreature);
}

CreatureAI* GetAI_npc_necropolis(Creature* pCreature)
{
    return new npc_necropolisAI(pCreature);
}

CreatureAI* GetAI_npc_damaged_shard(Creature* pCreature)
{
    return new npc_damaged_shardAI(pCreature);
}

void AddSC_scourge_invasion()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "npc_damaged_shard";
    pNewScript->GetAI = &GetAI_npc_damaged_shard;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_necropolis";
    pNewScript->GetAI = &GetAI_npc_necropolis;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_necropolis_controller";
    pNewScript->GetAI = &GetAI_npc_necropolis_controller;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_necrotic_shard";
    pNewScript->GetAI = &GetAI_npc_necrotic_shard;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_scourge_messenger";
    pNewScript->pGossipHello = &GossipHello_messenger;
    pNewScript->pGossipSelect = &GossipSelect_messenger;
    pNewScript->GetAI = &GetAI_npc_scourge_messenger;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_scourge_emissary";
    pNewScript->pGossipHello = &GossipHello_emissary;
    pNewScript->pGossipSelect = &GossipSelect_emissary;
    pNewScript->GetAI = &GetAI_npc_scourge_emissary;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_scourge_rewards";
    pNewScript->pGossipHello = &GossipHello_rewards;
    pNewScript->pGossipSelect = &GossipSelect_rewards;
    pNewScript->GetAI = &GetAI_npc_scourge_rewards;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_scourge_minion";
    pNewScript->GetAI = &GetAI_npc_scourge_minion;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_cultist_engineer";
    pNewScript->pGossipHello = &GossipHello_engineer;
    pNewScript->pGossipSelect = &GossipSelect_engineer;
    pNewScript->GetAI = &GetAI_npc_cultist_engineer;
    pNewScript->RegisterSelf();
}
