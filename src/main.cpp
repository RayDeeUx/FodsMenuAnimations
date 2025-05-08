#include <Geode/modify/MenuLayer.hpp>

using namespace geode::prelude;

#define REDASH_ID "ninxout.redash"
#define YAMM_ID "raydeeux.yetanotherqolmod"
#define VANILLA_PAGES_ID "alphalaneous.vanilla_pages"
#define GET_MOD Loader::get()->getLoadedMod
#define YAMM Loader::get()->isModLoaded(YAMM_ID)
#define REDASH Loader::get()->isModLoaded(REDASH_ID)
#define VANILLA_PAGES_LOADED Loader::get()->isModLoaded(VANILLA_PAGES_ID)
#define VANILLA_PAGES GET_MOD(VANILLA_PAGES_ID)
#define VANILLA_PAGES_MENULAYER_RIGHT GET_MOD(VANILLA_PAGES_ID)->getSettingValue<bool>("menulayer-right-menu")
#define VANILLA_PAGES_MENULAYER_BOTTOM GET_MOD(VANILLA_PAGES_ID)->getSettingValue<bool>("menulayer-bottom-menu")
#define GET_YAMM GET_MOD(YAMM_ID)
#define IS_AFFECTED_BY_YAMM(node) !node->getID().empty() && node->getID() == nodeChosenByYAMM

class $modify(MyMenuLayer, MenuLayer) {
	static void onModify(auto& self) {
		if (YAMM) (void) self.setHookPriorityAfterPost("MenuLayer::init", YAMM_ID);
		else if (REDASH) (void) self.setHookPriorityAfterPost("MenuLayer::init", REDASH_ID);
		else if (const geode::Mod* vanillaPages = VANILLA_PAGES; vanillaPages && (VANILLA_PAGES_MENULAYER_RIGHT || VANILLA_PAGES_MENULAYER_BOTTOM)) (void) self.setHookPriorityAfterPost("MenuLayer::init", VANILLA_PAGES_ID);
		else (void) self.setHookPriority("MenuLayer::init", -3998);
	}
	bool init() {
		if (!MenuLayer::init()) return false;

		if (!Mod::get()->getSettingValue<bool>("enabled")) return true;

		CCNode* mainMenu = this->getChildByID("main-menu");
		CCNode* bottomMenu = this->getChildByID("bottom-menu");
		CCNode* profileMenu = this->getChildByID("profile-menu");
		CCNode* rightSideMenu = this->getChildByID("right-side-menu");
		CCNode* topRightMenu = this->getChildByID("top-right-menu");
		CCNode* sideMenu = this->getChildByID("side-menu");
		CCNode* socialMediaMenu = this->getChildByID("social-media-menu");
		CCNode* moreGamesMenu = this->getChildByID("more-games-menu");
		CCNode* playerUsername = this->getChildByID("player-username");
		CCNode* closeMenu = this->getChildByID("close-menu");

		if (!mainMenu || !bottomMenu || !profileMenu || !rightSideMenu || !topRightMenu || !sideMenu || !socialMediaMenu || !moreGamesMenu || !playerUsername) return true;

		std::string nodeChosenByYAMM = "this-mod-doesnt-assign-node-ids-to-anything-lmfao"_spr;
		if (geode::Mod* yamm = GET_YAMM; yamm && YAMM) {
			const std::string& modID = yamm->getSettingValue<std::string>("pulseModID");
			const std::string& nodeID = yamm->getSettingValue<std::string>("pulseNodeID");
			if (!modID.empty() && Loader::get()->isModLoaded(modID)) nodeChosenByYAMM = fmt::format("{}/{}", modID, nodeID);
			else if (!nodeID.empty()) nodeChosenByYAMM = nodeID;
		}


		int i = 0;
		for (CCNode* node : CCArrayExt<CCNode*>(mainMenu->getChildren())) {
			if (IS_AFFECTED_BY_YAMM(node)) continue;
			node->setScale(0.f);
			node->setRotation(node->getID() == "play-button" ? -90.f : 90.f);

			if (node->getID() == "play-button") {
				CCDelayTime* playButtonDelay = CCDelayTime::create(.75f);
				CCFiniteTimeAction* eoScale = !Mod::get()->getSettingValue<bool>("classic-play-button-anim") ? static_cast<CCFiniteTimeAction*>(CCEaseBackOut::create(CCScaleTo::create(1.25f, 1.f))) : static_cast<CCFiniteTimeAction*>(CCEaseOut::create(CCScaleTo::create(1.25f, 1.f), 4.f));
				CCFiniteTimeAction* eoRotate = !Mod::get()->getSettingValue<bool>("classic-play-button-anim") ? static_cast<CCFiniteTimeAction*>(CCEaseBackOut::create(CCRotateTo::create(1.25f, 0.f))) : static_cast<CCFiniteTimeAction*>(CCEaseOut::create(CCRotateTo::create(1.25f, 0.f), 4.f));
				CCSpawn* whyDidFodUseCCSpawnAgain = CCSpawn::create(eoScale, eoRotate, nullptr);
				CCSequence* scaleAndRotateSequencePlayButton = CCSequence::create(playButtonDelay, whyDidFodUseCCSpawnAgain, nullptr);

				node->runAction(scaleAndRotateSequencePlayButton);
			} else {
				CCDelayTime* delay = CCDelayTime::create((.25f * static_cast<float>(i)) + .5f);
				CCEaseBackOut* eboScale = CCEaseBackOut::create(CCScaleTo::create(1.25f, 1.f));
				CCEaseBackOut* eboRotate = CCEaseBackOut::create(CCRotateTo::create(1.25f, 0.f));
				CCSpawn* whyDidFodUseCCSpawn = CCSpawn::create(eboScale, eboRotate, nullptr);
				CCSequence* scaleAndRotateSequence = CCSequence::create(delay, whyDidFodUseCCSpawn, nullptr);

				node->runAction(scaleAndRotateSequence);
			}

			i++;
		}
		i = 0; // reset incrementer

		if (CCNode* title = this->getChildByIDRecursive("main-title"); title && !(IS_AFFECTED_BY_YAMM(title))) {
			CCEaseBackOut* eboScale = CCEaseBackOut::create(CCScaleTo::create(2.f, 1.f));
			CCDelayTime* delay = CCDelayTime::create(.25f);
			CCSequence* titleSequence = CCSequence::create(delay, eboScale, nullptr);

			title->setScale(0.f);
			title->runAction(titleSequence);
		}

		if (!IS_AFFECTED_BY_YAMM(playerUsername)) {
			for (CCNode* sprite : CCArrayExt<CCNode*>(playerUsername->getChildren())) {
				CCDelayTime* delay = CCDelayTime::create((.2f * static_cast<float>(sprite->getTag())) + .5f);
				CCEaseElasticOut* eeoScale = CCEaseElasticOut::create(CCScaleTo::create(1.f, 1.f));
				CCSequence* usernameCharSequence = CCSequence::create(delay, eeoScale, nullptr);
				sprite->setScale(0.f);
				sprite->runAction(usernameCharSequence);
			}
		}

		if (CCNode* iThrewItOnTheGround = m_menuGameLayer; !Loader::get()->isModLoaded("undefined0.icon_ninja") && iThrewItOnTheGround) {
			for (CCNode* node : CCArrayExt<CCNode*>(iThrewItOnTheGround->getChildren())) {
				const float origYPos = node->getPositionY();

				CCDelayTime* delay = CCDelayTime::create(.5f);
				CCEaseExponentialOut* eeoMove = CCEaseExponentialOut::create(CCMoveBy::create(2.f, { 0.f, origYPos }));
				CCSequence* groundSequence = CCSequence::create(delay, eeoMove, nullptr);
				node->setPositionY(0.f);
				node->runAction(groundSequence);
			}
		}

		CCNode* theMenuToScaleFromZero = REDASH ? rightSideMenu : bottomMenu;
		auto tMTSFZChildren = theMenuToScaleFromZero->getChildren();
		if (REDASH && tMTSFZChildren) tMTSFZChildren->reverseObjects();
		for (!VANILLA_PAGES_MENULAYER_BOTTOM; CCNode* node : CCArrayExt<CCNode*>(tMTSFZChildren)) {
			if (!node->isVisible() || IS_AFFECTED_BY_YAMM(node)) continue;
			const float nodeOriginalScale = node->getScale();
			CCDelayTime* delayOne = CCDelayTime::create((static_cast<float>(i) * .25f) + 1.f);
			CCEaseExponentialOut* eeoScale = CCEaseExponentialOut::create(CCScaleTo::create(1.f, nodeOriginalScale));

			CCDelayTime* delayTwo = CCDelayTime::create((static_cast<float>(i) * .25f) + 2.f);
			CCEaseIn* eiScale = CCEaseIn::create(CCScaleTo::create(.25f, (nodeOriginalScale * 1.25f)), 4.f);
			CCEaseBackInOut* ebioScale = CCEaseBackInOut::create(CCScaleTo::create(.75f, nodeOriginalScale));

			node->setScale(0.f);
			node->runAction(CCSequence::create(delayOne, eeoScale, nullptr));
			node->runAction(CCSequence::create(delayTwo, eiScale, ebioScale, nullptr));
			i++;
		}
		if (VANILLA_PAGES_MENULAYER_BOTTOM) {
			const float nodeOrigYPos = theMenuToScaleFromZero->getPositionY();
			CCDelayTime* delay = CCDelayTime::create(1.f);
			CCEaseExponentialOut* eeoMove = CCEaseExponentialOut::create(CCMoveBy::create(1.f, { 0.f, 100.f }));

			theMenuToScaleFromZero->setPositionY(nodeOrigYPos - 100.f);
			theMenuToScaleFromZero->runAction(CCSequence::create(delay, eeoMove, nullptr));
		}

		if (CCNode* alphaBottomNav = this->getChildByID("bottom-menu-navigation-menu"); alphaBottomNav && !REDASH) {
			for (CCNode* node : CCArrayExt<CCNode*>(alphaBottomNav->getChildren())) {
				const float nodeOrigYPos = node->getPositionY();

				CCDelayTime* delay = CCDelayTime::create((static_cast<float>(bottomMenu->getChildrenCount()) * .25f) + 2.f);
				CCEaseExponentialOut* eeoMove = CCEaseExponentialOut::create(CCMoveBy::create(1.f, { 0.f, 75.f }));

				node->setPositionY(nodeOrigYPos - 75.f);
				node->runAction(CCSequence::create(delay, eeoMove, nullptr));
			}
		}
		i = 0;

		auto sideMenuChildren = sideMenu->getChildren();
		if (Mod::get()->getSettingValue<bool>("reverse-side-menus") && sideMenuChildren) sideMenuChildren->reverseObjects();
		for (CCNode* node : CCArrayExt<CCNode*>(sideMenuChildren)) {
			if (!node->isVisible()) continue;
			const float nodeOrigXPos = node->getPositionX();

			CCDelayTime* delay = CCDelayTime::create((static_cast<float>(i) * .25f) + 2.f);
			CCEaseExponentialOut* eeoMove = CCEaseExponentialOut::create(CCMoveBy::create(1.f, { 100.f, 0.f }));

			node->setPositionX(nodeOrigXPos - 100.f);
			node->runAction(CCSequence::create(delay, eeoMove, nullptr));

			i++;
		}
		i = 0;

		for (CCNode* node : CCArrayExt<CCNode*>(topRightMenu->getChildren())) {
			if (!node->isVisible()) continue;
			const float nodeOrigXPos = node->getPositionY();

			CCDelayTime* delay = CCDelayTime::create((static_cast<float>(i) * .25f) + 2.f);
			CCEaseExponentialOut* eeoMove = CCEaseExponentialOut::create(CCMoveBy::create(1.f, { 0.f, -100.f }));

			node->setPositionY(nodeOrigXPos + 100.f);
			node->runAction(CCSequence::create(delay, eeoMove, nullptr));

			i++;
		}
		i = 0;

		CCNode* theMenuToSlideFromRight = REDASH ? bottomMenu : rightSideMenu;
		auto rightSideMenuChildren = theMenuToSlideFromRight->getChildren();
		if (Mod::get()->getSettingValue<bool>("reverse-side-menus") && rightSideMenuChildren) rightSideMenuChildren->reverseObjects();
		for (!VANILLA_PAGES_MENULAYER_BOTTOM; CCNode* node : CCArrayExt<CCNode*>(rightSideMenuChildren)) {
			if (!node->isVisible()) continue;
			const float nodeOrigXPos = node->getPositionX();

			CCDelayTime* delay = CCDelayTime::create((static_cast<float>(i) * .25f) + 2.f);
			CCEaseExponentialOut* eeoMove = CCEaseExponentialOut::create(CCMoveBy::create(1.f, { -100.f, 0.f }));

			node->setPositionX(nodeOrigXPos + 100.f);
			node->runAction(CCSequence::create(delay, eeoMove, nullptr));

			i++;
		}
		if (VANILLA_PAGES_MENULAYER_BOTTOM) {
			const float nodeOrigXPos = theMenuToSlideFromRight->getPositionX();
			CCDelayTime* delay = CCDelayTime::create(1.f);
			CCEaseExponentialOut* eeoMove = CCEaseExponentialOut::create(CCMoveBy::create(1.f, { -100.f, 0.f }));

			theMenuToSlideFromRight->setPositionX(nodeOrigXPos + 100.f);
			theMenuToSlideFromRight->runAction(CCSequence::create(delay, eeoMove, nullptr));
		}

		if (CCNode* alphaRightNav = this->getChildByID("right-side-menu-navigation-menu")) {
			for (CCNode* node : CCArrayExt<CCNode*>(alphaRightNav->getChildren())) {
				const float nodeOrigXPos = node->getPositionX();

				CCDelayTime* delay = CCDelayTime::create((static_cast<float>(rightSideMenu->getChildrenCount()) * .25f) + 2.f);
				CCEaseExponentialOut* eeoMove = CCEaseExponentialOut::create(CCMoveBy::create(1.f, { -100.f, 0.f }));

				node->setPositionX(nodeOrigXPos + 100.f);
				node->runAction(CCSequence::create(delay, eeoMove, nullptr));
			}
		}
		i = 0;

		for (CCNode* node : CCArrayExt<CCNode*>(socialMediaMenu->getChildren())) {
			if (IS_AFFECTED_BY_YAMM(node)) continue;
			CCDelayTime* delay = CCDelayTime::create((.2f * static_cast<float>(i)) + 1.5f);
			CCEaseBackOut* eboScale = CCEaseBackOut::create(CCScaleTo::create(1.f, 1.f));
			CCSequence* sequence = CCSequence::create(delay, eboScale, nullptr);

			node->setScale(0.f);
			node->runAction(sequence);
			i++;
		}
		i = 0;

		for (closeMenu; CCNode* node : CCArrayExt<CCNode*>(closeMenu->getChildren())) {
			if (IS_AFFECTED_BY_YAMM(node)) continue;
			CCDelayTime* delay = CCDelayTime::create((.25f * static_cast<float>(i)) + static_cast<float>(Mod::get()->getSettingValue<double>("close-menu-delay")));
			CCEaseExponentialOut* eeoScale = CCEaseExponentialOut::create(CCScaleTo::create(1.25f, 1.f));
			CCSequence* sequence = CCSequence::create(delay, eeoScale, nullptr);

			node->setScale(0.f);
			node->runAction(sequence);

			i++;
		}
		i = 0;

		for (CCNode* node : CCArrayExt<CCNode*>(moreGamesMenu->getChildren())) {
			if (IS_AFFECTED_BY_YAMM(node)) continue;
			CCDelayTime* delay = CCDelayTime::create((.2f * static_cast<float>(i)) + 2.f);
			CCEaseBackOut* eboScale = CCEaseBackOut::create(CCScaleTo::create(1.f, 1.f));
			CCEaseBackOut* eboRotate = CCEaseBackOut::create(CCRotateTo::create(1.f, 0.f));
			CCSpawn* whyDidFodUseCCSpawn = CCSpawn::create(eboScale, eboRotate, nullptr);
			CCSequence* sequence = CCSequence::create(delay, whyDidFodUseCCSpawn, nullptr);

			node->setScale(0.f);
			node->setRotation(90.f);
			node->runAction(sequence);

			i++;
		}
		i = 0;

		for (CCNode* node : CCArrayExt<CCNode*>(profileMenu->getChildren())) {
			if (IS_AFFECTED_BY_YAMM(node)) continue;
			CCDelayTime* delay = CCDelayTime::create((.2f * static_cast<float>(i)) + .5f);
			CCEaseBackOut* eboScale = CCEaseBackOut::create(CCScaleTo::create(1.f, 1.f));
			CCEaseBackOut* eboRotate = CCEaseBackOut::create(CCRotateTo::create(1.f, 0.f));
			CCSpawn* whyDidFodUseCCSpawnAgain = CCSpawn::create(eboScale, eboRotate, nullptr);
			CCSequence* sequence = CCSequence::create(delay, whyDidFodUseCCSpawnAgain, nullptr);

			node->setScale(0.f);
			node->setRotation(-90.f);
			node->runAction(sequence);

			i++;
		}
		i = 0;

		CCNode* redashMenu = REDASH ? this->getChildByID("ninxout.redash/redash-menu") : nullptr;
		CCNode* redashHide = REDASH ? this->getChildByID("ninxout.redash/hide-button-menu") : nullptr;
		if (!REDASH || !redashMenu || !redashHide) return true;

		CCNode* redashMain = redashMenu->getChildByID("ninxout.redash/main-menu"); // rotate + scale
		CCNode* redashDailies = redashMenu->getChildByID("ninxout.redash/dailies-menu"); // scale
		CCNode* redashStats = redashMenu->getChildByID("ninxout.redash/stats-menu"); // move from top
		CCNode* redashBottom = redashMenu->getChildByID("ninxout.redash/bottom-menu"); // move from bottom
		CCNode* redashTop = redashMenu->getChildByID("ninxout.redash/top-menu"); // scale lock, move rope

		if (!redashMain || !redashDailies || !redashStats || !redashBottom || !redashTop) return true;

		for (redashMain->getChildren(); CCNode* node : CCArrayExt<CCNode*>(redashMain->getChildren())) {
			if (IS_AFFECTED_BY_YAMM(node)) continue;
			CCDelayTime* delay = CCDelayTime::create((.25f * static_cast<float>(i)) + .5f);
			CCEaseBackOut* eboScale = CCEaseBackOut::create(CCScaleTo::create(1.25f, 1.f));
			CCEaseBackOut* eboRotate = CCEaseBackOut::create(CCRotateTo::create(1.25f, 0.f));
			CCSpawn* whyDidFodUseCCSpawn = CCSpawn::create(eboScale, eboRotate, nullptr);
			CCSequence* scaleAndRotateSequence = CCSequence::create(delay, whyDidFodUseCCSpawn, nullptr);

			node->setScale(0.f);
			node->setRotation(i % 2 == 0 ? -90.f : 90.f);
			node->runAction(scaleAndRotateSequence);

			i++;
		}
		i = 0;

		for (redashDailies->getChildren(); CCNode* node : CCArrayExt<CCNode*>(redashDailies->getChildren())) {
			if (IS_AFFECTED_BY_YAMM(node)) continue;
			const float nodeOriginalScale = node->getScale();
			CCDelayTime* delayOne = CCDelayTime::create((static_cast<float>(i) * .25f) + 1.f);
			CCEaseExponentialOut* eeoScale = CCEaseExponentialOut::create(CCScaleTo::create(1.f, nodeOriginalScale));

			CCDelayTime* delayTwo = CCDelayTime::create((static_cast<float>(i) * .25f) + 2.f);
			CCEaseIn* eiScale = CCEaseIn::create(CCScaleTo::create(.25f, (nodeOriginalScale * 1.1f)), 4.f);
			CCEaseBackInOut* ebioScale = CCEaseBackInOut::create(CCScaleTo::create(.75f, nodeOriginalScale));

			node->setScale(0.f);
			node->runAction(CCSequence::create(delayOne, eeoScale, nullptr));
			node->runAction(CCSequence::create(delayTwo, eiScale, ebioScale, nullptr));

			i++;
		}
		i = 0;

		for (redashStats->getChildren(); CCNode* node : CCArrayExt<CCNode*>(redashStats->getChildren())) {
			if (IS_AFFECTED_BY_YAMM(node)) continue;
			const float nodeOrigYPos = node->getPositionY();

			CCDelayTime* delay = CCDelayTime::create((static_cast<float>(i) * .375f) + 1.f);
			CCEaseExponentialOut* eeoMove = CCEaseExponentialOut::create(CCMoveBy::create(1.f, { 0.f, -75.f }));

			node->setPositionY(nodeOrigYPos + 75.f);
			node->runAction(CCSequence::create(delay, eeoMove, nullptr));

			i++;
		}
		i = 0;

		for (redashBottom->getChildren(); CCNode* node : CCArrayExt<CCNode*>(redashBottom->getChildren())) {
			if (IS_AFFECTED_BY_YAMM(node)) continue;
			const float nodeOrigYPos = node->getPositionY();

			CCDelayTime* delay = CCDelayTime::create((static_cast<float>(i) * .5f) + 1.f);
			CCEaseExponentialOut* eeoMove = CCEaseExponentialOut::create(CCMoveBy::create(1.f, { 0.f, 75.f }));

			node->setPositionY(nodeOrigYPos - 75.f);
			node->runAction(CCSequence::create(delay, eeoMove, nullptr));

			i++;
		}
		i = 0;

		for (redashHide->getChildren(); CCNode* node : CCArrayExt<CCNode*>(redashHide->getChildren())) {
			if (IS_AFFECTED_BY_YAMM(node)) continue;
			const float nodeOrigYPos = node->getPositionY();

			CCDelayTime* delay = CCDelayTime::create((static_cast<float>(i) * .5f));
			CCEaseExponentialOut* eeoMove = CCEaseExponentialOut::create(CCMoveBy::create(1.f, { 0.f, 75.f }));

			node->setPositionY(nodeOrigYPos - 75.f);
			node->runAction(CCSequence::create(delay, eeoMove, nullptr));

			i++;
		}
		i = 0;

		auto redashTopChildren = redashTop->getChildren();
		if (redashTopChildren) redashTopChildren->reverseObjects();
		for (CCNode* node : CCArrayExt<CCNode*>(redashTopChildren)) {
			if (IS_AFFECTED_BY_YAMM(node)) continue;
			if (node->getID() == "garage-rope") {
				const float nodeOrigYPos = node->getPositionY();

				CCDelayTime* delay = CCDelayTime::create((static_cast<float>(i) * .25f) + 2.f);
				CCEaseExponentialOut* eeoMove = CCEaseExponentialOut::create(CCMoveBy::create(1.f, { 0.f, -75.f }));

				node->setPositionY(nodeOrigYPos + 75.f);
				node->runAction(CCSequence::create(delay, eeoMove, nullptr));
			} else {
				CCDelayTime* delay = CCDelayTime::create((static_cast<float>(i) * .25f) + 2.f);
				CCEaseBackOut* eboScale = CCEaseBackOut::create(CCScaleTo::create(1.f, 1.f));
				CCEaseBackOut* eboRotate = CCEaseBackOut::create(CCRotateTo::create(1.f, 0.f));
				CCSpawn* whyDidFodUseCCSpawnAgain = CCSpawn::create(eboScale, eboRotate, nullptr);
				CCSequence* sequence = CCSequence::create(delay, whyDidFodUseCCSpawnAgain, nullptr);

				node->setScale(0.f);
				node->setRotation(-90.f);
				node->runAction(sequence);
			}

			i++;
		}
		i = 0;

		return true;
	}
};