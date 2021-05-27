#ifndef GUI_H
#define GUI_H

#include "GUIBuilder.h"

#include <vector>

class Window;

class GUI {
public:
	GUI(Window &window);
	~GUI();

	void update();
	void draw();

	void hide();
	void show();

	void subscribe(GUIBuilder* builder);
private:
	std::vector<GUIBuilder*> builders;

	bool showGUI = true;
};

#endif // !GUI_H
