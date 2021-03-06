/*
 * Copyright (C) 2008-2018 TrinityCore <https://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ScriptMgr.h"
#include "InstanceScript.h"
#include "deadmines.h"
#include "TemporarySummon.h"
#include "WorldPacket.h"

ObjectData const creatureData[] =
{
    { BOSS_GLUBTOK,     DATA_GLUBTOK    },
    { 0,                0               }, // END
};

ObjectData const gameobjectData[] =
{
    { 0,            0 }, // END
};

DoorData const doorData[] =
{
    { GO_FACTORY_DOOR,      DATA_GLUBTOK,       DOOR_TYPE_PASSAGE   },
    { 0,                    0,                  DOOR_TYPE_ROOM      }, // END
};

class instance_deadmines : public InstanceMapScript
{
    public:
        instance_deadmines() : InstanceMapScript(DMScriptName, 36) { }

        struct instance_deadmines_InstanceMapScript : public InstanceScript
        {
            instance_deadmines_InstanceMapScript(InstanceMap* map) : InstanceScript(map)
            {
                SetHeaders(DataHeader);
                SetBossNumber(map->IsHeroic() ? EncounterCount : EncounterCount - 1); // Vanessa van Cleef only in heroic mode
                LoadDoorData(doorData);
                LoadObjectData(creatureData, gameobjectData);
                _teamInInstance = 0;
            }

            void OnPlayerEnter(Player* player) override
            {
                if (!_teamInInstance)
                    _teamInInstance = player->GetTeam();
            }

            void OnCreatureCreate(Creature* creature) override
            {
                InstanceScript::OnCreatureCreate(creature);

                switch (creature->GetEntry())
                {
                    // Horde restricted creatures
                    case NPC_SLINKY_SHARPSHIV:
                    case NPC_KAGTHA:
                    case NPC_MISS_MAYHEM:
                    case NPC_SHATTERED_HAND_ASSASSIN:
                    case NPC_MAYHEM_REAPER_PROTOTYPE:
                        if (_teamInInstance != HORDE)
                            creature->SetVisible(false);
                        break;
                    // Alliance restricted creatures
                    case NPC_STORMWIND_INVESTIGATOR:
                    case NPC_CRIME_SCENE_ALARM_O_BOT:
                    case NPC_STORMWIND_DEFENDER:
                    case NPC_LIEUTENANT_HORATIO_LAINE:
                    case NPC_QUARTERMASTER_LEWIS:
                        if (_teamInInstance != ALLIANCE)
                            creature->SetVisible(false);
                        break;
                    case NPC_FIREWALL_PLATTER_1A:
                    case NPC_FIREWALL_PLATTER_1B:
                    case NPC_FIREWALL_PLATTER_1C:
                    case NPC_FIREWALL_PLATTER_2A:
                    case NPC_FIREWALL_PLATTER_2B:
                    case NPC_FIREWALL_PLATTER_2C:
                        if (Creature* glubtok = GetCreature(DATA_GLUBTOK))
                            glubtok->AI()->JustSummoned(creature);
                        break;
                    default:
                        break;
                }
            }

            void SetData(uint32 type, uint32 data) override
            {
                switch (type)
                {
                    case DATA_TEAM_IN_INSTANCE:
                        _teamInInstance = data;
                        SaveToDB();
                        break;
                    default:
                        break;
                }
            }

            uint32 GetData(uint32 type) const override
            {
                switch (type)
                {
                    case DATA_TEAM_IN_INSTANCE:
                        return _teamInInstance;
                    default:
                        return 0;
                }
            }

            void WriteSaveDataMore(std::ostringstream& data) override
            {
                data << _teamInInstance;
            }

            void ReadSaveDataMore(std::istringstream& data) override
            {
                data >> _teamInInstance;
            }

        protected:
            uint32 _teamInInstance;
            GuidSet _arcaneBeamBunnyGUIDList;
        };


        InstanceScript* GetInstanceScript(InstanceMap* map) const override
        {
            return new instance_deadmines_InstanceMapScript(map);
        }
};

void AddSC_instance_deadmines()
{
    new instance_deadmines();
}
