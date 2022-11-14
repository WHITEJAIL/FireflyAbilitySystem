// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/FireflyAbilityManagerComponent.h"

#include "Ability/FireflyAbility.h"

// Sets default values for this component's properties
UFireflyAbilityManagerComponent::UFireflyAbilityManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UFireflyAbilityManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UFireflyAbilityManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UFireflyAbilityManagerComponent::GrantAbility(TSubclassOf<UFireflyAbility> AbilityToGrant)
{
	if (!IsValid(AbilityToGrant))
	{
		return;
	}

	bool bAlreadyGranted = false;
	for (auto OtherAbility : GrantedAbilities)
	{
		if (OtherAbility->GetClass() == AbilityToGrant)
		{
			bAlreadyGranted = true;
			break;
		}
	}
	if (bAlreadyGranted)
	{
		return;
	}

	FName NewAbilityName = FName(AbilityToGrant->GetName() + FString("_") + GetOwner()->GetName());
	UFireflyAbility* NewAbility = NewObject<UFireflyAbility>(this, AbilityToGrant, NewAbilityName);
	NewAbility->OnAbilityGranted();
	GrantedAbilities.Emplace(NewAbility);
}

void UFireflyAbilityManagerComponent::RemoveAbility(TSubclassOf<UFireflyAbility> AbilityToRemove)
{
	if (!IsValid(AbilityToRemove))
	{
		return;
	}

	UFireflyAbility* Ability = nullptr;
	for (auto OtherAbility : GrantedAbilities)
	{
		if (OtherAbility->GetClass() == AbilityToRemove)
		{
			Ability = OtherAbility;
			break;
		}
	}
	if (!Ability)
	{
		return;
	}

	if (Ability->IsActivating)
	{
		Ability->CancelAbility();
	}	
	GrantedAbilities.RemoveSingleSwap(Ability);
	Ability->MarkAsGarbage();
}

void UFireflyAbilityManagerComponent::RemoveAbilityOnEnded(TSubclassOf<UFireflyAbility> AbilityToRemove)
{
	if (!IsValid(AbilityToRemove))
	{
		return;
	}

	UFireflyAbility* Ability = nullptr;
	for (auto OtherAbility : GrantedAbilities)
	{
		if (OtherAbility->GetClass() == AbilityToRemove)
		{
			Ability = OtherAbility;
			break;
		}
	}
	if (!Ability)
	{
		return;
	}

	if (Ability->IsActivating)
	{
		Ability->bRemoveOnEndedExecution = true;
	}
	else
	{
		GrantedAbilities.RemoveSingleSwap(Ability);
		Ability->MarkAsGarbage();		
	}
}

bool UFireflyAbilityManagerComponent::TryActivateAbilityByClass(TSubclassOf<UFireflyAbility> AbilityToActivate, UFireflyAbility*& ActivatedAbility)
{
	if (!IsValid(AbilityToActivate))
	{
		return false;
	}

	UFireflyAbility* Ability = nullptr;
	for (auto OtherAbility : GrantedAbilities)
	{
		if (OtherAbility->GetClass() == AbilityToActivate)
		{
			Ability = OtherAbility;
			break;
		}
	}
	if (!Ability)
	{
		return false;
	}

	Ability->ActivateAbility();

	ActivatedAbility = Ability;
	return false;
}
