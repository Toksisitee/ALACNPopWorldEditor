#pragma once
/*

Alacn
alacn.uhahaa@gmail.com
http://alacn.dnsalias.org:8080/

*/

#define POPEDT_MENUBAR_CLASS						"PopEdtMenuBar"
#define POPEDT_MENUBAR_NAME							"Menu"

#define POPEDT_MENUBAR								"PopEdtMenuBar"
#define POPEDT_MENUBAR_WIDTH						120+9
#define POPEDT_MENUBAR_HEIGHT						50+13


#define SZ_MENU_FILE								"File"
#define SZ_MENU_EDIT								"Edit"
#define SZ_MENU_OPTIONS								"Options"

#define SZ_DEVICECONFIRM_TIME						"Auto Restoring in %d seconds..."

// Network
#define SZ_CONFIRM_NEW_LEVEL_NET					"Request received to start a new map. Do you accept?"
#define SZ_CONFIRM_NEW_LEVEL_CD						"You've already sent a request!"
#define SZ_CONFIRM_SYNC_LEVEL_ASK					"You are about to send a request to sync maps.\nYour map will be erased if the other user accepts. Continue?"
#define SZ_CONFIRM_SYNC_LEVEL						"Peer sent you a request for a map update.\nDo you accept?"
#define SZ_SYNC_LEVEL_WARNING						"Once the process begins please wait until the maps are synced!\nClose this dialog to begin."
//================================================================================================
#define SZ_CONFIRM_NEW_LEVEL						"Current Map will be lost!\ndo you want continue?"
#define SZ_CONFIRM_MAX_L2_OBJECTS					"Warning, this map has more objects than supported by Level2\nwould you like to switch to Level3 format instead?\n\nPressing No will result in some objects being lost."

#define SZ_OPENLEVEL_TITLE							"Open Level"
#define SZ_OPENLEVEL_FILTER							"Populous Levels Dat Files (*.dat)\0levl2*.dat\0All Files (*.*)\0*.*\0\0"
#define SZ_SAVELEVEL_FILTER							"Populous Level3 Format (*.dat)\0levl2*.dat\0Populous Level2 Format (*.hdr;*.dat)\0levl2*.dat\0\0"
#define SZ_OPENLEVEL_FAILED							"Open Level Failed"
#define SZ_SAVELEVEL_TITLE							"Save Level"
#define SZ_SAVELEVEL_DEFEXT							".dat"
#define SZ_SAVELEVEL_FAILED							"Save Level Failed"
#define SZ_SAVELEVEL_DONE_V2						"Level Saved using Level2 Format"
#define SZ_SAVELEVEL_DONE_V3						"Level Saved using Level3 Format"
#define SZ_EXPORT_HEIGHTMAP_TITLE					"Export Height Map"
#define SZ_EXPORT_HEIGHTMAP_FILTER					"Bitmaps (*.bmp)\0*.bmp\0All Files (*.*)\0*.*\0\0"
#define SZ_EXPORT_HEIGHTMAP_DEFEXT					".bmp"
#define SZ_EXPORT_HEIGHTMAP_FAILED					"Export HeightMap Failed"
#define SZ_EXPORT_HEIGHTMAP_DONE					"Done"

#define SZ_OPEN_AIATTRIB_TITLE						"Open AI Attributes"
#define SZ_OPEN_AIATTRIB_FILTER						"AI Attributes (cpatr*.dat)\0cpatr*.dat\0Dat Files (*.dat)\0*.dat\0All Files (*.*)\0*.*\0\0"
#define SZ_OPEN_AIATTRIB_FAILED						"Open AI Attributes Failed"
#define SZ_SAVE_AIATTRIB_TITLE						"Save AI Attributes"
#define SZ_SAVE_AIATTRIB_DEFEXT						".dat"
#define SZ_SAVE_AIATTRIB_FAILED						"Save AI Attributes Failed"
#define SZ_SAVE_AIATTRIB_DONE						"Done"
#define SZ_AIATTRIB_SAVE_CHANGE						"Do you want save AI Attributes changes?"

#define SZ_LANG2TEXT_SRC_TITLE						"Language To Text - Language"
#define SZ_LANG2TEXT_DST_TITLE						"Language To Text - Text"
#define SZ_TEXT2LANG_SRC_TITLE						"Text To Language - Text"
#define SZ_TEXT2LANG_DST_TITLE						"Text To Language - Language"
#define SZ_LANG_LANG_FILTER							"Populous Language (lang*.dat)\0lang*.dat\0All Files (*.*)\0*.*\0\0"
#define SZ_LANG_LANG_DEFEXT							".dat"
#define SZ_LANG_TEXT_FILTER							"Text File (*.txt)\0*.txt\0All Files (*.*)\0*.*\0\0"
#define SZ_LANG_TEXT_DEFEXT							".txt"
#define SZ_LANG_DONE								"Done"
#define SZ_LANG_SRC_OPEN_FAILED						"Open Source File Failed"
#define SZ_LANG_SRC_READ_FAILED						"Read Source File Failed"
#define SZ_LANG_DST_CREATE_FAILED					"Create Destiny File Failed"
#define SZ_LANG_DST_WRITE_FAILED					"Write Destiny File Failed"

#define SZ_OPTIONS									"Options"
#define SZ_ROTATE									"Rotate"
#define SZ_LANDBRIDGE_START							"Set Start"
#define SZ_LANDBRIDGE_END							"Set End"

#define SZ_COMPILE_SRC								"Text"
#define SZ_COMPILE_DST								"Script"
#define SZ_DECOMPILE_SRC							"Script"
#define SZ_DECOMPILE_DST							"Text"

#define SZ_COMPILE_LOG_CREATE_FAILED				"Create Log File Failed: '%s'"
#define SZ_COMPILE_SRC_OPEN_FAILED					"Open Source File Failed: '%s'"
#define SZ_COMPILE_SRC_READ_FAILED					"Read Source File Failed: '%s'"
#define SZ_COMPILE_DST_CREATE_FAILED				"Create Destiny File Failed: '%s'"
#define SZ_COMPILE_DST_WRITE_FAILED					"Write Destiny File Failed: '%s'"
#define SZ_COMPILE_DONE								"Compiled Successful!"
#define SZ_COMPILE_FAILED							"Compilation Failed.\nCheck the log file for details."
#define SZ_DECOMPILE_DST_CREATE_FAILED				"Create Destiny File Failed: '%s'"
#define SZ_DECOMPILE_SRC_OPEN_FAILED				"Open Source File Failed: '%s'"
#define SZ_DECOMPILE_SRC_READ_FAILED				"Read Source File Failed: '%s'"
#define SZ_DECOMPILE_DONE							"Decompiled Successful!"
#define SZ_DECOMPILE_FAILED							"Decompilation Failed.\nCheck the decompiled file for details."

#define SZ_AISCRIPT_LOG_TITLE						"Log File"
#define SZ_AISCRIPT_LOG_DEFEXT						".log"
#define SZ_AISCRIPT_LOG_FILTER						"Log Files (*.log)\0*.log\0Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0\0"
#define SZ_AISCRIPT_TXT_TITLE						"Text File"
#define SZ_AISCRIPT_TXT_DEFEXT						".txt"
#define SZ_AISCRIPT_TXT_FILTER						"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0\0"
#define SZ_AISCRIPT_SCR_TITLE						"AI Script File"
#define SZ_AISCRIPT_SCR_DEFEXT						".dat"
#define SZ_AISCRIPT_SCR_FILTER						"AI Script File (cpscr*.dat)\0cpscr*.dat\0Dat Files (*.dat)\0*.dat\0All Files (*.*)\0*.*\0\0"
#define SZ_AISCRIPT_MISSING_LOG						"Please Save the Log file first."
#define SZ_AISCRIPT_MISSING_SRC						"Please Open the Source file first."



#define COLOR_LIST_NEUTRAL							0x005C5C5C // ABGR
#define COLOR_LIST_BLUE								0x00800000
#define COLOR_LIST_RED								0x00000080
#define COLOR_LIST_YELLOW							0x00008080
#define COLOR_LIST_GREEN							0x00008000
#define COLOR_LIST_CYAN 							0x00FFFF00
#define COLOR_LIST_MAGENTA							0x00FF00FF
#define COLOR_LIST_BLACK							0x002E2E2E
#define COLOR_LIST_ORANGE							0x000048FF
#define COLOR_LIST_BG								0x00FFFFFF



#define IDX_OWNER_NEUTRAL							0
#define IDX_OWNER_BLUE								1
#define IDX_OWNER_RED								  2
#define IDX_OWNER_YELLOW							3
#define IDX_OWNER_GREEN								4
#define IDX_OWNER_CYAN								5
#define IDX_OWNER_MAGENTA							6
#define IDX_OWNER_BLACK							  7
#define IDX_OWNER_ORANGE							8
#define IDX_OWNER_HOSTBOT							9

#define IDX_T_PERSON								0
#define IDX_T_BUILDING								1
#define IDX_T_SCENERY								2
#define IDX_T_GENERAL								3
#define IDX_T_SPELL									4
#define IDX_T_CREATURE								5
#define IDX_T_VEHICLE								6
#define IDX_T_SPECIAL								7
#define IDX_T_EFFECT								8
#define IDX_T_SHOT								    9
#define IDX_T_DECORATION						    10

#define IDX_M_PERSON_WILD							0
#define IDX_M_PERSON_BRAVE							1
#define IDX_M_PERSON_WARRIOR						2
#define IDX_M_PERSON_PRIEST							3
#define IDX_M_PERSON_SPY							4
#define IDX_M_PERSON_FIREWARRIOR					5
#define IDX_M_PERSON_SHAMAN							6
#define IDX_M_PERSON_ANGELOFDEATH					7

#define IDX_M_BUILDING_HUT1							0
#define IDX_M_BUILDING_HUT2							1
#define IDX_M_BUILDING_HUT3							2
#define IDX_M_BUILDING_TOWER						3
#define IDX_M_BUILDING_TEMPLE						4
#define IDX_M_BUILDING_SPY_TRAIN					5
#define IDX_M_BUILDING_WARRIOR_TRAIN				6
#define IDX_M_BUILDING_FIREWARRIOR_TRAIN			7
#define IDX_M_BUILDING_BOAT_HUT						8
#define IDX_M_BUILDING_AIRSHIP_HUT					9
#define IDX_M_BUILDING_KNOWLEDGE					10
#define IDX_M_BUILDING_PRISON						11

#define IDX_M_SCENERY_TREE_1						0
#define IDX_M_SCENERY_TREE_2						1
#define IDX_M_SCENERY_TREE_3						2
#define IDX_M_SCENERY_TREE_4						3
#define IDX_M_SCENERY_TREE_5						4
#define IDX_M_SCENERY_TREE_6						5
#define IDX_M_SCENERY_PLANT_1						6
#define IDX_M_SCENERY_PLANT_2						7
#define IDX_M_SCENERY_STONE_HEAD					8
#define IDX_M_SCENERY_WOOD_PILE						9
#define IDX_M_SCENERY_RS_PILLAR						10
#define IDX_M_SCENERY_ROCK							11
#define IDX_M_SCENERY_FIRE							12
#define IDX_M_SCENERY_FORBIDDEN						13

#define IDX_M_SPECIAL_A								0
#define IDX_M_SPECIAL_B								1
#define IDX_M_SPECIAL_C								2
#define IDX_M_SPECIAL_D								3
#define IDX_M_SPECIAL_E								4
#define IDX_M_SPECIAL_F								5

#define IDX_M_DECORATION_BOOK						0
#define IDX_M_DECORATION_SHIELD						1
#define IDX_M_DECORATION_WELL						2
#define IDX_M_DECORATION_WINDMILL					3
#define IDX_M_DECORATION_WOODEN_HUT					4
#define	IDX_M_DECORATION_B_TOWER					5	
#define	IDX_M_DECORATION_R_TOWER					6	
#define	IDX_M_DECORATION_Y_TOWER					7		
#define	IDX_M_DECORATION_G_TOWER					8	
#define	IDX_M_DECORATION_B_BOAT_HUT					9
#define	IDX_M_DECORATION_R_BOAT_HUT					10
#define	IDX_M_DECORATION_Y_BOAT_HUT					11
#define	IDX_M_DECORATION_G_BOAT_HUT					12
#define	IDX_M_DECORATION_B_AIRSHIP_HUT				13
#define	IDX_M_DECORATION_R_AIRSHIP_HUT				14
#define	IDX_M_DECORATION_Y_AIRSHIP_HUT				15
#define	IDX_M_DECORATION_G_AIRSHIP_HUT				16
#define	IDX_M_DECORATION_B_SPY_TRAIN				17
#define	IDX_M_DECORATION_R_SPY_TRAIN				18
#define	IDX_M_DECORATION_Y_SPY_TRAIN				19
#define	IDX_M_DECORATION_G_SPY_TRAIN				20
#define	IDX_M_DECORATION_B_TEMPLE					21
#define	IDX_M_DECORATION_R_TEMPLE					22
#define	IDX_M_DECORATION_Y_TEMPLE					23
#define	IDX_M_DECORATION_G_TEMPLE					24
#define	IDX_M_DECORATION_B_FIREWARRIOR_TRAIN		25		
#define	IDX_M_DECORATION_R_FIREWARRIOR_TRAIN		26	
#define	IDX_M_DECORATION_Y_FIREWARRIOR_TRAIN		27	
#define	IDX_M_DECORATION_G_FIREWARRIOR_TRAIN		28
#define	IDX_M_DECORATION_B_WARRIOR_TRAIN			29
#define	IDX_M_DECORATION_R_WARRIOR_TRAIN			30
#define	IDX_M_DECORATION_Y_WARRIOR_TRAIN			31	
#define	IDX_M_DECORATION_G_WARRIOR_TRAIN			32	
#define	IDX_M_DECORATION_B_HUT1_A					33
#define	IDX_M_DECORATION_B_HUT1_B					34
#define	IDX_M_DECORATION_B_HUT1_C					35
#define	IDX_M_DECORATION_R_HUT1_A					36
#define	IDX_M_DECORATION_R_HUT1_B					37
#define	IDX_M_DECORATION_R_HUT1_C					38
#define	IDX_M_DECORATION_Y_HUT1_A					39
#define	IDX_M_DECORATION_Y_HUT1_B					40
#define	IDX_M_DECORATION_Y_HUT1_C					41
#define	IDX_M_DECORATION_G_HUT1_A					42
#define	IDX_M_DECORATION_G_HUT1_B					43
#define	IDX_M_DECORATION_G_HUT1_C					44
#define	IDX_M_DECORATION_B_HUT2_A					45
#define	IDX_M_DECORATION_B_HUT2_B					46
#define	IDX_M_DECORATION_B_HUT2_C					47
#define	IDX_M_DECORATION_R_HUT2_A					48
#define	IDX_M_DECORATION_R_HUT2_B					49
#define	IDX_M_DECORATION_R_HUT2_C					50
#define	IDX_M_DECORATION_Y_HUT2_A					51
#define	IDX_M_DECORATION_Y_HUT2_B					52
#define	IDX_M_DECORATION_Y_HUT2_C					53
#define	IDX_M_DECORATION_G_HUT2_A					54
#define	IDX_M_DECORATION_G_HUT2_B					55
#define	IDX_M_DECORATION_G_HUT2_C					56
#define	IDX_M_DECORATION_B_HUT3_A					57
#define	IDX_M_DECORATION_B_HUT3_B					58
#define	IDX_M_DECORATION_B_HUT3_C					59
#define	IDX_M_DECORATION_R_HUT3_A					60
#define	IDX_M_DECORATION_R_HUT3_B					61
#define	IDX_M_DECORATION_R_HUT3_C					62
#define	IDX_M_DECORATION_Y_HUT3_A					63
#define	IDX_M_DECORATION_Y_HUT3_B					64
#define	IDX_M_DECORATION_Y_HUT3_C					65
#define	IDX_M_DECORATION_G_HUT3_A					66
#define	IDX_M_DECORATION_G_HUT3_B					67
#define	IDX_M_DECORATION_G_HUT3_C					68
#define	IDX_M_DECORATION_BOAT						69
#define	IDX_M_DECORATION_BALLOON					70
#define IDX_M_DECORATION_TOTEM_0					71
#define IDX_M_DECORATION_TOTEM_1					72
#define IDX_M_DECORATION_KNOWLEDGE_0				73
#define IDX_M_DECORATION_KNOWLEDGE_1				74
#define IDX_M_DECORATION_KNOWLEDGE_2				75
#define IDX_M_DECORATION_PRISON						76
#define IDX_M_DECORATION_OBELISK					77
#define IDX_M_DECORATION_GARGOYLE_0					78
#define IDX_M_DECORATION_GARGOYLE_1					79
#define IDX_M_DECORATION_GARGOYLE_2					80
#define IDX_M_DECORATION_PILLAR						81
#define IDX_M_DECORATION_AOD_0						82
#define IDX_M_DECORATION_AOD_1						83
#define IDX_M_DECORATION_STONEHEAD					84
#define IDX_M_DECORATION_TREE_1						85
#define IDX_M_DECORATION_TREE_2						86
#define IDX_M_DECORATION_TREE_3						87
#define IDX_M_DECORATION_TREE_4						88
#define IDX_M_DECORATION_TREE_5						89
#define IDX_M_DECORATION_TREE_6						90

#define IDX_M_GENERAL_DISCOVERY						0
#define IDX_M_GENERAL_TRIGGER						1
#define IDX_M_GENERAL_BUILDING_ADD_ON				2
#define IDX_M_GENERAL_LIGHT							3

#define IDX_DISCOVERY_AVAILABILITY_PERMANENT		0
#define IDX_DISCOVERY_AVAILABILITY_LEVEL			1
#define IDX_DISCOVERY_AVAILABILITY_ONCE				2

#define IDX_DISCOVERY_T_BUILDING					0
#define IDX_DISCOVERY_T_SPELL						1
#define IDX_DISCOVERY_T_MANA						2

#define IDX_DISCOVERY_M_BUILDING_HUT1				0
#define IDX_DISCOVERY_M_BUILDING_TOWER				1
#define IDX_DISCOVERY_M_BUILDING_TEMPLE				2
#define IDX_DISCOVERY_M_BUILDING_SPY_TRAIN			3
#define IDX_DISCOVERY_M_BUILDING_WARRIOR_TRAIN		4
#define IDX_DISCOVERY_M_BUILDING_FIREWARRIOR_TRAIN	5
#define IDX_DISCOVERY_M_BUILDING_BOAT_HUT			6
#define IDX_DISCOVERY_M_BUILDING_AIRSHIP_HUT		7

#define IDX_DISCOVERY_M_SPELL_BLAST					0
#define IDX_DISCOVERY_M_SPELL_LIGHTNING				1
#define IDX_DISCOVERY_M_SPELL_TORNADO				2
#define IDX_DISCOVERY_M_SPELL_SWARM					3
#define IDX_DISCOVERY_M_SPELL_INVISIBILITY			4
#define IDX_DISCOVERY_M_SPELL_HYPNOTISM				5
#define IDX_DISCOVERY_M_SPELL_FIRESTORM				6
#define IDX_DISCOVERY_M_SPELL_GHOST_ARMY			7
#define IDX_DISCOVERY_M_SPELL_ERODE					8
#define IDX_DISCOVERY_M_SPELL_SWAMP					9
#define IDX_DISCOVERY_M_SPELL_LAND_BRIDGE			10
#define IDX_DISCOVERY_M_SPELL_ANGEL_OF_DEATH		11
#define IDX_DISCOVERY_M_SPELL_EARTHQUAKE			12
#define IDX_DISCOVERY_M_SPELL_FLATTEN				13
#define IDX_DISCOVERY_M_SPELL_VOLCANO				14
#define IDX_DISCOVERY_M_SPELL_CONVERT				15
#define IDX_DISCOVERY_M_SPELL_ARMAGEDDON			16
#define IDX_DISCOVERY_M_SPELL_MAGICAL_SHIELD		17
#define IDX_DISCOVERY_M_SPELL_BLOODLUST				18
#define IDX_DISCOVERY_M_SPELL_TELEPORT				19

#define IDX_TRIGGER_TYPE_PROXIMITY					0
#define IDX_TRIGGER_TYPE_TIMED						1
#define IDX_TRIGGER_TYPE_PLAYER_DEATH				2
#define IDX_TRIGGER_TYPE_SHAMAN_PROXIMITY			3
#define IDX_TRIGGER_TYPE_LIBRARY					4
#define IDX_TRIGGER_TYPE_SHAMAN_AOD					5

#define IDX_M_SPELL_BURN							0
#define IDX_M_SPELL_BLAST							1
#define IDX_M_SPELL_LIGHTNING						2
#define IDX_M_SPELL_TORNADO							3
#define IDX_M_SPELL_SWARM							4
#define IDX_M_SPELL_INVISIBILITY					5
#define IDX_M_SPELL_HYPNOTISM						6
#define IDX_M_SPELL_FIRESTORM						7
#define IDX_M_SPELL_GHOST_ARMY						8
#define IDX_M_SPELL_ERODE							9
#define IDX_M_SPELL_SWAMP							10
#define IDX_M_SPELL_LAND_BRIDGE						11
#define IDX_M_SPELL_ANGEL_OF_DEATH					12
#define IDX_M_SPELL_EARTHQUAKE						13
#define IDX_M_SPELL_FLATTEN							14
#define IDX_M_SPELL_VOLCANO							15
#define IDX_M_SPELL_CONVERT							16
#define IDX_M_SPELL_ARMAGEDDON						17
#define IDX_M_SPELL_MAGICAL_SHIELD					18
#define IDX_M_SPELL_BLOODLUST						19
#define IDX_M_SPELL_TELEPORT						20

#define IDX_M_EFFECT_SIMPLE_BLAST					0
#define IDX_M_EFFECT_SPRITE_CIRCLES					1
#define IDX_M_EFFECT_SMOKE							2
#define IDX_M_EFFECT_LIGHTNING_ELEM					3
#define IDX_M_EFFECT_BURN_CELL_OBSTACLES			4
#define IDX_M_EFFECT_FLATTEN_LAND					5
#define IDX_M_EFFECT_SPHERE_EXPLODE_1				6
#define IDX_M_EFFECT_FIREBALL						7
#define IDX_M_EFFECT_FIRECLOUD						8
#define IDX_M_EFFECT_GHOST_ARMY						9
#define IDX_M_EFFECT_INVISIBILITY					10
#define IDX_M_EFFECT_VOLCANO						11
#define IDX_M_EFFECT_HYPNOTISM						12
#define IDX_M_EFFECT_LIGHTNING_BOLT					13
#define IDX_M_EFFECT_SWAMP							14
#define IDX_M_EFFECT_ANGEL_OF_DEATH					15
#define IDX_M_EFFECT_WHIRLWIND						16
#define IDX_M_EFFECT_INSECT_PLAGUE					17
#define IDX_M_EFFECT_FIRESTORM						18
#define IDX_M_EFFECT_EROSION						19
#define IDX_M_EFFECT_LAND_BRIDGE					20
#define IDX_M_EFFECT_EARTHQUAKE						21
#define IDX_M_EFFECT_FLY_THINGUMMY					22
#define IDX_M_EFFECT_SPHERE_EXPLODE_AND_FIRE		23
#define IDX_M_EFFECT_BIG_FIRE						24
#define IDX_M_EFFECT_LIGHTNING						25
#define IDX_M_EFFECT_FLATTEN						26
#define IDX_M_EFFECT_LAVA_FLOW						27
#define IDX_M_EFFECT_PURIFY_LAND					28
#define IDX_M_EFFECT_EXPLOSION_1					29
#define IDX_M_EFFECT_EXPLOSION_2					30
#define IDX_M_EFFECT_LAVA_SQUARE					31
#define IDX_M_EFFECT_WW_ELEMENT						32
#define IDX_M_EFFECT_WW_DUST						33
#define IDX_M_EFFECT_HILL							34
#define IDX_M_EFFECT_VALLEY							35
#define IDX_M_EFFECT_PLACE_TREE						36
#define IDX_M_EFFECT_RISE							37
#define IDX_M_EFFECT_DIP							38
#define IDX_M_EFFECT_REIN_ROCK_DEBRIS				39
#define IDX_M_EFFECT_PLACE_SHAMAN					40
#define IDX_M_EFFECT_PLACE_WILD						41
#define IDX_M_EFFECT_BLDG_SMOKE						42
#define IDX_M_EFFECT_MUCH_SIMPLER_BLAST				43
#define IDX_M_EFFECT_TUMBLING_BRANCH				44
#define IDX_M_EFFECT_SPARKLE						45
#define IDX_M_EFFECT_SMALL_SPARKLE					46
#define IDX_M_EFFECT_EXPLOSION_3					47
#define IDX_M_EFFECT_ROCK_EXPLOSION					48
#define IDX_M_EFFECT_LAVA_GLOOP						49
#define IDX_M_EFFECT_SPLASH							50
#define IDX_M_EFFECT_SMOKE_CLOUD					51
#define IDX_M_EFFECT_SMOKE_CLOUD_CONSTANT			52
#define IDX_M_EFFECT_FIREBALL_2						53
#define IDX_M_EFFECT_GROUND_SHOCKWAVE				54
#define IDX_M_EFFECT_ORBITER						55
#define IDX_M_EFFECT_BIG_SPARKLE					56
#define IDX_M_EFFECT_METEOR							57
#define IDX_M_EFFECT_CONVERT_WILD					58
#define IDX_M_EFFECT_BLDG_SMOKE_2_FULL				59
#define IDX_M_EFFECT_BLDG_SMOKE_2_PARTIAL			60
#define IDX_M_EFFECT_BLDG_DAMAGED_SMOKE				61
#define IDX_M_EFFECT_DELETE_RS_PILLARS				62
#define IDX_M_EFFECT_SPELL_BLAST					63
#define IDX_M_EFFECT_FIRESTORM_SMOKE				64
#define IDX_M_EFFECT_REVEAL_FOG_AREA				65
#define IDX_M_EFFECT_SHIELD							66
#define IDX_M_EFFECT_BOAT_HUT_REPAIR				67
#define IDX_M_EFFECT_REEDY_GRASS					68
#define IDX_M_EFFECT_SWAMP_MIST						69
#define IDX_M_EFFECT_ARMAGEDDON						70
#define IDX_M_EFFECT_BLOODLUST						71
#define IDX_M_EFFECT_TELEPORT						72
#define IDX_M_EFFECT_ATLANTIS_SET					73
#define IDX_M_EFFECT_ATLANTIS_INVOKE				74
#define IDX_M_EFFECT_STATUE_TO_AOD					75
#define IDX_M_EFFECT_FILL_ONE_SHOTS					76
#define IDX_M_EFFECT_RESURRECT (77)
#define IDX_M_EFFECT_BIG_SMOKE (78)
#define IDX_M_EFFECT_FIRESTORM_BALL (79)
#define IDX_M_EFFECT_LAVA_FLOW_NEW (80)
#define IDX_M_EFFECT_LAVA_FLOW_NEW_SHALLOW (81)
#define IDX_M_EFFECT_LAVA_FLOW_NEW_DEEP (82)
#define IDX_M_EFFECT_WATER (83)
#define IDX_M_EFFECT_WATER_1X1 (84)
#define IDX_M_EFFECT_LAVA_FLOW_NEW_WHITE_SHALLOW (85)
#define IDX_M_EFFECT_LAVA_FLOW_NEW_WHITE_DEEP (86)
#define IDX_M_EFFECT_LAVA_FIXED (87)
#define IDX_M_EFFECT_SOUL (88)
#define IDX_M_EFFECT_NEW_PLANT1 (89)
#define IDX_M_EFFECT_NEW_PLANT2 (90)
#define IDX_M_EFFECT_NEW_PLANT3 (91)
#define IDX_M_EFFECT_NEW_PLANT4 (92)
#define IDX_M_EFFECT_NEW_BRANCH (93)
#define IDX_M_EFFECT_BONEPILE (94)
#define IDX_M_EFFECT_SMALL_SKULL (95)
#define IDX_M_EFFECT_SMALL_SKULL2 (96)
#define IDX_M_EFFECT_SMALL_SKULL3 (97)
#define IDX_M_EFFECT_SKULL_THING (98)
#define IDX_M_EFFECT_SKULL_THING_FIRE (99)
#define IDX_M_EFFECT_GHOST_SKULL (100)


#define IDX_M_CREATURE_EAGLE						0

#define IDX_M_VEHICLE_BOAT							0
#define IDX_M_VEHICLE_AIRSHIP						1

#define IDX_M_SHOT_STANDARD						    0
#define IDX_M_SHOT_STANDARD_2					    1
#define IDX_M_SHOT_STANDARD_3					    2
#define IDX_M_SHOT_FIREBALL						    3
#define IDX_M_SHOT_LIGHTNING					    4
#define IDX_M_SHOT_SUPER_WARRIOR				    5
#define IDX_M_SHOT_VOLCANO_FIREBALL_1			    6
#define IDX_M_SHOT_VOLCANO_FIREBALL_2			    7

#define SZ_FLAGS_BRUSH_VALUE_TXT					"Flag Value: %d"

#define SZ_BRUSH_SIZE_TXT							"Size: %d"
#define BRUSH_SIZE_MIN								0
#define BRUSH_SIZE_MAX								32
#define SZ_BRUSH_SPEED_TXT							"Speed: %d"
#define BRUSH_SPEED_MIN								0
#define BRUSH_SPEED_MAX								8


#define DLG_DEVICE_CONFIRM_TIMERID					1


// object filter flags
#define OFF_NEUTRAL									0x00000001
#define OFF_BLUE									0x00000002
#define OFF_RED										0x00000004
#define OFF_YELLOW									0x00000008
#define OFF_GREEN									0x00000010
#define OFF_UNKNOW									0x00000020
#define OFF_PERSON									0x00000040
#define OFF_BUILDING								0x00000080
#define OFF_SCENERY									0x00000100
#define OFF_GENERAL									0x00000200
#define OFF_VEHICLE									0x00000400
#define OFF_SPELL									0x00000800
#define OFF_EFFECT									0x00001000
#define OFF_SHOT									0x00002000
#define OFF_HOSTBOT									0x00008000
#define OFF_CYAN									0x00010000
#define OFF_MAGENTA									0x00020000
#define OFF_BLACK									0x00040000
#define OFF_ORANGE									0x00080000
#define OFF_ALL										0xFFFFFFFF

// SWAP TRIBE CONSTANTS.
constexpr int BLUE_TRIBE_SWAP_DROP_DOWN_INDEX = 0;
constexpr int RED_TRIBE_SWAP_DROP_DOWN_INDEX = 1;
constexpr int YELLOW_TRIBE_SWAP_DROP_DOWN_INDEX = 2;
constexpr int GREEN_TRIBE_SWAP_DROP_DOWN_INDEX = 3;
constexpr int CYAN_TRIBE_SWAP_DROP_DOWN_INDEX = 4;
constexpr int MAGENTA_TRIBE_SWAP_DROP_DOWN_INDEX = 5;
constexpr int BLACK_TRIBE_SWAP_DROP_DOWN_INDEX = 6;
constexpr int ORANGE_TRIBE_SWAP_DROP_DOWN_INDEX = 7;
constexpr int NEUTRAL_TRIBE_SWAP_DROP_DOWN_INDEX = 8;

extern std::string strMacro;
extern BYTE nSelectPriority;

extern HWND		hDlgDevice,
				hDlgDeviceConfirm,
				hDlgMenuBar,
				hDlgBrush,
				hDlgFlagsBrush,
				hDlgObject,
				hDlgFilter,
				hDlgDiscovery,
				hDlgTrigger,
				hDlgLink,
				hDlgList,
				hDlgAbout,
				hDlgAllies,
				hDlgSpellsBuildings,
				hDlgSpellsNotCharging,
				hDlgHeader,
				hDlgObjBank,
				hDlgMapType,
				hDlgInfo,
				hDlgSwapTribe,
				hDlgMarkers,
				hDlgAIAttrib,
				hDlgAIScript,
				hDlgMacro,
				hDlgObjectMore;


extern void LevelNew();
extern void LevelOpen();
extern void LevelSave();
extern void LevelSaveAs();
extern void ExportHeightMap();
extern void Lang2Text();
extern void Text2Lang();
extern void ShowHelp();
extern bool GetInt(char *str, int *i);
extern long InitializeDialogs();
extern long FinishDialogs();
extern void LockDialogs(bool lock);
extern int  ModalMsg(LPCTSTR lpText, LPCTSTR lpCaption, UINT uType);
extern void CenterDialog(HWND hWnd);

// device dialog
extern void DlgDeviceToggle();
extern int __stdcall DlgDeviceProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// device confirm dialog
extern void DlgDeviceConfirmToggle();
extern void DlgDeviceConfirmUpdate(HWND hWnd);
extern void DlgDeviceConfirmRestore();
extern int __stdcall DlgDeviceConfirmProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// menu bar
extern long InitializeMenuBar();
extern void MenuBarToggle();
extern long __stdcall MenuBarProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern void SetEditLand();
extern void SetEditObjects();
extern void SetEditMarkers();

// brush dialog
extern void DlgBrushToggle();
extern void DlgBrushUpdate(HWND hWnd);
extern int __stdcall DlgBrushProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// flags brush dialog
extern void DlgFlagsBrushToggle();
extern void DlgFlagsBrushUpdate(HWND hWnd);
extern int __stdcall DlgFlagsBrushProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// object dialog
extern void DlgObjectToggle();
extern int __stdcall DlgObjectProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern void DlgObjectCenterSelected();
extern void DlgObjectSelect(THING *t, bool center = true);
extern void DlgObjectUpdateInfo(HWND hWnd, bool lock = true);
extern void DlgObjectSetPersonList(HWND hWnd);
extern void DlgObjectSetBuildingList(HWND hWnd);
extern void DlgObjectSetSceneryList(HWND hWnd);
extern void DlgObjectSetSpecialList(HWND hWnd);
extern void DlgObjectSetGeneralList(HWND hWnd);
extern void DlgObjectSetSpellList(HWND hWnd);
extern void DlgObjectSetEffectList(HWND hWnd);
extern void DlgObjectSetCreatureList(HWND hWnd);
extern void DlgObjectSetVehicleList(HWND hWnd);
extern void DlgObjectSetShotList(HWND hWnd);
extern void DlgObjectSetDecorationList(HWND hWnd);
extern void DlgObjectNextObj();
extern void DlgObjectPrevObj();
extern THING* DlgObjectNewObj(float x = 0, float z = 0, UBYTE nType = 255, UBYTE nModel = 255, SBYTE nOwner = 127);
extern void DlgObjectDeleteObj();
extern THING* DlgObjectFindIdx(UWORD Idx);
extern void DlgObjectIdxToLink();
extern void DlgObjectResetIdxs();
extern void DlgObjectLinkToIdx();
extern void DlgObjectUnlinkObj(THING *a);
extern void DlgPaintDecorations();

// filter dialog
extern void DlgFilterToggle();
extern int __stdcall DlgFilterProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern void DlgFilterUpdate(HWND hWnd);
extern bool DlgFilterIsInFilter(THING *t);

// discovery dialog
extern void DlgDiscoveryToggle();
extern int __stdcall DlgDiscoveryProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern void DlgDiscoveryUpdateInfo(HWND hWnd, bool lock = true);
extern void DlgDiscoverySetBuildingList(HWND hWnd);
extern void DlgDiscoverySetSpellList(HWND hWnd);

// trigger dialog
extern void DlgTriggerToggle();
extern int __stdcall DlgTriggerProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern void DlgTriggerUpdateInfo(HWND hWnd, bool lock = true);

// link dialog
extern void DlgLinkToggle();
extern int __stdcall DlgLinkProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern void DlgLinkUpdateInfo(HWND hWnd, bool lock = true);
extern void DlgLinkFixLinks();

// list dialog
extern void DlgListToggle();
extern int __stdcall DlgListProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern void DlgListUpdateInfo(HWND hWnd, bool lock = true);

// about dialog
extern void DlgAboutToggle();
extern int __stdcall DlgAboutProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// all header dialogs
extern void UpdateHeaderDialogs();

// allies dialog
extern void DlgAlliesToggle();
extern void DlgAlliesProcModify(WPARAM wParam, bool bNetwork = false);
extern int __stdcall DlgAlliesProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern void DlgAlliesUpdate(HWND hWnd);

// spells/buildings dialog
extern void DlgSpellsBuildingsToggle();
extern int __stdcall DlgSpellsBuildingsProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern void DlgSpellsBuildingsProcModify(WPARAM wParam, bool bNetwork = false);
extern void DlgSpellsBuildingsUpdate(HWND hWnd);

// Script2 dialog
extern void DlgScript2();
extern int __stdcall DlgScript2Proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern void DlgScript2Update(HWND hWnd);


// spells not charging dialog
extern void DlgSpellsNotChargingToggle();
extern void DlgSpellsNotChargingProcModify(WPARAM wParam, bool bNetwork = false);
extern int __stdcall DlgSpellsNotChargingProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern void DlgSpellsNotChargingUpdate(HWND hWnd);

// header dialog
extern void DlgHeaderToggle();
extern int __stdcall DlgHeaderProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern void DlgHeaderUpdate(HWND hWnd);

// obj bank dialog
extern void DlgObjBankToggle();
extern void DlgObjBankPaint(HDC hdc);
extern void DlgObjBankUpdate(HWND hWnd);
extern int __stdcall DlgObjBankProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// map type dialog
extern void DlgMapTypeToggle();
extern void DlgMapTypePaint(HDC hdc);
extern void DlgMapTypeUpdate(HWND hWnd);
extern int __stdcall DlgMapTypeProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// info dialog
extern void DlgInfoToggle();
extern int __stdcall DlgInfoProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern void DlgInfoUpdate(HWND hWnd);

// swap tribe dialog
extern void DlgSwapTribeToggle();
extern void DlgSwapTribeUpdate(HWND hWnd);
extern void DlgSwapTribeSwap();
extern int __stdcall DlgSwapTribeProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// markers dialog
extern void DlgMarkersToggle();
extern void DlgMarkersUpdate(HWND hWnd);
extern void DlgMarkersCenterSelected();
extern int __stdcall DlgMarkersProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// ai attrib dialog
extern void DlgAIAttribToggle();
extern int __stdcall DlgAIAttribProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern void DlgAIAttribUpdate(HWND hWnd);
extern void DlgAIAttribUpdateAttrib(HWND hWnd);
extern bool DlgAIAttribSave();
extern bool DlgAIAttribSaveAs();
extern bool DlgAIAttribOpen();
extern bool DlgAIAttribSaveFile(char *filename);
extern bool DlgAIAttribOpenFile(char *filename);
extern void DlgAIAttribNew();

// ai script dialog
extern void DlgAIScriptToggle();
extern int __stdcall DlgAIScriptProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern void DlgAIScriptSetCompile(HWND hWnd);
extern void DlgAIScriptSetDecompile(HWND hWnd);
extern void DlgAIScriptCompile();
extern void DlgAIScriptDecompile();
extern void DlgAIScriptLog();
extern void DlgAIScriptSrc();
extern void DlgAIScriptDst();

extern bool fQuickDuplicate;
extern void PurgeObjects();
extern void ResetMarkers();
extern void DlgSetThingIndex(THING* pThing);

extern void DlgMacroToggle();
extern void DlgMacroCreate(std::string strMacro);
extern int __stdcall DlgMacroProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

extern void DlgObjectMoreToggle();
extern int __stdcall DlgObjectMoreProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern void DlgObjectMoreUpdate(HWND hWnd);