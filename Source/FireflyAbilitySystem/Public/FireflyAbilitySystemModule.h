// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Logging/LogMacros.h"

class FFireflyAbilitySystemModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

FIREFLYABILITYSYSTEM_API DECLARE_LOG_CATEGORY_EXTERN(LogFireflyAbilitySystem, Log, All);
FIREFLYABILITYSYSTEM_API DECLARE_LOG_CATEGORY_EXTERN(LogFireflyAttribute, Log, All);
FIREFLYABILITYSYSTEM_API DECLARE_LOG_CATEGORY_EXTERN(LogFireflyAbility, Log, All);
FIREFLYABILITYSYSTEM_API DECLARE_LOG_CATEGORY_EXTERN(LogFireflyEffect, Log, All);

FIREFLYABILITYSYSTEM_API FString GetContextNetRoleStringFireflyAS(UObject* ContextObject = nullptr);