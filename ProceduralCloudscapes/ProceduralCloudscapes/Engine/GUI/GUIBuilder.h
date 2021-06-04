#ifndef GUI_BUILDER_H
#define GUI_BUILDER_H

class GUIBuilder {
public:
	virtual void buildGUI() = 0;
	virtual void buildHiddenGUI() {};
};

#endif // !GUI_BUILDER_H
