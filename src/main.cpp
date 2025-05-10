// welcome to what is probably my worst C++ code yet.
// the truth is, i realized that the code quality would
// surely tank once i realized i couldn't take advantage
// of early return statements in a func hook like this.
// but what's done is done, and we can only move forward.
// --raydeeux

// if you know of a better way to port traditional GD mod
// source code over to geode *AND* adapt it to today's mods
// on the geode index, i'm all ears. have at it, go some fun,
// and please have a browser tab open fod's source code for reference:
// https://raw.githubusercontent.com/HJfod/cool-menu-animation/v1.1/main.cpp
// --raydeeux

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

#define SPEED_SETTING Mod::get()->getSettingValue<double>("animation-speed")
#define SPEED_MIN .01f
#define SPEED_MAX 16.f
#define DELAY_SETTING Mod::get()->getSettingValue<double>("animation-delay")
#define DELAY_MIN .0f
#define DELAY_MAX 10.f
#define EXTSN_SETTING Mod::get()->getSettingValue<double>("animation-extension")
#define EXTSN_MIN .0f
#define EXTSN_MAX 2.f
#define CLAMP_FLOAT(setting, min, max) static_cast<float>(setting < min ? min : (setting > max ? max : setting))
#define ANIM_SPEED CLAMP_FLOAT(SPEED_SETTING, SPEED_MIN, SPEED_MAX)
#define ANIM_DELAY CLAMP_FLOAT(DELAY_SETTING, DELAY_MIN, DELAY_MAX)
#define ANIM_EXTSN CLAMP_FLOAT(EXTSN_SETTING, EXTSN_MIN, EXTSN_MAX)
#define APPLY_ANIM_MODIFIERS(originalValue) ((originalValue / ANIM_SPEED) + ANIM_DELAY)
#define APPLY_ANIM_EXTENDERS(originalValue) ((originalValue / ANIM_SPEED) + ANIM_EXTSN)

bool stopLooping = false; // m_fields for a singlefile mod is silly --raydeeux

class $modify(MyMenuLayer, MenuLayer) {
	static void onModify(auto& self) {
		if (YAMM) (void) self.setHookPriorityAfterPost("MenuLayer::init", YAMM_ID);
		else if (REDASH) (void) self.setHookPriorityAfterPost("MenuLayer::init", REDASH_ID);
		else if (VANILLA_PAGES_LOADED) (void) self.setHookPriorityAfterPost("MenuLayer::init", VANILLA_PAGES_ID);
		else (void) self.setHookPriority("MenuLayer::init", -3998);
	}
	void determinePlayerVisibility(float dt) {
		if (!Mod::get()->getSettingValue<bool>("enabled")) return;
		if (Loader::get()->isModLoaded("undefined0.icon_ninja")) return;
		if (!m_menuGameLayer || !m_menuGameLayer->getChildren()) return;
		if (PlayerObject* player = m_menuGameLayer->m_playerObject; !player || player->getPositionX() > 0.f || player->getRealPosition().x > 0.f) {
			if (stopLooping && player->m_isDart && player->m_waveTrail) player->m_waveTrail->setVisible(true);
			if (player->m_isRobot) m_menuGameLayer->tryJump(.001f); // in case any whiny kids start yelling at me for why the robot isnt jumping
			return;
		}
		if (stopLooping) return;
		const float groundPos = m_menuGameLayer->m_groundLayer->getPositionY();
		for (CCNode* node : CCArrayExt<CCNode*>(m_menuGameLayer->getChildren())) {
			if (node == m_menuGameLayer->m_groundLayer || node == m_menuGameLayer->m_backgroundSprite) continue;
			node->setVisible(false);
			if (typeinfo_cast<CCMotionStreak*>(node) || typeinfo_cast<HardStreak*>(node)) continue;
			if (groundPos > 89.f) node->setVisible(true);
		}
		if (groundPos > 89.f) stopLooping = true;
	}
	bool init() {
		/*
			If you need more than 3 levels of indentation,
			you're screwed anyway, and should fix your program.
		*/
		// i'm usually in the same camp with linus torvalds when
		// it comes to levels of indentation, but i had to break
		// his rule just this once for reasons you'll see shortly.
		// (spoiler: cocos2d nullptr checks are really finicky)
		// please forgive me for my flagrant transgression.
		// --raydeeux
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

		if (!mainMenu || !bottomMenu || !profileMenu || !rightSideMenu || !topRightMenu || !sideMenu || !socialMediaMenu || !moreGamesMenu || !playerUsername) return true;

		// so here's a funny story about this code segment--
		// one of the nodes i tried to apply an animation on
		// wouldn't animate properly, then i realized that it
		// was caused by stuff from YetAnotherModMenu (YAMM).
		// so i just decided to skip a node if it was YAMM'd.
		// --raydeeux
		std::string nodeChosenByYAMM = "this-mod-doesnt-assign-node-ids-to-anything-lmfao"_spr;
		if (geode::Mod* yamm = GET_YAMM; yamm && YAMM) {
			const std::string& modID = yamm->getSettingValue<std::string>("pulseModID");
			const std::string& nodeID = yamm->getSettingValue<std::string>("pulseNodeID");
			if (!modID.empty() && Loader::get()->isModLoaded(modID)) nodeChosenByYAMM = fmt::format("{}/{}", modID, nodeID);
			else if (!nodeID.empty()) nodeChosenByYAMM = nodeID;
		}

		int i = 0;
		if (const auto mainMenuChildren = mainMenu->getChildren()) {
			for (CCNode* node : CCArrayExt<CCNode*>(mainMenuChildren)) {
				if (IS_AFFECTED_BY_YAMM(node)) continue;
				node->setScale(0.f);
				node->setRotation(node->getID() == "play-button" ? -90.f : 90.f);

				if (node->getID() == "play-button") {
					CCDelayTime* playButtonDelay = CCDelayTime::create(APPLY_ANIM_MODIFIERS(.75f));
					CCFiniteTimeAction* eoScale = !Mod::get()->getSettingValue<bool>("classic-play-button-anim") ? static_cast<CCFiniteTimeAction*>(CCEaseBackOut::create(CCScaleTo::create(APPLY_ANIM_EXTENDERS(1.25f), 1.f))) : static_cast<CCFiniteTimeAction*>(CCEaseOut::create(CCScaleTo::create(APPLY_ANIM_EXTENDERS(1.25f), 1.f), 4.f));
					CCFiniteTimeAction* eoRotate = !Mod::get()->getSettingValue<bool>("classic-play-button-anim") ? static_cast<CCFiniteTimeAction*>(CCEaseBackOut::create(CCRotateTo::create(APPLY_ANIM_EXTENDERS(1.25f), 0.f))) : static_cast<CCFiniteTimeAction*>(CCEaseOut::create(CCRotateTo::create(APPLY_ANIM_EXTENDERS(1.25f), 0.f), 4.f));
					CCSpawn* whyDidFodUseCCSpawnAgain = CCSpawn::create(eoScale, eoRotate, nullptr);
					CCSequence* scaleAndRotateSequencePlayButton = CCSequence::create(playButtonDelay, whyDidFodUseCCSpawnAgain, nullptr);

					node->runAction(scaleAndRotateSequencePlayButton);
				} else {
					CCDelayTime* delay = CCDelayTime::create(APPLY_ANIM_MODIFIERS(((.25f * static_cast<float>(i)) + .5f)));
					CCEaseBackOut* eboScale = CCEaseBackOut::create(CCScaleTo::create(APPLY_ANIM_EXTENDERS(1.25f), 1.f));
					CCEaseBackOut* eboRotate = CCEaseBackOut::create(CCRotateTo::create(APPLY_ANIM_EXTENDERS(1.25f), 0.f));
					CCSpawn* whyDidFodUseCCSpawn = CCSpawn::create(eboScale, eboRotate, nullptr);
					CCSequence* scaleAndRotateSequence = CCSequence::create(delay, whyDidFodUseCCSpawn, nullptr);

					node->runAction(scaleAndRotateSequence);
				}

				i++;
			}
		}
		i = 0; // reset incrementer

		if (CCNode* title = this->getChildByIDRecursive("main-title"); title && !(IS_AFFECTED_BY_YAMM(title))) {
			CCDelayTime* delay = CCDelayTime::create(APPLY_ANIM_MODIFIERS(.25f));
			CCEaseBackOut* eboScale = CCEaseBackOut::create(CCScaleTo::create(APPLY_ANIM_EXTENDERS(2.f), 1.f));
			CCSequence* titleSequence = CCSequence::create(delay, eboScale, nullptr);

			title->setScale(0.f);
			title->runAction(titleSequence);
		}

		if (const auto playerChildren = playerUsername->getChildren()) {
			for (CCNode* sprite : CCArrayExt<CCNode*>(playerChildren)) {
				CCDelayTime* delay = CCDelayTime::create(APPLY_ANIM_MODIFIERS(((.2f * static_cast<float>(sprite->getTag())) + .5f)));
				CCEaseElasticOut* eeoScale = CCEaseElasticOut::create(CCScaleTo::create(APPLY_ANIM_EXTENDERS(1.f), 1.f));
				CCSequence* usernameCharSequence = CCSequence::create(delay, eeoScale, nullptr);
				sprite->setScale(0.f);
				sprite->runAction(usernameCharSequence);
			}
		}

		if (CCNode* iThrewItOnTheGround = m_menuGameLayer; !Loader::get()->isModLoaded("undefined0.icon_ninja") && iThrewItOnTheGround && iThrewItOnTheGround->getChildren()) {
			stopLooping = false;
			for (CCNode* node : CCArrayExt<CCNode*>(iThrewItOnTheGround->getChildren())) {
				if (node != m_menuGameLayer->m_groundLayer) {
					if (node != m_menuGameLayer->m_backgroundSprite) node->setVisible(false);
					continue;
				}
				const float origYPos = node->getPositionY();

				CCDelayTime* delay = CCDelayTime::create(APPLY_ANIM_MODIFIERS(.5f));
				CCEaseExponentialOut* eeoMove = CCEaseExponentialOut::create(CCMoveBy::create(APPLY_ANIM_EXTENDERS(2.f), { 0.f, origYPos }));
				CCSequence* groundSequence = CCSequence::create(delay, eeoMove, nullptr);

				node->setPositionY(0.f);
				node->runAction(groundSequence);
			}
			this->schedule(schedule_selector(MyMenuLayer::determinePlayerVisibility));
		}

		if (CCNode* theMenuToScaleFromZero = REDASH ? rightSideMenu : bottomMenu) {
			if (VANILLA_PAGES_LOADED && (VANILLA_PAGES_MENULAYER_BOTTOM && theMenuToScaleFromZero == bottomMenu || VANILLA_PAGES_MENULAYER_RIGHT && theMenuToScaleFromZero == rightSideMenu)) {
				const float nodeOrigYPos = theMenuToScaleFromZero->getPositionY();
				CCDelayTime* delay = CCDelayTime::create(APPLY_ANIM_MODIFIERS(1.f));
				CCEaseExponentialOut* eeoMove = CCEaseExponentialOut::create(CCMoveBy::create(APPLY_ANIM_EXTENDERS(1.f), { 0.f, 100.f }));

				theMenuToScaleFromZero->setPositionY(nodeOrigYPos - 100.f);
				theMenuToScaleFromZero->runAction(CCSequence::create(delay, eeoMove, nullptr));
			} else if (auto tMTSFZChildren = theMenuToScaleFromZero->getChildren()) {
				if (REDASH) tMTSFZChildren->reverseObjects();
				for (CCNode* node : CCArrayExt<CCNode*>(tMTSFZChildren)) {
					if (!node->isVisible() || IS_AFFECTED_BY_YAMM(node)) continue;
					const float nodeOriginalScale = node->getScale();

					CCDelayTime* delayOne = CCDelayTime::create(APPLY_ANIM_MODIFIERS(((static_cast<float>(i) * .25f) + 1.f)));
					CCEaseExponentialOut* eeoScale = CCEaseExponentialOut::create(CCScaleTo::create(APPLY_ANIM_EXTENDERS(1.f), nodeOriginalScale));

					CCDelayTime* delayTwo = CCDelayTime::create(APPLY_ANIM_MODIFIERS(((static_cast<float>(i) * .25f) + 2.f)));
					CCEaseIn* eiScale = CCEaseIn::create(CCScaleTo::create(APPLY_ANIM_EXTENDERS(.25f), (nodeOriginalScale * 1.25f)), 4.f);
					CCEaseBackInOut* ebioScale = CCEaseBackInOut::create(CCScaleTo::create(APPLY_ANIM_EXTENDERS(.75f), nodeOriginalScale));

					node->setScale(0.f);
					node->runAction(CCSequence::create(delayOne, eeoScale, nullptr));
					node->runAction(CCSequence::create(delayTwo, eiScale, ebioScale, nullptr));

					i++;
				}
			}
		}
		i = 0;

		if (auto sideMenuChildren = sideMenu->getChildren()) {
			if (Mod::get()->getSettingValue<bool>("reverse-side-menus")) sideMenuChildren->reverseObjects();
			for (CCNode* node : CCArrayExt<CCNode*>(sideMenuChildren)) {
				if (!node->isVisible()) continue;
				const float nodeOrigXPos = node->getPositionX();

				CCDelayTime* delay = CCDelayTime::create(APPLY_ANIM_MODIFIERS(((static_cast<float>(i) * .25f) + 2.f)));
				CCEaseExponentialOut* eeoMove = CCEaseExponentialOut::create(CCMoveBy::create(APPLY_ANIM_EXTENDERS(1.f), { 100.f, 0.f }));

				node->setPositionX(nodeOrigXPos - 100.f);
				node->runAction(CCSequence::create(delay, eeoMove, nullptr));

				i++;
			}
		}
		i = 0;

		if (const auto topRightChildren = topRightMenu->getChildren()) {
			for (CCNode* node : CCArrayExt<CCNode*>(topRightChildren)) {
				if (!node->isVisible()) continue;
				const float nodeOrigXPos = node->getPositionY();

				CCDelayTime* delay = CCDelayTime::create(APPLY_ANIM_MODIFIERS(((static_cast<float>(i) * .25f) + 2.f)));
				CCEaseExponentialOut* eeoMove = CCEaseExponentialOut::create(CCMoveBy::create(APPLY_ANIM_EXTENDERS(1.f), { 0.f, -100.f }));

				node->setPositionY(nodeOrigXPos + 100.f);
				node->runAction(CCSequence::create(delay, eeoMove, nullptr));

				i++;
			}
		}
		i = 0;

		if (CCNode* theMenuToSlideFromRight = REDASH ? bottomMenu : rightSideMenu) {
			if (VANILLA_PAGES_LOADED && (VANILLA_PAGES_MENULAYER_BOTTOM && theMenuToSlideFromRight == bottomMenu || VANILLA_PAGES_MENULAYER_RIGHT && theMenuToSlideFromRight == rightSideMenu)) {
				const float nodeOrigXPos = theMenuToSlideFromRight->getPositionX();
				CCDelayTime* delay = CCDelayTime::create(APPLY_ANIM_MODIFIERS(1.f));
				CCEaseExponentialOut* eeoMove = CCEaseExponentialOut::create(CCMoveBy::create(APPLY_ANIM_EXTENDERS(1.f), { -100.f, 0.f }));

				theMenuToSlideFromRight->setPositionX(nodeOrigXPos + 100.f);
				theMenuToSlideFromRight->runAction(CCSequence::create(delay, eeoMove, nullptr));
			} else if (auto rightSideMenuChildren = theMenuToSlideFromRight->getChildren()) {
				if (Mod::get()->getSettingValue<bool>("reverse-side-menus")) rightSideMenuChildren->reverseObjects();
				for (CCNode* node : CCArrayExt<CCNode*>(rightSideMenuChildren)) {
					if (!node->isVisible()) continue;
					const float nodeOrigXPos = node->getPositionX();

					CCDelayTime* delay = CCDelayTime::create(APPLY_ANIM_MODIFIERS(((static_cast<float>(i) * .25f) + 2.f)));
					CCEaseExponentialOut* eeoMove = CCEaseExponentialOut::create(CCMoveBy::create(APPLY_ANIM_EXTENDERS(1.f), { -100.f, 0.f }));

					node->setPositionX(nodeOrigXPos + 100.f);
					node->runAction(CCSequence::create(delay, eeoMove, nullptr));

					i++;
				}
			}
		}
		i = 0;

		if (const auto socialMediaChildren = socialMediaMenu->getChildren()) {
			for (CCNode* node : CCArrayExt<CCNode*>(socialMediaChildren)) {
				if (IS_AFFECTED_BY_YAMM(node)) continue;
				CCDelayTime* delay = CCDelayTime::create(APPLY_ANIM_MODIFIERS(((.2f * static_cast<float>(i)) + 1.5f)));
				CCEaseBackOut* eboScale = CCEaseBackOut::create(CCScaleTo::create(APPLY_ANIM_EXTENDERS(1.f), 1.f));
				CCSequence* sequence = CCSequence::create(delay, eboScale, nullptr);

				node->setScale(0.f);
				node->runAction(sequence);

				i++;
			}
		}
		i = 0;

		if (CCNode* closeMenu = this->getChildByID("close-menu")) {
			if (const auto closeChildren = closeMenu->getChildren()) {
				for (CCNode* node : CCArrayExt<CCNode*>(closeChildren)) {
					if (IS_AFFECTED_BY_YAMM(node)) continue;
					CCDelayTime* delay = CCDelayTime::create(APPLY_ANIM_MODIFIERS(((.25f * static_cast<float>(i)) + static_cast<float>(Mod::get()->getSettingValue<double>("close-menu-delay")))));
					CCEaseExponentialOut* eeoScale = CCEaseExponentialOut::create(CCScaleTo::create(APPLY_ANIM_EXTENDERS(1.25f), 1.f));
					CCSequence* sequence = CCSequence::create(delay, eeoScale, nullptr);

					node->setScale(0.f);
					node->runAction(sequence);

					i++;
				}
			}
		}
		i = 0;

		if (const auto mgChildren = moreGamesMenu->getChildren()) {
			for (CCNode* node : CCArrayExt<CCNode*>(mgChildren)) {
				if (IS_AFFECTED_BY_YAMM(node)) continue;
				CCDelayTime* delay = CCDelayTime::create(APPLY_ANIM_MODIFIERS(((.2f * static_cast<float>(i)) + 2.f)));
				CCEaseBackOut* eboScale = CCEaseBackOut::create(CCScaleTo::create(APPLY_ANIM_EXTENDERS(1.f), 1.f));
				CCEaseBackOut* eboRotate = CCEaseBackOut::create(CCRotateTo::create(APPLY_ANIM_EXTENDERS(1.f), 0.f));
				CCSpawn* whyDidFodUseCCSpawn = CCSpawn::create(eboScale, eboRotate, nullptr);
				CCSequence* sequence = CCSequence::create(delay, whyDidFodUseCCSpawn, nullptr);

				node->setScale(0.f);
				node->setRotation(90.f);
				node->runAction(sequence);

				i++;
			}
		}
		i = 0;

		if (const auto profileChildren = profileMenu->getChildren()) {
			for (CCNode* node : CCArrayExt<CCNode*>(profileChildren)) {
				if (IS_AFFECTED_BY_YAMM(node)) continue;
				CCDelayTime* delay = CCDelayTime::create(APPLY_ANIM_MODIFIERS(((.2f * static_cast<float>(i)) + .5f)));
				CCEaseBackOut* eboScale = CCEaseBackOut::create(CCScaleTo::create(APPLY_ANIM_EXTENDERS(1.f), 1.f));
				CCEaseBackOut* eboRotate = CCEaseBackOut::create(CCRotateTo::create(APPLY_ANIM_EXTENDERS(1.f), 0.f));
				CCSpawn* whyDidFodUseCCSpawnAgain = CCSpawn::create(eboScale, eboRotate, nullptr);
				CCSequence* sequence = CCSequence::create(delay, whyDidFodUseCCSpawnAgain, nullptr);

				node->setScale(0.f);
				node->setRotation(-90.f);
				node->runAction(sequence);

				i++;
			}
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

		if (const auto ommMain = redashMain->getChildren()) {
			for (CCNode* node : CCArrayExt<CCNode*>(ommMain)) {
				if (IS_AFFECTED_BY_YAMM(node)) continue;
				CCDelayTime* delay = CCDelayTime::create(APPLY_ANIM_MODIFIERS(((.25f * static_cast<float>(i)) + .5f)));
				CCEaseBackOut* eboScale = CCEaseBackOut::create(CCScaleTo::create(APPLY_ANIM_EXTENDERS(1.25f), 1.f));
				CCEaseBackOut* eboRotate = CCEaseBackOut::create(CCRotateTo::create(APPLY_ANIM_EXTENDERS(1.25f), 0.f));
				CCSpawn* whyDidFodUseCCSpawn = CCSpawn::create(eboScale, eboRotate, nullptr);
				CCSequence* scaleAndRotateSequence = CCSequence::create(delay, whyDidFodUseCCSpawn, nullptr);

				node->setScale(0.f);
				node->setRotation(i % 2 == 0 ? -90.f : 90.f);
				node->runAction(scaleAndRotateSequence);

				i++;
			}
		}
		i = 0;

		if (const auto ommDailies = redashDailies->getChildren()) {
			for (CCNode* node : CCArrayExt<CCNode*>(ommDailies)) {
				if (IS_AFFECTED_BY_YAMM(node)) continue;
				const float nodeOriginalScale = node->getScale();
				CCDelayTime* delayOne = CCDelayTime::create(APPLY_ANIM_MODIFIERS(((static_cast<float>(i) * .25f) + 1.f)));
				CCEaseExponentialOut* eeoScale = CCEaseExponentialOut::create(CCScaleTo::create(APPLY_ANIM_EXTENDERS(1.f), nodeOriginalScale));

				CCDelayTime* delayTwo = CCDelayTime::create(APPLY_ANIM_MODIFIERS(((static_cast<float>(i) * .25f) + 2.f)));
				CCEaseIn* eiScale = CCEaseIn::create(CCScaleTo::create(APPLY_ANIM_EXTENDERS(.25f), (nodeOriginalScale * 1.1f)), 4.f);
				CCEaseBackInOut* ebioScale = CCEaseBackInOut::create(CCScaleTo::create(APPLY_ANIM_EXTENDERS(.75f), nodeOriginalScale));

				node->setScale(0.f);
				node->runAction(CCSequence::create(delayOne, eeoScale, nullptr));
				node->runAction(CCSequence::create(delayTwo, eiScale, ebioScale, nullptr));

				i++;
			}
		}
		i = 0;

		if (const auto ommStats = redashStats->getChildren()) {
			for (CCNode* node : CCArrayExt<CCNode*>(ommStats)) {
				if (IS_AFFECTED_BY_YAMM(node)) continue;
				const float nodeOrigYPos = node->getPositionY();

				CCDelayTime* delay = CCDelayTime::create(APPLY_ANIM_MODIFIERS(((static_cast<float>(i) * .375f) + 1.f)));
				CCEaseExponentialOut* eeoMove = CCEaseExponentialOut::create(CCMoveBy::create(APPLY_ANIM_EXTENDERS(1.f), { 0.f, -75.f }));

				node->setPositionY(nodeOrigYPos + 75.f);
				node->runAction(CCSequence::create(delay, eeoMove, nullptr));

				i++;
			}
		}
		i = 0;

		if (const auto ommButtom = redashBottom->getChildren()) {
			for (CCNode* node : CCArrayExt<CCNode*>(ommButtom)) {
				if (IS_AFFECTED_BY_YAMM(node)) continue;
				const float nodeOrigYPos = node->getPositionY();

				CCDelayTime* delay = CCDelayTime::create(APPLY_ANIM_MODIFIERS(((static_cast<float>(i) * .5f) + 1.f)));
				CCEaseExponentialOut* eeoMove = CCEaseExponentialOut::create(CCMoveBy::create(APPLY_ANIM_EXTENDERS(1.f), { 0.f, 75.f }));

				node->setPositionY(nodeOrigYPos - 75.f);
				node->runAction(CCSequence::create(delay, eeoMove, nullptr));

				i++;
			}
		}
		i = 0;

		if (const auto ommHide = redashHide->getChildren()) {
			for (CCNode* node : CCArrayExt<CCNode*>(ommHide)) {
				if (IS_AFFECTED_BY_YAMM(node)) continue;
				const float nodeOrigYPos = node->getPositionY();

				CCDelayTime* delay = CCDelayTime::create(APPLY_ANIM_MODIFIERS((static_cast<float>(i) * .5f)));
				CCEaseExponentialOut* eeoMove = CCEaseExponentialOut::create(CCMoveBy::create(APPLY_ANIM_EXTENDERS(1.f), { 0.f, 75.f }));

				node->setPositionY(nodeOrigYPos - 75.f);
				node->runAction(CCSequence::create(delay, eeoMove, nullptr));

				i++;
			}
		}
		i = 0;

		if (auto ommTop = redashTop->getChildren()) {
			ommTop->reverseObjects();
			for (CCNode* node : CCArrayExt<CCNode*>(ommTop)) {
				if (IS_AFFECTED_BY_YAMM(node)) continue;
				if (node->getID() == "garage-rope") {
					const float nodeOrigYPos = node->getPositionY();

					CCDelayTime* delay = CCDelayTime::create(APPLY_ANIM_MODIFIERS(((static_cast<float>(i) * .25f) + 2.f)));
					CCEaseExponentialOut* eeoMove = CCEaseExponentialOut::create(CCMoveBy::create(APPLY_ANIM_EXTENDERS(1.f), { 0.f, -75.f }));

					node->setPositionY(nodeOrigYPos + 75.f);
					node->runAction(CCSequence::create(delay, eeoMove, nullptr));
				} else {
					CCDelayTime* delay = CCDelayTime::create(APPLY_ANIM_MODIFIERS(((static_cast<float>(i) * .25f) + 2.f)));
					CCEaseBackOut* eboScale = CCEaseBackOut::create(CCScaleTo::create(APPLY_ANIM_EXTENDERS(1.f), 1.f));
					CCEaseBackOut* eboRotate = CCEaseBackOut::create(CCRotateTo::create(APPLY_ANIM_EXTENDERS(1.f), 0.f));
					CCSpawn* whyDidFodUseCCSpawnAgain = CCSpawn::create(eboScale, eboRotate, nullptr);
					CCSequence* sequence = CCSequence::create(delay, whyDidFodUseCCSpawnAgain, nullptr);

					node->setScale(0.f);
					node->setRotation(-90.f);
					node->runAction(sequence);
				}

				i++;
			}
		}
		i = 0;

		CCNode* redashBG = this->getChildByID("ninxout.redash/bottom-menu-bg");
		if (!redashBG) return true;
		if (CCScale9Sprite* ommBG = typeinfo_cast<CCScale9Sprite*>(redashBG)) {
			const GLubyte origOpacity = ommBG->getOpacity();

			CCDelayTime* delay = CCDelayTime::create(APPLY_ANIM_MODIFIERS(.0f));
			CCFadeTo* fadeIn = CCFadeTo::create(APPLY_ANIM_EXTENDERS(.5f), origOpacity);
			CCSequence* sequence = CCSequence::create(delay, fadeIn, nullptr);

			ommBG->setOpacity(0);
			ommBG->runAction(sequence);
		}

		return true;
	}
};