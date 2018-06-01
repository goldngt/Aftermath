/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef KNIFEJABCOMMAND_H_
#define KNIFEJABCOMMAND_H_

#include "CombatQueueCommand.h"

class KnifeJabCommand : public CombatQueueCommand {

protected:
	String skillName = "knifeJab";		        // Skill Name
	String skillNameDisplay = "Knife Jab";		// Skill Display Name for output message
	int delay = 30; 							//  30 second cool down timer after root expires

public:

	KnifeJabCommand(const String& name, ZoneProcessServer* server)
		: CombatQueueCommand(name, server) {
	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;
		ManagedReference<SceneObject*> object = server->getZoneServer()->getObject(target);
		CreatureObject* targetCreature = dynamic_cast<CreatureObject*>(object.get());

		if (targetCreature == NULL)
			return INVALIDTARGET;

		Locker clocker(targetCreature, creature);

		ManagedReference<PlayerObject*> player = creature->getPlayerObject();
		PlayerObject* targetPlayerObject = targetCreature->getPlayerObject();

		if (targetPlayerObject == NULL) {
			return INVALIDTARGET;
		} else if (player == NULL)
			return GENERALERROR;

		if (!creature->checkCooldownRecovery(skillName)){
			Time* timeRemaining = creature->getCooldownTime(skillName);
			creature->sendSystemMessage("You must wait " +  getCooldownString(timeRemaining->miliDifference() * -1)  + " to use " + skillNameDisplay + " again");
			return GENERALERROR;
		}

		int res = doCombatAction(creature, target);

		if (res == SUCCESS) {

			// Setup debuff.


			if (targetCreature != NULL) {
				Locker clocker(targetCreature, creature);

				ManagedReference<Buff*> buff = new Buff(targetCreature, getNameCRC(), 5, BuffType::OTHER);

				Locker locker(buff);
				if (targetCreature->hasBuff(BuffCRC::JEDI_FORCE_RUN_1)) {
					targetCreature->removeBuff(BuffCRC::JEDI_FORCE_RUN_1);
				}

				targetCreature->setSnaredState(5);
				StringBuffer targetRootMessage;

				targetRootMessage << "Your body has been weakened and you can't use the force to run for 5 seconds";
				targetCreature->sendSystemMessage(targetRootMessage.toString());

				targetCreature->addBuff(buff);
				creature->updateCooldownTimer(skillName, delay * 1000);

			}

		}
		return res;
	}

};

#endif //KNIFEJABCOMMAND_H_
