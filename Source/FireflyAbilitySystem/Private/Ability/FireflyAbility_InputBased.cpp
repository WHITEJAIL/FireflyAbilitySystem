// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/FireflyAbility_InputBased.h"

#include "EnhancedInputComponent.h"
#include "Ability/FireflyAbilityManagerComponent.h"

UFireflyAbility_InputBased::UFireflyAbility_InputBased(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFireflyAbility_InputBased::OnAbilityInputStarted()
{
	GetOwnerManager()->TryActivateAbilityByClass(GetClass());
	ReceiveOnAbilityInputStarted();
}

void UFireflyAbility_InputBased::OnAbilityInputOngoing()
{
	if (!bIsActivating)
	{
		return;
	}

	ReceiveOnAbilityInputOngoing();
}

void UFireflyAbility_InputBased::OnAbilityInputCanceled()
{
	if (!bIsActivating)
	{
		return;
	}

	ReceiveOnAbilityInputCanceled();
}

void UFireflyAbility_InputBased::OnAbilityInputTriggered()
{
	if (bActivateOnTriggered)
	{
		GetOwnerManager()->TryActivateAbilityByClass(GetClass());
		ReceiveOnAbilityInputTriggered();

		return;
	}
	else
	{
		if (!bIsActivating)
		{
			return;
		}

		ReceiveOnAbilityInputTriggered();
	}	
}

void UFireflyAbility_InputBased::OnAbilityInputCompleted()
{
	if (!bIsActivating)
	{
		return;
	}

	ReceiveOnAbilityInputCompleted();
}
