﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "FireflyAbilitySystemLibrary.h"

#include "DataRegistrySubsystem.h"
#include "FireflyAbility.h"
#include "FireflyAbilitySystemComponent.h"

UFireflyAbilitySystemComponent* UFireflyAbilitySystemLibrary::GetFireflyAbilitySystem(AActor* Actor)
{
	if (!IsValid(Actor->GetComponentByClass(UFireflyAbilitySystemComponent::StaticClass())))
	{
		return nullptr;
	}

	return CastChecked<UFireflyAbilitySystemComponent>(Actor->GetComponentByClass(
		UFireflyAbilitySystemComponent::StaticClass()));
}

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

void UFireflyAbilitySystemLibrary::SendNotifyEventToActor(AActor* TargetActor, FGameplayTag EventTag,
	FFireflyMessageEventData EventData)
{
	if (!IsValid(TargetActor) || !EventTag.IsValid())
	{
		return;
	}

	EventData.EventTag = EventTag;

	UFireflyAbilitySystemComponent* FireflyCore = GetFireflyAbilitySystem(TargetActor);
	if (!IsValid(FireflyCore))
	{
		return;
	}

	FireflyCore->HandleMessageEvent(EventTag, &EventData);
}