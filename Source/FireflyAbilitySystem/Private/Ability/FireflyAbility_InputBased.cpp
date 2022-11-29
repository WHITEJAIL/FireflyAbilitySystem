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
	if (!IsValid(GetOwnerManager()->TryActivateAbilityByClass(GetClass())))
	{
		return;
	}

	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		Server_OnAbilityInputStarted();
	}
	ReceiveOnAbilityInputStarted();
}

void UFireflyAbility_InputBased::Server_OnAbilityInputStarted_Implementation()
{
	OnAbilityInputStarted();
}

void UFireflyAbility_InputBased::OnAbilityInputOngoing()
{
	if (!bIsActivating)
	{
		return;
	}

	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		Server_OnAbilityInputOngoing();
	}
	ReceiveOnAbilityInputOngoing();
}

void UFireflyAbility_InputBased::Server_OnAbilityInputOngoing_Implementation()
{
	OnAbilityInputOngoing();
}

void UFireflyAbility_InputBased::OnAbilityInputCanceled()
{
	if (!bIsActivating)
	{
		return;
	}

	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		Server_OnAbilityInputCanceled();
	}
	ReceiveOnAbilityInputCanceled();
}

void UFireflyAbility_InputBased::Server_OnAbilityInputCanceled_Implementation()
{
	OnAbilityInputCanceled();
}

void UFireflyAbility_InputBased::OnAbilityInputTriggered()
{
	if (bActivateOnTriggered)
	{
		if (!IsValid(GetOwnerManager()->TryActivateAbilityByClass(GetClass())))
		{
			return;
		}

		if (GetOwnerRole() == ROLE_AutonomousProxy)
		{
			Server_OnAbilityInputTriggered();
		}
		ReceiveOnAbilityInputTriggered();

		return;
	}
	
	if (!bIsActivating)
	{
		return;
	}

	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		Server_OnAbilityInputTriggered();
	}
	ReceiveOnAbilityInputTriggered();		
}

void UFireflyAbility_InputBased::Server_OnAbilityInputTriggered_Implementation()
{
	OnAbilityInputTriggered();
}

void UFireflyAbility_InputBased::OnAbilityInputCompleted()
{
	if (!bIsActivating)
	{
		return;
	}

	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		Server_OnAbilityInputCompleted();
	}
	ReceiveOnAbilityInputCompleted();
}

void UFireflyAbility_InputBased::Server_OnAbilityInputCompleted_Implementation()
{
	OnAbilityInputCompleted();
}
