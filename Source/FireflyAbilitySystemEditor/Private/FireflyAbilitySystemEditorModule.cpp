// Copyright Epic Games, Inc. All Rights Reserved.

#include "FireflyAbilitySystemEditorModule.h"

#include "FireflyAbilitySystemSettingsDetails.h"

#define LOCTEXT_NAMESPACE "FFireflyAbilitySystemModule"

void FFireflyAbilitySystemEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomClassLayout("FireflyAbilitySystemSettings", FOnGetDetailCustomizationInstance::CreateStatic(&FFireflyAbilitySystemSettingsDetails::MakeInstance));
}

void FFireflyAbilitySystemEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FFireflyAbilitySystemEditorModule, FireflyAbilitySystem)