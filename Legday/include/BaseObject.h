#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "BaseComponent.h"
#include "Transform.h"

#define COMP_N 10

using namespace std;

class BaseObject
{
	unsigned int oid;

public:
	vector<BaseComponent*> components;
	Transform transform;
	
	BaseObject(unsigned int oid) :
		oid(oid),
		transform(Transform()),
		components(*(new vector<BaseComponent*>))
	{}

	~BaseObject(){
		deleteObj();
	}
	
	virtual void init() {
		std::cout << "no definition for 'init' in derived object" << oid << "\n";
	}

	virtual void run() {
		std::cout << "no definition for 'run' in derived object" << oid << "\n";
	}

	void initComponents() {
		for (BaseComponent* comp : components) {
			comp->initComponent();
		}
	}

	void runComponents() {
		for (BaseComponent* comp : components) {
			comp->execute();
		}
	}

	unsigned int getoid() { return oid; }
	
	Transform getTransform() { return transform; }
	vector<BaseComponent*> getComponents() { return components; }
	
	void addComponent(BaseComponent* component) {
		components.push_back(component);
	}

	void removeComponent(unsigned int cid) {
		int found = -1;
		for (int i = 0; i < components.size(); i++) {
			if (components[i]->getcid() == cid) {
				found = i;
				break;
			}
		}
		if (found != -1) {
			free(components[found]);
			components.erase(components.begin() + found);
		}
	}

	void deleteObj() {
		for (BaseComponent* comp : components) {
			comp->endComponent();
			free(comp);
		}
	}
};

