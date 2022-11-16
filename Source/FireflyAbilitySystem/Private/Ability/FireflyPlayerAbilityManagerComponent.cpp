// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/FireflyPlayerAbilityManagerComponent.h"

#include "EnhancedInputComponent.h"

UFireflyPlayerAbilityManagerComponent::UFireflyPlayerAbilityManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UEnhancedInputComponent* UFireflyPlayerAbilityManagerComponent::GetEnhancedInputComponentFromOwner() const
{
	if (!IsValid(GetOwner()))
	{
		return nullptr;
	}

	APawn* PawnOwner = Cast<APawn>(GetOwner());
	if (!IsValid(PawnOwner))
	{
		return nullptr;
	}

	if (!IsValid(PawnOwner->InputComponent))
	{
		return nullptr;
	}

	return Cast<UEnhancedInputComponent>(PawnOwner->InputComponent);
}

void UFireflyPlayerAbilityManagerComponent::OnAbilityInputActionStarted(UInputAction* Input)
{
	FFireflyAbilitiesBoundToInput* AbilitiesBoundToInput = AbilitiesInputBound.Find(Input);
	if (AbilitiesBoundToInput == nullptr)
	{
		return;
	}

	TArray<UFireflyAbility_InputBased*> Abilities;
	for (auto AbilityClass : AbilitiesBoundToInput->Abilities)
	{
		if (!IsValid(GetAbilityByClass(AbilityClass)))
		{
			continue;
		}

		UFireflyAbility_InputBased* Ability = Cast<UFireflyAbility_InputBased>(GetAbilityByClass(AbilityClass));
		if (Ability->bActivateOnTriggered || !Ability->CanActivateAbility())
		{
			continue;
		}

		Abilities.Emplace(Ability);
	}

	for (auto Ability : Abilities)
	{
		Ability->OnAbilityInputStarted();
	}
}

void UFireflyPlayerAbilityManagerComponent::OnAbilityInputActionOngoing(UInputAction* Input)
{
	FFireflyAbilitiesBoundToInput* AbilitiesBoundToInput = AbilitiesInputBound.Find(Input);
	if (AbilitiesBoundToInput == nullptr)
	{
		return;
	}

	TArray<UFireflyAbility_InputBased*> Abilities;
	for (auto AbilityClass : AbilitiesBoundToInput->Abilities)
	{
		if (!IsValid(GetAbilityByClass(AbilityClass)))
		{
			continue;
		}

		UFireflyAbility_InputBased* Ability = Cast<UFireflyAbility_InputBased>(GetAbilityByClass(AbilityClass));
		if (!Ability->bIsActivating)
		{
			continue;
		}

		Abilities.Emplace(Ability);
	}

	for (auto Ability : Abilities)
	{
		Ability->OnAbilityInputOngoing();
	}
}

void UFireflyPlayerAbilityManagerComponent::OnAbilityInputActionCanceled(UInputAction* Input)
{
	FFireflyAbilitiesBoundToInput* AbilitiesBoundToInput = AbilitiesInputBound.Find(Input);
	if (AbilitiesBoundToInput == nullptr)
	{
		return;
	}

	TArray<UFireflyAbility_InputBased*> Abilities;
	for (auto AbilityClass : AbilitiesBoundToInput->Abilities)
	{
		if (!IsValid(GetAbilityByClass(AbilityClass)))
		{
			continue;
		}

		UFireflyAbility_InputBased* Ability = Cast<UFireflyAbility_InputBased>(GetAbilityByClass(AbilityClass));
		if (!Ability->bIsActivating)
		{
			continue;
		}

		Abilities.Emplace(Ability);
	}

	for (auto Ability : Abilities)
	{
		Ability->OnAbilityInputCanceled();
	}
}

void UFireflyPlayerAbilityManagerComponent::OnAbilityInputActionTriggered(UInputAction* Input)
{
	FFireflyAbilitiesBoundToInput* AbilitiesBoundToInput = AbilitiesInputBound.Find(Input);
	if (AbilitiesBoundToInput == nullptr)
	{
		return;
	}

	TArray<UFireflyAbility_InputBased*> Abilities;
	for (auto AbilityClass : AbilitiesBoundToInput->Abilities)
	{
		if (!IsValid(GetAbilityByClass(AbilityClass)))
		{
			continue;
		}

		UFireflyAbility_InputBased* Ability = Cast<UFireflyAbility_InputBased>(GetAbilityByClass(AbilityClass));
		if (Ability->bActivateOnTriggered && !Ability->CanActivateAbility())
		{
			continue;
		}

		Abilities.Emplace(Ability);
	}

	for (auto Ability : Abilities)
	{
		Ability->OnAbilityInputTriggered();
	}
}

void UFireflyPlayerAbilityManagerComponent::OnAbilityInputActionCompleted(UInputAction* Input)
{
	FFireflyAbilitiesBoundToInput* AbilitiesBoundToInput = AbilitiesInputBound.Find(Input);
	if (AbilitiesBoundToInput == nullptr)
	{
		return;
	}

	TArray<UFireflyAbility_InputBased*> Abilities;
	for (auto AbilityClass : AbilitiesBoundToInput->Abilities)
	{
		if (!IsValid(GetAbilityByClass(AbilityClass)))
		{
			continue;
		}

		UFireflyAbility_InputBased* Ability = Cast<UFireflyAbility_InputBased>(GetAbilityByClass(AbilityClass));
		if (!Ability->bIsActivating)
		{
			continue;
		}

		Abilities.Emplace(Ability);
	}

	for (auto Ability : Abilities)
	{
		Ability->OnAbilityInputCompleted();
	}
}

void UFireflyPlayerAbilityManagerComponent::BindAbilityToInput(TSubclassOf<UFireflyAbility_InputBased> AbilityToBind,
                                                               UInputAction* InputToBind)
{
	if (!IsValid(InputToBind) || !IsValid(AbilityToBind))
	{
		return;
	}

	UEnhancedInputComponent* EnhancedInput = GetEnhancedInputComponentFromOwner();
	if (!IsValid(EnhancedInput))
	{
		return;
	}

	if (!IsValid(GetAbilityByClass(AbilityToBind)))
	{
		return;
	}

	UFireflyAbility_InputBased* Ability = Cast<UFireflyAbility_InputBased>(GetAbilityByClass(AbilityToBind));
	if (!IsValid(Ability))
	{
		return;
	}

	FFireflyAbilitiesBoundToInput& AbilitiesBoundToInput = AbilitiesInputBound.FindOrAdd(InputToBind);
	if (AbilitiesBoundToInput.Abilities.Num() == 0)
	{
		AbilitiesBoundToInput.HandleStarted = EnhancedInput->BindAction(InputToBind, ETriggerEvent::Started, this,
			&UFireflyPlayerAbilityManagerComponent::OnAbilityInputActionStarted, InputToBind).GetHandle();
		AbilitiesBoundToInput.HandleOngoing = EnhancedInput->BindAction(InputToBind, ETriggerEvent::Ongoing, this,
			&UFireflyPlayerAbilityManagerComponent::OnAbilityInputActionOngoing, InputToBind).GetHandle();
		AbilitiesBoundToInput.HandleCanceled = EnhancedInput->BindAction(InputToBind, ETriggerEvent::Canceled, this,
			&UFireflyPlayerAbilityManagerComponent::OnAbilityInputActionCanceled, InputToBind).GetHandle();
		AbilitiesBoundToInput.HandleTriggered = EnhancedInput->BindAction(InputToBind, ETriggerEvent::Triggered, this,
			&UFireflyPlayerAbilityManagerComponent::OnAbilityInputActionTriggered, InputToBind).GetHandle();
		AbilitiesBoundToInput.HandleCompleted = EnhancedInput->BindAction(InputToBind, ETriggerEvent::Completed, this,
			&UFireflyPlayerAbilityManagerComponent::OnAbilityInputActionCompleted, InputToBind).GetHandle();
	}

	AbilitiesBoundToInput.Abilities.AddUnique(AbilityToBind);
}

void UFireflyPlayerAbilityManagerComponent::UnbindAbilityWithInput(TSubclassOf<UFireflyAbility_InputBased> AbilityToUnbind,
	UInputAction* InputToUnbind)
{
	if (!IsValid(InputToUnbind) || !IsValid(AbilityToUnbind))
	{
		return;
	}

	UEnhancedInputComponent* EnhancedInput = GetEnhancedInputComponentFromOwner();
	if (!IsValid(EnhancedInput))
	{
		return;
	}

	if (!IsValid(GetAbilityByClass(AbilityToUnbind)))
	{
		return;
	}

	UFireflyAbility_InputBased* Ability = Cast<UFireflyAbility_InputBased>(GetAbilityByClass(AbilityToUnbind));
	if (!IsValid(Ability))
	{
		return;
	}

	FFireflyAbilitiesBoundToInput* AbilitiesBoundToInput = AbilitiesInputBound.Find(InputToUnbind);
	if (AbilitiesBoundToInput == nullptr)
	{
		return;
	}

	if (!AbilitiesBoundToInput->Abilities.Contains(AbilityToUnbind))
	{
		return;
	}

	AbilitiesBoundToInput->Abilities.RemoveSingleSwap(AbilityToUnbind);
	if (AbilitiesBoundToInput->Abilities.Num() == 0)
	{
		EnhancedInput->RemoveBindingByHandle(AbilitiesBoundToInput->HandleStarted);
		EnhancedInput->RemoveBindingByHandle(AbilitiesBoundToInput->HandleOngoing);
		EnhancedInput->RemoveBindingByHandle(AbilitiesBoundToInput->HandleCanceled);
		EnhancedInput->RemoveBindingByHandle(AbilitiesBoundToInput->HandleTriggered);
		EnhancedInput->RemoveBindingByHandle(AbilitiesBoundToInput->HandleCompleted);

		AbilitiesInputBound.Remove(InputToUnbind);
	}
}
