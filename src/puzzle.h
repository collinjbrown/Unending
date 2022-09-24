#ifndef PUZZLE_H
#define PUZZLE_H

class CubeComponent;
enum class Face;

struct Puzzle
{
	CubeComponent	cubeGoal;		// What cube does the player have to stand on to complete the puzzle?
	Face			faceGoal;		// What face of the cube does the player have to stand on to complete the puzzle?

	int				moveLimit;		// How many moves can the player make before the puzzle resets?				
};

#endif