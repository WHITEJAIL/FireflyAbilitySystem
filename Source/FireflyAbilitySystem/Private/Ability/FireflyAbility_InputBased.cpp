// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/FireflyAbility_InputBased.h"

#include "EnhancedInputComponent.h"

UFireflyAbility_InputBased::UFireflyAbility_InputBased(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool UFireflyAbility_InputBased::CanActivateAbility() const
{
	return Super::CanActivateAbility();
}

void UFireflyAbility_InputBased::BindToInput(UEnhancedInputComponent* EnhancedInput, UInputAction* InputToBind)
{
	HandleStarted = EnhancedInput->BindAction(InputToBind, ETriggerEvent::Started, this,
		&UFireflyAbility_InputBased::OnAbilityInputStarted).GetHandle();
	HandleOngoing = EnhancedInput->BindAction(InputToBind, ETriggerEvent::Ongoing, this,
		&UFireflyAbility_InputBased::OnAbilityInputOngoing).GetHandle();
	HandleCanceled = EnhancedInput->BindAction(InputToBind, ETriggerEvent::Canceled, this,
		&UFireflyAbility_InputBased::OnAbilityInputCanceled).GetHandle();
	HandleTriggered = EnhancedInput->BindAction(InputToBind, ETriggerEvent::Triggered, this,
		&UFireflyAbility_InputBased::OnAbilityInputTriggered).GetHandle();
	HandleCompleted = EnhancedInput->BindAction(InputToBind, ETriggerEvent::Completed, this,
		&UFireflyAbility_InputBased::OnAbilityInputCompleted).GetHandle();
}

void UFireflyAbility_InputBased::UnbindWithInput(UEnhancedInputComponent* EnhancedInput, UInputAction* InputToUnbind)
{
	EnhancedInput->RemoveBindingByHandle(HandleStarted);
	EnhancedInput->RemoveBindingByHandle(HandleOngoing);
	EnhancedInput->RemoveBindingByHandle(HandleCanceled);
	EnhancedInput->RemoveBindingByHandle(HandleTriggered);
	EnhancedInput->RemoveBindingByHandle(HandleCompleted);

	HandleStarted = HandleOngoing = HandleCanceled = HandleTriggered = HandleCompleted = -1;
}

void UFireflyAbility_InputBased::OnAbilityInputStarted()
{
	if (!bIsActivating)
	{
		ActivateAbility();
	}

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
	if (!bIsActivating)
	{
		return;
	}

	ReceiveOnAbilityInputTriggered();
}

void UFireflyAbility_InputBased::OnAbilityInputCompleted()
{
	if (!bIsActivating)
	{
		return;
	}

	ReceiveOnAbilityInputCompleted();
}
