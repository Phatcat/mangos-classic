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
SDName: Boss Revanchion
SD%Complete: 90
SDComment:
SDCategory: Scourge Invasion
EndScriptData */

#include "AI/ScriptDevAI/PreCompiledHeader.h"

/*######
## boss_revanchion
## Entry: 14690
######*/

enum Spells
{
    SPELL_TELEPORT              = 21463,
    SPELL_FROST_NOVA            = 29849,
    SPELL_ARCANE_EXPLOSION      = 22271,
    SPELL_BLINK                 = 14514,
    SPELL_FROST_BOLT            = 17503,
    SPELL_FROST_ARMOR           = 12544
};

enum Eents
{
    EVENT_BLINK                 = 1,
    EVENT_FROST_BOLT            = 2,
    EVENT_TELEPORT_FROST_NOVA   = 3,
    EVENT_FROST_NOVA            = 4,
    EVENT_ARCANE_EXPLOSION      = 5
};

enum Phases
{
    PHASE_ONE                   = 1,
    PHASE_TWO                   = 2
};

struct boss_revanchionAI : public ScriptedAI
{
    boss_revanchionAI(Creature* creature) : ScriptedAI(creature)
    {
        SetCombatMovement(false);

        m_creature->CastSpell(m_creature, SPELL_FROST_ARMOR, TRIGGERED_FULL_MASK);

        m_currentPhase = PHASE_ONE;
        m_arcaneExplosionsDone = 0;
        m_playerTriedToHideCounter = 0;

        Reset();
    }

    uint8 m_currentPhase;
    uint8 m_arcaneExplosionsDone;
    uint8 m_playerTriedToHideCounter;

    void Reset() override
    {
        m_events.Reset();

        m_events.ScheduleEvent(EVENT_BLINK, urand(6, 8) * IN_MILLISECONDS, PHASE_ONE);
        m_events.ScheduleEvent(EVENT_FROST_BOLT, 1 * IN_MILLISECONDS, PHASE_ONE);
        m_events.ScheduleEvent(EVENT_TELEPORT_FROST_NOVA, 15 * IN_MILLISECONDS, PHASE_ONE);

        if (!m_creature->HasAura(SPELL_FROST_ARMOR))
            m_creature->CastSpell(m_creature, SPELL_FROST_ARMOR, TRIGGERED_FULL_MASK);

        m_currentPhase = PHASE_ONE;
        m_arcaneExplosionsDone = 0;
        m_playerTriedToHideCounter = 0;
    }

    void UpdateAI(const uint32 diff) override
    {
        if (!DoSelectVictim())
            return;

        if (m_creature->IsCasting())
            return;

        if (m_currentPhase == PHASE_ONE)
        {
            while (uint32 eventId = m_events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_FROST_BOLT:
                    {
                        if (!m_creature->IsWithinLineOfSight(GetVictim()))
                        {
                            ++m_playerTriedToHideCounter;

                            m_events.ScheduleEvent(EVENT_FROST_BOLT, 1 * IN_MILLISECONDS);

                            if (m_playerTriedToHideCounter == 3)
                            {
                                m_creature->GetVictim()->ResetThreatList();
                                m_playerTriedToHideCounter = 0;
                            }

                            break;
                        }

                        m_creature->CastSpell(GetVictim(), SPELL_FROST_BOLT);
                        m_events.ScheduleEvent(EVENT_FROST_BOLT, 2.5 * IN_MILLISECONDS);

                        break;
                    }
                    case EVENT_BLINK:
                    {
                        Position blinkPosition;

                        if (m_creature->GetDistance(GetVictim()) > 40 && m_creature->IsWithinLineOfSight(GetVictim()))
                        {
                            blinkPosition = Position(GetVictim()->m_positionX, GetVictim()->m_positionY, GetVictim()->m_positionZ, m_creature->GetOrientation());
                        }
                        else
                        {
                            blinkPosition = m_creature->GetRandomPoint(*m_creature, 20);
                        }

                        blinkPosition = m_creature->GetValidPosition(*m_creature, blinkPosition);
                        m_creature->SetFacingTo(m_creature->GetAngle(blinkPosition));
                        m_creature->CastSpell(m_creature, SPELL_BLINK, TRIGGERED_FULL_MASK);
                        m_creature->RemoveAllAuras(AURA_REMOVE_BY_DISPEL, true);
                        m_events.ScheduleEvent(EVENT_BLINK, urand(6, 8) * IN_MILLISECONDS);

                        break;
                    }
                    case EVENT_TELEPORT_FROST_NOVA:
                    {
                        m_events.Reset();

                        if (m_creature->HasUnitState(UNIT_STATE_STUNNED))
                        {
                            m_events.ScheduleEvent(EVENT_TELEPORT_FROST_NOVA, 1 * IN_MILLISECONDS);
                            break;
                        }

                        if (Unit* target = GetTarget(ATTACKING_TARGET_RANDOM, 0))
                        {
                            TeleportTo(*target);
                            m_creature->ResetThreatList();
                            m_events.ScheduleEvent(EVENT_ARCANE_EXPLOSION, 2.5 * IN_MILLISECONDS);
                            m_events.ScheduleEvent(EVENT_FROST_NOVA, 0.5 * IN_MILLISECONDS);
                            m_currentPhase = PHASE_TWO;
                        }

                        break;
                    }
                }

                if (m_creature->IsCasting())
                    return;
            }

            DoMeleeAttackIfReady();
        }
        else if (m_currentPhase == PHASE_TWO)
        {
            while (uint32 eventId = m_events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_FROST_NOVA:
                    {
                        m_creature->CastSpell(m_creature, SPELL_FROST_NOVA);
                        break;
                    }
                    case EVENT_ARCANE_EXPLOSION:
                    {
                        m_creature->CastSpell(m_creature, SPELL_ARCANE_EXPLOSION);
                        m_events.ScheduleEvent(EVENT_ARCANE_EXPLOSION, 2 * IN_MILLISECONDS);

                        ++m_arcaneExplosionsDone;

                        if (m_arcaneExplosionsDone == 3)
                        {
                            m_events.Reset();
                            m_events.ScheduleEvent(EVENT_BLINK, urand(6, 8) * IN_MILLISECONDS);
                            m_events.ScheduleEvent(EVENT_FROST_BOLT, 1 * IN_MILLISECONDS);
                            m_events.ScheduleEvent(EVENT_TELEPORT_FROST_NOVA, 15 * IN_MILLISECONDS);
                            m_currentPhase = PHASE_ONE;
                            m_arcaneExplosionsDone = 0;
                        }

                        break;
                    }
                }

                if (m_creature->IsCasting())
                    return;
            }
        }
    }
};

CreatureAI* GetAI_boss_revanchion(Creature* creature)
{
    return new boss_revanchionAI(creature);
}


void AddSC_boss_revanchion()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_revanchion";
    pNewScript->GetAI = &GetAI_boss_revanchion;
    pNewScript->RegisterSelf();
}
