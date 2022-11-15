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

	bIsActivating = true;

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
	OnAbilityEnded.Broadcast();
	ReceiveEndAbility(false);
}

void UFireflyAbility::CancelAbility()
{
	if (!bIsActivating)
	{
		return;
	}

	bIsActivating = false;
	OnAbilityEnded.Broadcast();
	OnAbilityCanceled.Broadcast();
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

void UFireflyAbility::ExecuteTagRequirementOnActivated()
{
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

	TArray<FGameplayTag> TagsToAdd;
	TagsApplyToOwnerOnActivated.GetGameplayTagArray(TagsToAdd);
	for (auto Tag : TagsToAdd)
	{
		TagManager->AddTagToManager(Tag, 1);
	}
}

void UFireflyAbility::ExecuteTagRequirementOnEnded()
{
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

	TArray<FGameplayTag> TagsToRemove;
	TagsApplyToOwnerOnActivated.GetGameplayTagArray(TagsToRemove);
	for (auto Tag : TagsToRemove)
	{
		TagManager->RemoveTagFromManager(Tag, 1);
	}
}

bool UFireflyAbility::CanActivateAbility() const
{
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

	bool bOwnerHasRequiredTags = OwnerTags.HasAll(TagsRequireOwnerHasForActivation);
	bool bOwnerHasBlockTags = OwnerTags.HasAnyExact(TagsBlockActivationOnOwnerHas);

	bool bBlueprintCanActivate = true;
	if (bHasBlueprintCanActivate)
	{
		bBlueprintCanActivate = ReceiveCanActivateAbility();
	}

	return bBlueprintCanActivate && bOwnerHasRequiredTags && !bOwnerHasBlockTags && !bIsActivating;
}
