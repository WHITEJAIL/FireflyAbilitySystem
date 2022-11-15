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

	UFireflyAbility_InputBased* Ability = CastChecked<UFireflyAbility_InputBased>(GetAbilityByClass(AbilityToBind));
	if (!IsValid(Ability))
	{
		return;
	}

	FFireflyAbilitiesBoundToInput& AbilitiesBoundToInput = AbilitiesInputBound.FindOrAdd(InputToBind);
	Ability->BindToInput(EnhancedInput, InputToBind);
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

	UFireflyAbility_InputBased* Ability = CastChecked<UFireflyAbility_InputBased>(GetAbilityByClass(AbilityToUnbind));
	if (!IsValid(Ability))
	{
		return;
	}

	FFireflyAbilitiesBoundToInput* AbilitiesBoundToInput = AbilitiesInputBound.Find(InputToUnbind);
	if (!AbilitiesBoundToInput->Abilities.Contains(AbilityToUnbind))
	{
		return;
	}

	Ability->UnbindWithInput(EnhancedInput, InputToUnbind);
	AbilitiesBoundToInput->Abilities.RemoveSingleSwap(AbilityToUnbind);
	if (AbilitiesBoundToInput->Abilities.Num() == 0)
	{
		AbilitiesInputBound.Remove(InputToUnbind);
	}
}
