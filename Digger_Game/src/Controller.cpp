#include <Controller.h>
#include <sstream>
#include "utilities.h"
#include <experimental/vector>

// declare and initialize static var outside of the class
//Controller Controller::m_instance = Controller();

Controller::Controller()
	:m_window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Digger Game"),
	 m_data(), m_fileLevel(FILE_PATH), m_digger({0,0}), m_levelTime(0) ,
	m_eatSound(Resources::instance().getEatBuffer())
{
	//m_window.setFramerateLimit(90);

	if (m_fileLevel.bad()) {
		std::cerr << "coudn't open Board.txt" << std::endl;
		exit(EXIT_FAILURE);
	}
}
  
//------  run game loop ---------
void Controller::run()
{
	bool win = false;
	bool keepPlaying = true;

	while (keepPlaying)
	{
		if (!m_window.isOpen())
			return;

		if (!readLevel()) {
			win = true;
			break;
		}

		sf::Clock clock;

		startLevelAffects();
		
		while (levelOn(keepPlaying) && m_window.isOpen())
		{
			draw();

			float deltaTime = clock.restart().asSeconds();
			deltaTime = (deltaTime >= 1) ? 0 : deltaTime;

			for (auto& monster : m_monsterVec)
			{
				monster->move(deltaTime, *this);
			}

			//handel user input and calls digger.move() accordingly
			if (!processEvents(deltaTime))
			{
				m_window.close();
				return;
			}
			clock.restart();
			handlePlayerDeath();
		}
	}

	m_window.close();

	if (win)
		endGameAnnouncement(WIN_MASSAGE);
	else
		endGameAnnouncement(LOSE_MASSAGE);

	updateStats();
}


//------  its controls the running of the game  ---------

void Controller::resetMovablePosition()
{
	m_data.restartClock();
	m_digger.resetPosition();

	for (auto& monster : m_monsterVec)
	{
		monster->resetPosition();
	}
}

sf::Vector2f Controller::getDiggerPosition()
{
	return m_digger.getPosition();
}

//----- check if level ended  -------

void Controller::handlePlayerDeath()
{
	for (auto& monster : m_monsterVec)
	{
		if (monster->intersects(m_digger))
		{
			m_data.decLives();
			looseAffects();
			resetMovablePosition();
			break;
		}
	}
}

bool Controller::levelOn(bool& keepPlaying)
{
	if (!m_data.getDiamondsAmount())
	{
		endLevelAffects();
		//if player finished exit 1 loop (go to next level)
		return false;
	}

	if (m_data.getStonesLeft() < 0 || m_data.getTimeLeft() <= 0)
	{
		looseAffects();
		initLevel();
		m_data.decLives();
	}

	if (!m_data.getLives())
	{
		//if player loose exit 2 loops
		keepPlaying = false;
		return false;
	}

	return true;
}


bool Controller::pauseGame()
{

	m_data.pauseOrPlayClock();
	sf::Sprite background(Resources::instance().getGameBackround());
	background.setPosition(0, 0);
	background.scale(4.1f, 4.1f);
	background.setColor({ 255, 255, 255, 70 });

	sf::Sprite menuButton{Resources::instance().getMenuButton()};
	menuButton.setPosition(200.f, WINDOW_HEIGHT - 400.f);
	menuButton.scale(0.26f, 0.26f);

	//----  text Button ----
	sf::Font font;
	font.loadFromFile("C:/Windows/Fonts/georgiaz.ttf");
	sf::Text textButton = sf::Text("MENU", font);
	textButton.setPosition(sf::Vector2f(300.f, WINDOW_HEIGHT - 364.f));
	textButton.setFillColor(sf::Color::Black);
	textButton.setCharacterSize(38);

	drawWithoutDisplay();
	m_window.draw(background);
	m_window.draw(menuButton);
	m_window.draw(textButton);
	m_window.display();

	while (m_window.isOpen())
	{
		// getting user input from keyboard.
		if (auto event = sf::Event{}; m_window.waitEvent(event))
		{
			switch (event.type)
			{
			case sf::Event::Closed:
				return false;

			case sf::Event::KeyReleased:
				switch (event.key.code)
				{
				case sf::Keyboard::Escape:
					return false;

				case sf::Keyboard::Space:
					m_data.pauseOrPlayClock();
					return true;
				}

			case sf::Event::MouseButtonReleased:
			{ //scope made for local variable (location)

				auto position = m_window.mapPixelToCoords(
					{ event.mouseButton.x, event.mouseButton.y });

				//if user pressed on the menu button exit to menu.
				if (menuButton.getGlobalBounds().contains(position))
					return false;
			}
			break;

			}
		}
	}
	return true;
}

//----- reads events from the Keyboard  -------

bool Controller::processEvents(float deltaTime)
{
	sf::Event event;
	while (m_window.pollEvent(event))
	{
		switch (event.type)
		{
		case sf::Event::Closed:
			return false;

		case sf::Event::KeyReleased:
			switch (event.key.code)
			{
			case sf::Keyboard::Escape:
				return false;
			case sf::Keyboard::Space:
				if(!pauseGame())
					return false;
			}
		}
	}

	m_digger.move(deltaTime, *this);

	std::experimental::erase_if(m_edibleVec, [](const auto& edible)
		{ return edible->is_eaten(); });

	return true;
}

//------   display every object on window  ------

void Controller::draw()
{
	drawWithoutDisplay();
	m_window.display();
}

void Controller::drawWithoutDisplay()
{
	m_window.clear(MENU_BACKGROUND);

	sf::Sprite background(Resources::instance().getGameBackround());
	background.setPosition(0, 0);
	background.scale(4.1f, 4.1f);
	background.setColor({ 255, 255, 255, 180 });
	m_window.draw(background);

	m_window.draw(m_boardRect);

	for (const auto& edible : m_edibleVec)
		edible->draw(m_window);

	for (const auto& wall : m_wallVec)
		wall->draw(m_window);

	for (const auto& monster : m_monsterVec)
		monster->draw(m_window);

	m_digger.draw(m_window);

	m_data.draw(m_window);
}

//------ reads the next level if it does exist  ------

bool Controller::readLevel()
{
	std::string line;

	// if we reached the end of file finish the game.
	if (!std::getline(m_fileLevel, line)) {
		return false;
	}

	m_charBoard.clear();

	// reading the first to numbers from file 
	auto istr = std::istringstream(line);
	int allowedStones;
	istr >> m_rows >> m_columns >> allowedStones >> m_levelTime;

	m_data.setAllowedStones(allowedStones);
	
	float rec_width =  m_columns * OBJECT_WIDTH;
	float rec_height =  m_rows * OBJECT_HEIGHT;

	m_boardRect.setSize({ rec_width, rec_height });
	m_boardRect.setPosition({ (WINDOW_WIDTH - rec_width) / 2, DATA_HEIGHT });
	m_boardRect.setFillColor(RECTANGLE_COLOR_LIGHT);
	m_boardRect.setOutlineColor(LINE_COLOR);
	m_boardRect.setOutlineThickness(LINE_THICKNESS);

	// reading one level from file
	for (int i = 0; i < m_rows; i++) 
    {
		std::getline(m_fileLevel, line);
		addSpacesToString(line);
		m_charBoard.push_back(line);
	}

	//getting the empty line before next level
	std::getline(m_fileLevel, line);

	m_data.incLevel();

	initLevel();

	draw();

	return true;
}

//----- initialize every game object from m_charBoard  ---------

void Controller::initLevel()
{
	m_data.resetLevel();
	m_data.incTime(m_levelTime);

	m_edibleVec.clear();
	m_monsterVec.clear();
	m_wallVec.clear();

	int monsterCnt = 0; 

	for (int r = 0; r < m_rows; r++)
		for (int c = 0; c < m_columns; c++)
		{

			sf::Vector2f position{ m_boardRect.getPosition().x + c * OBJECT_WIDTH,
				m_boardRect.getPosition().y + r * OBJECT_HEIGHT };
			
			char ch = m_charBoard[r][c];

			switch (ch)
			{
			case SPACE:
				break;

			case DIGGER:
				m_digger.setStartPosition(position);// should also reset startingPosition.
				break;

			case MONSTER:
			    {
				monsterCnt++;
				// chose between smart and stupid monsters
				  
				  if (monsterCnt % 2 == 0)
					  m_monsterVec.push_back(std::make_unique<SmartMonster>(position));
				  else
					  m_monsterVec.push_back(std::make_unique<StupidMonster>(position));
			    }
			    break;

			case STONE:
				m_edibleVec.push_back(std::make_unique<Stone>(position));
				break;

			case WALL:
				m_wallVec.push_back(std::make_unique<Wall>(position));
				break;

			case DIAMOND:
				m_edibleVec.push_back(std::make_unique<Diamond>(position));
				m_data.incDiamondsAmount();
				break;
			
			case GIFT:
				auto num = random_generator(1, 7);

				if(num <= 2)
					m_edibleVec.push_back(std::make_unique<ScoreGift>(position));
				else if(num <= 4)
					m_edibleVec.push_back(std::make_unique<TimeGift>(position));
				else if(num <= 6)
					m_edibleVec.push_back(std::make_unique<StoneGift>(position));
				else if(num == 7)
					m_edibleVec.push_back(std::make_unique<LivesGift>(position));

				break;
			}
		}
}



//-- adding spaces to string, to match string size with board size ---------

void Controller::addSpacesToString(std::string& line)
{
	while (line.size() < m_columns)
		line += SPACE;
}


bool Controller::handleMovement(const Digger& obj)
{
	if(!validMovement(obj))
		return false;

	for (auto& edible : m_edibleVec)
	{
		if (obj.intersects(*edible))
		{
			edible->eatWithUpdate(m_data);
			m_eatSound.play();
			return true;
		}
	}
	return true;
}

bool Controller::handleMovement(const Monster& obj)
{
	if (!validMovement(obj))
		return false;

	for (auto& edible : m_edibleVec)
	{
		if (typeid(*edible) == typeid(Stone) && obj.intersects(*edible))
			return false;
	}

	return true;
}

bool Controller::validMovement(const Object& obj)
{

	for (auto& wall : m_wallVec)
	{
		if (obj.intersects(*wall))
		{
			return false;
		}
	}

	float height = obj.getRec().height;
	float width = obj.getRec().width;

	sf::Vector2f position = obj.getPosition();
	sf::Vector2f position2 = { position.x + width, position.y };
	sf::Vector2f position3 = { position.x, position.y + height };
	sf::Vector2f position4 = { position.x + width, position.y + height };

	if (!m_boardRect.getGlobalBounds().contains(position))
		return false;
	if (!m_boardRect.getGlobalBounds().contains(position2))
		return false;
	if (!m_boardRect.getGlobalBounds().contains(position3))
		return false;
	if (!m_boardRect.getGlobalBounds().contains(position4))
		return false;

	return true;
}

void Controller::endGameAnnouncement(std::string imaje)
{
	auto window = sf::RenderWindow(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "End Game");

	auto texture = sf::Texture() ;
	texture.loadFromFile(imaje);

	auto sprite = sf::Sprite(texture);
	sprite.setPosition(400, 0);
	sprite.scale({ 2.f, 2.f });

	// Handle events
	sf::Event event;
	while (window.waitEvent(event))
	{
		switch (event.type)
		{
		case sf::Event::Closed:
			window.close();
			return;
		
		case sf::Event::KeyReleased:
			switch (event.key.code)
			{
			case sf::Keyboard::Escape:
				window.close();
				return;
			}
		}
	

		window.clear(RECTANGLE_COLOR_LIGHT);
		window.draw(sprite);
		window.display();
	}
}

void Controller::updateStats()
{
	std::ifstream ifs;
	std::string line;
	std::string statsArr[STATS_ARRAY];

	ifs.open(FILE_STATISTIC);
	if (!ifs.is_open()) {
		std::cout << "error file is not open 2!!";
	}

	for (size_t i = 0; i < STATS_ARRAY; i++)
	{
		std::getline(ifs, line);
		statsArr[i] = line;
	}
	ifs.close();

	//update stats vec 
	for (size_t i = 0; i < STATS_ARRAY; i++)
	{
		int score;
		int level;
		auto istr = std::istringstream(statsArr[i]);
		istr >> level >> score;

		if (m_data.getScore() > score)
		{
			for (size_t j = STATS_ARRAY - 1; j > i; j--)
			{
				statsArr[j] = statsArr[j-1];
			}
			statsArr[i] = std::to_string(m_data.getLevel()) + " " + std::to_string(m_data.getScore());
			break;
		}
	}

// wright to file

	std::ofstream ofs;

	ofs.open(FILE_STATISTIC);
	if (!ofs.is_open()) {
		std::cout << "error file is not open 3!!";
	}

	for (size_t i = 0; i < STATS_ARRAY; i++)
	{
		ofs << statsArr[i] << std::endl;
	}

	ofs.close();
}

void Controller::looseAffects()
{
	sf::Clock clock;
	sf::Sound looser(Resources::instance().getLooserBuffer());
	looser.play();

	draw();

	while (clock.getElapsedTime().asSeconds() < 1.4f)
	{
		// Handle events
		sf::Event event;
		if (m_window.pollEvent(event))
		{
			switch (event.type)
			{
			case sf::Event::Closed:
				m_window.close();
				return;

			case sf::Event::KeyReleased:
				switch (event.key.code)
				{
				case sf::Keyboard::Escape:
					m_window.close();
					return;
				}
			}
		}

	}
}

void Controller::endLevelAffects()
{
	
	sf::Clock clock;
	sf::Sound totach(Resources::instance().getTotachBuffer());
	totach.play();

	draw();

	while (clock.getElapsedTime().asSeconds() < 2.5f)
	{
		// Handle events
		sf::Event event;
		if (m_window.pollEvent(event))
		{
			switch (event.type)
			{
			case sf::Event::Closed:
				m_window.close();
				return;

			case sf::Event::KeyReleased:
				switch (event.key.code)
				{
				case sf::Keyboard::Escape:
					m_window.close();
					return;
				}
			}
		}

	}

}

void Controller::startLevelAffects()
{
	m_data.pauseOrPlayClock();

	sf::Text level;
	sf::Font font;
	sf::Clock clock;
	sf::Sound start(Resources::instance().getStartBuffer());
	start.play();

	font.loadFromFile("C:/Windows/Fonts/georgiaz.ttf");

	level = sf::Text("", font);
	level.setPosition(sf::Vector2f(470, 250));
	level.setFillColor(sf::Color::Black);
	level.setCharacterSize(180);
	level.setString("Level    " + std::to_string(m_data.getLevel()));

	drawWithoutDisplay();
	m_window.draw(level);
	m_window.display();

	while(clock.getElapsedTime().asSeconds() < 2.0f)
	{
		// Handle events
		sf::Event event;
		if (m_window.pollEvent(event))
		{
			switch (event.type)
			{
			case sf::Event::Closed:
				m_window.close();
				return;

			case sf::Event::KeyReleased:
				switch (event.key.code)
				{
				case sf::Keyboard::Escape:
					m_window.close();
					return;
				}
			}
		}

	}

	m_data.pauseOrPlayClock();
}
