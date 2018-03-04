#ifndef SCOURGEINVASION_H
#define SCOURGEINVASION_H

#include "../framework/Policies/Lock.h"

enum ScourgeInvasionData
{
    WORLD_STATE_EASTERN_PLAGUELANDS     = 2264,
    WORLD_STATE_TANARIS                 = 2263,
    WORLD_STATE_BURNING_STEPPES         = 2262,
    WORLD_STATE_BLASTED_LANDS           = 2261,
    WORLD_STATE_AZSHARA                 = 2260,
    WORLD_STATE_WINTERSPRING            = 2259,

    WORLD_STATE_EPL_TIMER               = 30001,
    WORLD_STATE_TANARIS_TIMER           = 30002,
    WORLD_STATE_BS_TIMER                = 30003,
    WORLD_STATE_BL_TIMER                = 30004,
    WORLD_STATE_AZSHARA_TIMER           = 30005,
    WORLD_STATE_WS_TIMER                = 30006,

    WORLD_STATE_DEAD_NECROPOLIS         = 2503,

    EVENT_SCOURGE_INVASION              = 17,
    EVENT_SCOURGE_INVASION_PERSISTENT   = 45,

    ZONE_TANARIS                        = 440,
    ZONE_WINTERSPRING                   = 618,
    ZONE_BURNING_STEPPES                = 46,
    ZONE_AZSHARA                        = 16,
    ZONE_BLASTED_LANDS                  = 4,
    ZONE_EASTERN_PLAGUELANDS            = 139,
    ZONE_STORMWIND                      = 1519,
    ZONE_UNDERCITY                      = 1497,
    ZONE_ELWYNN                         = 12,
    ZONE_DUN_MOROGH                     = 1,
    ZONE_TIRISFAL_GLADES                = 85,
    ZONE_MULGORE                        = 215,
    ZONE_DUROTAR                        = 14,
    ZONE_TELDRASSIL                     = 141,

    NPC_AD_INITIATE                     = 16384,
    NPC_AD_CLERIC                       = 16435,
    NPC_AD_PRIEST                       = 16436,
    NPC_AD_PALADIN                      = 16395,
    NPC_AD_CRUSADER                     = 16433,
    NPC_AD_CHAMPION                     = 16434,

    NPC_PALLID_HORROR                   = 16394,
    NPC_PATCHWORK_TERROR                = 16382,

    NPC_NECROPOLIS                      = 16401,
    NPC_NECROPOLIS_CONTROLLER           = 16214,

    NPC_SYLVANNAS_WINDRUNNER            = 10181,

    REWARD_STAGE_1                      = 50,
    REWARD_STAGE_2                      = 100,
    REWARD_STAGE_3                      = 150,

    NECROS_MAX_BLASTED_LANDS            = 2,
    NECROS_MAX_BURNING_STEPPES          = 2,
    NECROS_MAX_EASTERN_PLAGUELANDS      = 2,
    NECROS_MAX_WINTERSPRING             = 3,
    NECROS_MAX_AZSHARA                  = 2,
    NECROS_MAX_TANARIS                  = 3,

    GO_CIRCLE                           = 181227,
    GO_UNDEAD_FIRE                      = 181173,
    GO_UNDEAD_FIRE_AURA                 = 181174,
    GO_NECROPOLIS                       = 181223,

    MIN_NECRO_RESPAWN_TIME              = 4 * HOUR,
    MAX_NECRO_RESPAWN_TIME              = 6 * HOUR,
};

static std::map<uint32, uint32> ZoneToWorldState =
{
    {ZONE_TANARIS, WORLD_STATE_TANARIS},
    {ZONE_WINTERSPRING, WORLD_STATE_WINTERSPRING},
    {ZONE_BURNING_STEPPES, WORLD_STATE_BURNING_STEPPES},
    {ZONE_BLASTED_LANDS, WORLD_STATE_BLASTED_LANDS},
    {ZONE_AZSHARA, WORLD_STATE_AZSHARA},
    {ZONE_EASTERN_PLAGUELANDS, WORLD_STATE_EASTERN_PLAGUELANDS}
};

static std::map<uint32, uint32> ZoneToMaxNecros =
{
    { ZONE_TANARIS, NECROS_MAX_TANARIS },
    { ZONE_WINTERSPRING, NECROS_MAX_WINTERSPRING },
    { ZONE_BURNING_STEPPES, NECROS_MAX_BURNING_STEPPES },
    { ZONE_BLASTED_LANDS, NECROS_MAX_BLASTED_LANDS },
    { ZONE_AZSHARA, NECROS_MAX_AZSHARA },
    { ZONE_EASTERN_PLAGUELANDS, NECROS_MAX_EASTERN_PLAGUELANDS }
};

static std::map<uint32, uint32> ZoneToBossEntries =
{
    { ZONE_STORMWIND, 1748 },
    { ZONE_UNDERCITY, NPC_SYLVANNAS_WINDRUNNER },
};

static std::map<uint32, int32> ZoneToBossTextId =
{
    { ZONE_STORMWIND, -1000026 },
    { ZONE_UNDERCITY, -1000025 },
};

static std::map<uint32, uint64> ZoneToWorlStateTimers =
{
    { ZONE_EASTERN_PLAGUELANDS, WORLD_STATE_EPL_TIMER },
    { ZONE_TANARIS ,            WORLD_STATE_TANARIS_TIMER },
    { ZONE_BURNING_STEPPES,     WORLD_STATE_BS_TIMER },
    { ZONE_BLASTED_LANDS,       WORLD_STATE_BL_TIMER },
    { ZONE_AZSHARA,             WORLD_STATE_AZSHARA_TIMER },
    { ZONE_WINTERSPRING,        WORLD_STATE_WS_TIMER },
};

static std::map<uint32, std::vector<uint32>> ZoneToNecroWorldStates =
{
    { ZONE_EASTERN_PLAGUELANDS, { 110252, 110253 } },
    { ZONE_TANARIS , { 110258, 110259, 110261 } },
    { ZONE_BURNING_STEPPES, { 110256, 110257 } },
    { ZONE_BLASTED_LANDS, { 110254, 110255 } },
    { ZONE_AZSHARA, { 110260, 110263 } },
    { ZONE_WINTERSPRING, { 110250, 110251, 110262 } },
};

struct ZoneInfo
{
    ZoneInfo(uint8 necrosSpawned = 0, uint8 necrosRespawned = 0) : m_necrosSpawned(necrosSpawned), m_necrosRespawned(necrosRespawned) {}

    uint8 m_necrosRespawned;
    uint8 m_necrosSpawned;
};

class ScourgeInvasion : public MaNGOS::Singleton<ScourgeInvasion>
{
    public:
        ScourgeInvasion();
        ~ScourgeInvasion() {}

        bool m_initialized;
        uint32 m_spawnCheckTimer;

        void FillInitialWorldStates(WorldPacket& data, uint32& count);
        void SendUpdateWorldState(uint32 field, uint32 value);

        void OnNecropolisSpawn(Creature* creature);
        void OnNecropolisDespawn(Creature* creature);
        void SetNecroWorldStatesReady(uint32 zoneId);
        void InitializeNecros(uint32 zoneId);
        uint8 GetAliveNecropolisCount(uint32 zoneId = 0) const;
        bool IsNecropolisReady(Creature* creature) const;
        uint32 GetDeadNecropolisCount() const;

        void Update(const uint32 diff);
    private:
        void Initialize();

        std::map<uint32, uint8> m_zoneData;

        std::map<uint32, ZoneInfo> m_zoneInfo;

        typedef std::mutex LOCK_TYPE;
        typedef std::lock_guard<LOCK_TYPE> LOCK_GUARD;

        mutable LOCK_TYPE m_lock;
};

#define sScourgeInvasionMgr MaNGOS::Singleton<ScourgeInvasion>::Instance()

#endif