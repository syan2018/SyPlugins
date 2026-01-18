#pragma once

#include "Modules/ModuleManager.h"

class FSyCoreEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
