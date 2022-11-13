#ifndef PUZZLE_H
#define PUZZLE_H

#include <string>
#include <fstream>

class CubeComponent;
enum class Face;

struct CubeData
{
	bool exists;
	std::string textureName;
};

struct PuzzleData
{
	static const unsigned int width = 10;
	static const unsigned int height = 10;
	static const unsigned int depth = 10;

	CubeData data[width][height][depth];

	PuzzleData(std::string filepath)
	{
		/*----------------------------------------*/
		/* Pre-Loading the Data                   */
		/*----------------------------------------*/

		for (int x = 0; x < width; x++)
		{
			for (int y = 0; y < height; y++)
			{
				for (int z = 0; z < depth; z++)
				{
					data[x][y][z] = { false, "" };
				}
			}
		}

		/*----------------------------------------*/
		/* Reading the File                       */
		/*----------------------------------------*/

		std::ifstream file;
		file.open(filepath.c_str());

		if (!file)
		{
			std::cout << "Unable to open the indicated file." << std::endl;
			return;
		}

		std::string data;
		while (std::getline(file, data))
		{
			std::cout << data << std::endl;
		}

		file.close();

		/*----------------------------------------*/
		/* Loading the Data                       */
		/*----------------------------------------*/

		/*
			Each puzzle file will be, for now, stored in a plaintext file
			which will feature some amount of data about each cube that
			should appear in the puzzle. At the beginning of each file, we'll
			include some header information including a string indicating the
			name of the puzzle, delineated by "::" from the move limit of the
			puzzle, delineated by "::" from three integers indicating the coord-
			-inates of the cube marking the goal of the puzzle, delinated by "::"
			from an integer indicating the face of that cube which will act as the
			final goal of the puzzle. Then, we'll delineate this header from the
			cube data via a "//".
			Following this will be a variable amount of data indicating the position
			and textures of the various cubes that appear in the puzzle. These clusters
			of data will only describe cubes that are filled, not empty-space cubes.

				- 3 integers (x, y, & z) describing the cube's location.
				- 1 
		*/
	}
};

struct Puzzle
{
	CubeComponent*	cubeGoal;		// What cube does the player have to stand on to complete the puzzle?
	Face			faceGoal;		// What face of the cube does the player have to stand on to complete the puzzle?

	unsigned int	moveLimit;		// How many moves can the player make before the puzzle resets?

	PuzzleData		data;			// The data about how the puzzle should be layed out.

	/*Puzzle(unsigned int moveLimit, PuzzleData data)
	{
		this->moveLimit = moveLimit;

		this->data = data;


	}*/
};

#endif