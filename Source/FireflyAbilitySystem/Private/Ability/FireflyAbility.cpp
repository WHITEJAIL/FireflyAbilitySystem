// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/FireflyAbility.h"

#include "FireflyTagManagerComponent.h"
#include "Ability/FireflyAbilityManagerComponent.h"

UFireflyAbility::UFireflyAbility(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	auto ImplementedInBlueprint = [](const UFunction* Func) -> bool
	{
		return Func && ensure(Func->GetOuter())
			&& Func->GetOuter()->IsA(UBlueprintGeneratedClass::StaticClass());
	};

	{
		static FName FuncName = FName(TEXT("ReceiveCanActivateAbility"));
		UFunction* CanActivateFunction = GetClass()->FindFunctionByName(FuncName);
		bHasBlueprintCanActivate = ImplementedInBlueprint(CanActivateFunction);
	}
}

UWorld* UFireflyAbility::GetWorld() const
{
	if (AActor* OwnerActor = GetOwnerActor())
	{
		return OwnerActor->GetWorld();
	}

	return nullptr;
}

AActor* UFireflyAbility::GetOwnerActor() const
{
	if (!IsValid(GetOwnerManager()))
	{
		return nullptr;
	}

	return GetOwnerManager()->GetOwner();
}

UFireflyAbilityManagerComponent* UFireflyAbility::GetOwnerManager() const
{
	if (!IsValid(GetOuter()))
	{
		return nullptr;
	}

	return Cast<UFireflyAbilityManagerComponent>(GetOuter());
}

void UFireflyAbility::OnAbilityGranted_Implementation()
{
}

void UFireflyAbility::ActivateAbility()
{
	if (bIsActivating)
	{
		return;
	}

	if (bCancelRequiredAbilities)
	{
		for (auto Ability : GetOwnerManager()->GetActivatingAbilities())
		{
			if (RequiredActivatingAbilities.Contains(Ability->GetClass()))
			{
				Ability->CancelAbility();
			}
		}
	}

	bIsActivating = true;
	ExecuteTagRequirementToOwner(true);	
	OnAbilityActivated.Broadcast();
	ReceiveActivateAbility();
}

void UFireflyAbility::EndAbility()
{
	if (!bIsActivating)
	{
		return;		
	}
	
	bIsActivating = false;
	ExecuteTagRequirementToOwner(false);
	OnAbilityEnded.Broadcast();
	GetOwnerManager()->OnAbilityEndActivation(this);
	ReceiveEndAbility(false);
}

void UFireflyAbility::CancelAbility()
{
	if (!bIsActivating)
	{
		return;
	}

	bIsActivating = false;
	ExecuteTagRequirementToOwner(false);
	OnAbilityEnded.Broadcast();
	OnAbilityCanceled.Broadcast();
	GetOwnerManager()->OnAbilityEndActivation(this);
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

void UFireflyAbility::SetCooldownTime(float NewCooldownTime)
{
	CooldownTime = NewCooldownTime;
}

void UFireflyAbility::SetCooldownTags(FGameplayTagContainer NewCooldownTags)
{
	CooldownTags = NewCooldownTags;
}

void UFireflyAbility::ExecuteTagRequirementToOwner(bool bIsActivated)
{
	GetOwnerManager()->UpdateBlockAndCancelAbilityTags(TagsOfAbilitiesWillBeBlocked, TagsOfAbilitiesWillBeCanceled, bIsActivated);

	if (!IsValid(GetOwnerActor()))
	{
		return;
	}

	if (!IsValid(GetOwnerActor()->GetComponentByClass(UFireflyTagManagerComponent::StaticClass())))
	{
		return;
	}

	UFireflyTagManagerComponent* TagManager = Cast<UFireflyTagManagerComponent>(GetOwnerActor()->GetComponentByClass(
		UFireflyTagManagerComponent::StaticClass()));

	TArray<FGameplayTag> TagsToUpdate;
	TagsApplyToOwnerOnActivated.GetGameplayTagArray(TagsToUpdate);
	for (auto Tag : TagsToUpdate)
	{
		bIsActivated ? TagManager->AddTagToManager(Tag, 1) : TagManager->RemoveTagFromManager(Tag, 1);
	}
}

bool UFireflyAbility::CanActivateAbility() const
{
	if (bIsActivating)
	{
		return false;
	}	

	if (!IsValid(GetOwnerActor()))
	{
		return false;
	}

	if (!IsValid(GetOwnerActor()->GetComponentByClass(UFireflyTagManagerComponent::StaticClass())))
	{
		return false;
	}

	UFireflyTagManagerComponent* TagManager = Cast<UFireflyTagManagerComponent>(GetOwnerActor()->GetComponentByClass(
		UFireflyTagManagerComponent::StaticClass()));
	FGameplayTagContainer OwnerTags = TagManager->GetContainedTags();

	if (OwnerTags.HasAnyExact(TagsBlockActivationOnOwnerHas))
	{
		return false;
	}

	bool bOwnerHasRequiredTags = OwnerTags.HasAll(TagsRequireOwnerHasForActivation);

	bool bBlueprintCanActivate = true;
	if (bHasBlueprintCanActivate)
	{
		bBlueprintCanActivate = ReceiveCanActivateAbility();
	}

	bool bHasRequiredActivatingAbility = true;
	if (RequiredActivatingAbilities.Num())
	{
		bHasRequiredActivatingAbility = false;
		for (auto OtherAbility : GetOwnerManager()->GetActivatingAbilities())
		{
			if (RequiredActivatingAbilities.Contains(OtherAbility->GetClass()))
			{
				bHasRequiredActivatingAbility = true;
				break;
			}
		}
	}	

	return bBlueprintCanActivate
		&& bOwnerHasRequiredTags
		&& bHasRequiredActivatingAbility;
}
