// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/FireflyAbilityManagerComponent.h"

#include "FireflyTagManagerComponent.h"
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

UFireflyAbility* UFireflyAbilityManagerComponent::GetGrantedAbilityByClass(TSubclassOf<UFireflyAbility> AbilityType) const
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

	if (IsValid(GetGrantedAbilityByClass(AbilityToGrant)))
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

	UFireflyAbility* Ability = GetGrantedAbilityByClass(AbilityToRemove);
	if (!IsValid(Ability))
	{
		return;
	}

	if (Ability->bIsActivating)
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

	UFireflyAbility* Ability = GetGrantedAbilityByClass(AbilityToRemove);
	if (!IsValid(Ability))
	{
		return;
	}

	if (Ability->bIsActivating)
	{
		Ability->bRemoveOnEndedExecution = true;
	}
	else
	{
		GrantedAbilities.RemoveSingleSwap(Ability);
		Ability->MarkAsGarbage();		
	}
}

UFireflyAbility* UFireflyAbilityManagerComponent::TryActivateAbilityByClass(TSubclassOf<UFireflyAbility> AbilityToActivate)
{
	if (!IsValid(AbilityToActivate))
	{
		return nullptr;
	}

	UFireflyAbility* Ability = GetGrantedAbilityByClass(AbilityToActivate);
	if (!IsValid(Ability))
	{
		return nullptr;
	}
	if (!Ability)
	{
		return nullptr;
	}

	if (!Ability->CanActivateAbility())
	{
		return nullptr;
	}

	TArray<FGameplayTag> BlockTagArray;
	BlockAbilityTags.GetKeys(BlockTagArray);
	FGameplayTagContainer BlockTags = FGameplayTagContainer::CreateFromArray(BlockTagArray);
	if (Ability->TagsForAbilityAsset.HasAnyExact(BlockTags))
	{
		return nullptr;
	}

	Ability->ActivateAbility();
	ActivatingAbilities.Emplace(Ability);
	return Ability;
}

void UFireflyAbilityManagerComponent::CancelAbilitiesWithTags(FGameplayTagContainer CancelTags)
{
	for (auto Ability : GetActivatingAbilities())
	{
		if (Ability->TagsForAbilityAsset.HasAnyExact(CancelTags))
		{
			Ability->CancelAbility();
		}
	}
}

void UFireflyAbilityManagerComponent::OnAbilityEndActivation(UFireflyAbility* AbilityJustEnded)
{
	ActivatingAbilities.RemoveSingleSwap(AbilityJustEnded);
}

void UFireflyAbilityManagerComponent::UpdateBlockAndCancelAbilityTags(FGameplayTagContainer BlockTags,
	FGameplayTagContainer CancelTags, bool bIsActivated)
{
	if (bIsActivated)
	{
		CancelAbilitiesWithTags(CancelTags);

		TArray<FGameplayTag> Tags;
		CancelTags.GetGameplayTagArray(Tags);
		for (auto TagToAdd : Tags)
		{
			int32& Count = BlockAbilityTags.FindOrAdd(TagToAdd);
			++Count;
		}		
	}
	else
	{
		TArray<FGameplayTag> Tags;
		CancelTags.GetGameplayTagArray(Tags);
		for (auto TagToRemove : Tags)
		{
			if (!BlockAbilityTags.Contains(TagToRemove))
			{
				continue;
			}

			int32* CountToMinus = BlockAbilityTags.Find(TagToRemove);
			*CountToMinus = FMath::Clamp<int32>(*CountToMinus - 1, 0, *CountToMinus);

			if (*CountToMinus == 0)
			{
				BlockAbilityTags.Remove(TagToRemove);
			}
		}		
	}
}
