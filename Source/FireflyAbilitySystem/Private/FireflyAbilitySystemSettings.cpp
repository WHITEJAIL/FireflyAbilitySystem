// Fill out your copyright notice in the Description page of Project Settings.


#include "FireflyAbilitySystemSettings.h"
#include "FireflyAbilitySystemTypes.h"

UFireflyAbilitySystemSettings::UFireflyAbilitySystemSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFireflyAbilitySystemSettings* UFireflyAbilitySystemSettings::Get()
{
	return CastChecked<UFireflyAbilitySystemSettings>(UFireflyAbilitySystemSettings::StaticClass()->GetDefaultObject());
}

void UFireflyAbilitySystemSettings::PostInitProperties()
{
	Super::PostInitProperties();
#if WITH_EDITOR
	LoadSurfaceType();
#endif
}

void UFireflyAbilitySystemSettings::LoadSurfaceType()
{
	// read "AttributeType" defines and set meta data for the enum
	// find the enum
	UEnum* Enum = StaticEnum<EFireflyAttributeType>();
	// we need this Enum
	check(Enum);

	const FString KeyName = TEXT("DisplayName");
	const FString HiddenMeta = TEXT("Hidden");
	const FString UnusedDisplayName = TEXT("Unused");

	// remainders, set to be unused
	for (int32 EnumIndex = 1; EnumIndex < Enum->NumEnums(); ++EnumIndex)
	{
		// if meta data isn't set yet, set name to "Unused" until we fix property window to handle this
		// make sure all hide and set unused first
		// if not hidden yet
		if (!Enum->HasMetaData(*HiddenMeta, EnumIndex))
		{
			Enum->SetMetaData(*HiddenMeta, TEXT(""), EnumIndex);
			Enum->SetMetaData(*KeyName, *UnusedDisplayName, EnumIndex);
		}
	}

	for (auto Iter = AttributeTypes.CreateConstIterator(); Iter; ++Iter)
	{
		// @todo only for editor
		Enum->SetMetaData(*KeyName, *Iter->Name.ToString(), Iter->Type);
		// also need to remove "Hidden"
		Enum->RemoveMetaData(*HiddenMeta, Iter->Type);
	}
}
