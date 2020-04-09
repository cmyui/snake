#include <iostream>
#include <Windows.h>
#include <cinttypes>
#include <chrono>
#include <vector>
#include <bitset>

using namespace std;
using namespace std::chrono;

// TODO: actually calc these
#define MIN_WIDTH  66
#define MIN_HEIGHT 40
#define M_PI 3.14159265358979323846
#define clr(s, w, h) for (int i = 0; i < w * h; s[i] = ' ', i++)

namespace cmyui {
	const double horizontalCompensation = 1.7751937984496124031007751937984;

	struct Resolution {
		int nWidth, nHeight;
		Resolution(int width = 0, int height = 0) { setResolution(width, height); }
		void setResolution(int width, int height) {
			nWidth = max(width, MIN_WIDTH);
			nHeight = max(height, MIN_HEIGHT);
		}

		auto getResolution() const -> pair<int, int> {
			return make_pair(nWidth, nHeight);
		}
	};

	class Timer {
		system_clock::time_point t1, t2;
		float _fElapsedTime;
	public:
		Timer() : t1(system_clock::now()), t2(system_clock::now()), _fElapsedTime(0.0f) {}

		void tick() {
			t1 = system_clock::now();
			_fElapsedTime = duration<float>(t1 - t2).count();
			t2 = t1;
		}

		float time() const { return _fElapsedTime; }
	};

	template<typename T>
	struct Entity {
		T x, y;

		Entity(T _x = 0, T _y = 0) : x(_x), y(_y) {}

		auto getTile() const -> pair<int, int> {
			return make_pair((int)x, (int)y);
		}

		int getScreenPos(int width) const {
			return ((int)y * width) + (int)x;
		}

		void randomizePos(int x_cap, int y_cap) {
			x = 1 + (rand() % x_cap - 1);
			y = 1 + (rand() % y_cap - 1);
		}

		void setPos(pair<int, int> pos) {
			x = pos.first, y = pos.second;
		}

		void zero() { x = 0, y = 0; }
	};

	struct User : public Entity<float> {
		int nTailLength;
		bitset<2> direction;
		vector<pair<int, int>> tail;

		User(float _x = 0.0f, float _y = 0.0f, int _tailLength = 0) : Entity(_x, _y), nTailLength(_tailLength) {
			direction.reset(); // set to 0x00 (north)
		}
	};

	struct Fruit : public Entity<int> {
		Fruit(int _x = 0, int _y = 0) : Entity(_x, _y) {}

		operator bool() const { return x && y; }
		bool operator!() const { return !bool(*this); }
	};

	template<typename T>
	T getT() {
		T val = 0;
		for (;;) {
			cin >> val;
			cin.ignore(0x400, '\n');

			if (!cin) {
				cin.clear();
				cout << "Invalid Integer, try again: ";
				continue;
			} else break;
		}
		return val;
	}

	template<typename T>
	T getTInRange(T min, T max) {
		T val = 0;
		for (;;) {
			val = getT<T>();
			if (val < min || val > max) {
				cout << "Invalid selection, try again: ";
				continue;
			} else break;
		}
		return val;
	}

	template<typename T>
	bool inRange(T val, T low, T high) {
		return ((val - low) <= (high - low));
	}
}

int main() {
	cout << "Welcome to the pain train" << endl;
	cout << "Pain lv: "; float fSpeed = cmyui::getT<float>();

	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	SetConsoleTitleA("cmyui's fuck off snake");

	CONSOLE_SCREEN_BUFFER_INFO consoleInfo;

	for (;;) {
		GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
		cmyui::Resolution res(consoleInfo.dwSize.X, consoleInfo.dwSize.Y);
		cmyui::Timer timer = cmyui::Timer();

		char* screen = new char[res.nWidth * res.nHeight];
		clr(screen, res.nWidth, res.nHeight);

		DWORD dwBytesWritten = 0;
		bool gameOver = false;

		/* Player & fruit stuff */
		cmyui::User u(res.nWidth / 2.0f, res.nHeight / 2.0f, 0);
		cmyui::Fruit f(0, 0);

		for (;;) {
			if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
				break; // Esc to exit safely

			/* Get our current resolution */
			GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
			if (consoleInfo.dwSize.X != res.nWidth || consoleInfo.dwSize.Y != res.nHeight) {
				res.setResolution(consoleInfo.dwSize.X, consoleInfo.dwSize.Y);
				delete[] screen; // this is going to be DISGUSTING if we don't add a cooldown, it will rapidly rewrite memory EACH fucking frame
				screen = new char[res.nWidth * res.nHeight];
			}

			timer.tick();

			/* Move player (north and west are unintuitively backwards since we're printing from top left) */
			if (!u.direction.test(1) && !u.direction.test(0)) /* North */ u.y -= (fSpeed * timer.time());
			if (!u.direction.test(1) &&  u.direction.test(0)) /* West  */ u.x -= (fSpeed * timer.time()) * cmyui::horizontalCompensation;
			if ( u.direction.test(1) && !u.direction.test(0)) /* South */ u.y += (fSpeed * timer.time());
			if ( u.direction.test(1) &&  u.direction.test(0)) /* East  */ u.x += (fSpeed * timer.time()) * cmyui::horizontalCompensation;

			if (u.x > res.nWidth) u.x = 0.0f;
			if (u.x < 0.0f) u.x = res.nWidth;
			if (u.y > res.nHeight) u.y = 0.0f;
			if (u.y < 0.0f) u.y = res.nHeight;

			/* Handle W/A/S/D to change direction [ TODO: test without extra () ] */
			//if (u.direction.any()) u.direction.reset(); // 00 (NORTH)
			if ((GetAsyncKeyState((short)'W') & 0x8000) && !(!u.direction.test(1) && !u.direction.test(0))) {
				if ( u.direction.test(1)) u.direction.set(1, 0);
				if ( u.direction.test(0)) u.direction.set(0, 0);
			}
			if ((GetAsyncKeyState((short)'A') & 0x8000) && !(!u.direction.test(1) &&  u.direction.test(0))) {
				if ( u.direction.test(1)) u.direction.set(1, 0);
				if (!u.direction.test(0)) u.direction.set(0, 1);
			}
			if ((GetAsyncKeyState((short)'S') & 0x8000) && !( u.direction.test(1) && !u.direction.test(0))) {
				if (!u.direction.test(1)) u.direction.set(1, 1);
				if ( u.direction.test(0)) u.direction.set(0, 0);
			}
			if ((GetAsyncKeyState((short)'D') & 0x8000) && !( u.direction.test(1) &&  u.direction.test(0))) {
				if (!u.direction.test(1)) u.direction.set(1, 1);
				if (!u.direction.test(0)) u.direction.set(0, 1);
			}

			/* Generate fruit location if empty */
			if (!f) f.randomizePos(res.nWidth, res.nHeight);

			clr(screen, res.nWidth, res.nHeight);

			/* Check if player is on fruit */
			if (u.getTile() == f.getTile()) {
				f.zero();
				u.nTailLength++;
			} else screen[f.getScreenPos(res.nWidth)] = 'D'; // Draw fruit

			// Draw player
			screen[u.getScreenPos(res.nWidth)] = 'X';

			if (u.tail.empty() || (u.nTailLength && u.tail[0] != u.getTile())) {
				u.tail.insert(u.tail.begin(), make_pair((int)u.x, (int)u.y));
				if (!u.tail.empty() && u.tail.size() - 1 > u.nTailLength)
					u.tail.pop_back();
			}

			for (vector<pair<int, int>>::iterator it = u.tail.begin() + 1; it != u.tail.end(); ++it) {
				if (*it == u.getTile()) gameOver = true;
				screen[(it->second * res.nWidth) + it->first] = gameOver ? '?' : 'x';
			}

			if (gameOver) { // spaghetti
				snprintf(screen, 30, "Game over - Press R to retry.");
				WriteConsoleOutputCharacterA(hConsole, screen, res.nWidth * res.nHeight, { 0, 0 }, &dwBytesWritten);
				break;
			}

			snprintf(screen, 51, "FPS: %4.2f | X: %3.2f | Y: %3.2f | Score: %4d", min(1.0f / timer.time(), 9999.99f), u.x, u.y, u.nTailLength * 10); // FPS counter
			snprintf(screen + (res.nWidth - 15), 15, "Res: %4dx%4d", res.nWidth, res.nHeight);
			snprintf(screen + ((res.nWidth * 2) - 9), 9, "Dir: %1d:%1d", u.direction.test(1), u.direction.test(0));
			WriteConsoleOutputCharacterA(hConsole, screen, res.nWidth * res.nHeight, { 0, 0 }, &dwBytesWritten);
		}

		delete[] screen;
		if (gameOver) while (!(GetAsyncKeyState((short)'R') & 0x8000));
		else break;
	}
	return 0;
}
