#include "../game/AI/ScriptDevAI/scripts/world/scourge_invasion/scourge_invasion_mgr.h"
#include "../game/World/world.h"
#include "../game/GameEvents/GameEventMgr.h"
#include "../shared/Timer.h"

INSTANTIATE_SINGLETON_1(ScourgeInvasion);

ScourgeInvasion::ScourgeInvasion() : m_initialized(false), m_spawnCheckTimer(60000)
{
}

void ScourgeInvasion::Initialize()
{
    if (sWorld.getWorldState(WORLD_STATE_DEAD_NECROPOLIS) == 0)
    {
        m_zoneData[ZONE_TANARIS]                = ZoneToMaxNecros[ZONE_TANARIS];
        m_zoneData[ZONE_WINTERSPRING]           = ZoneToMaxNecros[ZONE_WINTERSPRING];
        m_zoneData[ZONE_BURNING_STEPPES]        = ZoneToMaxNecros[ZONE_BURNING_STEPPES];
        m_zoneData[ZONE_BLASTED_LANDS]          = ZoneToMaxNecros[ZONE_BLASTED_LANDS];
        m_zoneData[ZONE_AZSHARA]                = ZoneToMaxNecros[ZONE_AZSHARA];
        m_zoneData[ZONE_EASTERN_PLAGUELANDS]    = ZoneToMaxNecros[ZONE_EASTERN_PLAGUELANDS];
    }
    else
    {
        m_zoneData[ZONE_TANARIS]                = sWorld.getWorldState(ZoneToWorldState[ZONE_TANARIS]);
        m_zoneData[ZONE_WINTERSPRING]           = sWorld.getWorldState(ZoneToWorldState[ZONE_WINTERSPRING]);
        m_zoneData[ZONE_BURNING_STEPPES]        = sWorld.getWorldState(ZoneToWorldState[ZONE_BURNING_STEPPES]);
        m_zoneData[ZONE_BLASTED_LANDS]          = sWorld.getWorldState(ZoneToWorldState[ZONE_BLASTED_LANDS]);
        m_zoneData[ZONE_AZSHARA]                = sWorld.getWorldState(ZoneToWorldState[ZONE_AZSHARA]);
        m_zoneData[ZONE_EASTERN_PLAGUELANDS]    = sWorld.getWorldState(ZoneToWorldState[ZONE_EASTERN_PLAGUELANDS]);
    }

    for (auto const& zoneData : m_zoneData)
    {
        uint32 zoneId = zoneData.first;

        sWorld.setWorldState(ZoneToWorldState[zoneId], m_zoneData[zoneId]);
        SendUpdateWorldState(ZoneToWorldState[zoneId], m_zoneData[zoneId]);
    }

    for (auto const& zone : m_zoneData)
        InitializeNecros(zone.first);

    m_initialized = true;
}

void ScourgeInvasion::Update(const uint32 uiDiff)
{
    if (sGameEventMgr.IsActiveEvent(EVENT_SCOURGE_INVASION) || sWorld.getWorldState(WORLD_STATE_DEAD_NECROPOLIS) != 0)
    {
        if (!sGameEventMgr.IsActiveEvent(EVENT_SCOURGE_INVASION_PERSISTENT))
            sGameEventMgr.StartEvent(EVENT_SCOURGE_INVASION_PERSISTENT, true);
    }

    if (!sGameEventMgr.IsActiveEvent(EVENT_SCOURGE_INVASION))
        return;

    if (!m_initialized)
        Initialize();

    if (m_spawnCheckTimer < uiDiff)
    {
        for (auto const& zone : m_zoneData)
        {
            uint32 zoneId = zone.first;
            if (uint64 timer = sWorld.getWorldState(ZoneToWorlStateTimers[zoneId]))
            {
                if (uint64(WorldTimer::getMSTime()) < timer)
                    continue;

                SetNecroWorldStatesReady(zoneId);
            }
        }

        m_spawnCheckTimer = 60000;
    }
    else m_spawnCheckTimer -= uiDiff;
}

void ScourgeInvasion::SetNecroWorldStatesReady(uint32 zoneId)
{
    for (auto const& necro : ZoneToNecroWorldStates[zoneId])
    {
        sWorld.setWorldState(necro, 2);
        sWorld.setWorldState(ZoneToWorlStateTimers[zoneId], 0);
    }

    m_zoneData[zoneId] = ZoneToMaxNecros[zoneId];
    sWorld.setWorldState(ZoneToWorldState[zoneId], m_zoneData[zoneId]);
    SendUpdateWorldState(ZoneToWorldState[zoneId], m_zoneData[zoneId]);
}

void ScourgeInvasion::InitializeNecros(uint32 zoneId)
{
    for (auto const& necro : ZoneToNecroWorldStates[zoneId])
    {
        if (sWorld.getWorldState(necro) == 0)
            sWorld.setWorldState(necro, 2);
    }
}

void ScourgeInvasion::FillInitialWorldStates(WorldPacket& data, uint32& count)
{
    if (!sGameEventMgr.IsActiveEvent(EVENT_SCOURGE_INVASION))
        return;

    GuardRead guard(m_lock);
    for (auto const& value : m_zoneData)
    {
        uint32 zoneId = value.first;
        uint32 worldState = ZoneToWorldState[zoneId];
        FillInitialWorldState(data, count, worldState, value.second > 0);
    }
}

void ScourgeInvasion::SendUpdateWorldState(uint32 field, uint32 value)
{
    GuardRead guard(m_lock);
    auto const& players = sObjectAccessor.GetPlayers();

    for (auto const& data : players)
        data.second->SendUpdateWorldState(field, value);
}

void ScourgeInvasion::OnNecropolisSpawn(Creature* creature)
{
    GuardWrite guard(m_lock);

    if (sWorld.getWorldState(creature->GetGUIDLow()))
        return;

    sWorld.setWorldState(creature->GetGUIDLow(), 2);
}

void ScourgeInvasion::OnNecropolisDespawn(Creature* creature)
{
    GuardWrite guard(m_lock);

    uint32 zoneId = creature->GetZoneId();

    if (m_zoneData[zoneId] > 0)
        m_zoneData[zoneId] -= 1;

    sWorld.setWorldState(WORLD_STATE_DEAD_NECROPOLIS, sWorld.getWorldState(WORLD_STATE_DEAD_NECROPOLIS) + 1);
    sWorld.setWorldState(ZoneToWorldState[zoneId], m_zoneData[zoneId]);
    sWorld.setWorldState(creature->GetGUIDLow(), 1);
    SendUpdateWorldState(ZoneToWorldState[zoneId], m_zoneData[zoneId]);

    // set timer for new invasion
    if (m_zoneData[zoneId] == 0)
    {
        uint64 timer = WorldTimer::getMSTime() + urand(MIN_NECRO_RESPAWN_TIME, MAX_NECRO_RESPAWN_TIME);
        sWorld.setWorldState(ZoneToWorlStateTimers[zoneId], timer);
    }
}

bool ScourgeInvasion::IsNecropolisReady(Creature* creature) const
{
    GuardRead guard(m_lock);
    return sWorld.getWorldState(creature->GetGUIDLow()) == 2;
}

uint32 ScourgeInvasion::GetDeadNecropolisCount() const
{
    GuardRead guard(m_lock);
    return sWorld.getWorldState(WORLD_STATE_DEAD_NECROPOLIS);
}

uint8 ScourgeInvasion::GetAliveNecropolisCount(uint32 zoneId /*= 0*/) const
{
    uint8 count = 0;

    {
        GuardRead guard(m_lock);
        for (auto const& data : m_zoneData)
        {
            if (!zoneId || data.first == zoneId)
                count += data.second;
        }
    }

    return count;
}
