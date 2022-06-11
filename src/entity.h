#ifndef ENTITY_H
#define ENTITY_H

#include <vector>
#include <unordered_map>

class Component;

class Entity
{
private:
	int ID;
	int scene;
	std::string name;

public:
	std::unordered_map<int, Component*> componentIDMap;
	std::vector<Component*> components;

	int				GetID();
	int				GetScene();
	std::string		GetName();

	void			SetID(int ID);
	void			SetScene(int scene);
	void			SetName(std::string name);

	Entity(int ID, int scene, std::string name);
};

#endif