#ifndef COLOR_H
#define COLOR_H

#include <cstdint>
#include <glm/glm.hpp>
#include <iostream>
#include <imgui.h>

/// <summary>
/// 'f' suffix in the methods consider a value from range [0.f, 1.0f]
/// no suffix in the methods consider a value from range [0, 255]
/// </summary>

class Color {
public:
	Color() {
		// initial color is black
	}
	Color(int _r, int _g, int _b) {
		setR(_r);
		setB(_g);
		setG(_b);
	}
	Color(float _r, float _g, float _b) {
		setRf(_r);
		setGf(_g);
		setBf(_b);
	}
	Color(const Color& color) {
		r = color.r;
		g = color.g;
		b = color.b;
	}
	~Color() {}

	ImVec4 toIMGUI() const { return ImVec4(getRf(), getGf(), getBf(), 1.0); }
	static Color fromIMGUI(ImVec4 imgui) { return Color(imgui.x, imgui.y, imgui.z); }

	// GETTERS
	// -------

	glm::vec3 get() const { return glm::vec3(getR(), getG(), getB()); }
	glm::vec3 getf() const { return glm::vec3(getRf(), getGf(), getBf()); }

	int getR() const { return static_cast<int>(r); }
	float getRf() const { return static_cast<float>((float)r / UINT8_MAX); }
	int getG() const { return static_cast<int>(g); }
	float getGf() const { return static_cast<float>((float)g / UINT8_MAX); }
	int getB() const { return static_cast<int>(b); }
	float getBf() const { return static_cast<float>((float)b / UINT8_MAX); }
	// -------

	// SETTERS
	// -------

	void set(int _r, int _g, int _b) {
		setR(_r);
		setG(_g);
		setB(_b);
	}
	void setf(float _r, float _g, float _b) {
		setRf(_r);
		setGf(_g);
		setBf(_b);
	}

	void setR(int _r) {
		if (overflowCheck(_r)) {
			r = static_cast<uint8_t>(_r);
		} 
		else {
			std::cout << "ERROR::COLOR::STACK_OVERFLOW R parameter is invalidly set!" << std::endl;
		}
	}
	void setRf(float _r) {
		if (overflowCheckf(_r)) {
			r = static_cast<uint8_t>(_r * UINT8_MAX);
		}
		else {
			std::cout << "ERROR::COLOR::STACK_OVERFLOW::F R parameter is invalidly set!" << std::endl;
		}
	}
	void setG(int _g) {
		if (overflowCheck(_g)) {
			g = static_cast<uint8_t>(_g);
		}
		else {
			std::cout << "ERROR::COLOR::STACK_OVERFLOW G parameter is invalidly set!" << std::endl;
		}
	}
	void setGf(float _g) {
		if (overflowCheckf(_g)) {
			g = static_cast<uint8_t>(_g * UINT8_MAX);
		}
		else {
			std::cout << "ERROR::COLOR::STACK_OVERFLOW::F G parameter is invalidly set!" << std::endl;
		}
	}
	void setB(int _b) {
		if (overflowCheck(_b)) {
			b = static_cast<uint8_t>(_b);
		}
		else {
			std::cout << "ERROR::COLOR::STACK_OVERFLOW B parameter is invalidly set!" << std::endl;
		}
	}
	void setBf(float _b) {
		if (overflowCheckf(_b)) {
			b = static_cast<uint8_t>(_b * UINT8_MAX);
		}
		else {
			std::cout << "ERROR::COLOR::STACK_OVERFLOW::F B parameter is invalidly set!" << std::endl;
		}
	}
	// -------
private:
	bool overflowCheck(int _value) {
		if (_value < 0 || _value > UINT8_MAX)
		{
			return false;
		}
		return true;
	}

	bool overflowCheckf(float _value) {
		if (_value < 0.f || _value > 1.0f) {
			return false;
		}
		return true;
	}

	uint8_t r{ 0 };
	uint8_t g{ 0 };
	uint8_t b{ 0 };
};

#endif // !COLOR_H
