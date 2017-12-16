#include "Player.hpp"
#include "../Action/WalkAction.hpp"
#include "../Action/ItemAction.hpp"

#ifdef _WIN32
#include <Windows.h>
#endif

namespace
{
	Action::Ptr getMoveAction(int x, int y, bool shift)
	{
		if (shift)
			return std::make_unique<RunAction>(x, y);
		else
			return std::make_unique<WalkAction>(x, y);
	}
}

Player::Player()
{
	initializeKeys();
	initializeActions();
}

Action::Ptr Player::getAction(sf::Keyboard::Key key)
{
	auto found = keyBinding.find(key);
	bool shift = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) || sf::Keyboard::isKeyPressed(sf::Keyboard::RShift);

	if (shift && key == sf::Keyboard::Comma) // <
		return commandBinding[Command::Ascend](shift);

	if (shift && key == sf::Keyboard::Period) // >
		return commandBinding[Command::Descend](shift);

	if (found != keyBinding.end())
		return commandBinding[found->second](shift);

#ifdef _WIN32
	// Numpad5 without Num Lock
	if (GetAsyncKeyState(VK_CLEAR) & 0x8000)
		return commandBinding[Command::Rest](shift);
#endif

	return nullptr;
}

void Player::initializeKeys()
{
	// Arrow keys
	keyBinding[sf::Keyboard::Left]     = Command::MoveLeft;
	keyBinding[sf::Keyboard::Right]    = Command::MoveRight;
	keyBinding[sf::Keyboard::Up]       = Command::MoveUp;
	keyBinding[sf::Keyboard::Down]     = Command::MoveDown;
	keyBinding[sf::Keyboard::Home]     = Command::MoveLeftUp;
	keyBinding[sf::Keyboard::End]      = Command::MoveLeftDown;
	keyBinding[sf::Keyboard::PageUp]   = Command::MoveRightUp;
	keyBinding[sf::Keyboard::PageDown] = Command::MoveRightDown;

	// Numpad keys
	keyBinding[sf::Keyboard::Numpad4]  = Command::MoveLeft;
	keyBinding[sf::Keyboard::Numpad6]  = Command::MoveRight;
	keyBinding[sf::Keyboard::Numpad8]  = Command::MoveUp;
	keyBinding[sf::Keyboard::Numpad2]  = Command::MoveDown;
	keyBinding[sf::Keyboard::Numpad7]  = Command::MoveLeftUp;
	keyBinding[sf::Keyboard::Numpad1]  = Command::MoveLeftDown;
	keyBinding[sf::Keyboard::Numpad9]  = Command::MoveRightUp;
	keyBinding[sf::Keyboard::Numpad3]  = Command::MoveRightDown;

	// Vi keys
	keyBinding[sf::Keyboard::H]        = Command::MoveLeft;
	keyBinding[sf::Keyboard::L]        = Command::MoveRight;
	keyBinding[sf::Keyboard::K]        = Command::MoveUp;
	keyBinding[sf::Keyboard::J]        = Command::MoveDown;
	keyBinding[sf::Keyboard::Y]        = Command::MoveLeftUp;
	keyBinding[sf::Keyboard::B]        = Command::MoveLeftDown;
	keyBinding[sf::Keyboard::U]        = Command::MoveRightUp;
	keyBinding[sf::Keyboard::N]        = Command::MoveRightDown;
	
	// Rest
	keyBinding[sf::Keyboard::Delete]   = Command::Rest;
	keyBinding[sf::Keyboard::Numpad5]  = Command::Rest;
	keyBinding[sf::Keyboard::Period]   = Command::Rest;
	keyBinding[sf::Keyboard::Z]        = Command::Rest;

	// Pick up
	keyBinding[sf::Keyboard::Space]    = Command::Interact;
	keyBinding[sf::Keyboard::Return]   = Command::Interact;
	keyBinding[sf::Keyboard::Comma]    = Command::PickUp;
	keyBinding[sf::Keyboard::G]        = Command::PickUp;

	keyBinding[sf::Keyboard::S]        = Command::Search;
}

void Player::initializeActions()
{
	commandBinding[Command::MoveLeft]      = [] (bool shift) { return getMoveAction(-1,  0, shift); };
	commandBinding[Command::MoveRight]     = [] (bool shift) { return getMoveAction( 1,  0, shift); };
	commandBinding[Command::MoveUp]        = [] (bool shift) { return getMoveAction( 0, -1, shift); };
	commandBinding[Command::MoveDown]      = [] (bool shift) { return getMoveAction( 0,  1, shift); };
	
	commandBinding[Command::MoveLeftUp]    = [] (bool shift) { return getMoveAction(-1, -1, shift); };
	commandBinding[Command::MoveLeftDown]  = [] (bool shift) { return getMoveAction(-1,  1, shift); };
	commandBinding[Command::MoveRightUp]   = [] (bool shift) { return getMoveAction( 1, -1, shift); };
	commandBinding[Command::MoveRightDown] = [] (bool shift) { return getMoveAction( 1,  1, shift); };
	
	commandBinding[Command::Rest]          = [] (bool shift) { return getMoveAction( 0,  0, shift); }; // RestAction
	commandBinding[Command::PickUp]        = [] (bool shift) { return shift ? nullptr : std::make_unique<PickUpAction>(); };
	commandBinding[Command::Search]        = [] (bool shift) { return shift ? nullptr : std::make_unique<SearchAction>(); };
	commandBinding[Command::Interact]      = [] (bool shift) { return shift ? nullptr : std::make_unique<InteractAction>(); };

	commandBinding[Command::Ascend]        = [] (bool shift) { return shift ? std::make_unique<AscendAction>() : nullptr; };
	commandBinding[Command::Descend]       = [] (bool shift) { return shift ? std::make_unique<DescendAction>() : nullptr; };
}
