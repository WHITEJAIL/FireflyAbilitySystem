// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/FireflyAbility.h"

UFireflyAbility::UFireflyAbility(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFireflyAbility::StartAbility()
{
	OnAbilityStarted.Broadcast();
	ReceiveStartAbility();
}

void UFireflyAbility::EndAbility()
{
	OnAbilityEnded.Broadcast();
	ReceiveEndAbility(false);
}

void UFireflyAbility::CancelAbility()
{
	OnAbilityEnded.Broadcast();
	OnAbilityCanceled.Broadcast();
	ReceiveEndAbility(true);
}

bool UFireflyAbility::CheckAbilityCost_Implementation() const
{
	return false;
}

void UFireflyAbility::ApplyAbilityCost_Implementation() const
{
	OnAbilityCostCommitted.Broadcast();
}

bool UFireflyAbility::CheckAbilityCooldown_Implementation() const
{
	return false;
}

void UFireflyAbility::ApplyAbilityCooldown_Implementation() const
{
	OnAbilityCooldownCommitted.Broadcast();
}

bool UFireflyAbility::CommitAbilityCost()
{
	if (!CheckAbilityCost())
	{
		return false;
	}

	ApplyAbilityCost();
	return true;
}

bool UFireflyAbility::CommitAbilityCooldown()
{
	if (!CheckAbilityCooldown())
	{
		return false;
	}

	ApplyAbilityCooldown();
	return true;
}

bool UFireflyAbility::CommitAbility()
{
	return CommitAbilityCost() && CommitAbilityCooldown();
}