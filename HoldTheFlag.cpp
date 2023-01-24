// HoldTheFlag.cpp
//

#include "bzfsAPI.h"

int checkRange(int min, int max, int amount) {
    int num = 0;
    if ((amount >= min) && (amount <= max)) {
        num = 1;
    } else if ((amount < min) || (amount > max)) {
        num = 0;
    } else {
        num = -1;
    }
    return num;
}

int checkPlayerSlot(int player) {
    return checkRange(0,199,player); // 199 because of array.
}

// Checks if posX and posY are within border range of x and y.
// a.k.a. x and y form a point and if pos is within the range of a border formed at x and y point.
int ifWithinBorder(float x, float y, float posX, float posY, float border) {
    float rangeX[2];
    float rangeY[2];
    rangeX[0] = (x + border);
    rangeX[1] = (x - border);
    rangeY[0] = (y + border);
    rangeY[1] = (y - border);
    // If within the range, both checks should pass and we should get 1 in return.
    if ((posX <= rangeX[0]) && (posX >= rangeX[1])) {
        if ((posY <= rangeY[0]) && (posY >= rangeY[1])) {
            return 1;
        }
    }
    return 0;
}

bz_eTeamType flagToTeamValue(const char* flagType) {
    if (strcmp("R*", flagType) == 0) {
        return eRedTeam;
    } else if (strcmp("G*", flagType) == 0) {
        return eGreenTeam;
    } else if (strcmp("B*", flagType) == 0) {
        return eBlueTeam;
    } else if (strcmp("P*", flagType) == 0) {
        return ePurpleTeam;
    }  else {
        return eNoTeam;
    }
}

void killTeamByPlayer(bz_eTeamType teamToKill, int killedByPlayer) {
    if (teamToKill != eObservers) {
        bz_APIIntList *player_list = bz_newIntList();
        bz_getPlayerIndexList(player_list);

        for (unsigned int i = 0; i < player_list->size(); i++) {
            if (bz_getPlayerTeam(player_list->get(i)) == teamToKill) {
                bz_killPlayer((player_list->get(i)), false, killedByPlayer);
            }
        }
        bz_deleteIntList(player_list);
    }
}

class HoldTheFlag : public bz_Plugin
{
public:
    virtual const char* Name () {return "HoldTheFlag";}
    virtual void Init ( const char* config );
    virtual void Event ( bz_EventData *  eventData  );
    virtual void Cleanup();
    bool botJoin = false;
    float boxpoint[2]={-270.0, 0.0};
    float basepoint[2]={270.0, 0.0};
    float boxSize[2]={30.0, 30.0};
    int spawnSelection[200];
};

BZ_PLUGIN(HoldTheFlag)

void HoldTheFlag::Init ( const char* /*commandLine*/ )
{
    bz_debugMessage(4,"HoldTheFlag plugin loaded");

    Register(bz_eAllowSpawn);
    Register(bz_eGetAutoTeamEvent);
    Register(bz_ePlayerUpdateEvent);
    Register(bz_eGetPlayerSpawnPosEvent);
    Register(bz_ePlayerSpawnEvent);
    Register(bz_ePlayerPartEvent);
}

void HoldTheFlag::Cleanup() {
    Flush();
}

void HoldTheFlag::Event ( bz_EventData *eventData ) {
    switch (eventData->eventType) {

        case bz_eAllowSpawn: {
            bz_AllowSpawnData_V2* allowSpawn = (bz_AllowSpawnData_V2*)eventData;
            if (allowSpawn->team == eGreenTeam) {
                allowSpawn->allow = true;
                allowSpawn->kickPlayer = false;
                botJoin = true;
            }

    // Data
    // ---
    // (int)          playerID - This value is the player ID for the joining player.
    // (bz_eTeamType) team - The team the player belongs to.
    // (bool)         handled - Whether or not the plugin will be handling the respawn or not.
    // (bool)         allow - Set to false if the player should not be allowed to spawn.
    // (bool)         kickPlayer - Set to false to prevent kicking the player for not being allowed to spawn.
    // (bz_ApiString) kickReason - The logged reason the player is being kicked if `kickPlayer` is true.
    // (bz_ApiString) message - The message sent to the player that is not allowed to spawn if `allow` is false; this message is only sent to each player once.
    // (double)       eventTime - The server time the event occurred (in seconds.)
        }break;

        case bz_eGetAutoTeamEvent: {
            bz_GetAutoTeamEventData_V1* autoTeamData = (bz_GetAutoTeamEventData_V1*)eventData;
            if (botJoin == true) {
                if (autoTeamData->team != eObservers) {
                    autoTeamData->team = eRogueTeam;
                    autoTeamData->handled = true;
                }
            }
// Data
// ---
// (int)          playerID - ID of the player that is being added to the game.
// (bz_ApiString) callsign - Callsign of the player that is being added to the game.
// (bz_eTeamType) team - The team that the player will be added to.
// (bool)         handled - The current state representing if other plug-ins have modified the default team.
// (double)       eventTime - This value is the local server time of the event.
        }break;

        case bz_ePlayerUpdateEvent: {
            bz_PlayerUpdateEventData_V1* updateData = (bz_PlayerUpdateEventData_V1*)eventData;
            float pos[3];
            pos[0] = updateData->lastState.pos[0];
            pos[1] = updateData->lastState.pos[1];

            if (checkPlayerSlot(updateData->playerID)==1) {
                if (((boxpoint[0] + boxSize[0]) >= pos[0]) && ((boxpoint[0] - boxSize[0]) <= pos[0])) {
                    if (((boxpoint[1] + boxSize[1]) >= pos[1]) && ((boxpoint[1] - boxSize[1]) <= pos[1])) {
                        int flagID = bz_getPlayerFlagID(updateData->playerID);
                        if (flagID != -1) {
                            bz_eTeamType flagTeam = flagToTeamValue(bz_getFlagName(flagID).c_str());
                            printf("%s\n", bz_getFlagName(flagID).c_str());
                            
                            if (flagTeam == eGreenTeam) {
                            puts("made it");
                                bz_resetFlag(flagID);
                                bz_killPlayer(updateData->playerID, false, BZ_SERVER);
                                bz_incrementPlayerLosses(updateData->playerID, -1);
                                killTeamByPlayer(eRogueTeam, updateData->playerID);
                                for(int i=0; i<=199;i++) {
                                    spawnSelection[i]=1;
                                }
                            }
                        }
                    }
                }
            }
            

// Data
// ---
// (int)          playerID - ID of the player that sent the update
// (bz_PlayerUpdateState) state - The original state the tank was in
// (bz_PlayerUpdateState) lastState - The second state the tank is currently in to show there was an update
// (double)       stateTime - The time the state was updated
// (double)       eventTime - The current server time
        }break;

        case bz_eGetPlayerSpawnPosEvent: {
            bz_GetPlayerSpawnPosEventData_V1* spawnPosData = (bz_GetPlayerSpawnPosEventData_V1*)eventData;
            int player = spawnPosData->playerID;
            if (checkPlayerSlot(player)==1) {
                if (spawnSelection[player] == 1) {
                    int solve=0;
                    // If we try to solve problem, we are not done yet.
                    // Give it 25 tries and if we aren't done by then, we simply give out default spawn.
                    // Maybe up amount a bit?
                    float spawningPos[3]={0.0, 0.0, 0.0};
                    float rotation=0.0;
                    float *spawningRot;
                    spawningRot = &rotation;
                    for(int i = 0; i <= 14000; i++) {
                        bz_getStandardSpawn(player, spawningPos, spawningRot);
                        solve = ifWithinBorder(basepoint[0], basepoint[1], spawningPos[0], spawningPos[1], 30.0);
                        //bz_sendTextMessagef(BZ_SERVER,player,"(X = %.2f) (Y = %.2f) (XPos = %.2f) (YPos = %.2f)", 
                            //pos[0], pos[1], spawningPos[0], spawningPos[1]);
                        if (solve == 1) {
                            bz_sendTextMessagef(BZ_SERVER,player,"%d attempts to calculate spawn position.", i);
                            break;
                        }
                    }
                    if (solve == 1) {
                        spawnPosData->handled = true;
                        spawnPosData->pos[0]= spawningPos[0];
                        spawnPosData->pos[1]= spawningPos[1];
                        spawnPosData->pos[2]= spawningPos[2];
                        spawnPosData->rot   = rotation;
                    } else { 
                    // This message isn't sent to players who have not selected a spawn. (Conditionals handle this properly.)
                        bz_sendTextMessage(BZ_SERVER,player,"Spawn calculation failed. Using default spawn algorithm.");
                        // This message seems more like a debug message, is this a good thing or not?
                    }
                }
            }

// Data
// ---
// (int)          playerID - ID of the player that is requesting the spawn position.
// (bz_eTeamType) team - The team the player is currently in.
// (bool)         handled - The current state representing if other plug-ins have modified the spawn position.
// (float[3])     pos - Position where the player will be spawned. This value is initialized to the server
// (float)        rot - The rotational direction that the player will be spawned at. This value is initialized
// (double)       eventTime - The local server time of the event.
        }break;

        case bz_ePlayerSpawnEvent: {
        bz_PlayerSpawnEventData_V1* spawnData = (bz_PlayerSpawnEventData_V1*)eventData;
        int player = spawnData->playerID;
        if (checkPlayerSlot(player) == 1) {
            if (spawnSelection[player] == 1) {
                spawnSelection[player] = -1;
            }
        }
        
        }break;


        case bz_ePlayerPartEvent: {
        bz_PlayerJoinPartEventData_V1* partData = (bz_PlayerJoinPartEventData_V1*)eventData;
        int player = partData->playerID;
        if (checkPlayerSlot(player) == 1) {
            if (spawnSelection[player] == 1) {
                spawnSelection[player] = -1;
            }
        }
        
        }break;

        default:{ 
        }break;
        
    }
}
