#pragma once

#include "Modules/ModuleManager.h"

class FSyGameplayEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
