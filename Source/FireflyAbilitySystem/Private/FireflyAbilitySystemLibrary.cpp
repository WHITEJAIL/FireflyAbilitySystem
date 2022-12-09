// Fill out your copyright notice in the Description page of Project Settings.


#include "FireflyAbilitySystemLibrary.h"

#include "DataRegistrySubsystem.h"
#include "FireflyAbility.h"

TSubclassOf<UFireflyAbility> UFireflyAbilitySystemLibrary::GetAbilityClassFromCache(FName AbilityID)
{
	UDataRegistrySubsystem* SubsystemDR = UDataRegistrySubsystem::Get();
	if (!IsValid(SubsystemDR))
	{
		return nullptr;
	}

	const FFireflyAbilityTableRow* AbilityRow = SubsystemDR->GetCachedItem<FFireflyAbilityTableRow>(
		FDataRegistryId(FName("DR_FireflyAbilities"), AbilityID));
	if (!AbilityRow)
	{
		return nullptr;
	}

	return AbilityRow->AbilityClass.LoadSynchronous();
}

TSubclassOf<UFireflyEffect> UFireflyAbilitySystemLibrary::GetEffectClassFromCache(FName EffectID)
{
	UDataRegistrySubsystem* SubsystemDR = UDataRegistrySubsystem::Get();
	if (!IsValid(SubsystemDR))
	{
		return nullptr;
	}

	const FFireflyEffectTableRow* EffectRow = SubsystemDR->GetCachedItem<FFireflyEffectTableRow>(
		FDataRegistryId(FName("DR_FireflyEffects"), EffectID));
	if (!EffectRow)
	{
		return nullptr;
	}

	return EffectRow->EffectClass.LoadSynchronous();
}

FString UFireflyAbilitySystemLibrary::GetAttributeTypeName(EFireflyAttributeType AttributeType)
{
	const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EFireflyAttributeType"), true);
	if (!EnumPtr)
	{
		return FString("Invalid");
	}

	return EnumPtr->GetDisplayNameTextByValue(AttributeType).ToString();
}
