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

#define SPEED_MIN .01f
#define SPEED_MAX 16.f
#define DELAY_MIN .0f
#define DELAY_MAX 10.f
#define DURTN_MIN .0f
#define DURTN_MAX 2.f
#define CLAMP_FLOAT(setting, min, max) std::clamp(setting, min, max)
#define ANIM_SPEED CLAMP_FLOAT(speed, SPEED_MIN, SPEED_MAX)
#define ANIM_DELAY CLAMP_FLOAT(delaySetting, DELAY_MIN, DELAY_MAX)
#define ANIM_DURTN CLAMP_FLOAT(addtlDuration, DURTN_MIN, DURTN_MAX)
#define APPLY_ANIM_MODIFIERS(originalValue) ((originalValue / ANIM_SPEED) + ANIM_DELAY)
#define APPLY_ANIM_EXTENDERS(originalValue) ((originalValue / ANIM_SPEED) + ANIM_DURTN)
#define REPLAY_COOLDOWN APPLY_ANIM_EXTENDERS(highestI)
#define UPDATE_I\
	if (highestI < i) highestI = i;\
	i = 0;

bool enabled = true;
bool classic = false;
bool reverse = false;
bool rplyBtn = false;
bool alowRpy = false;
bool queuing = false;

float speed = 1.0f;
float delaySetting = 0.0f;
float addtlDuration = 0.0f;

int highestI = 0;

float elapsedTime = 0.f;

bool stopLooping = false; // m_fields for a singlefile mod is silly --raydeeux
bool jumpedAlready = false; // m_fields for a singlefile mod is silly --raydeeux

class $modify(MyMenuLayer, MenuLayer) {
	static void onModify(auto& self) {
		if (YAMM) (void) self.setHookPriorityAfterPost("MenuLayer::init", YAMM_ID);
		else if (REDASH) (void) self.setHookPriorityAfterPost("MenuLayer::init", REDASH_ID);
		else if (VANILLA_PAGES_LOADED) (void) self.setHookPriorityAfterPost("MenuLayer::init", VANILLA_PAGES_ID);
		else (void) self.setHookPriority("MenuLayer::init", -3998);
	}
	void determinePlayerVisibility(float dt) {
		if (!enabled) return;
		if (Loader::get()->isModLoaded("undefined0.icon_ninja")) return;
		if (!m_menuGameLayer || !m_menuGameLayer->getChildren()) return;
		PlayerObject* player = m_menuGameLayer->m_playerObject;
		if (player && (player->getPositionX() > 0.f || player->getRealPosition().x > 0.f)) {
			if (stopLooping && player->m_isDart && player->m_waveTrail) player->m_waveTrail->setVisible(true);
			else if (stopLooping && !jumpedAlready && player->m_isRobot) {
				// in case any whiny kids start yelling at me for why the robot isnt jumping --raydeeux
				m_menuGameLayer->tryJump(.1f);
				jumpedAlready = true;
			}
			return;
		}
		if (stopLooping) {
			if (jumpedAlready) this->unschedule(schedule_selector(MyMenuLayer::determinePlayerVisibility));
			return;
		}
		const float groundPos = m_menuGameLayer->m_groundLayer->getPositionY();
		for (CCNode* node : CCArrayExt<CCNode*>(m_menuGameLayer->getChildren())) {
			if (node == m_menuGameLayer->m_groundLayer || node == m_menuGameLayer->m_backgroundSprite) continue;
			node->setVisible(false);
			if (node == player->m_waveTrail || node == player->m_regularTrail || node == player->m_shipStreak) continue;
			if (groundPos > 89.f) node->setVisible(true);
		}
		if (groundPos > 89.f) stopLooping = true;
	}
	void allowReplay(float dt) {
		if (alowRpy || elapsedTime >= REPLAY_COOLDOWN) {
			this->unschedule(schedule_selector(MyMenuLayer::allowReplay));
			return;
		}
		elapsedTime += dt;
		if (elapsedTime < REPLAY_COOLDOWN) return;
		if (CCNode* animateButton = this->getChildByIDRecursive("animate-button"_spr)) {
			static_cast<CCMenuItemSpriteExtra*>(animateButton)->setEnabled(true);
			static_cast<CCMenuItemSpriteExtra*>(animateButton)->setColor({255, 255, 255});
		}
		elapsedTime = 0.f;
		alowRpy = true;
	}
	bool init() {
		if (!MenuLayer::init()) return false;

		alowRpy = false;
		elapsedTime = 0.f;
		CCNode* menu = REDASH ? this->getChildByID("right-side-menu") : this->getChildByID("bottom-menu");
		if (!enabled || !menu) return true;
		if (rplyBtn) {
			CCSprite* animateSprite = CCSprite::createWithSpriteFrameName("edit_eAnimateBtn_001.png");
			animateSprite->setScale(336.f / 162.f);
			animateSprite->setID("animate-sprite"_spr);
			CCMenuItemSpriteExtra* animateButton = CCMenuItemSpriteExtra::create(CircleButtonSprite::create(animateSprite), this, menu_selector(MyMenuLayer::animateWrapper));
			animateButton->setID("animate-button"_spr);
			animateButton->setTag(5282025);
			menu->addChild(animateButton);
			menu->updateLayout();
		}

		if (!queuing) MyMenuLayer::animate();
		else Loader::get()->queueInMainThread([this] { MyMenuLayer::animate(); });

		return true;
	}
	void animateWrapper(CCObject* sender) {
		if (!enabled || !sender || !rplyBtn || sender->getTag() != 5282025) return;
		if (!alowRpy) return FLAlertLayer::create("WATCH IT, BUDDY!", "<c_>I'M STILL ON COOLDOWN, FOR CRYING OUT LOUD!</c>\n<c_>LEAVE ME ALONE!</c>", "Contemplate Life")->show();
		MyMenuLayer::animate();
	}
	void animate() {
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
		if (!enabled) return;
		alowRpy = false;

		CCNode* mainMenu = this->getChildByID("main-menu");
		CCNode* bottomMenu = this->getChildByID("bottom-menu");
		CCNode* profileMenu = this->getChildByID("profile-menu");
		CCNode* rightSideMenu = this->getChildByID("right-side-menu");
		CCNode* topRightMenu = this->getChildByID("top-right-menu");
		CCNode* sideMenu = this->getChildByID("side-menu");
		CCNode* socialMediaMenu = this->getChildByID("social-media-menu");
		CCNode* moreGamesMenu = this->getChildByID("more-games-menu");
		CCNode* playerUsername = this->getChildByID("player-username");

		if (!mainMenu || !bottomMenu || !profileMenu || !rightSideMenu || !topRightMenu || !sideMenu || !socialMediaMenu || !moreGamesMenu || !playerUsername) return;

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

		if (CCNode* animateButton = this->getChildByIDRecursive("animate-button"_spr)) {
			static_cast<CCMenuItemSpriteExtra*>(animateButton)->setEnabled(false);
			static_cast<CCMenuItemSpriteExtra*>(animateButton)->setColor({128, 128, 128});
		}

		int i = 0;
		highestI = 0;
		if (const auto mainMenuChildren = mainMenu->getChildren(); mainMenuChildren && mainMenu->isVisible()) {
			for (CCNode* node : CCArrayExt<CCNode*>(mainMenuChildren)) {
				if (IS_AFFECTED_BY_YAMM(node)) continue;
				node->setScale(0.f);
				node->setRotation(node->getID() == "play-button" ? -90.f : 90.f);

				if (node->getID() == "play-button") {
					CCDelayTime* playButtonDelay = CCDelayTime::create(APPLY_ANIM_MODIFIERS(.75f));
					CCFiniteTimeAction* eoScale = !classic ? static_cast<CCFiniteTimeAction*>(CCEaseBackOut::create(CCScaleTo::create(APPLY_ANIM_EXTENDERS(1.25f), 1.f))) : static_cast<CCFiniteTimeAction*>(CCEaseOut::create(CCScaleTo::create(APPLY_ANIM_EXTENDERS(1.25f), 1.f), 4.f));
					CCFiniteTimeAction* eoRotate = !classic ? static_cast<CCFiniteTimeAction*>(CCEaseBackOut::create(CCRotateTo::create(APPLY_ANIM_EXTENDERS(1.25f), 0.f))) : static_cast<CCFiniteTimeAction*>(CCEaseOut::create(CCRotateTo::create(APPLY_ANIM_EXTENDERS(1.25f), 0.f), 4.f));
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
		UPDATE_I

		if (CCNode* title = this->getChildByIDRecursive("main-title"); title && !(IS_AFFECTED_BY_YAMM(title))) {
			CCDelayTime* delay = CCDelayTime::create(APPLY_ANIM_MODIFIERS(.25f));
			CCEaseBackOut* eboScale = CCEaseBackOut::create(CCScaleTo::create(APPLY_ANIM_EXTENDERS(2.f), 1.f));
			CCSequence* titleSequence = CCSequence::create(delay, eboScale, nullptr);

			title->setScale(0.f);
			title->runAction(titleSequence);
		}

		if (const auto playerChildren = playerUsername->getChildren(); playerChildren && playerUsername->isVisible()) {
			for (CCNode* sprite : CCArrayExt<CCNode*>(playerChildren)) {
				CCDelayTime* delay = CCDelayTime::create(APPLY_ANIM_MODIFIERS(((.2f * static_cast<float>(sprite->getTag())) + .5f)));
				CCEaseElasticOut* eeoScale = CCEaseElasticOut::create(CCScaleTo::create(APPLY_ANIM_EXTENDERS(1.f), 1.f));
				CCSequence* usernameCharSequence = CCSequence::create(delay, eeoScale, nullptr);
				sprite->setScale(0.f);
				sprite->runAction(usernameCharSequence);
			}
		}

		if (CCNode* iThrewItOnTheGround = m_menuGameLayer; !Loader::get()->isModLoaded("undefined0.icon_ninja") && iThrewItOnTheGround && iThrewItOnTheGround->getChildren() && iThrewItOnTheGround->isVisible()) {
			stopLooping = false;
			jumpedAlready = false;
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
			} else if (auto tMTSFZChildren = theMenuToScaleFromZero->getChildren(); tMTSFZChildren && theMenuToScaleFromZero->isVisible()) {
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
		UPDATE_I

		if (auto sideMenuChildren = sideMenu->getChildren(); sideMenuChildren && sideMenu->isVisible()) {
			if (reverse) sideMenuChildren->reverseObjects();
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
		UPDATE_I

		if (const auto topRightChildren = topRightMenu->getChildren(); topRightChildren && topRightMenu->isVisible()) {
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
		UPDATE_I

		if (CCNode* theMenuToSlideFromRight = REDASH ? bottomMenu : rightSideMenu) {
			if (VANILLA_PAGES_LOADED && (VANILLA_PAGES_MENULAYER_BOTTOM && theMenuToSlideFromRight == bottomMenu || VANILLA_PAGES_MENULAYER_RIGHT && theMenuToSlideFromRight == rightSideMenu)) {
				const float nodeOrigXPos = theMenuToSlideFromRight->getPositionX();
				CCDelayTime* delay = CCDelayTime::create(APPLY_ANIM_MODIFIERS(1.f));
				CCEaseExponentialOut* eeoMove = CCEaseExponentialOut::create(CCMoveBy::create(APPLY_ANIM_EXTENDERS(1.f), { -100.f, 0.f }));

				theMenuToSlideFromRight->setPositionX(nodeOrigXPos + 100.f);
				theMenuToSlideFromRight->runAction(CCSequence::create(delay, eeoMove, nullptr));
			} else if (auto rightSideMenuChildren = theMenuToSlideFromRight->getChildren(); rightSideMenuChildren && theMenuToSlideFromRight->isVisible()) {
				if (reverse) rightSideMenuChildren->reverseObjects();
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
		UPDATE_I

		if (const auto socialMediaChildren = socialMediaMenu->getChildren(); socialMediaChildren && socialMediaMenu->isVisible()) {
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
		UPDATE_I

		if (CCNode* closeMenu = this->getChildByID("close-menu")) {
			if (const auto closeChildren = closeMenu->getChildren(); closeChildren && closeMenu->isVisible()) {
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
		UPDATE_I

		if (const auto mgChildren = moreGamesMenu->getChildren(); mgChildren && moreGamesMenu->isVisible()) {
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
		UPDATE_I

		if (const auto profileChildren = profileMenu->getChildren(); profileChildren && profileMenu->isVisible()) {
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
		UPDATE_I

		CCNode* redashMenu = REDASH ? this->getChildByID("ninxout.redash/redash-menu") : nullptr;
		CCNode* redashHide = REDASH ? this->getChildByID("ninxout.redash/hide-button-menu") : nullptr;
		if (!REDASH || !redashMenu || !redashHide) {
			if (rplyBtn) this->schedule(schedule_selector(MyMenuLayer::allowReplay));
			return;
		}

		CCNode* redashMain = redashMenu->getChildByID("ninxout.redash/main-menu"); // rotate + scale
		CCNode* redashDailies = redashMenu->getChildByID("ninxout.redash/dailies-menu"); // scale
		CCNode* redashStats = redashMenu->getChildByID("ninxout.redash/stats-menu"); // move from top
		CCNode* redashBottom = redashMenu->getChildByID("ninxout.redash/bottom-menu"); // move from bottom
		CCNode* redashTop = redashMenu->getChildByID("ninxout.redash/top-menu"); // scale lock, move rope

		if (!redashMain || !redashDailies || !redashStats || !redashBottom || !redashTop) return;

		if (const auto ommMain = redashMain->getChildren(); ommMain && redashMain->isVisible()) {
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
		UPDATE_I

		if (const auto ommDailies = redashDailies->getChildren(); ommDailies && redashDailies->isVisible()) {
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
		UPDATE_I

		if (const auto ommStats = redashStats->getChildren(); ommStats && redashStats->isVisible()) {
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
		UPDATE_I

		if (const auto ommButtom = redashBottom->getChildren(); ommButtom && redashBottom->isVisible()) {
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
		UPDATE_I

		if (const auto ommHide = redashHide->getChildren(); ommHide && redashHide->isVisible()) {
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
		UPDATE_I

		if (auto ommTop = redashTop->getChildren(); ommTop && redashTop->isVisible()) {
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
		UPDATE_I

		CCNode* redashBG = this->getChildByID("ninxout.redash/bottom-menu-bg");
		if (!redashBG) return;
		CCScale9Sprite* ommBG = static_cast<CCScale9Sprite*>(redashBG);
		const GLubyte origOpacity = ommBG->getOpacity();

		CCDelayTime* delay = CCDelayTime::create(APPLY_ANIM_MODIFIERS(.0f));
		CCFadeTo* fadeIn = CCFadeTo::create(APPLY_ANIM_EXTENDERS(.5f), origOpacity);
		CCSequence* sequence = CCSequence::create(delay, fadeIn, nullptr);

		ommBG->setOpacity(0);
		ommBG->runAction(sequence);

		this->schedule(schedule_selector(MyMenuLayer::allowReplay));
	}
};

$on_mod(Loaded) {
	enabled = Mod::get()->getSettingValue<bool>("enabled");
	classic = Mod::get()->getSettingValue<bool>("classic-play-button-anim");
	queuing = Mod::get()->getSettingValue<bool>("queue-in-main-thread");
	reverse = Mod::get()->getSettingValue<bool>("reverse-side-menus");
	rplyBtn = Mod::get()->getSettingValue<bool>("add-replay-button");
	speed = Mod::get()->getSettingValue<double>("animation-speed");
	delaySetting = Mod::get()->getSettingValue<double>("animation-delay");
	addtlDuration = Mod::get()->getSettingValue<double>("animation-duration");
	listenForSettingChanges<bool>("enabled", [](bool updatedEnabledSetting) {
		enabled = updatedEnabledSetting;
	});
	listenForSettingChanges<double>("animation-speed", [](double speedUpdated) {
		speed = speedUpdated;
	});
	listenForSettingChanges<double>("animation-delay", [](double delayUpdated) {
		delaySetting = delayUpdated;
	});
	listenForSettingChanges<double>("animation-duration", [](double addtlDurationUpdated) {
		addtlDuration = addtlDurationUpdated;
	});
	listenForSettingChanges<bool>("classic-play-button-anim", [](bool newClassic) {
		classic = newClassic;
	});
	listenForSettingChanges<bool>("reverse-side-menus", [](bool newReverse) {
		reverse = newReverse;
	});
	listenForSettingChanges<bool>("add-replay-button", [](bool updatedReplayBtn) {
		rplyBtn = updatedReplayBtn;
	});
	listenForSettingChanges<bool>("queue-in-main-thread", [](bool newQueuing) {
		queuing = newQueuing;
	});
}