/*
 * TownSpellResearchTest.cpp, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */
#include "StdInc.h"

#include "../../lib/constants/EntityIdentifiers.h"
#include "../../lib/entities/faction/CTown.h"
#include "../../lib/mapObjects/CGTownInstance.h"

TEST(TownSpellResearchTest, CandidateStartsAfterVisibleLibrarySpells)
{
	CGTownInstance town(nullptr);
	town.ID = Obj::TOWN;
	town.subID = FactionID::TOWER.getNum();
	town.addBuilding(town.getTown()->getBuildingType(BuildingSubID::LIBRARY));

	EXPECT_EQ(town.spellsAtLevel(5, false), 2);
	EXPECT_EQ(town.spellResearchCandidateIndex(4), 2);
}
