// Fill out your copyright notice in the Description page of Project Settings.


#include "FireflyAbilitySystemLibrary.h"

#include "FireflyAbility.h"
#include "FireflyAbilitySystemSettings.h"

UDataTable* UFireflyAbilitySystemLibrary::GetAbilityDataTable()
{
	return UFireflyAbilitySystemSettings::Get()->AbilityTable.LoadSynchronous();
}

TSubclassOf<UFireflyAbility> UFireflyAbilitySystemLibrary::GetAbilityClassFromDataTable(FName AbilityID)
{
	UDataTable* AbilityTable = GetAbilityDataTable();
	if (!IsValid(AbilityTable))
	{
		return nullptr;
	}

	FFireflyAbilityTableRow* AbilityTableRow = AbilityTable->FindRow<FFireflyAbilityTableRow>(AbilityID, FString("Find Ability Row"));
	if (!AbilityTableRow)
	{
		return nullptr;
	}
	AbilityTableRow->AbilityClass.IsValid();
	TSubclassOf<UFireflyAbility> OutAbilityClass = AbilityTableRow->AbilityClass.LoadSynchronous();
	
	return OutAbilityClass;
}

UDataTable* UFireflyAbilitySystemLibrary::GetEffectDataTable()
{
	return UFireflyAbilitySystemSettings::Get()->EffectTable.LoadSynchronous();
}

TSubclassOf<UFireflyEffect> UFireflyAbilitySystemLibrary::GetEffectClassFromDataTable(FName EffectID)
{
	UDataTable* EffectTable = GetEffectDataTable();
	if (!IsValid(EffectTable))
	{
		return nullptr;
	}

	FFireflyEffectTableRow* EffectTableRow = EffectTable->FindRow<FFireflyEffectTableRow>(EffectID, FString("Find Effect Row"));
	if (!EffectTableRow)
	{
		return nullptr;
	}
	EffectTableRow->EffectClass.IsValid();
	TSubclassOf<UFireflyEffect> OutEffectClass = EffectTableRow->EffectClass.LoadSynchronous();

	return OutEffectClass;
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
