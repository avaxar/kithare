![Kithare](assets/banner.png) <br/>
![State](https://img.shields.io/badge/state-unfinished-ff2222.svg)
![Version](https://img.shields.io/badge/version-0.0.0-00ffaa.svg)

# Kithare
 An open source general purpose statically-typed cross-platform interpreted Python and C like programming language.

## Community / Contact
- [Discord server](https://discord.gg/hXvY8CzS7A)

## Building
- Kithare uses a python helper script to make building easier. So in order to
build Kithare, you need Python 3.6 or above installed
- For advanced usage, you may checkout the "build.py" file, which contains instructions on how to
use the build script to achieve more things
- A basic HOWTO to building Kithare is given below

### Windows
#### MSVC / Visual Studio
- Make sure you have Visual Studio 2019 with C++ build tools and Windows 10 SDK installed.
- Run `python3 build.py --msvc-deps`. It'll download the dependencies such as SDL and create the build destination directory with the dependencies' DLLs copied.
- Open the solution at `msvc/Kithare.sln`.
- See `Kithare` in the Solution Explorer. If the name's not bolded, right click and click "Set as Startup Project".
- You can now build it by clicking "Local Windows Debugger".

#### MinGW
- Make sure you have MinGW (aka MinGW-w64) installed, and its `bin` directory be put in 
the PATH.
- Run `python3 build.py`. It will automatically download the dependencies such as SDL and 
compile Kithare.

### Other platforms
- Make sure you have the GCC compiler.
- Install the development libraries for these: `SDL2`, `SDL2_mixer`, `SDL2_image`, `SDL2_ttf`, `SDL2_net`. You may use your disto's package manager to do this.
- A recommended way to do this on Mac is to use Homebrew. Just run
`brew install gcc sdl2 sdl2_image sdl2_mixer sdl2_net sdl2_ttf`
- Run `python3 build.py`.

## Kithare example programs
 A simple hello world
```py
import std;


def main() {
	std.println("Hello, world!");
}
```
 FizzBuzz
```py
import std;


def main() {
    for int i = 0, i < 1500, i += 1 {
        str string = "";
        if i % 3 == 0 {
            string += "Fizz";
        }
        if i % 5 == 0 {
            string += "Buzz";
        }
        if i % 3 != 0 && i % 5 != 0 {
            string += str(i);
        }
        std.println(string);
    }
}
```
 A simple calculator
```py
import std;

def main() {
	while true {
		int lvalue = int(std.input("Enter your first number: "));
		int rvalue = int(std.input("Enter your second number: "));
		str op = std.input("Enter your operator (+, -, *, /): ");

		if op == "+" {
			std.println("Your result is: " + str(lvalue + rvalue));
		}
		elif op == "-" {
			std.println("Your result is: " + str(lvalue - rvalue));
		}
		elif op == "*" {
			std.println("Your result is: " + str(lvalue * rvalue));
		}
		elif op == "/" {
			std.println("Your result is: " + str(lvalue / rvalue));
		}
		else {
			std.println("Invalid operator!");
		}
	}
}
```
 A simple calculator, but with lambdas
```py
import std;

def main() {
	while true {
		int lvalue = int(std.input("Enter your first number: "));
		int rvalue = int(std.input("Enter your second number: "));
		str op = std.input("Enter your operator (+, -, *, /): ");
		func!(int(int, int)) operation;

		if op == "+" {
			operation = def &(int a, int b) -> int {
				return a + b;
			};
		}
		elif op == "-" {
			operation = def &(int a, int b) -> int {
				return a - b;
			};
		}
		elif op == "*" {
			operation = def &(int a, int b) -> int {
				return a * b;
			};
		}
		elif op == "/" {
			operation = def &(int a, int b) -> int {
				return a / b;
			};
		}
		else {
			std.println("Invalid operator!");
			continue;
		}

		std.println("Your result is: " + str(operation(lvalue, rvalue)));
	}
}
```
 OOP, with operator overloading and templating
```py
class Vector2!T {
	T x;
	T y;

	/* Constructor */
	def new(T x, T y) {
		self.x = x;
		self.y = y;
	}

	/* Destructor */
	def delete() {}

	/* Overloading the + operator */
	def add(Vector2 other) -> Vector2 {
		return Vector2(self.x + other.x, self.y + other.y);
	}
}
```
 OOP, with polymorphism
```py
import std;

/* Inheriting `object` would mark it as polymorphic, making all of its methods virtual */
class Entity(object) {
	int age;

	def sayAge() {
		std.println("This entity's age is " + str(self.age));
	}
}

class Monke(Entity) {
	int iq;

	def sayAge() {
		std.println("This monke's age is " + str(self.age));
	}
}

class Human(Monke) {
	str name;

	def new(int age, int iq, str name) {
		self.age = age;
		self.iq = iq;
		self.name = name;
	}

	def sayAge() {
		std.println("This human's age is " + str(self.age));
	}
}

class Axolotl(Entity) {
	ubyte image_face[4][256][256];
}

def main() {
	Entity entity = Human(-1000, 1, "Avaxar");
	entity.sayAge();
	if castable!Human(entity) {
		/* `ref` handles the object by reference */
		ref Human human = cast!Human(entity);
		std.println("The human's name is " + human.name);
	}
	else {
		std.println("Not human");
	}
}
```
 Array maths
```py
import std;

def main() {
	float position[3] = (4, 5, 0);
	float an_object[3] = (2, 1, 1);
	float relative_position_of_object[3] = an_object - position;
	float dot_product = dot(position, an_object);
	float scaled_position = position * 2.0;
}
```
