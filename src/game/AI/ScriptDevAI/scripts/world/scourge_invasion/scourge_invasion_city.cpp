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
        if (victimEntry == NPC_STORMWIND_GUARD || victimEntry == NPC_UNDERCITY_GUARDIAN
            || victimEntry == NPC_STORMWIND_ROYAL_GUARD || victimEntry == NPC_UNDERCITY_DEATHGUARD_ELITE)
        {
            float fx, fy, fz;
            m_creature->GetNearPoint(victim, fx, fy, fz, 0, urand(0, 10), 0);
            m_creature->SummonCreature(m_zoneToCityEliteGuardId[m_creature->GetZoneId()], fx, fy, fz, 0, TEMPSPAWN_TIMED_OR_CORPSE_DESPAWN, 300000);
        }
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


struct npc_pallid_horrorAI : public ScriptedAI
{
    npc_pallid_horrorAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_uiSpiritAuraTimer = 1000;

        Reset();

        DoScriptText(urand(0, 1) ? CITY_INVADER_SPAWN_YELL_1 : CITY_INVADER_SPAWN_YELL_2, m_creature);
    }
    uint32 m_uiSpiritAuraTimer;
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

    void JustDied(Unit* /*killer*/) override
    {
        uint32 questCrystalSpellId = m_creature->GetZoneId() == ZONE_STORMWIND ? SPELL_SUMMON_CRACKED_NECROTIC_CRYSTAL : SPELL_SUMMON_FAINT_NECROTIC_CRYSTAL;
        m_creature->CastSpell(m_creature, questCrystalSpellId, TRIGGERED_FULL_MASK);
    }

//    Need to see if this can be handled in with their faction.
//    bool IsTargetInvalid(Unit const* target) { return target->IsPlayer() && !target->IsInCombatWith(m_creature); }

    void KilledUnit(Unit* victim) override
    {
        uint32 victimEntry = victim->GetEntry();

        if (victimEntry == NPC_STORMWIND_GUARD || victimEntry == NPC_UNDERCITY_GUARDIAN
            || victimEntry == NPC_STORMWIND_ROYAL_GUARD || victimEntry == NPC_UNDERCITY_DEATHGUARD_ELITE)
        {
            float fx, fy, fz;
            m_creature->GetNearPoint(victim, fx, fy, fz, 0, urand(0, 10), 0);
            m_creature->SummonCreature(m_zoneToCityEliteGuardId[m_creature->GetZoneId()], fx, fy, fz, 0, TEMPSPAWN_TIMED_OR_CORPSE_DESPAWN, 300000);
        }
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (m_uiSpiritAuraTimer < uiDiff)
        {
            if (!m_creature->HasAura(SPELL_SPIRIT_PARTICLES))
                if (DoCastSpellIfCan(m_creature, SPELL_SPIRIT_PARTICLES, TRIGGERED_FULL_MASK) == CAST_OK)
                    m_uiSpiritAuraTimer = 1000;
        }
        else
            m_uiSpiritAuraTimer -= uiDiff;

        // Check if we have a valid target, otherwise do nothing
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

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

    void SummonFlameshockers(uint8 amount)
    {
        if (!m_creature->isAlive())
            return;

        for (uint8 i = 0; i < amount; ++i)
        {
            float fx, fy, fz;
            m_creature->GetNearPoint(m_creature, fx, fy, fz, 0, urand(5, 10), 0);

            m_creature->SummonCreature(NPC_FLAMESHOCKER, fx, fy, fz, 0, TEMPSPAWN_DEAD_DESPAWN, 180000);
        }
    }
};

/*######
## npc_patchwork_terror
## Entry: 16382
######*/

struct npc_patchwork_terrorAI : public ScriptedAI
{
    npc_patchwork_terrorAI(Creature* pCreature) : npc_escortAI(pCreature)
    {
        Reset();
        DoScriptText(urand(0, 1) ? CITY_INVADER_SPAWN_YELL_1 : CITY_INVADER_SPAWN_YELL_2, m_creature);
    npc_patchwork_terrorAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    }
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
        if (victimEntry == NPC_STORMWIND_GUARD || victimEntry == NPC_UNDERCITY_GUARDIAN
            || victimEntry == NPC_STORMWIND_ROYAL_GUARD || victimEntry == NPC_UNDERCITY_DEATHGUARD_ELITE)
        {
            float fx, fy, fz;
            m_creature->GetNearPoint(victim, fx, fy, fz, 0, urand(0, 10), 0);
            m_creature->SummonCreature(fx, fy, fz, 0, m_zoneToCityEliteGuardId[m_creature->GetZoneId()], TEMPSPAWN_TIMED_OR_CORPSE_DESPAWN, 300000);
        }
    }

    void UpdateAI(const uint32 uiDiff) override
    {
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

   void SummonFlameshockers(uint8 amount)
    {
       if (!m_creature->isAlive())
           return;

        for (uint8 i = 0; i < amount; ++i)
        {
            float fx, fy, fz;
            m_creature->GetNearPoint(m_creature, fx, fy, fz, 0, urand(5, 10), 0);

            m_creature->SummonCreature(NPC_FLAMESHOCKER, fx, fy, fz, 0, TEMPSPAWN_DEAD_DESPAWN, 180000);
        }
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