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
SDName: Scourge Invasion City
SD%Complete: 90
SDComment:
SDCategory: Scourge Invasion
EndScriptData */

#include "../game/AI/ScriptDevAI/scripts/world/scourge_invasion/scourge_invasion_mgr.h"
#include "../game/AI/ScriptDevAI/scripts/world/scourge_invasion/scourge_invasion.h"
#include "../game/AI/ScriptDevAI/PreCompiledHeader.h"
#include "../game/AI/ScriptDevAI/base/escort_ai.h""
#include "../game/Grids/GridNotifiers.h"
#include "../game/Grids/CellImpl.h"
#include "../game/Grids/GridNotifiersImpl.h"

/* ContentData
 npc_pallid_horror
 npc_patchwork_terror
 npc_flameshocker
EndContentData */


/*######
## npc_flameshocker
## Entry: 16383
######*/

struct npc_flameshockerAI : public ScriptedAI
{
    npc_flameshockerAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    uint32 m_uiTouchTimer;

    void Reset() override { m_uiTouchTimer = urand(15000, 25000); }

    void JustRespawned() override { Reset(); }

//    Need to see if this can be handled in with their faction.
//    bool IsTargetInvalid(Unit const* target) { return target->IsPlayer() && !target->IsInCombatWith(m_creature); }

    void KilledUnit(Unit* victim) override
    {
        uint32 victimEntry = victim->GetEntry();
        if (victimEntry == NPC_STORMWIND_GUARD || victimEntry == NPC_UNDERCITY_GUARDIAN || victimEntry == NPC_STORMWIND_ROYAL_GUARD || victimEntry == NPC_UNDERCITY_DEATHGUARD_ELITE)
            m_creature->SummonCreature(m_zoneToCityEliteGuardId[m_creature->GetZoneId()], victim->GetRandomPoint(*victim, 10), TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 300 * IN_MILLISECONDS);
    }

    void JustDied(Unit* /*killer*/) override
    {
        DoCastSpellIfCan(m_creature, SPELL_FLAMESHOCKER_REVENGE, TRIGGERED_FULL_MASK);
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        // Check if we have a valid target, otherwise do nothing
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_uiTouchTimer < uiDiff)
        {
            if (!m_creature->getVictim()->HasAura(SPELL_FLAMESHOCKER_TOUCH))
            {
                if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_FLAMESHOCKER_TOUCH) == CAST_OK)
                    m_uiTouchTimer = urand(15000, 25000);
            }
        }
        else
            m_uiTouchTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

/*######
## npc_pallid_horror
## Entry: 16394
######*/


struct npc_pallid_horrorAI : public npc_escortAI
{
    uint8 m_fleeingPhasesCounter;

    int32 m_stormwindKeepX = -8439;
    int32 m_royalQuarterX = 1298;
    int32 m_swStartingPointX = -8795;

    bool m_fleeingPhase;

    std::list<Escort_Waypoint> const* m_waypoints;
    std::vector<int32> m_invadedCityAreas;

    npc_pallid_horrorAI(Creature* pCreature) : npc_escortAI(pCreature)
    {
        m_uiSpiritAuraTimer = 1000;
        m_uiCheckInvadedCityAreaTimer = 5000;
        m_fleeingPhasesCounter = 0;

        m_fleeingPhase = false;

        Reset();

        m_creature->MonsterYellToZone(urand(0, 1) ? CITY_INVADER_SPAWN_YELL_1 : CITY_INVADER_SPAWN_YELL_2, 0, nullptr);

        m_creature->DelayFor(1 * IN_MILLISECONDS, [](Creature* c)
        {
            if (npc_pallid_horror::npc_pallid_horrorAI* ai = dynamic_cast<npc_pallid_horror::npc_pallid_horrorAI*>(c->AI()))
                ai->StartEscort();
        });
    }
    uint32 m_uiSpiritAuraTimer;
    uint32 m_uiCheckInvadedCityAreaTimer;
    uint32 m_uiKnockAwayTimer;
    uint32 m_uiStompTimer;
    uint32 m_uiTrampleTimer;
    uint32 m_uiFearTimer;

    void Reset() override
    {
        m_uiKnockAwayTimer = urand(5000, 8000);
        m_uiStompTimer = urand(9000, 15000);
        m_uiTrampleTimer = urand(7000, 10000);
        m_uiFearTimer = urand(5000, 12000);
    }

    void EnterEvadeMode() override
    {
        if (m_fleeingPhase && !m_creature->HasAura(SPELL_RUNNING_SPEED))
        {
            m_creature->SetCombatStartPosition(*m_creature);
            m_creature->DisableAttacking(true);
            SetCurrentWaypoint(GetCurrentWaypoint() + 1);
        }

        npc_escortAI::EnterEvadeMode();

        if (m_fleeingPhase && !m_creature->HasAura(SPELL_RUNNING_SPEED))
        {
            m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            DoCastSpellIfCan(m_creature, SPELL_INCAPACITATING_SHOUT, TRIGGERED_FULL_MASK);
            DoCastSpellIfCan(m_creature, SPELL_RUNNING_SPEED, TRIGGERED_FULL_MASK);
        }

        SetRun(true);
    }

    void JustDied(Unit* /*killer*/) override
    {
        uint32 questCrystalSpellId = m_creature->GetZoneId() == ZONE_STORMWIND ? SPELL_SUMMON_CRACKED_NECROTIC_CRYSTAL : SPELL_SUMMON_FAINT_NECROTIC_CRYSTAL;
        m_creature->AI->DoCastSpellIfCan(m_creature, questCrystalSpellId, TRIGGERED_FULL_MASK);
    }

    void SpellAuraRemoved(AuraRemoveMode /*mode*/, Unit* /*caster*/, SpellEntry const* spell) override
    {
        if (spell->Id == SPELL_RUNNING_SPEED)
            EndFleeing();
    }

//    Need to see if this can be handled in with their faction.
//    bool IsTargetInvalid(Unit const* target) { return target->IsPlayer() && !target->IsInCombatWith(m_creature); }

    void KilledUnit(Unit* victim) override
    {
        uint32 victimEntry = victim->GetEntry();

        if (victimEntry == NPC_STORMWIND_GUARD || victimEntry == NPC_UNDERCITY_GUARDIAN || victimEntry == NPC_STORMWIND_ROYAL_GUARD || victimEntry == NPC_UNDERCITY_DEATHGUARD_ELITE)
            m_creature->SummonCreature(m_zoneToCityEliteGuardId[m_creature->GetZoneId()], victim->GetRandomPoint(*victim, 10), TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 300 * IN_MILLISECONDS);
    }

    void UpdateEscortAI(const uint32 uiDiff) override
    {
        if (m_uiSpiritAuraTimer < uiDiff)
        {
            if (!m_creature->HasAura(SPELL_SPIRIT_PARTICLES))
                if (DoCastSpellIfCan(m_creature, SPELL_SPIRIT_PARTICLES, TRIGGERED_FULL_MASK) == CAST_OK)
                    m_uiSpiritAuraTimer = 1000;
        }
        else
            m_uiSpiritAuraTimer -= uiDiff;

        if (m_uiCheckInvadedCityAreaTimer < uiDiff)
        {
            if (Creature* cityAreaMarker = GetClosestCreatureWithEntry(m_creature, NPC_CITY_AREA_MARKER, 70))
            {
                int32 cityAreaPosX = int32(cityAreaMarker->GetRespawnPosition().GetPositionX());

                for (auto currentCityAreaPosX : m_invadedCityAreas)
                {
                    if (currentCityAreaPosX == cityAreaPosX)
                    {
                        m_uiCheckInvadedCityAreaTimer = 5000;
                        return;
                    }
                }

                if (cityAreaPosX == m_stormwindKeepX)
                {
                    if (Creature* bolvar = GetClosestCreatureWithEntry(m_creature, NPC_BOLVAR, 100))
                        bolvar->MonsterYellToZone(m_cityAreaMarkerPositionToTextId[cityAreaPosX], 0, nullptr);
                }
                else if (cityAreaPosX == m_royalQuarterX)
                {
                    if (Creature* varimathras = GetClosestCreatureWithEntry(m_creature, NPC_VARIMATHRAS, 100))
                        varimathras->MonsterYellToZone(m_cityAreaMarkerPositionToTextId[cityAreaPosX], 0, nullptr);
                }
                else
                {
                    if (Creature* cityGuard = m_creature->SummonCreature(m_zoneToCityGuardId[m_creature->GetZoneId()], 0, TEMPSUMMON_DEAD_DESPAWN, 180))
                        cityGuard->MonsterYellToZone(m_cityAreaMarkerPositionToTextId[cityAreaPosX], 0, nullptr);
                }

                m_invadedCityAreas.push_back(cityAreaPosX);

            }

            m_uiCheckInvadedCityAreaTimer = 5000;
        }
        else
            m_uiCheckInvadedCityAreaTimer -= uiDiff;

        if (m_fleeingPhase || m_creature->HasAura(SPELL_RUNNING_SPEED))
            return;

        // Check if we have a valid target, otherwise do nothing
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if ((m_creature->GetHealthPercent() <= 75 && m_fleeingPhasesCounter == 0) ||
            (m_creature->GetHealthPercent() <= 50 && m_fleeingPhasesCounter == 1) ||
            (m_creature->GetHealthPercent() <= 25 && m_fleeingPhasesCounter == 2))
        {
            StartFleeing();
            return;
        }

        // Check if we are casting / channelling, return to not interrupt process and pause CDs
        if (m_creature->IsNonMeleeSpellCasted(false))
            return;

        if (m_uiKnockAwayTimer < uiDiff)
        {
            if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, SPELL_KNOCK_AWAY, SELECT_FLAG_IN_MELEE_RANGE))
                if (DoCastSpellIfCan(m_creature, SPELL_KNOCK_AWAY) == CAST_OK)
                    m_uiKnockAwayTimer = urand(5000, 8000);
        }
        else
            m_uiKnockAwayTimer -= uiDiff;
        
        if (m_uiStompTimer < uiDiff)
        {
            if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, SPELL_STOMP, SELECT_FLAG_IN_MELEE_RANGE))
                if (DoCastSpellIfCan(m_creature, SPELL_STOMP) == CAST_OK)
                    m_uiStompTimer = urand(9000, 15000);
        }
        else
            m_uiStompTimer -= uiDiff;

         if (m_uiTrampleTimer < uiDiff)
        {
            if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, SPELL_TRAMPLE, SELECT_FLAG_IN_MELEE_RANGE))
                if (DoCastSpellIfCan(m_creature, SPELL_TRAMPLE) == CAST_OK)
                    m_uiTrampleTimer = urand(7000, 10000);
        }
        else
            m_uiTrampleTimer -= uiDiff;

         if (m_uiFearTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_FEAR_INVADERS) == CAST_OK)
                m_uiFearTimer = urand(5000, 12000);
        }
        else
            m_uiFearTimer -= uiDiff;

        // Check if we are casting / channelling, return to not interrupt process and pause CDs
        if (m_creature->IsNonMeleeSpellCasted(false))
            return;

        DoMeleeAttackIfReady();
    }

    void StartFleeing()
    {
        Reset();
        ++m_fleeingPhasesCounter;

        if (Creature* cityAreaMarker = GetClosestCreatureWithEntry(m_creature, NPC_CITY_AREA_MARKER, 60))
            if (int32(cityAreaMarker->GetRespawnPosition().GetPositionX()) == m_stormwindKeepX || int32(cityAreaMarker->GetRespawnPosition().GetPositionX()) == m_royalQuarterX)
                return;

        m_fleeingPhase = true;
        EnterEvadeMode();
    }

    void EndFleeing()
    {
        Reset();
        m_fleeingPhase = false;
        m_creature->DisableAttacking(false);
        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        SummonFlameshockers(4);
    }

    void StartEscort()
    {
        SetWaypoints();
        SummonFlameshockers(8);
        Start(FLAG_NONE, m_waypoints, true);
        SetRun(true);
    }

    void SetWaypoints()
    {
        int32 cityAreaPosX = int32(m_creature->GetRespawnPosition().GetPositionX());
        std::map<int32, uint32> customWaypointIds = m_creature->GetZoneId() == ZONE_STORMWIND ? m_swPosXToCustomWaypointId : m_ucPosXToCustomWaypointId;

        if (cityAreaPosX == m_swStartingPointX)
            cityAreaPosX = urand(0, 1);

        m_waypoints = sScriptMgr.GetCustomWaypoints(customWaypointIds[cityAreaPosX]);

        if (!m_waypoints)
        {
            TC_ERROR("SCOURGE INVASION: Pallid Horror WP cannot be loaded!");
            return;
        }
    }

    void SummonFlameshockers(uint8 amount)
    {
        if (!m_creature->isAlive())
            return;

        for (uint8 i = 0; i < amount; ++i)
        {
            Position destination = m_creature->GetRandomPoint(*m_creature, urand(5, 10));
            destination = m_creature->GetValidPosition(*m_creature, destination);

            m_creature->SummonCreature(NPC_FLAMESHOCKER, destination, TEMPSUMMON_DEAD_DESPAWN, 180);
        }
    }
};

/*######
## npc_patchwork_terror
## Entry: 16382
######*/

struct npc_patchwork_terrorAI : public npc_escortAI
{
    int32 m_stormwindKeepX = -8439;
    int32 m_royalQuarterX = 1298;
    int32 m_swStartingPointX = -8795;

    std::list<Escort_Waypoint> const* m_waypoints;
    std::vector<int32> m_invadedCityAreas;

    npc_patchwork_terrorAI(Creature* pCreature) : npc_escortAI(pCreature)
    {
        m_uiCheckInvadedCityAreaTimer = 5000;

        Reset();

        m_creature->MonsterYellToZone(urand(0, 1) ? CITY_INVADER_SPAWN_YELL_1 : CITY_INVADER_SPAWN_YELL_2, 0, nullptr);

        m_creature->DelayFor(1000, [](Creature* c)
        {
            if (npc_patchwork_terror::npc_patchwork_terrorAI* ai = dynamic_cast<npc_patchwork_terror::npc_patchwork_terrorAI*>(c->AI()))
                ai->StartEscort();
        });
    }

    uint32 m_uiCheckInvadedCityAreaTimer;
    uint32 m_uiViciousRendTimer;
    uint32 m_uiEchoingRoarTimer;
    uint32 m_uiCleaveTimer;
    uint32 m_uiThrowAxeTimer;
    uint32 m_uiFearTimer;

    void Reset() override 
    {
        m_uiViciousRendTimer = urand(10000, 15000);
        m_uiEchoingRoarTimer = (12000, 20000);
        m_uiCleaveTimer = urand(5000, 9000);
        m_uiThrowAxeTimer = urand(18000, 32000);
        m_uiFearTimer = urand(5000, 12000);
    }

//    Need to see if this can be handled in with their faction.
//    bool IsTargetInvalid(Unit const* target) { return target->IsPlayer() && !target->IsInCombatWith(m_creature); }

    void KilledUnit(Unit* victim) override
    {
        uint32 victimEntry = victim->GetEntry();
        if (victimEntry == NPC_STORMWIND_GUARD || victimEntry == NPC_UNDERCITY_GUARDIAN || victimEntry == NPC_STORMWIND_ROYAL_GUARD || victimEntry == NPC_UNDERCITY_DEATHGUARD_ELITE)
            m_creature->SummonCreature(m_zoneToCityEliteGuardId[m_creature->GetZoneId()], victim->GetRandomPoint(*victim, 10), TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 300 * IN_MILLISECONDS);
    }

    void UpdateEscortAI(const uint32 uiDiff) override
    {
        if (m_uiCheckInvadedCityAreaTimer < uiDiff)
        {
            if (Creature* cityAreaMarker = GetClosestCreatureWithEntry(m_creature, NPC_CITY_AREA_MARKER, 70))
            {
                int32 cityAreaPosX = int32(cityAreaMarker->GetRespawnPosition().GetPositionX());

                for (auto currentCityAreaPosX : m_invadedCityAreas)
                {
                    if (currentCityAreaPosX == cityAreaPosX)
                    {
                        m_uiCheckInvadedCityAreaTimer =  5000;
                        return;
                    }
                }

                if (cityAreaPosX == m_stormwindKeepX)
                {
                    if (Creature* bolvar = GetClosestCreatureWithEntry(m_creature, NPC_BOLVAR, 100))
                        bolvar->MonsterYellToZone(m_cityAreaMarkerPositionToTextId[cityAreaPosX], 0, nullptr);
                }
                else if (cityAreaPosX == m_royalQuarterX)
                {
                    if (Creature* varimathras = GetClosestCreatureWithEntry(m_creature, NPC_VARIMATHRAS, 100))
                        varimathras->MonsterYellToZone(m_cityAreaMarkerPositionToTextId[cityAreaPosX], 0, nullptr);
                }
                else
                {
                    if (Creature* cityGuard = m_creature->SummonCreature(m_zoneToCityGuardId[m_creature->GetZoneId()], 0, TEMPSUMMON_DEAD_DESPAWN, 30))
                        cityGuard->MonsterYellToZone(m_cityAreaMarkerPositionToTextId[cityAreaPosX], 0, nullptr);
                }

                m_invadedCityAreas.push_back(cityAreaPosX);
            }

            m_uiCheckInvadedCityAreaTimer = 5000;
        }
        else
            m_uiCheckInvadedCityAreaTimer -= uiDiff;

        // Check if we have a valid target, otherwise do nothing
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Check if we are casting / channelling, return to not interrupt process and pause CDs
        if (m_creature->IsNonMeleeSpellCasted(false))
            return;

        if (m_uiViciousRendTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_VICIOUS_REND) == CAST_OK)
                m_uiViciousRendTimer = urand(10000, 15000);
        }
        else
            m_uiViciousRendTimer -= uiDiff;

        if (m_uiEchoingRoarTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_ECHOING_ROAR) == CAST_OK)
                m_uiEchoingRoarTimer = urand(12000, 20000);
        }
        else
            m_uiEchoingRoarTimer -= uiDiff;

        if (m_uiCleaveTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_CLEAVE) == CAST_OK)
                m_uiCleaveTimer = urand(5000, 9000);
        }
        else
            m_uiCleaveTimer -= uiDiff;

        if (m_uiThrowAxeTimer < uiDiff)
        {
			if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, SPELL_THROW_AXE))
				if (DoCastSpellIfCan(target, SPELL_THROW_AXE) == CAST_OK)
					m_uiThrowAxeTimer = urand(18000, 32000);
        }
        else
            m_uiThrowAxeTimer -= uiDiff;

        if (m_uiFearTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_FEAR_INVADERS) == CAST_OK)
                m_uiFearTimer = urand(5000, 12000);
        }
        else
            m_uiFearTimer -= uiDiff;

        // Check if we are casting / channelling, return to not interrupt process and pause CDs
        if (m_creature->IsNonMeleeSpellCasted(false))
            return;

        DoMeleeAttackIfReady();
    }

    void StartEscort()
    {
        SetWaypoints();
        SummonFlameshockers(8);
        Start(FLAG_NONE, m_waypoints, true);
        SetRun(true);
    }

   void SummonFlameshockers(uint8 amount)
    {
       if (!m_creature->isAlive())
           return;

        for (uint8 i = 0; i < amount; ++i)
        {
            Position destination = m_creature->GetRandomPoint(*m_creature, urand(5, 10));
            destination = m_creature->GetValidPosition(*m_creature, destination);

            m_creature->SummonCreature(NPC_FLAMESHOCKER_CLONE, destination, TEMPSUMMON_DEAD_DESPAWN, 180);
        }
    }

   void SetWaypoints()
   {
       int32 cityAreaPosX = int32(m_creature->GetRespawnPosition().GetPositionX());
       std::map<int32, uint32> customWaypointIds = m_creature->GetZoneId() == ZONE_STORMWIND ? m_swPosXToCustomWaypointId : m_ucPosXToCustomWaypointId;

       if (cityAreaPosX == m_swStartingPointX)
           cityAreaPosX = urand(0, 1);

       m_waypoints = sScriptMgr.GetCustomWaypoints(customWaypointIds[cityAreaPosX]);

       if (!m_waypoints)
           TC_ERROR("SCOURGE INVASION: Patchwork Terror WP cannot be loaded!");
   }
};

CreatureAI* GetAI_npc_flameshocker(Creature* pCreature)
{
    return new npc_flameshockerAI(pCreature);
}

CreatureAI* GetAI_npc_pallid_horror(Creature* pCreature)
{
    return new npc_pallid_horrorAI(pCreature);
}

CreatureAI* GetAI_npc_patchwork_terror(Creature* pCreature)
{
    return new npc_patchwork_terrorAI(pCreature);
}

void AddSC_scourge_invasion_city()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "npc_flameshocker";
    pNewScript->GetAI = &GetAI_npc_flameshocker;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_pallid_horror";
    pNewScript->GetAI = &GetAI_npc_pallid_horror;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_patchwork_terror";
    pNewScript->GetAI = &GetAI_npc_patchwork_terror;
    pNewScript->RegisterSelf();
}