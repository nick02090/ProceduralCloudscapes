#ifndef SKYBOX_ENVIRONMENT_H
#define SKYBOX_ENVIRONMENT_H

#include "Environment.h"

struct SkyboxData : EnvironmentData {

};

class SkyboxEnvironment : public Environment {
public:
	SkyboxEnvironment();
	~SkyboxEnvironment();

	void update() override;
};

#endif // !SKYBOX_ENVIRONMENT_H
