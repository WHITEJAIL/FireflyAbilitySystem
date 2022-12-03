// Fill out your copyright notice in the Description page of Project Settings.


#include "FireflyAbilitySystemLibrary.h"

#include "FireflyAbilitySystemSettings.h"
#include "FireflyAbility.h"

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

	FTableRowBase* AbilityRow = AbilityTable->FindRow<FTableRowBase>(AbilityID, FString("Find Ability Row"));

	TSubclassOf<UFireflyAbility> OutAbilityClass = nullptr;
	for (TFieldIterator<FProperty> It(AbilityRow->StaticStruct(), EFieldIteratorFlags::IncludeSuper); It; ++It)
	{
		FProperty* Property = *It;
		if (const FSoftClassProperty* AbilityClassProp = CastField<FSoftClassProperty>(Property))
		{
			const TSoftClassPtr<UObject>* AbilityClassPtr = reinterpret_cast<const TSoftClassPtr<UObject>*>(Property);
			OutAbilityClass = &*AbilityClassPtr->LoadSynchronous();

			break;
		}
		FClassProperty* ClassProperty = CastField<FClassProperty>(Property);
		if (ClassProperty)
		{
			OutAbilityClass = ClassProperty->ContainerPtrToValuePtr<UClass>(AbilityRow);

			break;
		}
	}

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

	FTableRowBase* EffectRow = EffectTable->FindRow<FTableRowBase>(EffectID, FString("Find Effect Row"));

	TSubclassOf<UFireflyEffect> OutEffectClass = nullptr;
	for (TFieldIterator<FProperty> It(EffectRow->StaticStruct(), EFieldIteratorFlags::IncludeSuper); It; ++It)
	{
		FProperty* Property = *It;
		if (const FSoftClassProperty* EffectClassProp = CastField<FSoftClassProperty>(Property))
		{
			const TSoftClassPtr<UObject>* EffectClassPtr = reinterpret_cast<const TSoftClassPtr<UObject>*>(Property);
			OutEffectClass = &*EffectClassPtr->LoadSynchronous();

			break;
		}
		
		FClassProperty* ClassProperty = CastField<FClassProperty>(Property);
		if (ClassProperty)
		{
			OutEffectClass = ClassProperty->ContainerPtrToValuePtr<UClass>(EffectRow);

			break;
		}		
	}

	return OutEffectClass;
}
