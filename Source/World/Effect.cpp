#include "Effect.hpp"
#include "World.hpp"
#include "../Graphics/BloomEffect.hpp" // REMOVE:
#include "../Graphics/Color.hpp"

#include <iostream>

Effect::Effect(int flag, int duration)
	: flag(flag)
	, duration(duration)
{
}

int Effect::getFlag() const
{
	return flag;
}

int Effect::getDuration() const
{
	return duration;
}

bool Effect::isFinished() const
{
	// NOTE: from <= to <
	return duration < 0;
}

void Effect::lengthen(int duration)
{
	this->duration += duration;

#ifdef _DEBUG
	if (flag == Actor::NoCommand)
		std::cout << "No Command: " << duration << '\n';
#endif
}

void Effect::start(Actor& actor)
{
	switch (flag)
	{
	case Actor::IsBlind:
		message(actor.chooseStr(L"oh, bummer!  Everything is dark!  Help!",
								L"a cloak of darkness falls around you."), Color::White);
		getWorld().setFovRange(0.f);
		break;

	case Actor::IsLevitating:
		message(actor.chooseStr(L"oh, wow!  You're floating in the air!",
								L"you start to float in the air."), Color::White);
		break;
		
	case Actor::IsHasted:
		if (actor.hasFlag(Actor::IsSlowed))
		{
			actor.removeEffect(Actor::IsSlowed);
			return;
		}
		actor.ntimes = 2;
		break;

	case Actor::IsConfused:
		// message(actor.chooseStr(L"what a tripy feeling!",
								// L"wait, what's going on here. Huh? What? Who?"), Color::White);
		break;

	case Actor::IsHallucinating:
		message(L"oh, wow!  Everything seems so cosmic!", Color::White);
		BloomEffect::hallucination = true; // UNDONE:
		getLevel().colorize();
		break;

	case Actor::IsSlowed:
		if (actor.hasFlag(Actor::IsHasted))
		{
			actor.removeEffect(Actor::IsHasted);
			return;
		}
		break;

	case Actor::NoCommand:
		actor.removeFlag(Actor::IsRunning);
#ifdef _DEBUG
		std::cout << "No Command: " << duration << '\n';
#endif
		break;
	}

	actor.addFlag(static_cast<Actor::Flags>(flag));
}

void Effect::update(Actor& actor)
{
	if (flag != Actor::NoMove)
		--duration;

	if (duration > 0 && flag == Actor::IsHasted)
		actor.ntimes = 2;
}

void Effect::end(Actor& actor)
{
	actor.removeFlag(static_cast<Actor::Flags>(flag));

	/* daemons.c */
	switch (flag)
	{
	case Actor::CanSee:
		// unsee: Turn off the ability to see invisible
		break;

	case Actor::IsBlind:
		// sight: He gets his sight back
		message(actor.chooseStr(L"far out!  Everything is all cosmic again.",
								L"the veil of darkness lifts."), Color::White);
		getWorld().resetFovRange();
		break;

	case Actor::IsLevitating:
		// land: Land from a levitation potion
		message(actor.chooseStr(L"bummer!  You've hit the ground.",
								L"you float gently to the ground."), Color::White);
		break;

	case Actor::IsHasted:
		// nohaste: End the hasting
		message(L"you feel yourself slowing down.", Color::White);
		break;

	case Actor::IsConfused:
		// unconfuse: Release the poor player from his confusion
		message(L"you feel less " + actor.chooseStr(L"trippy", L"confused") + L" now.", Color::White);
		break;

	case Actor::IsHallucinating:
		// come_down: Take the hero down off her acid trip
		message(L"everything looks SO boring now.", Color::White);
		BloomEffect::hallucination = false;
		getLevel().repaint();
		break;

	case Actor::NoCommand:
		if (actor.getType() == Actor::Hero)
		{
			actor.addFlag(Actor::IsRunning);
			message(L"you can move again.", Color::White);
		}
		break;
	}
}
