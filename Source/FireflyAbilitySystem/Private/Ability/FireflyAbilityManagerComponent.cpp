// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/FireflyAbilityManagerComponent.h"

#include "Ability/FireflyAbility.h"
#include "EnhancedInputComponent.h"

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

UFireflyAbility* UFireflyAbilityManagerComponent::GetAbilityByClass(TSubclassOf<UFireflyAbility> AbilityType) const
{
	UFireflyAbility* OutAbility = nullptr;
	for (auto Ability : GrantedAbilities)
	{
		if (Ability->GetClass() == AbilityType)
		{
			OutAbility = Ability;
			break;
		}
	}

	return OutAbility;
}

void UFireflyAbilityManagerComponent::GrantAbility(TSubclassOf<UFireflyAbility> AbilityToGrant)
{
	if (!IsValid(AbilityToGrant))
	{
		return;
	}

	if (IsValid(GetAbilityByClass(AbilityToGrant)))
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

	UFireflyAbility* Ability = GetAbilityByClass(AbilityToRemove);
	if (!IsValid(Ability))
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

	UFireflyAbility* Ability = GetAbilityByClass(AbilityToRemove);
	if (!IsValid(Ability))
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

	UFireflyAbility* Ability = GetAbilityByClass(AbilityToActivate);
	if (!IsValid(Ability))
	{
		return false;
	}
	if (!Ability)
	{
		return false;
	}

	Ability->ActivateAbility();

	ActivatedAbility = Ability;
	return true;
}

UEnhancedInputComponent* UFireflyAbilityManagerComponent::GetEnhancedInputComponentFromOwner() const
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

void UFireflyAbilityManagerComponent::AbilityBindToInput(TSubclassOf<UFireflyAbility> AbilityToBind,
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

	UFireflyAbility* Ability = GetAbilityByClass(AbilityToBind);
	if (!IsValid(Ability))
	{
		return;
	}

	/*EnhancedInput->BindAction(InputToBind, ETriggerEvent::Started, this,
		&UFireflyAbilityManagerComponent::OnAbilityInputStarted).GetHandle();
	EnhancedInput->BindAction(InputToBind, ETriggerEvent::Ongoing, this,
		&UFireflyAbilityManagerComponent::OnAbilityInputOngoing).GetHandle();
	EnhancedInput->BindAction(InputToBind, ETriggerEvent::Canceled, this, 
		&UFireflyAbilityManagerComponent::OnAbilityInputCanceled).GetHandle();
	EnhancedInput->BindAction(InputToBind, ETriggerEvent::Triggered, this,
		&UFireflyAbilityManagerComponent::OnAbilityInputTriggered).GetHandle();
	EnhancedInput->BindAction(InputToBind, ETriggerEvent::Completed, this,
		&UFireflyAbilityManagerComponent::OnAbilityInputCompleted).GetHandle();*/
}

void UFireflyAbilityManagerComponent::AbilityUnbindWithInput(TSubclassOf<UFireflyAbility> AbilityToUnbind,
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

	UFireflyAbility* Ability = GetAbilityByClass(AbilityToUnbind);
	if (!IsValid(Ability))
	{
		return;
	}
}
