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
	SetIsReplicated(true);

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

UFireflyAbility* UFireflyAbilityManagerComponent::GetGrantedAbilityByID(FName AbilityID) const
{
	return *GrantedAbilities.Find(AbilityID);
}

UFireflyAbility* UFireflyAbilityManagerComponent::GetGrantedAbilityByClass(TSubclassOf<UFireflyAbility> AbilityType) const
{
	UFireflyAbility* OutAbility = nullptr;
	TArray<UFireflyAbility*> Abilities = TArray<UFireflyAbility*>{};
	GrantedAbilities.GenerateValueArray(Abilities);
	for (auto Ability : Abilities)
	{
		if (Ability->GetClass() == AbilityType)
		{
			OutAbility = Ability;
			break;
		}
	}

	return OutAbility;
}

void UFireflyAbilityManagerComponent::Client_OnAbilityGranted_Implementation(FName AbilityID, UFireflyAbility* AbilityJustGranted)
{
	if (IsValid(*GrantedAbilities.Find(AbilityID)))
	{
		return;
	}
	GrantedAbilities.Add(AbilityID, AbilityJustGranted);
}

void UFireflyAbilityManagerComponent::Client_OnAbilityRemoved_Implementation(FName AbilityID, UFireflyAbility* AbilityJustRemoved)
{
	if (!IsValid(*GrantedAbilities.Find(AbilityID)))
	{
		return;
	}
	GrantedAbilities.Remove(AbilityID);
}

void UFireflyAbilityManagerComponent::GrantAbility(FName AbilityID, TSubclassOf<UFireflyAbility> AbilityToGrant)
{
	if (!IsValid(AbilityToGrant))
	{
		return;
	}

	if (IsValid(*GrantedAbilities.Find(AbilityID)))
	{
		return;
	}

	FName NewAbilityName = FName(AbilityToGrant->GetName() + FString("_") + GetOwner()->GetName());
	UFireflyAbility* NewAbility = NewObject<UFireflyAbility>(this, AbilityToGrant, NewAbilityName);
	NewAbility->OnAbilityGranted();
	GrantedAbilities.Emplace(AbilityID, NewAbility);
}

void UFireflyAbilityManagerComponent::RemoveAbility(FName AbilityID, TSubclassOf<UFireflyAbility> AbilityToRemove)
{
	if (!IsValid(AbilityToRemove))
	{
		return;
	}

	UFireflyAbility* Ability = *GrantedAbilities.Find(AbilityID);
	if (!IsValid(Ability))
	{
		return;
	}

	if (Ability->bIsActivating)
	{
		Ability->CancelAbility();
	}	
	GrantedAbilities.Remove(AbilityID);
	Ability->MarkAsGarbage();
}

void UFireflyAbilityManagerComponent::RemoveAbilityOnEnded(FName AbilityID, TSubclassOf<UFireflyAbility> AbilityToRemove)
{
	if (!IsValid(AbilityToRemove))
	{
		return;
	}

	UFireflyAbility* Ability = *GrantedAbilities.Find(AbilityID);
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
		GrantedAbilities.Remove(AbilityID);
		Ability->MarkAsGarbage();		
	}
}

void UFireflyAbilityManagerComponent::Server_TryActivateAbility_Implementation(UFireflyAbility* AbilityToActivate, bool bNeedValidation)
{
	if (bNeedValidation)
	{
		if (!AbilityToActivate->CanActivateAbility())
		{
			AbilityToActivate->Client_CancelAbility();
			return;
		}

		TArray<FGameplayTag> BlockTagArray;
		BlockAbilityTags.GetKeys(BlockTagArray);
		FGameplayTagContainer BlockTags = FGameplayTagContainer::CreateFromArray(BlockTagArray);
		if (AbilityToActivate->TagsForAbilityAsset.HasAnyExact(BlockTags))
		{
			AbilityToActivate->Client_CancelAbility();
			return;
		}
	}

	AbilityToActivate->ActivateAbility();
	ActivatingAbilities.Emplace(AbilityToActivate);
}

UFireflyAbility* UFireflyAbilityManagerComponent::TryActivateAbilityByID(FName AbilityID)
{
	UFireflyAbility* Ability = *GrantedAbilities.Find(AbilityID);
	if (!IsValid(Ability))
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

	if (GetOwnerRole() == ROLE_Authority)
	{
		Server_TryActivateAbility(Ability, false);
	}
	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		Ability->ActivateAbility();
		ActivatingAbilities.Emplace(Ability);
		Server_TryActivateAbility(Ability, true);
	}

	return Ability;
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

	if (GetOwnerRole() == ROLE_Authority)
	{
		Server_TryActivateAbility(Ability, false);
	}
	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		Ability->ActivateAbility();
		ActivatingAbilities.Emplace(Ability);
		Server_TryActivateAbility(Ability, true);
	}

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
	ActivatingAbilities.RemoveSingle(AbilityJustEnded);
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
