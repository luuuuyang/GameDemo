#include <iostream>
#include <fstream>
#include <algorithm> // max

void readFile(char** gameBoardChar, int* size, const char* filename);

template<typename T>
class Array2D
{
public:
	Array2D() : mArray2D(0), mWidth(0), mHeight(0) {}
	~Array2D()
	{
		delete[] mArray2D;
		mArray2D = nullptr;
	}
	void setSize(int width, int height)
	{
		mWidth = width;
		mHeight = height;
		mArray2D = new T[mWidth * mHeight];
	}
	T& operator()(int x, int y)
	{
		return mArray2D[y * mWidth + x];
	}
	const T& operator()(int x, int y) const
	{
		return mArray2D[y * mWidth + x];
	}
private:
	T* mArray2D;
	int mWidth;
	int mHeight;
};

class State
{
public:
	State(char* gameBoardChar, int size);
	void draw();
	void update(const char input);
	bool endGame();
private:
	enum Object
	{
		OBJ_WALL,
		OBJ_SPACE,
		OBJ_BOX,
		OBJ_PEOPLE,
		OBJ_UNKNOWN,
		OBJ_GOAL = (1 << 7)
	};

	void setSize(char* gameBoardChar, int size);

	int mWidth;
	int mHeight;
	Array2D<unsigned char> mObjects;
};

int main()
{
	const char* filename = "gameBoardChar.txt";
	char* gameBoardChar;
	int fileSize;

	readFile(&gameBoardChar, &fileSize, filename);

	State* state = new State(gameBoardChar, fileSize);

	while (true)
	{
		state->draw();

		if (state->endGame())
		{
			break;
		}

		char input;
		std::cout << "w:up s:down a:left d:right. command?\n";
		std::cin >> input;

		state->update(input);
	}

	std::cout << "You win! Congratulations!\n";
	std::cout << "Press any key to exit.";

	delete[] gameBoardChar;
	gameBoardChar = nullptr;

	std::cin.get();
	return 0;
}

void readFile(char** gameBoardChar, int* fileSize, const char* filename)
{
	std::ifstream in(filename, std::ifstream::binary);
	if (!in)
	{
		*gameBoardChar = nullptr;
		*fileSize = 0;
	}
	else
	{
		in.seekg(0, std::ifstream::end);
		*fileSize = static_cast<int>(in.tellg());
		in.seekg(0, std::ifstream::beg);
		*gameBoardChar = new char[*fileSize];
		in.read(*gameBoardChar, *fileSize);
		in.close();
	}
}

State::State(char* gameBoardChar, int size)
{
	setSize(gameBoardChar, size);
	mObjects.setSize(mWidth, mHeight);

	for (auto y = 0; y < mHeight; y++)
	{
		for (auto x = 0; x < mWidth; x++)
		{
			mObjects(x, y) = OBJ_WALL;
		}
	}

	auto x = 0, y = 0;
	for (auto i = 0; i < size; i++)
	{
		unsigned char o;

		switch (gameBoardChar[i])
		{
		case '#': o = OBJ_WALL; break;
		case ' ': o = OBJ_SPACE; break;
		case '.': o = OBJ_SPACE | OBJ_GOAL; break;
		case 'o': o = OBJ_BOX; break;
		case 'p': o = OBJ_PEOPLE; break;
		case 'O': o = OBJ_BOX | OBJ_GOAL; break;
		case 'P': o = OBJ_PEOPLE | OBJ_GOAL; break;
		case '\n': o = OBJ_UNKNOWN; x = 0; y++; break;
		default: o = OBJ_UNKNOWN; break;
		}

		if (o != OBJ_UNKNOWN)
		{
			mObjects(x, y) = o;
			x++;
		}
	}
}

void State::setSize(char* gameBoardChar, int size)
{
	auto x = 0, y = 0;
	for (auto i = 0; i < size; i++)
	{
		switch (gameBoardChar[i])
		{
		case '#': case ' ': case '.': case 'o':
		case 'p': case 'O': case 'P':
			x++; break;
		case '\n':
			mWidth = std::max(mWidth, x);
			x = 0; y++;
			mHeight = std::max(mHeight, y);
			break;
		}
	}
}

void State::draw()
{
	system("cls");

	for (int y = 0; y < mHeight; y++)
	{
		for (int x = 0; x < mWidth; x++)
		{
			auto o = mObjects(x, y);

			switch (o)
			{
			case OBJ_SPACE: std::cout << ' '; break;
			case OBJ_WALL: std::cout << '#'; break;
			case OBJ_BOX: std::cout << 'o'; break;
			case OBJ_PEOPLE: std::cout << 'p'; break;
			case (OBJ_SPACE | OBJ_GOAL): std::cout << '.'; break;
			case (OBJ_BOX | OBJ_GOAL): std::cout << 'O'; break;
			case (OBJ_PEOPLE | OBJ_GOAL): std::cout << 'P'; break;
			}
		}
		std::cout << std::endl;
	}
}

void State::update(const char input)
{
	auto dx = 0, dy = 0;
	switch (input)
	{
	case 'w': dy = -1; break;
	case 's': dy = 1; break;
	case 'a': dx = -1; break;
	case 'd': dx = 1; break;
	}

	const int& width = mWidth;
	const int& height = mHeight;

	int px = -1, py = -1;
	for (py = 0; py < height; py++)
	{
		auto foundPeople = false;
		for (px = 0; px < width; px++)
		{
			if ((mObjects(px, py) & ~OBJ_GOAL) == OBJ_PEOPLE)
			{
				foundPeople = true;
				break;
			}
		}
		if (foundPeople)
		{
			break;
		}
	}

	const auto tx1 = px + dx;
	const auto ty1 = py + dy;
	if (tx1 < 0 || ty1 < 0 || tx1 >= width || ty1 >= height)
	{
		return;
	}

	if ((mObjects(tx1, ty1) & ~OBJ_GOAL) == OBJ_SPACE)
	{
		mObjects(tx1, ty1) = mObjects(tx1, ty1) & OBJ_GOAL | OBJ_PEOPLE;
		mObjects(px, py) = mObjects(px, py) & OBJ_GOAL | OBJ_SPACE;
	}
	else if ((mObjects(tx1, ty1) & ~OBJ_GOAL) == OBJ_BOX)
	{
		const auto tx2 = tx1 + dx;
		const auto ty2 = ty1 + dy;
		if (tx2 < 0 || ty2 < 0 || tx2 >= width || ty2 >= height)
		{
			return;
		}

		if ((mObjects(tx2, ty2) & ~OBJ_GOAL) == OBJ_SPACE)
		{
			mObjects(tx2, ty2) = mObjects(tx2, ty2) & OBJ_GOAL | OBJ_BOX;
			mObjects(tx1, ty1) = mObjects(tx1, ty1) & OBJ_GOAL | OBJ_PEOPLE;
			mObjects(px, py) = mObjects(px, py) & OBJ_GOAL | OBJ_SPACE;
		}
	}
}

bool State::endGame()
{
	for (auto y = 0; y < mHeight; y++)
	{
		for (auto x = 0; x < mWidth; x++)
		{
			if (mObjects(x, y) == OBJ_BOX)
			{
				return false;
			}
		}
	}
	return true;
}
