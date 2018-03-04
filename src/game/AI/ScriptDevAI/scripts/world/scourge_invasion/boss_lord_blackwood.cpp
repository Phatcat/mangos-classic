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
SDName: Boss Lord Blackwood
SD%Complete: 90
SDComment:
SDCategory: Scourge Invasion
EndScriptData */

#include "AI/ScriptDevAI/PreCompiledHeader.h"

/*######
## boss_lord_blackwood
## Entry: 14695
######*/

enum Spells
{
    SPELL_SHIELD_BASH       = 11972,
    SPELL_SHIELD_CHARGE     = 19131,
    SPELL_MULTISHOT         = 21390,
    SPELL_SHOOT             = 20463
};

enum Eents
{
    EVENT_MULTISHOT         = 1,
    EVENT_SHOOT             = 2,
    EVENT_SHIELD_BASH       = 3,
    EVENT_SHIELD_CHARGE     = 4
};

struct boss_lord_blackwoodAI : public ScriptedAI
{
    boss_lord_blackwoodAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        SetRangedMovement(8, 30, RANGED_NEVER_RUN_AWAY);

        Reset();
    }

    void Reset() override
    {
        m_events.Reset();

        m_events.ScheduleEvent(EVENT_SHIELD_BASH, urand(8, 12) * IN_MILLISECONDS);
        m_events.ScheduleEvent(EVENT_SHIELD_CHARGE, urand(10, 15) * IN_MILLISECONDS);
        m_events.ScheduleEvent(EVENT_MULTISHOT, 2 * IN_MILLISECONDS);
        m_events.ScheduleEvent(EVENT_SHOOT, 1 * IN_MILLISECONDS);
    }

    void UpdateAI(const uint32 diff) override
    {
        if (GetVictim()) // Hack fix - ranged movement and ai need to be adjusted - that is for a bit later
        {
            if (m_creature->GetDistance(GetVictim()) < 8)
                UnsetRangedMovement();
            else
                SetRangedMovement(8, 30, RANGED_NEVER_RUN_AWAY);
        }

        if (!DoSelectVictim())
            return;

        m_events.Update(diff);

        if (m_creature->IsCasting())
            return;

        if (m_creature->HasUnitState(UNIT_STATE_CHARGE))
            return;

        while (uint32 eventId = m_events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_SHIELD_BASH:
                {
                    if (Unit* target = GetTarget(ATTACKING_TARGET_RANDOM, 0, SPELL_SHIELD_BASH, SELECT_FLAG_IN_MELEE_RANGE | SELECT_FLAG_CASTING_SPELL))
                    {
                        m_creature->CastSpell(target, SPELL_SHIELD_BASH);
                        m_events.ScheduleEvent(EVENT_SHIELD_BASH, urand(8, 12) * IN_MILLISECONDS);
                    }
                    else
                        m_events.ScheduleEvent(EVENT_SHIELD_BASH, 1 * IN_MILLISECONDS);

                    break;
                }
                case EVENT_SHIELD_CHARGE:
                {
                    if (Unit* target = GetTarget(ATTACKING_TARGET_FURTHEST, 0, SPELL_SHIELD_CHARGE, SELECT_FLAG_NOT_IN_MELEE_RANGE))
                    {
                        m_creature->CastSpell(target, SPELL_SHIELD_CHARGE);
                        m_events.ScheduleEvent(EVENT_SHIELD_CHARGE, urand(10, 15) * IN_MILLISECONDS);
                    }
                    else
                        m_events.ScheduleEvent(EVENT_SHIELD_CHARGE, 1 * IN_MILLISECONDS);

                    break;
                }
                case EVENT_MULTISHOT:
                {
                    m_events.ScheduleEvent(EVENT_MULTISHOT, 1 * IN_MILLISECONDS);

                    if (m_creature->HasUnitState(UNIT_STATE_NO_COMBAT_MOVEMENT))
                    {
                        if (Unit* target = GetTarget(ATTACKING_TARGET_RANDOM, 0, SPELL_MULTISHOT, SELECT_FLAG_NOT_IN_MELEE_RANGE))
                        {
                            m_creature->CastSpell(target, SPELL_MULTISHOT);
                            m_events.RescheduleEvent(EVENT_MULTISHOT, 5 * IN_MILLISECONDS);
                        }
                    }

                    break;
                }
                case EVENT_SHOOT:
                {
                    if (m_creature->HasUnitState(UNIT_STATE_NO_COMBAT_MOVEMENT))
                    {
                        m_creature->CastSpell(GetVictim(), SPELL_SHOOT);
                        m_events.ScheduleEvent(EVENT_SHOOT, 3 * IN_MILLISECONDS);
                    }
                    else
                        m_events.ScheduleEvent(EVENT_SHOOT, 1 * IN_MILLISECONDS);

                    break;
                }
            }

            if (m_creature->IsCasting())
                return;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_lord_blackwood(Creature* pCreature)
{
    return new boss_lord_blackwoodAI(pCreature);
}

void AddSC_boss_lord_blackwood()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_lord_blackwood";
    pNewScript->GetAI = &GetAI_boss_lord_blackwood;
    pNewScript->RegisterSelf();
}
