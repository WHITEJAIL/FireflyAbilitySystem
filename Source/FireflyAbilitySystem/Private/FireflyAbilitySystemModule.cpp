// Copyright Epic Games, Inc. All Rights Reserved.

#include "FireflyAbilitySystemModule.h"

#define LOCTEXT_NAMESPACE "FFireflyAbilitySystemModule"

void FFireflyAbilitySystemModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FFireflyAbilitySystemModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

DEFINE_LOG_CATEGORY(LogFireflyAbilitySystem);
DEFINE_LOG_CATEGORY(LogFireflyAttribute);
DEFINE_LOG_CATEGORY(LogFireflyAbility);
DEFINE_LOG_CATEGORY(LogFireflyEffect);

FString GetContextNetRoleStringFireflyAS(UObject* ContextObject)
{
	ENetRole Role = ROLE_None;

	if (AActor* Actor = Cast<AActor>(ContextObject))
	{
		Role = Actor->GetLocalRole();
	}
	else if (UActorComponent* Component = Cast<UActorComponent>(ContextObject))
	{
		Role = Component->GetOwnerRole();
	}

	if (Role != ROLE_None)
	{
		return (Role == ROLE_Authority) ? TEXT("Authority") :
			((Role == ROLE_AutonomousProxy) ? TEXT("Autonomous") : TEXT("Simulated"));
	}
	else
	{
#if WITH_EDITOR
		if (GIsEditor)
		{
			extern ENGINE_API FString GPlayInEditorContextString;
			return GPlayInEditorContextString;
		}
#endif
	}

	return TEXT("[]");
}

	
IMPLEMENT_MODULE(FFireflyAbilitySystemModule, FireflyAbilitySystem)