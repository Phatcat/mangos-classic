#ifndef SCOURGE_IVASION
#define SCOURGE_IVASION

// Gossip Option Cultist Engineer
#define GOSSIP_DISRUPT_CULTIST  "Use 8 necrotic runes and disrupt his ritual."

// Gossip Option Reward NPCs
#define GOSSIP_NPC_REWARDS "Give me one of your magic items"

// Gossip Options Emissary and Messenger
#define GOSSIP_WHATS_HAPPENING "What's Happening?"
#define GOSSIP_WHAT_CAN_I_DO "What can I do?"
#define GOSSIP_WHERE_BATTLING "Where are we battling the Scourge?"
#define GOSSIP_BATTLES_WON "How many battles have we won?"
#define GOSSIP_GO_BACK "Go back"
#define GOSSIP_OPTION_AZSHARA "Azshara"
#define GOSSIP_OPTION_BLASTED_LANDS "Blasted Lands"
#define GOSSIP_OPTION_BURNING_STEPPES "Burning Steppes"
#define GOSSIP_OPTION_TANARIS "Tanaris Desert"
#define GOSSIP_OPTION_EASTERN_PLAGUELANDS "Eastern Plaguelands"
#define GOSSIP_OPTION_WINTERSPRING "Winterspring"

enum Npcs
{
    // NPCs
    NPC_NECROTIC_SHARD                      = 16136,
    NPC_DAMAGED_NECROTIC_SHARD              = 16172,
    NPC_CULTIST_ENGINEER                    = 16230,

    // Minions high level
    NPC_GHOUL_BERSERKER                     = 16141,
    NPC_SKELETAL_SHOCKTROOPER               = 16299,
    NPC_SPECTRAL_SOLDIER                    = 16298,

    // Rares
    NPC_RARE_LUMBERING_HORROR               = 14697,
    NPC_RARE_SPIRIT_OF_THE_DAMNED           = 16379,
    NPC_RARE_BONE_WITCH                     = 16380,

    // Upon disrupting Cultist Engineer's Trance
    NPC_SHADOW_OF_DOOM                      = 16143,

    // Minions low level
    NPC_SPECTRAL_APPARITION                 = 16423,
    NPC_SKELETAL_SOLDIER                    = 16422,
    NPC_SPECTRAL_SPIRIT                     = 16437,
    NPC_SKELETAL_TROOPER                    = 16438,

    // Invading Cities
    NPC_FLAMESHOCKER                        = 16383,
    NPC_STORMWIND_GUARD                     = 68,
    NPC_STORMWIND_ELITE_GUARD               = 16396,
    NPC_STORMWIND_ROYAL_GUARD               = 1756,
    NPC_UNDERCITY_GUARDIAN                  = 5624,
    NPC_UNDERCITY_DEATHGUARD_ELITE          = 7980,
    NPC_UNDERCITY_ELITE_GUARD               = 16432,
    NPC_UNDERCITY_ROAYL_DEATHGUARD          = 13839,
    NPC_VARIMATHRAS                         = 2425,
    NPC_BOLVAR                              = 1748,
};

enum Spells
{
    // Object / NPC Spell / Visual Ids
    SPELL_NECROPOLIS_TO_CAMPS_VISUAL        = 28326,
    SPELL_CAMP_TO_NECROPOLIS_VISUAL         = 28367,
    SPELL_COM_CAMP_RECEIVE_VISUAL           = 28449,
    SPELL_ACTIVATE_CAMP_VISUAL              = 28351,
    SPELL_MINIONS_SPAWN_VISUAL              = 17321,
    SPELL_DISRUPT_CULTIST_TRANCE_VISUAL     = 28234,
    SPELL_CREATE_CRYSTAL                    = 28344,
    SPELL_DAMAGE_CRYSTAL                    = 28041,
    SPELL_ZAP_CRYSTAL                       = 28032,
    SPELL_ZAP_DAMAGED_CRYSTAL               = 28056,
    SPELL_SUMMON_CRYSTAL_CORPSE             = 27895,
    SPELL_SUMMON_CRACKED_NECROTIC_CRYSTAL   = 28424,
    SPELL_SUMMON_FAINT_NECROTIC_CRYSTAL     = 28699,
    SPELL_SUMMON_SHADOW_OF_DOOM             = 31315,
    SPELL_SCOURGE_STRIKE                    = 28265,
    SPELL_CULTIST_ENGINEER_SPAWN_VISUAL     = 12980,
    SPELL_BUTTRESS_CHANNEL                  = 28078,

    // Minion Spell Ids
    SPELL_RIBBON_OF_SOULS                   = 16243,
    SPELL_BONE_SHARDS                       = 17014,
    SPELL_ARCANE_BOLT                       = 13748,
    SPELL_STRONG_CLEAVE                     = 8255,
    SPELL_HAMSTRING                         = 26141,
    SPELL_INFECTED_BITE                     = 7367,
    SPELL_GHOUL_ENRAGE                      = 8269,
    SPELL_SUNDER_ARMOR                      = 11971,
    SPELL_DEMORALIZING_SHOUT                = 16244,
    SPELL_STRIKE                            = 15580,
    SPELL_FEAR                              = 12096,
    SPELL_MINDFLAY                          = 22919,
    SPELL_SOUL_REVIVAL                      = 28681,
    SPELL_DARK_STRIKE                       = 19777,

    // Reward spells
    SPELL_CREATE_LESSER_MARK_OF_THE_DAWN    = 28319,
    SPELL_CREATE_MARK_OF_THE_DAWN           = 28320,
    SPELL_CREATE_GREATER_MARK_OF_THE_DAWN   = 28321,

    // Reward NPC mounts
    SPELL_SUMMON_AD_PALADIN_MOUNT           = 16082,
    SPELL_SUMMON_AD_CRUSADER_MOUNT          = 13819,
    SPELL_SUMMON_AD_CHAMPION_MOUNT          = 23214,
    SPELL_SUMMON_AD_INITIATE_MOUNT          = 17463,
    SPELL_SUMMON_AD_CLERIC_MOUNT            = 5784,
    SPELL_SUMMON_AD_PRIEST_MOUNT            = 23161,

    // Pallid Horror Spells
    SPELL_RUNNING_SPEED                     = 9175,
    SPELL_INCAPACITATING_SHOUT              = 18328,
    SPELL_KNOCK_AWAY                        = 11130,
    SPELL_FEAR_INVADERS                     = 27990,
    SPELL_STOMP                             = 12612,
    SPELL_TRAMPLE                           = 5568,
    SPELL_SUM_FAINT_NECRO_SHARD             = 28699,
    SPELL_SPIRIT_PARTICLES                  = 28126,

    // Patchwork Terror
    SPELL_CLEAVE                            = 19642,
    SPELL_THROW_AXE                         = 16075,
    SPELL_ECHOING_ROAR                      = 10967,
    SPELL_VICIOUS_REND                      = 14331,

    // Flameshocker
    SPELL_FLAMESHOCKER_TOUCH                = 28314,
    SPELL_FLAMESHOCKER_REVENGE              = 28323,
};

enum TextGossips
{
    // Reward npcs
    GOSSIP_TEXT_IF_SAME_FACTION             = 8,
    GOSSIP_TEXT_IF_ALLIANCE                 = 9,
    GOSSIP_TEXT_IF_HORDE                    = 10,

    // Emissary and Messenger
    GOSSIP_TEXT_IF_NOT_INVADED              = 15,
    GOSSIP_TEXT_EMISSARY_HELLO              = 11,
    GOSSIP_TEXT_WHAT_IS_HAPPENING           = 12,
    GOSSIP_TEXT_WHAT_CAN_I_DO               = 13,
    GOSSIP_TEXT_WHERE_BATTLING              = 14,
    GOSSIP_TEXT_NOT_INVADED                 = 15,
    GOSSIP_TEXT_AZSHARA_INVADED             = 16,
    GOSSIP_TEXT_BLASTED_LANDS_INVADED       = 17,
    GOSSIP_TEXT_BURNING_STEPPES_INVADED     = 18,
    GOSSIP_TEXT_TANARIS_INVADED             = 19,
    GOSSIP_TEXT_EASTERN_PLAGUELANDS_INVADED = 20,
    GOSSIP_TEXT_WINTERSPRING_INVADED        = 21,
    GOSSIP_TEXT_BATTLES_WON                 = 22,

    // Emote Texts Argent Messenger
    TEXT_AZSHARA_INVADED                    = -1534051,
    TEXT_BLASTED_LANDS_INVADED              = -1534052,
    TEXT_BURNING_STEPPES_INVADED            = -1534053,
    TEXT_TANARIS_INVADED                    = -1534054,
    TEXT_EASTERN_PLAGUELANDS_INVADED        = -1534055,
    TEXT_WINTERSPRING_INVADED               = -1534056,
};

enum Misc
{
    // Camp Types
    CAMP_TYPE_GHOUL_SKELETON                = 1,
    CAMP_TYPE_GHOST_GHOUL                   = 2,
    CAMP_TYPE_GHOST_SKELETON                = 3,

    // Scourge Invaders Factions
    FACTION_UNDEAD_SCOURGE                  = 21,

    // Required amount of killed Minions
    // to spawn a rare Minion
    REQUIRED_DEAD_MINIONS_MIN               = 175,
    REQUIRED_DEAD_MINIONS_MAX               = 225,

    // Items
    ITEM_NECROTIC_RUNE                      = 22484,
    ITEM_LESSER_MARK_OF_THE_DAWN            = 23194,
    ITEM_MARK_OF_THE_DAWN                   = 23195,
    ITEM_GREATER_MARK_OF_THE_DAWN           = 23196,

    // Emotes
    EMOTE_CRY                               = 11,
    EMOTE_LAUGH                             = 18,
    EMOTE_TALK                              = 1,

    // Waypoints
    WAYPOINTS_SW_1                          = 7,
    WAYPOINTS_SW_2                          = 9,
    WAYPOINTS_SW_3                          = 10,
    WAYPOINTS_UC_1                          = 8,
    WAYPOINTS_UC_2                          = 11,
    WAYPOINTS_UC_3                          = 12,

    // Respawn timers
    MINIONS_RESPAWN_TIME_MIN                = 300,
    MINIONS_RESPAWN_TIME_MAX                = 480,

    // City Invader Yell Texts
    CITY_INVADER_SPAWN_YELL_1               = -1534057,
    CITY_INVADER_SPAWN_YELL_2               = -1534058,
};

struct MinionToRespawn
{
    MinionToRespawn(uint32 entry, time_t when) : m_entry(entry), m_when(when) {}

    uint32 m_entry;
    time_t m_when;
};

struct MinionToSpawn
{
    MinionToSpawn(uint32 entry, Position const& position) : m_entry(entry), m_position(position) {}

    uint32 m_entry;
    Position m_position;
};

static std::vector<uint32> m_invadedZonesGossips =
{
    GOSSIP_TEXT_AZSHARA_INVADED,
    GOSSIP_TEXT_BLASTED_LANDS_INVADED,
    GOSSIP_TEXT_BURNING_STEPPES_INVADED,
    GOSSIP_TEXT_TANARIS_INVADED,
    GOSSIP_TEXT_EASTERN_PLAGUELANDS_INVADED,
    GOSSIP_TEXT_WINTERSPRING_INVADED
};

static std::vector<uint32> m_invadedZoneIds =
{
    ZONE_AZSHARA,
    ZONE_BLASTED_LANDS,
    ZONE_BURNING_STEPPES,
    ZONE_TANARIS,
    ZONE_EASTERN_PLAGUELANDS,
    ZONE_WINTERSPRING
};

static std::vector<int32> m_invadedZoneTextIds =
{
    TEXT_AZSHARA_INVADED,
    TEXT_BLASTED_LANDS_INVADED,
    TEXT_BURNING_STEPPES_INVADED,
    TEXT_TANARIS_INVADED,
    TEXT_EASTERN_PLAGUELANDS_INVADED,
    TEXT_WINTERSPRING_INVADED
};

static std::map<int32, uint32> m_swPosXToCustomWaypointId =
{
    { -8665,     WAYPOINTS_SW_1 },
    { 0,         WAYPOINTS_SW_2 },
    { 1,         WAYPOINTS_SW_3 }
};

static std::map<int32, uint32> m_ucPosXToCustomWaypointId =
{
    { 1791,     WAYPOINTS_UC_1 },
    { 1722,     WAYPOINTS_UC_2 },
    { 1814,     WAYPOINTS_UC_3 }
};

static std::map<uint32, uint32> m_zoneToCityGuardId =
{
    { ZONE_STORMWIND,   NPC_STORMWIND_GUARD },
    { ZONE_UNDERCITY,   NPC_UNDERCITY_GUARDIAN },
};

static std::map<uint32, uint32> m_zoneToCityEliteGuardId =
{
    { ZONE_STORMWIND,   NPC_STORMWIND_ELITE_GUARD },
    { ZONE_UNDERCITY,   NPC_UNDERCITY_ELITE_GUARD },
};

static std::map<int32, int32> m_cityAreaMarkerPositionToTextId =
{
    // Park
    { -8757, -1534010 },
    // Mage Quarters
    { -8938, -1534011 },
    // Cathedral Square
    { -8636, -1534012 },
    // Trade District
    { -8803, -1534013 },
    // Dwarfen Disctrict
    { -8423, -1534014 },
    // Old Town
    { -8678, -1534015 },
    // Stormwind Keep
    { -8507, -1534016 },
    // Stormwind Keep King
    { -8439, -1534017 },
    // Trade Quarter
    { 1596,  -1534018 },
    // Magic Quarter
    { 1721,  -1534024 },
    // War Quarter
    { 1752,  -1534019 },
    // Apathecarium
    { 1497,  -1534020 },
    // Rogue Quarter
    { 1479,  -1534021 },
    // Royal
    { 1416,  -1534022 },
    // Royal Queen
    { 1298,  -1534023 },
};

static std::vector<int32> m_messengersNotAnnouncingPosX =
{
     1739,
    -8948,
     1497,
    -1260,
     9934
};

#endif
