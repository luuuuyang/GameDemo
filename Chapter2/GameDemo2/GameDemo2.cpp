#include "GameLib/Framework.h"
#include <algorithm>
#include <fstream>

void readFile(char** buffer, int* size, const char* filename);
void mainLoop();

template<typename T> 
class Array2D
{
public:
	Array2D() : mArray(nullptr), mSize0(0), mSize1(0) {}
	~Array2D()
	{
		delete[] mArray;
		mArray = nullptr;
	}
	void setSize(int size0, int size1)
	{
		mSize0 = size0;
		mSize1 = size1;
		mArray = new T[size0 * size1];
	}
	T& operator()(int index0, int index1)
	{
		return mArray[index1 * mSize0 + index0];
	}
	const T& operator()(int index0, int index1) const
	{
		return mArray[index1 * mSize0 + index0];
	}
private:
	T* mArray;
	int mSize0;
	int mSize1;
};

class State
{
public:
	State(const char* stageData, int size);
	void update(char input);
	void draw() const;
	bool hasCleared() const;
private:
	enum Object
	{
		OBJ_SPACE,
		OBJ_WALL,
		OBJ_BLOCK,
		OBJ_MAN,

		OBJ_UNKNOWN,
	};
	void setSize(const char* stageData, int size);

	void drawCell(int x, int y, unsigned color) const;

	int mWidth;
	int mHeight;
	Array2D<Object> mObjects;
	Array2D<bool> mGoalFlags;
};

State* gState = nullptr;

namespace GameLib {
	void Framework::update() {
		mainLoop();
	}
}

void mainLoop() {
	if (!gState) {
		const char* filename = "gameBoardChar.txt";
		char* stageData;
		int fileSize;
		readFile(&stageData, &fileSize, filename);

		if (!stageData) {
			GameLib::cout << "stage file could not be read." << GameLib::endl;
			return;
		}

		gState = new State(stageData, fileSize);

		delete[] stageData;
		stageData = nullptr;

		gState->draw();
		return;
	}

	bool cleared = false;
	if (gState->hasCleared()) {
		cleared = true;
	}

	GameLib::cout << "a:left s:right w:up z:down. command?" << GameLib::endl;
	char input;
	GameLib::cin >> input;
	gState->update(input);

	gState->draw();

	if (cleared) {
		GameLib::cout << "Congratulation! you win." << GameLib::endl;
		delete gState;
		gState = nullptr;
	}

	if (input == 'q')
	{
		GameLib::Framework::instance().requestEnd();
	}

	if (GameLib::Framework::instance().isEndRequested())
	{
		if (gState)
		{
			delete gState;
			gState = nullptr;
			return;
		}
	}
}

void readFile(char** buffer, int* size, const char* filename)
{
	std::ifstream in(filename, std::ifstream::binary);
	if (!in)
	{
		*buffer = 0;
		*size = 0;
	}
	else
	{
		in.seekg(0, std::ifstream::end);
		*size = static_cast<int>(in.tellg());
		in.seekg(0, std::ifstream::beg);
		*buffer = new char[*size];
		in.read(*buffer, *size);
	}
}

State::State(const char* stageData, int size)
{
	setSize(stageData, size);
	mObjects.setSize(mWidth, mHeight);
	mGoalFlags.setSize(mWidth, mHeight);
	for (int y = 0; y < mHeight; ++y)
	{
		for (int x = 0; x < mWidth; ++x)
		{
			mObjects(x, y) = OBJ_WALL;
			mGoalFlags(x, y) = false;
		}
	}
	int x = 0;
	int y = 0;
	for (int i = 0; i < size; ++i)
	{
		Object t;
		bool goalFlag = false;
		switch (stageData[i])
		{
		case '#': t = OBJ_WALL; break;
		case ' ': t = OBJ_SPACE; break;
		case 'o': t = OBJ_BLOCK; break;
		case 'O': t = OBJ_BLOCK; goalFlag = true; break;
		case '.': t = OBJ_SPACE; goalFlag = true; break;
		case 'p': t = OBJ_MAN; break;
		case 'P': t = OBJ_MAN; goalFlag = true; break;
		case '\n': x = 0; ++y; t = OBJ_UNKNOWN; break;
		default: t = OBJ_UNKNOWN; break;
		}
		if (t != OBJ_UNKNOWN)
		{
			mObjects(x, y) = t;
			mGoalFlags(x, y) = goalFlag;
			++x;
		}
	}
}

void State::setSize(const char* stageData, int size)
{
	mWidth = mHeight = 0;
	int x = 0;
	int y = 0;
	for (int i = 0; i < size; ++i) {
		switch (stageData[i]) {
		case '#': case ' ': case 'o': case 'O':
		case '.': case 'p': case 'P':
			++x;
			break;
		case '\n':
			++y;
			mWidth = std::max(mWidth, x);
			mHeight = std:: max(mHeight, y);
			x = 0;
			break;
		}
	}
}

void State::drawCell(int x, int y, unsigned color) const
{
	unsigned* vram = GameLib::Framework::instance().videoMemory();
	int windowWidth = GameLib::Framework::instance().width();

	for (int i = 0; i < 16; i++)
	{
		for (int j = 0; j < 16; j++)
		{
			vram[(y * 16 + i) * windowWidth + (x * 16 + j)] = color;
		}
	}
}

void State::draw() const
{
	
	for (int y = 0; y < mHeight; ++y)
	{
		for (int x = 0; x < mWidth; ++x)
		{
			Object o = mObjects(x, y);
			bool goalFlag = mGoalFlags(x, y);
			unsigned color;
			if (goalFlag)
			{
				switch (o)
				{
				case OBJ_SPACE: GameLib::cout << '.'; color = 0x0000ff; break;
				case OBJ_WALL: GameLib::cout << '#'; color = 0xffffff; break;
				case OBJ_BLOCK: GameLib::cout << 'O'; color = 0xff00ff; break;
				case OBJ_MAN: GameLib::cout << 'P'; color = 0x00ffff; break;
				}
			}
			else
			{
				switch (o)
				{
				case OBJ_SPACE: GameLib::cout << ' '; color = 0x000000; break;
				case OBJ_WALL: GameLib::cout << '#'; color = 0xffffff; break;
				case OBJ_BLOCK: GameLib::cout << 'o'; color = 0xff0000; break;
				case OBJ_MAN: GameLib::cout << 'p'; color = 0x00ff00; break;
				}
			}
			drawCell(x, y, color);
		}
		GameLib::cout << GameLib::endl;
	}
}

void State::update(char input)
{
	int dx = 0;
	int dy = 0;
	switch (input)
	{
	case 'a': dx = -1; break;
	case 's': dx = 1; break;
	case 'w': dy = -1; break;
	case 'z': dy = 1; break;
	}
	int w = mWidth;
	int h = mHeight;
	Array2D< Object >& o = mObjects;
	int x, y;
	x = y = -1;
	bool found = false;
	for (y = 0; y < mHeight; ++y)
	{
		for (x = 0; x < mWidth; ++x)
		{
			if (o(x, y) == OBJ_MAN)
			{
				found = true;
				break;
			}
		}
		if (found)
		{
			break;
		}
	}
	int tx = x + dx;
	int ty = y + dy;
	if (tx < 0 || ty < 0 || tx >= w || ty >= h) {
		return;
	}
	if (o(tx, ty) == OBJ_SPACE)
	{
		o(tx, ty) = OBJ_MAN;
		o(x, y) = OBJ_SPACE;
	}
	else if (o(tx, ty) == OBJ_BLOCK)
	{
		int tx2 = tx + dx;
		int ty2 = ty + dy;
		if (tx2 < 0 || ty2 < 0 || tx2 >= w || ty2 >= h)
		{
			return;
		}
		if (o(tx2, ty2) == OBJ_SPACE)
		{
			o(tx2, ty2) = OBJ_BLOCK;
			o(tx, ty) = OBJ_MAN;
			o(x, y) = OBJ_SPACE;
		}
	}
}

bool State::hasCleared() const
{
	for (int y = 0; y < mHeight; ++y)
	{
		for (int x = 0; x < mWidth; ++x)
		{
			if (mObjects(x, y) == OBJ_BLOCK && mGoalFlags(x, y) == false)
			{
				return false;
			}
		}
	}
	return true;
}
